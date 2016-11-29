#include "stdafx.h"
#include "common.h"

ScreenCapturer::ScreenCapturer()
{
	hdcMemDC = hdcScreen = NULL;
	hbmScreen = NULL;

	hdcScreen = GetDC(NULL);
	hdcMemDC = CreateCompatibleDC(NULL);
	if (!hdcScreen || !hdcMemDC) { fail("can't get DC."); }
	
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);
	assert(width > 0 && height > 0);

	hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
	HBITMAP hbmRet = (HBITMAP) SelectObject(hdcMemDC, hbmScreen);
	if (!hbmRet) { fail("can't select bitmap."); }

	int nRet = GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);
	if (!nRet) { fail("can't get bitmap object."); }

	bitcount = 0;
}

ScreenCapturer::~ScreenCapturer()
{
	if (hbmScreen) DeleteObject(hbmScreen);
	if (hdcMemDC) DeleteObject(hdcMemDC);
	if (hdcScreen) ReleaseDC(NULL, hdcScreen);
}

void ScreenCapturer::SetDepth(int new_bitcount)
{
	bitcount = new_bitcount;
	dwSizeofInfo = sizeof(BITMAPINFOHEADER);
	switch (bitcount) {
		case 1: dwSizeofInfo += 2 * sizeof(RGBQUAD); break;
		case 4: dwSizeofInfo += 16 * sizeof(RGBQUAD); break;
		case 8: dwSizeofInfo += 256 * sizeof(RGBQUAD); break;
		case 16: case 24: case 32: break;
		default: fail("invalid bitcount %d.", bitcount);
	};

	dwBmpSize = (((bmpScreen.bmWidth * bitcount + 7) / 8) + 3) / 4 * 4 * bmpScreen.bmHeight;
}

void ScreenCapturer::GetBufferSize(size_t *infosize, size_t *bitssize)
{
	*infosize = dwSizeofInfo;
	*bitssize = dwBmpSize;
}

void ScreenCapturer::CaptureFrame(BITMAPINFO *info, void *bits)
{
	memset(info, 0, dwSizeofInfo);
	info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);	
	info->bmiHeader.biWidth = bmpScreen.bmWidth;
	info->bmiHeader.biHeight = bmpScreen.bmHeight;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = bitcount;
	info->bmiHeader.biCompression = BI_RGB;

	BOOL bRet = BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);
	if (!bRet) plog("BitBlt failed.");
	
	//memset(bits, 0, dwBmpSize);
	int nRet = GetDIBits(hdcMemDC, hbmScreen, 0, (UINT) bmpScreen.bmHeight, bits, info, DIB_RGB_COLORS);
	if (!nRet) plog("GetDIBits failed.");
}