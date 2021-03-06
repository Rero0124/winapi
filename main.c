#include <windows.h>
#include <WinSock2.h>

HINSTANCE g_hInst;

HANDLE tcpThread;
static BOOL ThreadRun = TRUE;

HWND hwnd1;
HBITMAP hBitmap;
HGDIOBJ oldBitmap;

int ScreenWidth;
int ScreenHeight;

DWORD APIENTRY CreateTcpThread(LPVOID lpParam) {
    const int buflen = 4096;
    char buf[buflen];
    WSADATA wsadata;
    SOCKET listensock;
    SOCKET clientsock;
    SOCKADDR_IN addr_server;
    SOCKADDR_IN addr_client;
    int addrlen_ctl = sizeof(SOCKADDR);

    WSAStartup((WORD)(2.0), &wsadata);

    listensock = socket(AF_INET, SOCK_STREAM, 0);
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addr_server.sin_port = htons(7878);

    bind(listensock, (SOCKADDR *) &addr_server, sizeof(SOCKADDR));

    listen(listensock, 1);
    while(1){
        clientsock = accept(listensock, (SOCKADDR *)&addr_client, &addrlen_ctl);
    }

    recv(clientsock, buf, buflen, 0);
    WSACleanup();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int suspendCount;

    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_CLOSE:
            if (MessageBoxW(hWnd, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
            {
                DestroyWindow(hWnd);
            }

        case WM_CREATE:
            {
                HWND btn1 = CreateWindowW(L"button", L"실행", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 20, 100, 25, hWnd, (HMENU)1, g_hInst, NULL);
                HWND btn2 = CreateWindowW(L"button", L"중지", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 60, 100, 25, hWnd, (HMENU)2, g_hInst, NULL);
                SendMessageW(btn1, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
                SendMessageW(btn2, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            }
            return 0;

        case WM_PAINT:
            {
                HDC hdcMem;
                BITMAP bitmap;
                RECT rc;
                HDC hdc;
                PAINTSTRUCT ps;

                GetClientRect(hWnd, &rc);
                hdc = BeginPaint(hWnd, &ps);
                TextOutW(hdc, 100, 150, L"server", 6);
                if(ThreadRun) TextOutW(hdc, 100, 100, L"실행중", 3);
                else TextOutW(hdc, 100, 100, L"중지", 2);
                EndPaint(hWnd, &ps);
            }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case 1:
                ThreadRun = TRUE;
                do { 
                    suspendCount = ResumeThread(tcpThread); 
                } while(suspendCount > 0);
                InvalidateRect(hWnd, NULL, TRUE);
                break;

            case 2:
                ThreadRun = FALSE;
                SuspendThread(tcpThread);
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            }
            return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

INT APIENTRY WinMain(HINSTANCE hIns, HINSTANCE hPrev, LPSTR cmd, INT nShow)
{    
    LPCWSTR CLASS_NAME = L"Sample Window Class";
    WNDCLASSW wc;
    HWND hWnd;
    MSG msg;

    DWORD tcpThreadId = 0;
    DWORD tcpThreadParam = 0;
    DWORD capThreadId = 0;
    DWORD capThreadParam = 0;

    ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    g_hInst = hIns;

    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hIns;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    hWnd = CreateWindowW(CLASS_NAME, L"서버", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hIns, NULL);

    if (hWnd == NULL)
    {
        return 0;
    }

    hwnd1 = hWnd;

    tcpThread = CreateThread(NULL, 0, CreateTcpThread, &tcpThreadParam, 0, &tcpThreadId);

    ShowWindow(hWnd, nShow);

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}