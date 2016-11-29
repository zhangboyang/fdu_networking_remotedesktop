#pragma once

class FrameTimer {
	DWORD last;
	int interval;
public:
	void Reset(int intervalms);
	void WaitNextFrame();
};