#include "editor.h"
#include "resource.h"
#include <cmath>

EditorWindow::EditorWindow(HINSTANCE hInstance) 
    : m_hWnd(NULL), m_hToolBar(NULL), m_hInstance(hInstance), m_currentShape(IDM_SHAPE_POINT), 
      m_isDrawing(false), m_xStart(0), m_yStart(0), m_xEnd(0), m_yEnd(0), m_shapeCount(0) {
    m_pcshape = new Shape*[m_maxShapes];
}

EditorWindow::~EditorWindow() {
    for (int i = 0; i < m_shapeCount; i++) {
        delete m_pcshape[i];
        m_pcshape[i] = nullptr;
    }
    delete[] m_pcshape;
}

bool EditorWindow::Create(PCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight) {
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = EditorWindow::WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MAIN_MENU);
    wc.lpszClassName = L"Lab3EditorClass";

    RegisterClassExW(&wc);

    m_hWnd = CreateWindowW(
        L"Lab3EditorClass", lpWindowName, dwStyle,
        x, y, nWidth, nHeight,
        NULL, NULL, m_hInstance, this
    );

    return (m_hWnd ? true : false);
}

LRESULT CALLBACK EditorWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    EditorWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (EditorWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    } else {
        pThis = (EditorWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT EditorWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        OnCreate();
        break;
    case WM_SIZE:
        OnSize();
        break;
    case WM_NOTIFY:
        OnNotify(lParam);
        break;
    case WM_INITMENUPOPUP:
        OnInitMenuPopup((HMENU)wParam);
        break;
    case WM_COMMAND:
        OnCommand(wParam, lParam);
        break;
    case WM_LBUTTONDOWN:
        OnLButtonDown(lParam);
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(lParam);
        break;
    case WM_LBUTTONUP:
        OnLButtonUp(lParam);
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hWnd, &ps);
        OnPaint(hdc);
        EndPaint(m_hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void EditorWindow::OnCreate() {
    //ініціалізація загальних елементів керування (для Toolbar)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    //створення панелі інструментів із підтримкою підказок
    m_hToolBar = CreateWindowExW(0, TOOLBARCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS, 0, 0, 0, 0,
        m_hWnd, (HMENU)2000, m_hInstance, NULL);

    SendMessage(m_hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

    //додавання стандартних іконок Windows (заглушки для фігур)
    TBADDBITMAP tbab;
    tbab.hInst = HINST_COMMCTRL;
    tbab.nID = IDB_STD_SMALL_COLOR;
    SendMessage(m_hToolBar, TB_ADDBITMAP, 0, (LPARAM)&tbab);

    TBBUTTON tbb[4];
    ZeroMemory(tbb, sizeof(tbb));

    tbb[0].iBitmap = STD_FILENEW;
    tbb[0].fsState = TBSTATE_ENABLED;
    tbb[0].fsStyle = TBSTYLE_BUTTON;
    tbb[0].idCommand = IDM_SHAPE_POINT;

    tbb[1].iBitmap = STD_FILEOPEN;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = TBSTYLE_BUTTON;
    tbb[1].idCommand = IDM_SHAPE_LINE;

    tbb[2].iBitmap = STD_FILESAVE;
    tbb[2].fsState = TBSTATE_ENABLED;
    tbb[2].fsStyle = TBSTYLE_BUTTON;
    tbb[2].idCommand = IDM_SHAPE_RECT;

    tbb[3].iBitmap = STD_PRINTPRE;
    tbb[3].fsState = TBSTATE_ENABLED;
    tbb[3].fsStyle = TBSTYLE_BUTTON;
    tbb[3].idCommand = IDM_SHAPE_ELLIPSE;

    SendMessage(m_hToolBar, TB_ADDBUTTONS, 4, (LPARAM)&tbb);
}

void EditorWindow::OnSize() {
    SendMessage(m_hToolBar, TB_AUTOSIZE, 0, 0);
}

//обробник OnNotify для спливаючих підказок
void EditorWindow::OnNotify(LPARAM lParam) {
    LPNMHDR lpnm = (LPNMHDR)lParam;
    if (lpnm->code == TTN_GETDISPINFOW) {
        LPTOOLTIPTEXTW lpttt = (LPTOOLTIPTEXTW)lParam;
        lpttt->hinst = m_hInstance;
        switch (lpttt->hdr.idFrom) {
        case IDM_SHAPE_POINT:
            lpttt->lpszText = const_cast<LPWSTR>(L"Крапка");
            break;
        case IDM_SHAPE_LINE:
            lpttt->lpszText = const_cast<LPWSTR>(L"Лінія");
            break;
        case IDM_SHAPE_RECT:
            lpttt->lpszText = const_cast<LPWSTR>(L"Прямокутник");
            break;
        case IDM_SHAPE_ELLIPSE:
            lpttt->lpszText = const_cast<LPWSTR>(L"Еліпс");
            break;
        }
    }
}

void EditorWindow::OnInitMenuPopup(HMENU hMenu) {
    CheckMenuItem(hMenu, IDM_SHAPE_POINT, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_POINT ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_LINE, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_LINE ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_RECT, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_RECT ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_ELLIPSE, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_ELLIPSE ? MF_CHECKED : MF_UNCHECKED));
}

void EditorWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
    case IDM_SHAPE_POINT:
        m_currentShape = IDM_SHAPE_POINT;
        break;
    case IDM_SHAPE_LINE:
        m_currentShape = IDM_SHAPE_LINE;
        break;
    case IDM_SHAPE_RECT:
        m_currentShape = IDM_SHAPE_RECT;
        break;
    case IDM_SHAPE_ELLIPSE:
        m_currentShape = IDM_SHAPE_ELLIPSE;
        break;
    case IDM_FILE_EXIT:
        DestroyWindow(m_hWnd);
        break;
    }
}

