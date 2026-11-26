#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define NOMINMAX

#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <locale>
#include <algorithm>
#include <cmath>

constexpr wchar_t LAB6_CLASS_NAME[] = L"Lab6ManagerClass";
constexpr wchar_t OBJ3_CLASS_NAME[] = L"Object3GraphClass";

struct Object3State {
    HWND hWnd = NULL;
    std::vector<double> data;

    UINT msgObj3Ready = 0;
    UINT msgObj3Draw = 0;
};

bool ReadVectorFromClipboard(HWND hWnd, std::vector<double>& outVec) {
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return false;
    if (!OpenClipboard(hWnd)) return false;

    std::wstring text;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        const wchar_t* pszText = static_cast<const wchar_t*>(GlobalLock(hData));
        if (pszText) {
            text = pszText;
            GlobalUnlock(hData);
        }
    }
    CloseClipboard();

    if (text.empty()) return false;

    for (wchar_t& ch : text) {
        if (ch == L',') ch = L'.';
    }

    std::wistringstream iss(text);
    iss.imbue(std::locale::classic());

    std::vector<double> parsed;
    double val = 0.0;
    while (iss >> val) {
        parsed.push_back(val);
    }

    if (parsed.empty()) return false;
    outVec = std::move(parsed);
    return true;
}

void DrawArrowHead(HDC hdc, POINT tip, POINT from) {
    double dx = static_cast<double>(tip.x - from.x);
    double dy = static_cast<double>(tip.y - from.y);
    double len = std::hypot(dx, dy);
    if (len < 1e-6) return;

    double ux = dx / len;
    double uy = dy / len;
    double arrowLen = 9.0;
    double arrowHalfW = 4.0;

    POINT pts[3];
    pts[0] = tip;
    pts[1] = {
        static_cast<LONG>(std::lround(tip.x - ux * arrowLen - uy * arrowHalfW)),
        static_cast<LONG>(std::lround(tip.y - uy * arrowLen + ux * arrowHalfW))
    };
    pts[2] = {
        static_cast<LONG>(std::lround(tip.x - ux * arrowLen + uy * arrowHalfW)),
        static_cast<LONG>(std::lround(tip.y - uy * arrowLen - ux * arrowHalfW))
    };
    Polygon(hdc, pts, 3);
}

