#pragma once

template <class T>
class ProducerConsumerQueueMux;

template <class T>
class ProducerConsumerQueue {
	friend class ProducerConsumerQueueMux<T>;
private:
	CRITICAL_SECTION mutex;
	HANDLE empty, fill;
	std::queue<T> queue;
	HANDLE exitevent;

	volatile bool appendblock;
	volatile bool appenddisable;
public:
	ProducerConsumerQueue(int maxcount)
	{
		if (maxcount < 0) maxcount = LONG_MAX;
		InitializeCriticalSection(&mutex);
		empty = CreateSemaphore(NULL, maxcount, maxcount, NULL);
		fill = CreateSemaphore(NULL, 0, maxcount, NULL);
		exitevent = CreateEvent(NULL, TRUE, FALSE, NULL);

		appendblock = true;
		appenddisable = false;
	}
	~ProducerConsumerQueue()
	{
		CloseHandle(exitevent);
		CloseHandle(fill);
		CloseHandle(empty);
		DeleteCriticalSection(&mutex);
	}
	void SetAppendBlock(bool val)
	{
		appendblock = val;
	}
	void SetAppendDisable(bool val)
	{
		appenddisable = val;
	}
	void NotifyClose()
	{
		SetEvent(exitevent);
	}
	int Append(T cur)
	{
		int ret;
		DWORD dwRet;
		BOOL bRet;
		if (appenddisable) return -1;
		if (appendblock) {
			HANDLE h[2] = { exitevent, empty };
			dwRet = WaitForMultipleObjects(2, h, FALSE, INFINITE);
			if (dwRet == WAIT_OBJECT_0) return -3;
			assert(dwRet == WAIT_OBJECT_0 + 1);
		} else {
			dwRet = WaitForSingleObject(empty, 0);
			if (dwRet == WAIT_TIMEOUT) return -2;
			assert(dwRet == WAIT_OBJECT_0);
		}
		EnterCriticalSection(&mutex);
		if (appenddisable) {
			bRet = ReleaseSemaphore(empty, 1, NULL);
			assert(bRet);
			ret = -1;
		} else {
			queue.push(cur);
			ret = 0;
		}
		LeaveCriticalSection(&mutex);
		ReleaseSemaphore(fill, 1, NULL);
		return ret;
	}
	int Get(T *val)
	{
		BOOL bRet;
		HANDLE h[2] = { exitevent, fill };
		if (WaitForMultipleObjects(2, h, FALSE, INFINITE) == WAIT_OBJECT_0) return -3;
		EnterCriticalSection(&mutex);
		*val = queue.front();
		queue.pop();
		LeaveCriticalSection(&mutex);
		bRet = ReleaseSemaphore(empty, 1, NULL);
		assert(bRet);
		return 0;
	}
	int PeekWithoutRemove(T *val)
	{
		int ret = -1;
		EnterCriticalSection(&mutex);
		if (!queue.empty()) {
			*val = queue.front();
			ret = 0;
		}
		LeaveCriticalSection(&mutex);
		return ret;
	}
};

template <class T>
class ProducerConsumerQueueMux {
private:
	int N;
	ProducerConsumerQueue<T> **queuearray;
	HANDLE exitevent;
public:
	ProducerConsumerQueueMux(int N, int maxcount) : N(N)
	{
		queuearray = new ProducerConsumerQueue<T> *[N];
		for (int i = 0; i < N; i++) {
			queuearray[i] = new ProducerConsumerQueue<T>(maxcount);
		}
		exitevent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	~ProducerConsumerQueueMux()
	{
		for (int i = 0; i < N; i++) {
			delete queuearray[i];
		}
		delete[] queuearray;
		CloseHandle(exitevent);
	}
	ProducerConsumerQueue<T> *GetIndividualQueue(int id)
	{
		assert(0 <= id && id < N);
		return queuearray[id];
	}
	void NotifyClose()
	{
		SetEvent(exitevent);
		for (int i = 0; i < N; i++) {
			queuearray[i]->NotifyClose();
		}
	}
	int Get(T *val)
	{
		int ret;
		BOOL bRet;
		HANDLE *handlearray = new HANDLE[N + 1];
		handlearray[0] = exitevent;
		for (int i = 1; i <= N; i++) {
			handlearray[i] = queuearray[i - 1]->fill;
		}
		DWORD result = WaitForMultipleObjects(N + 1, handlearray, FALSE, INFINITE);
		assert(WAIT_OBJECT_0 <= result && result <= WAIT_OBJECT_0 + N);
		if (result == WAIT_OBJECT_0) { ret = -3; goto done; }
		
		int id = result - WAIT_OBJECT_0 - 1;
		EnterCriticalSection(&queuearray[id]->mutex);
		*val = queuearray[id]->queue.front();
		queuearray[id]->queue.pop();
		LeaveCriticalSection(&queuearray[id]->mutex);
		bRet = ReleaseSemaphore(queuearray[id]->empty, 1, NULL);
		assert(bRet);
		ret = 0;
done:
		delete[] handlearray;
		return ret;
	}
};