#include "stdafx.h"
#include "common.h"

void FrameTimer::Reset(int intervalms)
{
	interval = intervalms;
	last = timeGetTime();
}

void FrameTimer::WaitNextFrame()
{
	DWORD cur;
    while (1) {
        cur = timeGetTime();
        if (cur >= last + interval) break;
		int ms = interval - (cur - last) - 0;
        if (ms > 0) Sleep(ms);
    }
	last = cur;
}
