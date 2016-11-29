#include "stdafx.h"
#include "common.h"

PeriodCounter::PeriodCounter()
{
	Reset();
}

void PeriodCounter::Reset()
{
	count = 0;
	count_per_sec = -1;
	last = timeGetTime();
}

int PeriodCounter::IncreaseCount(int val)
{
	count += val;
	DWORD cur = timeGetTime();
	if (cur >= last + 1000) {
		count_per_sec = count * 1000.0 / (cur - last);
		count = 0;
		last = cur;
		return 1;
	} else {
		return 0;
	}
}

double PeriodCounter::GetCountsPerSecond()
{
	return count_per_sec;
}