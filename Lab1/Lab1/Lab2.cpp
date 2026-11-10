#include <windows.h>
#include "resource.h"
#include "module1.h"
#include "module2.h"
static wchar_t g_displayText[256] = L"Результат з'явиться ось тут";
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
        switch (wmId) {
        case IDM_WORK1:
            if (ShowModule1Dialog(hInst, hWnd, g_displayText, 256)) {
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        case IDM_WORK2:
            if (ShowModule2Dialog(hInst, hWnd, g_displayText, 256)) {
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        }
    }
    break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        TextOutW(hdc, 20, 20, g_displayText, lstrlenW(g_displayText));
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Lab1WindowClass";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDM_MYMENU);
    RegisterClassW(&wc);
    HWND hWnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Лабораторна робота №1 - Lab1",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL, NULL, hInstance, NULL
    );
    if (hWnd == NULL) {
        return 0;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}