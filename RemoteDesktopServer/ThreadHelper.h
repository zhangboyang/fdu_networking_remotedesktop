#pragma once

class ThreadHelper {
private:
	HANDLE thread;
	static DWORD WINAPI ThreadProcWrapper(LPVOID lpParam);
public:
	ThreadHelper();
	~ThreadHelper();
	virtual void ThreadProc() = 0;
	void StartThread();
	void WaitThread();
	bool PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
};