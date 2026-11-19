#include "editor.h"
#include "resource.h"
#include <commdlg.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

MyEditor::MyEditor()
    : m_hWnd(NULL), m_hToolBar(NULL), m_hInstance(NULL), m_currentShape(IDM_SHAPE_POINT),
      m_isDrawing(false), m_xStart(0), m_yStart(0), m_xEnd(0), m_yEnd(0),
      m_pDraftShape(nullptr), m_shapeCount(0), m_selectedIndex(-1), m_pTable(nullptr) {
    m_pcshape = new Shape*[m_maxShapes];
}

MyEditor::~MyEditor() {
    delete m_pDraftShape;
    ClearShapes();
    delete[] m_pcshape;
}

void MyEditor::Init(HINSTANCE hInstance) {
    m_hInstance = hInstance;
}

bool MyEditor::IsDialogMsg(MSG* pMsg) {
    if (!m_pTable) return false;
    HWND hDlg = m_pTable->GetDialogHandle();
    return (hDlg && IsWindow(hDlg) && IsDialogMessageW(hDlg, pMsg));
}

void MyEditor::ClearShapes() {
    for (int i = 0; i < m_shapeCount; i++) {
        delete m_pcshape[i];
        m_pcshape[i] = nullptr;
    }
    m_shapeCount = 0;
    m_selectedIndex = -1;
}

void MyEditor::OnTableRowSelected(int rowIndex) {
    m_selectedIndex = rowIndex;
    for (int i = 0; i < m_shapeCount; i++) {
        if (m_pcshape[i]) {
            m_pcshape[i]->SetSelected(i == m_selectedIndex);
        }
    }
    if (m_hWnd && IsWindow(m_hWnd)) {
        InvalidateRect(m_hWnd, NULL, TRUE);
    }
}

void MyEditor::OnTableRowDeleted(int rowIndex) {
    if (rowIndex < 0 || rowIndex >= m_shapeCount) return;

    delete m_pcshape[rowIndex];
    for (int i = rowIndex; i < m_shapeCount - 1; i++) {
        m_pcshape[i] = m_pcshape[i + 1];
    }
    m_pcshape[m_shapeCount - 1] = nullptr;
    m_shapeCount--;

    OnTableRowSelected(-1);
    SyncTable();
}

void MyEditor::SyncTable() {
    if (!m_pTable || !m_pTable->IsOpen()) return;

    std::vector<TableRowData> rows;
    rows.reserve(m_shapeCount);
    for (int i = 0; i < m_shapeCount; i++) {
        if (m_pcshape[i]) {
            rows.push_back({
                m_pcshape[i]->GetName(),
                m_pcshape[i]->GetX1(),
                m_pcshape[i]->GetY1(),
                m_pcshape[i]->GetX2(),
                m_pcshape[i]->GetY2()
            });
        }
    }
    m_pTable->UpdateData(rows);
}

void MyEditor::SaveToFile() {
    wchar_t szFile[MAX_PATH] = L"shapes.txt";

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"Текстові файли (*.txt)\0*.txt\0Усі файли (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (!GetSaveFileNameW(&ofn)) {
        return;
    }

    std::ofstream out{ std::filesystem::path(szFile) };
    if (!out.is_open()) {
        MessageBoxW(m_hWnd, L"Не вдалося відкрити файл для запису.", L"Помилка", MB_OK | MB_ICONERROR);
        return;
    }

    for (int i = 0; i < m_shapeCount; i++) {
        if (m_pcshape[i]) {
            out << m_pcshape[i]->GetTypeTag() << ' '
                << m_pcshape[i]->GetX1() << ' '
                << m_pcshape[i]->GetY1() << ' '
                << m_pcshape[i]->GetX2() << ' '
                << m_pcshape[i]->GetY2() << '\n';
        }
    }

    out.close();
    MessageBoxW(m_hWnd, L"Множину об'єктів успішно збережено у файл.", L"Збереження", MB_OK | MB_ICONINFORMATION);
}

void MyEditor::LoadFromFile() {
    wchar_t szFile[MAX_PATH] = L"";

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"Текстові файли (*.txt)\0*.txt\0Усі файли (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (!GetOpenFileNameW(&ofn)) {
        return;
    }

    std::ifstream in{ std::filesystem::path(szFile) };
    if (!in.is_open()) {
        MessageBoxW(m_hWnd, L"Не вдалося відкрити файл для читання.", L"Помилка", MB_OK | MB_ICONERROR);
        return;
    }

    ClearShapes();

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string tag;
        long x1 = 0, y1 = 0, x2 = 0, y2 = 0;

        if (iss >> tag >> x1 >> y1 >> x2 >> y2) {
            if (m_shapeCount >= m_maxShapes) {
                break;
            }
            Shape* shape = CreateShapeByTag(tag);
            if (shape) {
                shape->SetCoord(x1, y1, x2, y2);
                m_pcshape[m_shapeCount++] = shape;
            }
        }
    }

    in.close();

    OnTableRowSelected(-1);
    SyncTable();
}

