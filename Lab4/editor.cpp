#include "editor.h"
#include "resource.h"
#include <cmath>

MyEditor::MyEditor()
    : m_hWnd(NULL), m_hToolBar(NULL), m_hInstance(NULL), m_currentShape(IDM_SHAPE_POINT),
      m_isDrawing(false), m_xStart(0), m_yStart(0), m_xEnd(0), m_yEnd(0),
      m_pDraftShape(nullptr), m_shapeCount(0) {
    m_pcshape = new Shape*[m_maxShapes];
}

MyEditor::~MyEditor() {
    delete m_pDraftShape;
    for (int i = 0; i < m_shapeCount; i++) {
        delete m_pcshape[i];
        m_pcshape[i] = nullptr;
    }
    delete[] m_pcshape;
}

void MyEditor::Init(HINSTANCE hInstance) {
    m_hInstance = hInstance;
}

bool MyEditor::Create(PCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight) {
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MyEditor::WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MAIN_MENU);
    wc.lpszClassName = L"Lab4EditorClass";

    RegisterClassExW(&wc);

    m_hWnd = CreateWindowW(
        L"Lab4EditorClass", lpWindowName, dwStyle,
        x, y, nWidth, nHeight,
        NULL, NULL, m_hInstance, this
    );

    return (m_hWnd ? true : false);
}

LRESULT CALLBACK MyEditor::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MyEditor* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (MyEditor*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hwnd;
    } else {
        pThis = (MyEditor*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT MyEditor::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

void MyEditor::OnCreate() {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    m_hToolBar = CreateWindowExW(0, TOOLBARCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS, 0, 0, 0, 0,
        m_hWnd, (HMENU)2000, m_hInstance, NULL);

    SendMessage(m_hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 4, 0);
    HBITMAP hBmp = (HBITMAP)LoadImageW(m_hInstance, MAKEINTRESOURCEW(IDB_TOOLBAR), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    ImageList_AddMasked(hImageList, hBmp, RGB(192, 192, 192));
    DeleteObject(hBmp);
    SendMessage(m_hToolBar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

    TBBUTTON tbb[4];
    ZeroMemory(tbb, sizeof(tbb));

    tbb[0].iBitmap = 0; tbb[0].fsState = TBSTATE_ENABLED; tbb[0].fsStyle = TBSTYLE_BUTTON; tbb[0].idCommand = IDM_SHAPE_POINT;
    tbb[1].iBitmap = 1; tbb[1].fsState = TBSTATE_ENABLED; tbb[1].fsStyle = TBSTYLE_BUTTON; tbb[1].idCommand = IDM_SHAPE_LINE;
    tbb[2].iBitmap = 2; tbb[2].fsState = TBSTATE_ENABLED; tbb[2].fsStyle = TBSTYLE_BUTTON; tbb[2].idCommand = IDM_SHAPE_RECT;
    tbb[3].iBitmap = 3; tbb[3].fsState = TBSTATE_ENABLED; tbb[3].fsStyle = TBSTYLE_BUTTON; tbb[3].idCommand = IDM_SHAPE_ELLIPSE;

    SendMessage(m_hToolBar, TB_ADDBUTTONS, 4, (LPARAM)&tbb);
}

void MyEditor::OnSize() {
    SendMessage(m_hToolBar, TB_AUTOSIZE, 0, 0);
}

void MyEditor::OnNotify(LPARAM lParam) {
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

void MyEditor::OnInitMenuPopup(HMENU hMenu) {
    CheckMenuItem(hMenu, IDM_SHAPE_POINT, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_POINT ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_LINE, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_LINE ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_RECT, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_RECT ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_ELLIPSE, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_ELLIPSE ? MF_CHECKED : MF_UNCHECKED));
}

void MyEditor::OnCommand(WPARAM wParam, LPARAM lParam) {
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

Shape* MyEditor::CreateSelectedShape() {
    switch (m_currentShape) {
    case IDM_SHAPE_POINT:   return new PointShape();
    case IDM_SHAPE_LINE:    return new LineShape();
    case IDM_SHAPE_RECT:    return new RectShape();
    case IDM_SHAPE_ELLIPSE: return new EllipseShape();
    default:                return nullptr;
    }
}

void MyEditor::DrawRubberBand() {
    if (!m_pDraftShape) return;

    HDC hdc = GetDC(m_hWnd);
    int oldROP = SetROP2(hdc, R2_NOTXORPEN);
    // Пунктирний стиль лінії для "гумового" сліду (вимога п.3)
    HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    m_pDraftShape->SetCoord(m_xStart, m_yStart, m_xEnd, m_yEnd);
    m_pDraftShape->DrawRubberBand(hdc);

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    SetROP2(hdc, oldROP);
    ReleaseDC(m_hWnd, hdc);
}

void MyEditor::OnLButtonDown(LPARAM lParam) {
    if (m_shapeCount < m_maxShapes) {
        m_pDraftShape = CreateSelectedShape();
        if (m_pDraftShape) {
            m_isDrawing = true;
            m_xStart = LOWORD(lParam);
            m_yStart = HIWORD(lParam);
            m_xEnd = m_xStart;
            m_yEnd = m_yStart;
            SetCapture(m_hWnd);
        }
    }
}

void MyEditor::OnMouseMove(LPARAM lParam) {
    if (m_isDrawing && m_pDraftShape) {
        DrawRubberBand();
        m_xEnd = LOWORD(lParam);
        m_yEnd = HIWORD(lParam);
        DrawRubberBand();
    }
}

void MyEditor::OnLButtonUp(LPARAM lParam) {
    if (m_isDrawing && m_pDraftShape) {
        DrawRubberBand();
        m_xEnd = LOWORD(lParam);
        m_yEnd = HIWORD(lParam);
        m_isDrawing = false;
        ReleaseCapture();

        m_pDraftShape->SetCoord(m_xStart, m_yStart, m_xEnd, m_yEnd);
        m_pcshape[m_shapeCount++] = m_pDraftShape;
        m_pDraftShape = nullptr;
        InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

void MyEditor::OnPaint(HDC hdc) {
    for (int i = 0; i < m_shapeCount; i++) {
        if (m_pcshape[i]) {
            m_pcshape[i]->Show(hdc);
        }
    }
}