void EditorWindow::DrawRubberBand(long x1, long y1, long x2, long y2, int shapeType) {
    HDC hdc = GetDC(m_hWnd);
    int oldROP = SetROP2(hdc, R2_NOTXORPEN);
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
        Rectangle(hdc, x1, y1, x2, y2);
    } else if (shapeType == IDM_SHAPE_ELLIPSE) {
        long dx = std::abs(x2 - x1);
        long dy = std::abs(y2 - y1);
        Ellipse(hdc, x1 - dx, y1 - dy, x1 + dx, y1 + dy);
    }

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    SetROP2(hdc, oldROP);
    ReleaseDC(m_hWnd, hdc);
}

void EditorWindow::OnLButtonDown(LPARAM lParam) {
    if (m_shapeCount < m_maxShapes) {
        m_isDrawing = true;
        m_xStart = LOWORD(lParam);
        m_yStart = HIWORD(lParam);
        m_xEnd = m_xStart;
        m_yEnd = m_yStart;
        SetCapture(m_hWnd);
    }
}

void EditorWindow::OnMouseMove(LPARAM lParam) {
    if (m_isDrawing) {
        DrawRubberBand(m_xStart, m_yStart, m_xEnd, m_yEnd, m_currentShape);
        m_xEnd = LOWORD(lParam);
        m_yEnd = HIWORD(lParam);
        DrawRubberBand(m_xStart, m_yStart, m_xEnd, m_yEnd, m_currentShape);
    }
}

void EditorWindow::OnLButtonUp(LPARAM lParam) {
    if (m_isDrawing) {
        DrawRubberBand(m_xStart, m_yStart, m_xEnd, m_yEnd, m_currentShape);
        m_xEnd = LOWORD(lParam);
        m_yEnd = HIWORD(lParam);
        m_isDrawing = false;
        ReleaseCapture();

        if (m_shapeCount < m_maxShapes) {
            Shape* pShape = nullptr;
            if (m_currentShape == IDM_SHAPE_POINT) pShape = new PointShape();
            else if (m_currentShape == IDM_SHAPE_LINE) pShape = new LineShape();
            else if (m_currentShape == IDM_SHAPE_RECT) pShape = new RectangleShape();
            else if (m_currentShape == IDM_SHAPE_ELLIPSE) pShape = new EllipseShape();

            if (pShape) {
                pShape->SetCoord(m_xStart, m_yStart, m_xEnd, m_yEnd);
                m_pcshape[m_shapeCount++] = pShape;
                InvalidateRect(m_hWnd, NULL, TRUE);
            }
        }
    }
}

void EditorWindow::OnPaint(HDC hdc) {
    for (int i = 0; i < m_shapeCount; i++) {
        if (m_pcshape[i]) {
            m_pcshape[i]->Show(hdc);
        }
    }
}