bool MyEditor::Create(PCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight) {
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MyEditor::WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MAIN_MENU);
    wc.lpszClassName = L"Lab5EditorClass";

    RegisterClassExW(&wc);

    m_hWnd = CreateWindowW(
        L"Lab5EditorClass", lpWindowName, dwStyle,
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
        if (m_pTable) m_pTable->Close();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void MyEditor::CreateToolbar() {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 6, 0);

    HDC hdcScreen = GetDC(m_hWnd);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, 96, 16);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmp);

    COLORREF maskColor = RGB(255, 0, 255);
    HBRUSH hBgBrush = CreateSolidBrush(maskColor);
    RECT rcFull = { 0, 0, 96, 16 };
    FillRect(hdcMem, &rcFull, hBgBrush);
    DeleteObject(hBgBrush);

    HPEN hBlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hBlackPen);

    //крапка
    HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBlackBrush);
    Ellipse(hdcMem, 6, 6, 11, 11);
    SelectObject(hdcMem, hOldBrush);
    DeleteObject(hBlackBrush);

    //лінія
    MoveToEx(hdcMem, 16 + 3, 12, NULL);
    LineTo(hdcMem, 16 + 13, 3);

    //прямокутник(з жовтим заповненням)
    HBRUSH hYellowBrush = CreateSolidBrush(RGB(255, 255, 0));
    hOldBrush = (HBRUSH)SelectObject(hdcMem, hYellowBrush);
    Rectangle(hdcMem, 32 + 3, 3, 32 + 13, 13);
    SelectObject(hdcMem, hOldBrush);
    DeleteObject(hYellowBrush);

    //еліпс(без заповнення)
    hOldBrush = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
    Ellipse(hdcMem, 48 + 2, 4, 48 + 14, 12);
    SelectObject(hdcMem, hOldBrush);

    //лінія з кружечками
    MoveToEx(hdcMem, 64 + 4, 11, NULL);
    LineTo(hdcMem, 64 + 12, 4);
    hOldBrush = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
    Ellipse(hdcMem, 64 + 2, 9, 64 + 6, 13);
    Ellipse(hdcMem, 64 + 10, 2, 64 + 14, 6);
    SelectObject(hdcMem, hOldBrush);

    //каркас куба
    hOldBrush = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
    Rectangle(hdcMem, 80 + 2, 6, 80 + 11, 14);
    Rectangle(hdcMem, 80 + 5, 2, 80 + 14, 10);
    MoveToEx(hdcMem, 80 + 2, 6, NULL);  LineTo(hdcMem, 80 + 5, 2);
    MoveToEx(hdcMem, 80 + 11, 6, NULL); LineTo(hdcMem, 80 + 14, 2);
    MoveToEx(hdcMem, 80 + 2, 14, NULL); LineTo(hdcMem, 80 + 5, 10);
    MoveToEx(hdcMem, 80 + 11, 14, NULL);LineTo(hdcMem, 80 + 14, 10);
    SelectObject(hdcMem, hOldBrush);

    SelectObject(hdcMem, hOldPen);
    DeleteObject(hBlackPen);
    SelectObject(hdcMem, hOldBmp);
    DeleteDC(hdcMem);
    ReleaseDC(m_hWnd, hdcScreen);

    ImageList_AddMasked(hImageList, hBmp, maskColor);
    DeleteObject(hBmp);

    TBBUTTON tbb[6];
    ZeroMemory(tbb, sizeof(tbb));

    tbb[0].iBitmap = 0; tbb[0].idCommand = IDM_SHAPE_POINT;   tbb[0].fsState = TBSTATE_ENABLED | TBSTATE_CHECKED; tbb[0].fsStyle = TBSTYLE_CHECKGROUP;
    tbb[1].iBitmap = 1; tbb[1].idCommand = IDM_SHAPE_LINE;    tbb[1].fsState = TBSTATE_ENABLED;                   tbb[1].fsStyle = TBSTYLE_CHECKGROUP;
    tbb[2].iBitmap = 2; tbb[2].idCommand = IDM_SHAPE_RECT;    tbb[2].fsState = TBSTATE_ENABLED;                   tbb[2].fsStyle = TBSTYLE_CHECKGROUP;
    tbb[3].iBitmap = 3; tbb[3].idCommand = IDM_SHAPE_ELLIPSE; tbb[3].fsState = TBSTATE_ENABLED;                   tbb[3].fsStyle = TBSTYLE_CHECKGROUP;
    tbb[4].iBitmap = 4; tbb[4].idCommand = IDM_SHAPE_LINEOO;  tbb[4].fsState = TBSTATE_ENABLED;                   tbb[4].fsStyle = TBSTYLE_CHECKGROUP;
    tbb[5].iBitmap = 5; tbb[5].idCommand = IDM_SHAPE_CUBE;    tbb[5].fsState = TBSTATE_ENABLED;                   tbb[5].fsStyle = TBSTYLE_CHECKGROUP;

    m_hToolBar = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        m_hWnd, (HMENU)2000, m_hInstance, NULL
    );

    SendMessage(m_hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(m_hToolBar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
    SendMessage(m_hToolBar, TB_ADDBUTTONS, 6, (LPARAM)&tbb);
    SendMessage(m_hToolBar, TB_AUTOSIZE, 0, 0);
}

void MyEditor::OnCreate() {
    CreateToolbar();
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
        case IDM_SHAPE_LINEOO:
            lpttt->lpszText = const_cast<LPWSTR>(L"Лінія з кружечками");
            break;
        case IDM_SHAPE_CUBE:
            lpttt->lpszText = const_cast<LPWSTR>(L"Каркас куба");
            break;
        }
    }
}

