#pragma once

class ScreenCapturer {
	HDC hdcMemDC, hdcScreen;
	HBITMAP hbmScreen;
	BITMAP bmpScreen;
	int width, height;
	int bitcount;
	DWORD dwSizeofInfo;
	DWORD dwBmpSize;
public:
	ScreenCapturer();
	~ScreenCapturer();

	void SetDepth(int new_bitcount);
	void GetBufferSize(size_t *infosize, size_t *bitssize);
	void CaptureFrame(BITMAPINFO *info, void *bits);
};