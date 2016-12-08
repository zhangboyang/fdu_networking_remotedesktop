#include "stdafx.h"
#include "common.h"

DWORD WINAPI ThreadHelper::ThreadProcWrapper(LPVOID lpParam)
{
	static_cast<ThreadHelper *> (lpParam)->ThreadProc();
	return 0;
}

ThreadHelper::ThreadHelper()
{
	thread = NULL;
}

ThreadHelper::~ThreadHelper()
{
	assert(thread == NULL);
}

void ThreadHelper::StartThread()
{
	DWORD tid;
	assert(thread == NULL);
	thread = CreateThread(NULL, 0, ThreadProcWrapper, this, 0, &tid);
}

void ThreadHelper::WaitThread()
{
	if (thread) {
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
		thread = NULL;
	}
}

bool ThreadHelper::PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return !!PostThreadMessage(GetThreadId(thread), Msg, wParam, lParam);
}