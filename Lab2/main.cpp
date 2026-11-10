#include <windows.h>
#include "resource.h"
HINSTANCE hInst;
LPCSTR szTitle = "Графічний редактор-[крапка]";
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEA(IDR_MAIN_MENU);
    wc.lpszClassName = "Lab2WindowClass";
    if (!RegisterClassExA(&wc)) return 0;
    HWND hWnd = CreateWindowA("Lab2WindowClass", szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    if (!hWnd) return 0;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_SHAPE_POINT:
            SetWindowTextA(hWnd, "Графічний редактор-[крапка]");
            break;
        case IDM_SHAPE_LINE:
            SetWindowTextA(hWnd, "Графічний редактор-[лінія]");
            break;
        case IDM_SHAPE_RECT:
            SetWindowTextA(hWnd, "Графічний редактор-[прямокутник]");
            break;
        case IDM_SHAPE_ELLIPSE:
            SetWindowTextA(hWnd, "Графічний редактор-[еліпс]");
            break;
        case IDM_FILE_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }
    return 0;
}