#pragma once

class PeriodCounter {
	int count;
	double count_per_sec;
	DWORD last;
public:
	PeriodCounter();
	void Reset();
	int IncreseCount(int val = 1);
	double GetCountsPerSecond();
};