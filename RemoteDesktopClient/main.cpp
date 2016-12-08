#include "stdafx.h"
#include "common.h"

static HWND hWnd;
static SOCKET clisocket;

static HDC hdcMemDC = NULL;
static HBITMAP hbmScreen = NULL;
static void SendPacket(int type, char *data, size_t len);

struct pkthdr {
	size_t len;
	int type;
	char data[0];
};


static BITMAPINFO *bmpinfo = NULL;
static size_t bmpinfo_size = 0;
static char *bits = NULL;
static size_t bits_len = 0;
static size_t blksz = SCRPKT_MAXDATA;
static void UpdateRemoteScreenInfo(char *data, size_t len)
{
	if (len == 0) {
		PostQuitMessage(0);
		return;
	}
	bmpinfo = (BITMAPINFO *) realloc(bmpinfo, len);
	bmpinfo_size = len;
	memcpy(bmpinfo, data, len);
	// FIXME: verify the data!
	plog("screen info: %dx%d\n", bmpinfo->bmiHeader.biWidth, bmpinfo->bmiHeader.biHeight);
	bits_len = (((bmpinfo->bmiHeader.biWidth * bmpinfo->bmiHeader.biBitCount + 7) / 8) + 3) / 4 * 4 * bmpinfo->bmiHeader.biHeight;
	bits = (char *) realloc(bits, bits_len);
	memset(bits, 0, bits_len);

	if (hbmScreen) CloseHandle(hbmScreen);
	hbmScreen = CreateCompatibleBitmap(GetDC(hWnd), bmpinfo->bmiHeader.biWidth, bmpinfo->bmiHeader.biHeight);
	SelectObject(hdcMemDC, hbmScreen);

	RECT rc = {0, 0, bmpinfo->bmiHeader.biWidth, bmpinfo->bmiHeader.biHeight};
	AdjustWindowRect(&rc, GetWindowLong(hWnd, GWL_STYLE), FALSE);
	SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE);
}

FrameTimer wndupd;
int wnddirty = 0;
static void TryRedrawScreen(int dirty)
{
	if (dirty) wnddirty = 1;
	if (wndupd.IsNextFrame()) {
		if (wnddirty) {
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
			wnddirty = 0;
		}
	}
}
static void UpdateRemoteScreen(int id, char *data, size_t len)
{
	size_t offset = id * blksz;
	if (offset + len > bits_len) { // FIXME: overflow
		PostQuitMessage(0);
		return;
	}
	memcpy(bits + offset, data, len);
	TryRedrawScreen(1);
}

static void HandleScreenPacket(char *data, size_t len)
{
	ScrPktHdr *scrpkt = (ScrPktHdr *) data;
	char *buf;
	size_t datalen = len - sizeof(ScrPktHdr);
	size_t orgsize;
	switch (scrpkt->type) {
		case SCRPKT_PADDING:
			break;
		case SCRPKT_BITMAPINFO:
			UpdateRemoteScreenInfo(scrpkt->data, datalen);
			break;
		case SCRPKT_BITMAPDATA:
			UpdateRemoteScreen(scrpkt->id, scrpkt->data, datalen);
			break;
		case SCRPKT_BITMAPDATA_COMPRESSED:
			orgsize = MiniLZO::Instance()->GetOrigSize(scrpkt->data, datalen);
			if (orgsize > SCRPKT_MAXDATA) {
				PostQuitMessage(0);
				break;
			}
			buf = (char *)malloc(orgsize);
			if (!MiniLZO::Instance()->Decompress(buf, scrpkt->data, datalen)) {
				PostQuitMessage(0);
				break;
			}
			UpdateRemoteScreen(scrpkt->id, buf, orgsize);
			free(buf);
			break;
		default: PostQuitMessage(0);
	}
}

static void HandlePacket(pkthdr *pkt)
{
	switch (pkt->type) {
		case RDSERVICE_SCREENSENDER: HandleScreenPacket(pkt->data, pkt->len); break;
		default: PostQuitMessage(0);
	}
}

static size_t TryParsePacket(char *data, size_t len)
{
	if (len < sizeof(pkthdr)) return 0;
	pkthdr *pkt = (pkthdr *)data;
	size_t pktsz = pkt->len + sizeof(pkthdr);
	if (pkt->len <= len - sizeof(pkthdr)) {
		HandlePacket(pkt);
		return pktsz;
	} else {
		return 0;
	}
}


static char recvbuf[MAX_MSGPACKETSIZE_HARD];
static const char *recvbuf_end = recvbuf + sizeof(recvbuf);
static char *parse_ptr = recvbuf;
static char *recv_ptr = recvbuf;

PeriodCounter recvcnt, recvbytecnt;
static void TryRecvData()
{
	int block_flag = 0;
	while (!block_flag) {
		// try recv as much as possible
		while (recv_ptr < recvbuf_end) {
			int r = recv(clisocket, recv_ptr, recvbuf_end - recv_ptr, 0);
			if (r >= 0) {
				recv_ptr += r;
			} else {
				int err = WSAGetLastError();
				if (err == WSAEWOULDBLOCK) {
					block_flag = 1;
					break;
				} else {
					PostQuitMessage(1);
					return;
				}
			}
		}

		// try parse buffer
		size_t pktsz = TryParsePacket(parse_ptr, recv_ptr - parse_ptr);
		assert(pktsz <= (unsigned)(recv_ptr - parse_ptr));
		if (pktsz > 0) {
			if (recvcnt.IncreaseCount()) {
				plog("recv packet rate: %f\n", recvcnt.GetCountsPerSecond());
			}
			if (recvbytecnt.IncreaseCount(pktsz)) {
				plog("recv data rate: %f MB\n", recvbytecnt.GetCountsPerSecond() / 1048576.0);
			}
		}
		parse_ptr += pktsz;

		// detect invalid packet
		if (recv_ptr == recvbuf_end && parse_ptr == recvbuf) {
			PostQuitMessage(1);
			return;
		}

		// try move buffer
		if (recv_ptr >= recvbuf_end) {
			size_t valid_len = recvbuf_end - parse_ptr;
			memmove(recvbuf, parse_ptr, valid_len);
			parse_ptr = recvbuf;
			recv_ptr = recvbuf + valid_len;
		}
	}
}


