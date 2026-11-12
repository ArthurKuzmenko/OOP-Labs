#include <windows.h>
#include <cmath>
#include "resource.h"
#include "shape.h"

#define MAX_SHAPES 112 //12 варіант:12+100=112

HINSTANCE hInst;
LPCWSTR szTitle = L"Графічний редактор"; //заголовок без типу фігури
int currentShape = IDM_SHAPE_POINT;
bool isDrawing = false;
long xStart = 0, yStart = 0, xEnd = 0, yEnd = 0;
//12 вар:динамічний масив вказівників(12 mod 3 = 0)
Shape** pcshape = new Shape*[MAX_SHAPES]; 
int shapeCount = 0;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawRubberBand(HWND hWnd, long x1, long y1, long x2, long y2, int shapeType) {
    HDC hdc = GetDC(hWnd);
    int oldROP = SetROP2(hdc, R2_NOTXORPEN);
    
    //суцільний слід чорного кольору(12 mod 4 = 0)
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    if (shapeType == IDM_SHAPE_POINT) {
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x1 + 1, y1 + 1);
    } else if (shapeType == IDM_SHAPE_LINE) {
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
    } else if (shapeType == IDM_SHAPE_RECT) {
        //прямокутник:ввід по двох протилежних кутах
        Rectangle(hdc, x1, y1, x2, y2);
    } else if (shapeType == IDM_SHAPE_ELLIPSE) {
        //еліпс:ввід від центру до кута
        long dx = std::abs(x2 - x1);
        long dy = std::abs(y2 - y1);
        Ellipse(hdc, x1 - dx, y1 - dy, x1 + dx, y1 + dy);
    }
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    SetROP2(hdc, oldROP);
    ReleaseDC(hWnd, hdc);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MAIN_MENU);
    wc.lpszClassName = L"Lab2WindowClass";
    if (!RegisterClassExW(&wc)) return 0;    
    HWND hWnd = CreateWindowW(L"Lab2WindowClass", szTitle, WS_OVERLAPPEDWINDOW,
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
    //12 вар:позначка поточного типу об'єкту в меню(12 mod 2 = 0)
    case WM_INITMENUPOPUP:
        CheckMenuItem((HMENU)wParam, IDM_SHAPE_POINT, MF_BYCOMMAND | (currentShape == IDM_SHAPE_POINT ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem((HMENU)wParam, IDM_SHAPE_LINE, MF_BYCOMMAND | (currentShape == IDM_SHAPE_LINE ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem((HMENU)wParam, IDM_SHAPE_RECT, MF_BYCOMMAND | (currentShape == IDM_SHAPE_RECT ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem((HMENU)wParam, IDM_SHAPE_ELLIPSE, MF_BYCOMMAND | (currentShape == IDM_SHAPE_ELLIPSE ? MF_CHECKED : MF_UNCHECKED));
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_SHAPE_POINT:
            currentShape = IDM_SHAPE_POINT;
            break;
        case IDM_SHAPE_LINE:
            currentShape = IDM_SHAPE_LINE;
            break;
        case IDM_SHAPE_RECT:
            currentShape = IDM_SHAPE_RECT;
            break;
        case IDM_SHAPE_ELLIPSE:
            currentShape = IDM_SHAPE_ELLIPSE;
            break;
        case IDM_FILE_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    
    case WM_LBUTTONDOWN:
        if (shapeCount < MAX_SHAPES) {
            isDrawing = true;
            xStart = LOWORD(lParam);
            yStart = HIWORD(lParam);
            xEnd = xStart;
            yEnd = yStart;
            SetCapture(hWnd);
        }
        break;
    case WM_MOUSEMOVE:
        if (isDrawing) {
            DrawRubberBand(hWnd, xStart, yStart, xEnd, yEnd, currentShape);
            xEnd = LOWORD(lParam);
            yEnd = HIWORD(lParam);
            DrawRubberBand(hWnd, xStart, yStart, xEnd, yEnd, currentShape);
        }
        break;
    case WM_LBUTTONUP:
        if (isDrawing) {
            DrawRubberBand(hWnd, xStart, yStart, xEnd, yEnd, currentShape);
            xEnd = LOWORD(lParam);
            yEnd = HIWORD(lParam);
            isDrawing = false;
            ReleaseCapture();
            if (shapeCount < MAX_SHAPES) {
                Shape* pShape = nullptr;
                if (currentShape == IDM_SHAPE_POINT) pShape = new PointShape();
                else if (currentShape == IDM_SHAPE_LINE) pShape = new LineShape();
                else if (currentShape == IDM_SHAPE_RECT) pShape = new RectangleShape();
                else if (currentShape == IDM_SHAPE_ELLIPSE) pShape = new EllipseShape();
              
                if (pShape) {
                    pShape->SetCoord(xStart, yStart, xEnd, yEnd);
                    pcshape[shapeCount++] = pShape;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
        }
        break;   
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        for (int i = 0; i < shapeCount; i++) {
            if (pcshape[i]) {
                pcshape[i]->Show(hdc);
            }
        }
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        for (int i = 0; i < shapeCount; i++) {
            delete pcshape[i];
            pcshape[i] = nullptr;
        }
        delete[] pcshape; //очищення динамічного масиву
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}