void DrawGraph(HDC hdc, HWND hWnd, const Object3State* state) {
    RECT rc;
    GetClientRect(hWnd, &rc);

    HFONT hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hFont));
    SetBkMode(hdc, TRANSPARENT);

    if (state->data.empty()) {
        const wchar_t* waitMsg = L"Очікування даних в Clipboard...";
        TextOutW(hdc, 14, 14, waitMsg, lstrlenW(waitMsg));
        SelectObject(hdc, hOldFont);
        return;
    }

    auto [minIt, maxIt] = std::minmax_element(state->data.begin(), state->data.end());
    double dataMin = *minIt;
    double dataMax = *maxIt;

    std::wostringstream header;
    header.imbue(std::locale::classic());
    header << std::fixed << std::setprecision(2)
           << L"Графік y = f(x) з Clipboard (n = " << state->data.size()
           << L", Ymin = " << dataMin << L", Ymax = " << dataMax << L")";
    std::wstring headerStr = header.str();
    TextOutW(hdc, 14, 10, headerStr.c_str(), static_cast<int>(headerStr.size()));

    const int marginLeft = 64;
    const int marginRight = 28;
    const int marginTop = 38;
    const int marginBottom = 42;

    int plotW = rc.right - marginLeft - marginRight;
    int plotH = rc.bottom - marginTop - marginBottom;
    if (plotW < 60 || plotH < 60) {
        SelectObject(hdc, hOldFont);
        return;
    }

    double yLow = std::min(0.0, dataMin);
    double yHigh = std::max(0.0, dataMax);
    double ySpan = yHigh - yLow;
    if (ySpan < 1e-9) {
        yLow -= 1.0;
        yHigh += 1.0;
        ySpan = 2.0;
    } else {
        double pad = ySpan * 0.08;
        yLow -= pad;
        yHigh += pad;
        ySpan = yHigh - yLow;
    }

    int n = static_cast<int>(state->data.size());
    int xMaxIdx = std::max(1, n - 1);

    auto mapX = [&](double idx) -> int {
        return marginLeft + static_cast<int>(std::lround((idx / xMaxIdx) * plotW));
    };
    auto mapY = [&](double val) -> int {
        return marginTop + static_cast<int>(std::lround(((yHigh - val) / ySpan) * plotH));
    };

    HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(220, 220, 220));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hGridPen));

    const int yTicks = 6;
    for (int t = 0; t <= yTicks; ++t) {
        double val = yLow + (ySpan * t) / yTicks;
        int py = mapY(val);

        MoveToEx(hdc, marginLeft, py, NULL);
        LineTo(hdc, marginLeft + plotW, py);

        std::wostringstream lbl;
        lbl.imbue(std::locale::classic());
        lbl << std::fixed << std::setprecision(2) << val;
        std::wstring s = lbl.str();

        RECT lblRect = { 4, py - 8, marginLeft - 6, py + 8 };
        DrawTextW(hdc, s.c_str(), -1, &lblRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }

    int xStep = std::max(1, xMaxIdx / std::max(1, plotW / 45));
    for (int i = 0; i < n; i += xStep) {
        int px = mapX(i);
        MoveToEx(hdc, px, marginTop, NULL);
        LineTo(hdc, px, marginTop + plotH);

        std::wstring s = std::to_wstring(i);
        RECT lblRect = { px - 18, marginTop + plotH + 6, px + 18, marginTop + plotH + 22 };
        DrawTextW(hdc, s.c_str(), -1, &lblRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    HPEN hAxisPen = CreatePen(PS_SOLID, 1, RGB(40, 40, 40));
    HBRUSH hAxisBrush = CreateSolidBrush(RGB(40, 40, 40));
    SelectObject(hdc, hAxisPen);
    HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hAxisBrush));

    int x0 = mapX(0.0);
    int y0 = mapY(0.0);
    y0 = std::clamp(y0, marginTop, marginTop + plotH);

    // Вісь Y
    MoveToEx(hdc, x0, marginTop + plotH, NULL);
    LineTo(hdc, x0, marginTop - 6);
    DrawArrowHead(hdc, { x0, marginTop - 8 }, { x0, marginTop + 10 });
    TextOutW(hdc, x0 + 6, marginTop - 12, L"Y", 1);

    // Вісь X
    MoveToEx(hdc, marginLeft, y0, NULL);
    LineTo(hdc, marginLeft + plotW + 8, y0);
    DrawArrowHead(hdc, { marginLeft + plotW + 10, y0 }, { marginLeft + plotW - 10, y0 });
    TextOutW(hdc, marginLeft + plotW + 12, y0 - 8, L"X", 1);

    // Графік y = f(x)
    HPEN hGraphPen = CreatePen(PS_SOLID, 2, RGB(0, 102, 204));
    HBRUSH hPointBrush = CreateSolidBrush(RGB(0, 102, 204));
    SelectObject(hdc, hGraphPen);
    SelectObject(hdc, hPointBrush);

    std::vector<POINT> pts(n);
    for (int i = 0; i < n; ++i) {
        pts[i] = { mapX(i), mapY(state->data[i]) };
    }

    if (n >= 2) {
        Polyline(hdc, pts.data(), n);
    }

    if (n <= 120) {
        for (const POINT& pt : pts) {
            Ellipse(hdc, pt.x - 3, pt.y - 3, pt.x + 4, pt.y + 4);
        }
    }

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hGraphPen);
    DeleteObject(hPointBrush);
    DeleteObject(hAxisPen);
    DeleteObject(hAxisBrush);
    DeleteObject(hGridPen);
    SelectObject(hdc, hOldFont);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Object3State* state = nullptr;

    if (message == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        state = reinterpret_cast<Object3State*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        state->hWnd = hWnd;
    } else {
        state = reinterpret_cast<Object3State*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (state && message == state->msgObj3Draw) {
        ReadVectorFromClipboard(hWnd, state->data);
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    }

    switch (message) {
    case WM_CREATE:
        if (state) {
            ReadVectorFromClipboard(hWnd, state->data);
            HWND hLab6 = FindWindowW(LAB6_CLASS_NAME, NULL);
            if (hLab6) {
                PostMessageW(hLab6, state->msgObj3Ready, 0, 0);
            }
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (state) {
            DrawGraph(hdc, hWnd, state);
        }
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    Object3State state;
    state.msgObj3Ready = RegisterWindowMessageW(L"LAB6_OBJ3_READY");
    state.msgObj3Draw  = RegisterWindowMessageW(L"LAB6_OBJ3_DRAW");

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = OBJ3_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowW(
        OBJ3_CLASS_NAME, L"Object3 — Графік функції (Варіант 3)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 580, 420, NULL, NULL, hInstance, &state
    );
    if (!hWnd) return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}