static std::queue<std::pair<char *, std::pair<size_t, size_t> > > sendqueue; // (ptr, (sent, total))

static void TrySendData()
{
	while (!sendqueue.empty()) {
		std::pair<char *, std::pair<size_t, size_t> > &cur = sendqueue.front();
		int r = send(clisocket, cur.first + cur.second.first, cur.second.second - cur.second.first, 0);
		if (r >= 0) {
			cur.second.first += r;
			if (cur.second.first >= cur.second.second) {
				free(cur.first);
				sendqueue.pop();
			}
		} else {
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK) {
				PostQuitMessage(1);
			}
			break;
		}
	}
}
static void SendData(const char *data, size_t len)
{
	char *buf = (char *) malloc(len);
	memcpy(buf, data, len);
	sendqueue.push(std::make_pair(buf, std::make_pair(0, len)));
	TrySendData();
}

static void SendPacket(int type, char *data, size_t len)
{
	size_t pktlen = sizeof(pkthdr) + len;
	pkthdr *pkt = (pkthdr *) malloc(pktlen);
	memcpy(pkt->data, data, len);
	pkt->type = type;
	pkt->len = len;
	sendqueue.push(std::make_pair((char *)pkt, std::make_pair(0, pktlen)));
	TrySendData();
}


static void UpdateSocket(DWORD wParam, DWORD lParam)
{
	WORD ev = WSAGETSELECTEVENT(lParam);
	WORD err = WSAGETSELECTERROR(lParam);
	assert(clisocket == wParam);
	switch (ev) {
		case FD_READ: TryRecvData(); break;
		case FD_WRITE: TrySendData(); break;
		case FD_CLOSE: PostQuitMessage(0); break;
	}
}
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

	switch (message) {
	case WM_TIMER:
		//printf("TIMER!\n");
		TryRedrawScreen(0);
		break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
		RECT rc;
		GetClientRect(hWnd, &rc);
		if (bits) {
			//StretchDIBits(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 0, 0, bmpinfo->bmiHeader.biWidth, bmpinfo->bmiHeader.biHeight, bits, bmpinfo, DIB_RGB_COLORS, SRCCOPY);
			SetDIBits(hdcMemDC, hbmScreen, 0, bmpinfo->bmiHeader.biHeight, bits, bmpinfo, DIB_RGB_COLORS);
			BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcMemDC, 0, 0, SRCCOPY);
		} else {
			const char *greeting = "please wait ...";
			TextOut(hdc, 5, 5, greeting, strlen(greeting));
		}
		EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_USER:
		UpdateSocket(wParam, lParam);
		break;
	/*case WM_KEYUP:
		do {
			size_t pktlen = sizeof(struct CtrlPktHdr) + 1;
			CtrlPktHdr *pkt = (CtrlPktHdr *) malloc(pktlen);
			pkt->type = CTRLPKT_KEYBOARD;
			pkt->len = 1;
			pkt->data[0] = 0x41;
			SendPacket(RDSERVICE_CONTROLRECEIVER, (char *)pkt, pktlen);
		} while (0);
		break;*/
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


int main()
{
	char nodename[MAXLINE];
	char servname[MAXLINE] = "1223";

	scanf(MAXLINEFMT, nodename);
	
	// init
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fail("can't initialize WSA.");
	}

	// create socket
	struct addrinfo hints;
	struct addrinfo *srvinfo;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(nodename, servname, &hints, &srvinfo) != 0) {
		fail("getaddrinfo() failed.");
	}
	clisocket = socket(srvinfo->ai_family, srvinfo->ai_socktype, srvinfo->ai_protocol);
    if (clisocket == INVALID_SOCKET) {
		fail("socket() failed.");
	}
	
	/*BOOL flag = TRUE;
	int r = setsockopt(clisocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(BOOL));
	assert(r == 0);*/

	// connect to server
	if (connect(clisocket, srvinfo->ai_addr, srvinfo->ai_addrlen) == SOCKET_ERROR) {
		fail("connect() failed, %d.", WSAGetLastError());
	}
	freeaddrinfo(srvinfo);

	// create and show window
	HINSTANCE hInstance = GetModuleHandle(NULL);

	static TCHAR szWindowClass[] = _T("win32app");
	static TCHAR szTitle[] = _T("Remote Desktop Client (view only)");

	WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex)) {
        fail("RegisterClassEx() failed.");
        return 1;
    }
 
	RECT rc = {0, 0, 100, 100};
	AdjustWindowRect(&rc, wcex.style, FALSE);

	hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd) {
		fail("CreateWindow() failed.");
	}

	// register socket to window
	WSAAsyncSelect(clisocket, hWnd, WM_USER, FD_READ | FD_WRITE | FD_CLOSE);

	// send password
	char psw[] = "123456";
	SendPacket(RDSERVICE_PASSWORD, psw, strlen(psw));

	// show windows and pump message
	ShowWindow(hWnd, TRUE);
	UpdateWindow(hWnd);

	hdcMemDC = CreateCompatibleDC(GetDC(hWnd));

	int target_period = 1000 / SCRFRAME_FPS;
	SetTimer(hWnd, 0, target_period, NULL);
	wndupd.Reset(target_period);
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

//	CloseHandle(hdcMemDC);

    return (int) msg.wParam;
}