void MyEditor::OnInitMenuPopup(HMENU hMenu) {
    CheckMenuItem(hMenu, IDM_SHAPE_POINT,   MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_POINT   ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_LINE,    MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_LINE    ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_RECT,    MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_RECT    ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_ELLIPSE, MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_ELLIPSE ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_LINEOO,  MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_LINEOO  ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_SHAPE_CUBE,    MF_BYCOMMAND | (m_currentShape == IDM_SHAPE_CUBE    ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hMenu, IDM_VIEW_TABLE,    MF_BYCOMMAND | (m_pTable && m_pTable->IsOpen()     ? MF_CHECKED : MF_UNCHECKED));
}

void MyEditor::SelectShape(int shapeId) {
    m_currentShape = shapeId;
    SendMessage(m_hToolBar, TB_CHECKBUTTON, shapeId, MAKELPARAM(TRUE, 0));
}

void MyEditor::OnCommand(WPARAM wParam, LPARAM lParam) {
    int id = LOWORD(wParam);
    switch (id) {
    case IDM_SHAPE_POINT:
    case IDM_SHAPE_LINE:
    case IDM_SHAPE_RECT:
    case IDM_SHAPE_ELLIPSE:
    case IDM_SHAPE_LINEOO:
    case IDM_SHAPE_CUBE:
        SelectShape(id);
        break;
    case IDM_VIEW_TABLE:
        if (m_pTable) {
            if (m_pTable->IsOpen()) {
                m_pTable->Close();
            } else {
                m_pTable->Show(m_hInstance, m_hWnd);
                SyncTable();
            }
        }
        break;
    case IDM_FILE_OPEN:
        LoadFromFile();
        break;
    case IDM_FILE_SAVE:
        SaveToFile();
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
    case IDM_SHAPE_LINEOO:  return new LineWithCirclesShape();
    case IDM_SHAPE_CUBE:    return new CubeShape();
    default:                return nullptr;
    }
}

Shape* MyEditor::CreateShapeByTag(const std::string& tag) {
    if (tag == "POINT")   return new PointShape();
    if (tag == "LINE")    return new LineShape();
    if (tag == "RECT")    return new RectShape();
    if (tag == "ELLIPSE") return new EllipseShape();
    if (tag == "LINEOO")  return new LineWithCirclesShape();
    if (tag == "CUBE")    return new CubeShape();
    return nullptr;
}

void MyEditor::DrawRubberBand() {
    if (!m_pDraftShape) return;

    HDC hdc = GetDC(m_hWnd);
    int oldROP = SetROP2(hdc, R2_NOTXORPEN);
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

        OnTableRowSelected(-1);
        SyncTable();
    }
}

void MyEditor::OnPaint(HDC hdc) {
    for (int i = 0; i < m_shapeCount; i++) {
        if (m_pcshape[i]) {
            m_pcshape[i]->Show(hdc);
        }
    }
}