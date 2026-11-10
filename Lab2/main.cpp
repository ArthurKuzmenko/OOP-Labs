#include <windows.h>
#include <cmath>
#include "resource.h"
#include "shape.h"

HINSTANCE hInst;
LPCSTR szTitle = "Графічний редактор-[крапка]";
int currentShape = IDM_SHAPE_POINT;
bool isDrawing = false;
long xStart = 0, yStart = 0, xEnd = 0, yEnd = 0;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawRubberBand(HWND hWnd, long x1, long y1, long x2, long y2, int shapeType) {
    HDC hdc = GetDC(hWnd);
    int oldROP = SetROP2(hdc, R2_NOTXORPEN);
    HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0)); //пунктир чорного кольору
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    if (shapeType == IDM_SHAPE_POINT) {
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x1 + 1, y1 + 1);
    } else if (shapeType == IDM_SHAPE_LINE) {
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
    } else if (shapeType == IDM_SHAPE_RECT) {
        //ввід від центру до кута
        long dx = std::abs(x2 - x1);
        long dy = std::abs(y2 - y1);
        Rectangle(hdc, x1 - dx, y1 - dy, x1 + dx, y1 + dy);
    } else if (shapeType == IDM_SHAPE_ELLIPSE) {
        //ввід по двох протилежних кутах
        Ellipse(hdc, x1, y1, x2, y2);
    }
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    SetROP2(hdc, oldROP);
    ReleaseDC(hWnd, hdc);
}
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
            currentShape = IDM_SHAPE_POINT;
            SetWindowTextA(hWnd, "Графічний редактор-[крапка]");
            break;
        case IDM_SHAPE_LINE:
            currentShape = IDM_SHAPE_LINE;
            SetWindowTextA(hWnd, "Графічний редактор-[лінія]");
            break;
        case IDM_SHAPE_RECT:
            currentShape = IDM_SHAPE_RECT;
            SetWindowTextA(hWnd, "Графічний редактор-[прямокутник]");
            break;
        case IDM_SHAPE_ELLIPSE:
            currentShape = IDM_SHAPE_ELLIPSE;
            SetWindowTextA(hWnd, "Графічний редактор-[еліпс]");
            break;
        case IDM_FILE_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_LBUTTONDOWN:
        isDrawing = true;
        xStart = LOWORD(lParam);
        yStart = HIWORD(lParam);
        xEnd = xStart;
        yEnd = yStart;
        SetCapture(hWnd);
        break;
    case WM_MOUSEMOVE:
        if (isDrawing) {
            DrawRubberBand(hWnd, xStart, yStart, xEnd, yEnd, currentShape); //стирає старий слід
            xEnd = LOWORD(lParam);
            yEnd = HIWORD(lParam);
            DrawRubberBand(hWnd, xStart, yStart, xEnd, yEnd, currentShape); //малює новий слід
        }
        break;
    case WM_LBUTTONUP:
        if (isDrawing) {
            DrawRubberBand(hWnd, xStart, yStart, xEnd, yEnd, currentShape); //стирає останній гумовий слід
            xEnd = LOWORD(lParam);
            yEnd = HIWORD(lParam);
            isDrawing = false;
            ReleaseCapture();
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