#include <windows.h>
#include <WinSock2.h>

HINSTANCE g_hInst;

HANDLE tcpThread;
static BOOL ThreadRun = TRUE;
static BOOL capBool = FALSE;

HWND hwnd1;
HBITMAP hBitmap;
HGDIOBJ oldBitmap;

int ScreenWidth;
int ScreenHeight;

DWORD tcpThreadId = 0;
DWORD tcpThreadParam = 0;

int capture() {
    while(1){
        HDC hScrDC, hMemDC;
        BITMAP bitmap;
        DWORD dwByteSizeHigh;
        BOOL run = FALSE;

        hScrDC = CreateDCW(L"DISPLAY", NULL, NULL, NULL);
        hMemDC = CreateCompatibleDC(hScrDC);
        hBitmap = CreateCompatibleBitmap(hScrDC, ScreenWidth, ScreenHeight);
        SelectObject(hMemDC, hBitmap);

        BitBlt(hMemDC, 0, 0, ScreenWidth, ScreenHeight, hScrDC, 0, 0, SRCCOPY);

        DeleteDC(hMemDC);
        DeleteDC(hScrDC);
        
        capBool = TRUE;
        InvalidateRect(hwnd1, NULL, FALSE);
        while(1){
            if(capBool == FALSE) {
                DeleteObject(hBitmap);
                break;
            }
        }
        Sleep(33);
	}
    
    return 1;
}

DWORD APIENTRY CreateTcpThread(LPVOID lpParam) {
    int nReturn = 0;
    char temp[1024] = {0};
    WSADATA wsadata = {0};
    SOCKET clientsock = INVALID_SOCKET;
    SOCKADDR_IN addr = {0};
    int addrlen = sizeof(addr);
    const char g_szIpAddress[16+1] = "127.0.0.1";
    const unsigned short g_uPort = 7878;

    printf("Client starts...\n");

    nReturn = WSAStartup((WORD)(2.0), &wsadata);
    if (nReturn != 0) {
        printf("WSAStartup failed. Error No. %d\n", WSAGetLastError());
        goto Done;
    }

    clientsock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientsock == INVALID_SOCKET) {
        printf("socket failed. Error No. %d\n", WSAGetLastError());
        goto Done;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(g_szIpAddress);
    addr.sin_port = htons(g_uPort);

    nReturn = connect(clientsock, (SOCKADDR *)&addr, addrlen);
    if (nReturn == SOCKET_ERROR) {
        printf("connect failed. Error No. %d\n", WSAGetLastError());
        goto Done;
    }

    printf("succese \n");
    while(1) {
        while(capture() == 1) {
            GetFileSize()
        }
    }

    Done:
    
    if (clientsock != INVALID_SOCKET) {
        closesocket(clientsock);
    }
    WSACleanup();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int suspendCount1;
    int suspendCount2;

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
                if(capBool) {
                    hdcMem = CreateCompatibleDC(hdc);
                    oldBitmap = SelectObject(hdcMem, hBitmap);
                    GetObjectW(hBitmap, sizeof(bitmap), &bitmap);
                    StretchBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top,  hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
                    SelectObject(hdcMem, oldBitmap);
                    DeleteDC(hdcMem);
                    DeleteObject(oldBitmap);
                    capBool = FALSE;
                }
                TextOutW(hdc, 100, 150, L"client", 6);
                if(ThreadRun) TextOutW(hdc, 100, 100, L"실행중", 3);
                else TextOutW(hdc, 100, 100, L"중지", 2);
                EndPaint(hWnd, &ps);
            }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case 1:
            {
                ThreadRun = TRUE;
                DWORD dwWait = WaitForSingleObject(tcpThread, 0);
                if(dwWait != WAIT_TIMEOUT) {
                    tcpThread = CreateThread(NULL, 0, CreateTcpThread, &tcpThreadParam, 0, &tcpThreadId);
                } else {
                    do { 
                        suspendCount = ResumeThread(tcpThread); 
                    } while(suspendCount > 0);
                }
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            }
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

    hWnd = CreateWindowW(CLASS_NAME, L"클라이언트", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hIns, NULL);

    if (hWnd == NULL)
    {
        return 0;
    }

    hwnd1 = hWnd;

    tcpThread = CreateThread(NULL, 0, CreateTcpThread, &tcpThreadParam, 0, &tcpThreadId);

    ShowWindow(hWnd, nShow);
    UpdateWindow(hWnd);

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}