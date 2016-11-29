#include "stdafx.h"
#include "common.h"

static void TryRecvData(HWND hWnd, SOCKET s)
{

}

static void 
static void TrySendData(HWND hWnd, SOCKET s)
{
}

static void UpdateSocket(HWND hWnd, DWORD wParam, DWORD lParam)
{
	WORD ev = WSAGETSELECTEVENT(lParam);
	WORD err = WSAGETSELECTERROR(lParam);
	SOCKET s = wParam;
	switch (ev) {
		case FD_READ: TryRecvData(hWnd, s); break;
		case FD_WRITE: TrySendData(hWnd, s); break;
		case FD_CLOSE: PostQuitMessage(0); break;
	}
}
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR greeting[] = _T("Hello, World!");

	switch (message) {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        TextOut(hdc, 5, 5, greeting, _tcslen(greeting));

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_USER:
		UpdateSocket(lParam);
		break;
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
	SOCKET clisocket = socket(srvinfo->ai_family, srvinfo->ai_socktype, srvinfo->ai_protocol);
    if (clisocket == INVALID_SOCKET) {
		fail("socket() failed.");
	}
	
	// connect to server
	if (connect(clisocket, srvinfo->ai_addr, srvinfo->ai_addrlen) == SOCKET_ERROR) {
		fail("connect() failed, %d.", WSAGetLastError());
	}
	freeaddrinfo(srvinfo);

	// create and show window
	HINSTANCE hInstance = GetModuleHandle(NULL);

	static TCHAR szWindowClass[] = _T("win32app");
	static TCHAR szTitle[] = _T("Remote Desktop Client");

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
 
	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
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

	// show windows and pump message
	ShowWindow(hWnd, TRUE);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}