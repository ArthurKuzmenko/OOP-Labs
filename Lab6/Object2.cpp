#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <locale>
#include <algorithm>

constexpr wchar_t LAB6_CLASS_NAME[] = L"Lab6ManagerClass";
constexpr wchar_t OBJ2_CLASS_NAME[] = L"Object2VectorClass";

struct VectorParams {
    int n;
    double minVal;
    double maxVal;
};

struct Object2State {
    HWND hWnd = NULL;
    std::vector<double> data;
    VectorParams lastParams{ 0, 0.0, 0.0 };

    UINT msgObj2Ready = 0;
    UINT msgObj2Done = 0;
};

bool WriteVectorToClipboard(HWND hWnd, const std::vector<double>& vec) {
    if (!OpenClipboard(hWnd)) return false;

    EmptyClipboard();

    std::wostringstream wss;
    wss.imbue(std::locale::classic());
    wss << std::setprecision(10);
    for (size_t i = 0; i < vec.size(); ++i) {
        wss << vec[i];
        if (i + 1 < vec.size()) {
            wss << L"\r\n";
        }
    }

    std::wstring text = wss.str();
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) {
        CloseClipboard();
        return false;
    }

    void* ptr = GlobalLock(hMem);
    if (ptr) {
        memcpy(ptr, text.c_str(), bytes);
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
    } else {
        GlobalFree(hMem);
    }

    CloseClipboard();
    return true;
}

void GenerateVector(Object2State* state, const VectorParams& params) {
    state->lastParams = params;
    state->data.clear();
    state->data.reserve(params.n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(params.minVal, params.maxVal);

    for (int i = 0; i < params.n; ++i) {
        state->data.push_back(dist(gen));
    }
}

void DrawVectorGrid(HDC hdc, HWND hWnd, const Object2State* state) {
    RECT rc;
    GetClientRect(hWnd, &rc);

    HFONT hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hFont));
    SetBkMode(hdc, TRANSPARENT);

    if (state->data.empty()) {
        const wchar_t* waitMsg = L"Очікування параметрів від програми Lab6...";
        TextOutW(hdc, 14, 14, waitMsg, lstrlenW(waitMsg));
        SelectObject(hdc, hOldFont);
        return;
    }

    std::wostringstream header;
    header.imbue(std::locale::classic());
    header << std::fixed << std::setprecision(2)
           << L"Вектор double (n = " << state->data.size()
           << L", діапазон [" << state->lastParams.minVal
           << L"; " << state->lastParams.maxVal << L"]) скопійовано у Clipboard:";
    std::wstring headerStr = header.str();
    TextOutW(hdc, 12, 10, headerStr.c_str(), static_cast<int>(headerStr.size()));

    int startX = 12;
    int startY = 36;
    int colWidth = 96;
    int rowHeight = 22;

    int availW = std::max(100, static_cast<int>(rc.right - startX * 2));
    int cols = std::max(1, availW / colWidth);

    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hGridPen));

    for (size_t i = 0; i < state->data.size(); ++i) {
        int row = static_cast<int>(i) / cols;
        int col = static_cast<int>(i) % cols;

        int x = startX + col * colWidth;
        int y = startY + row * rowHeight;

        if (y > rc.bottom) break;

        RECT cellRect = { x, y, x + colWidth, y + rowHeight };
        Rectangle(hdc, cellRect.left, cellRect.top, cellRect.right, cellRect.bottom);

        std::wostringstream cellText;
        cellText.imbue(std::locale::classic());
        cellText << L"[" << i << L"] " << std::fixed << std::setprecision(2) << state->data[i];
        std::wstring s = cellText.str();

        RECT textRect = { x + 4, y + 3, x + colWidth - 4, y + rowHeight - 2 };
        DrawTextW(hdc, s.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hGridPen);
    SelectObject(hdc, hOldFont);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Object2State* state = nullptr;

    if (message == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        state = reinterpret_cast<Object2State*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        state->hWnd = hWnd;
    } else {
        state = reinterpret_cast<Object2State*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    switch (message) {
    case WM_CREATE:
        if (state) {
            HWND hLab6 = FindWindowW(LAB6_CLASS_NAME, NULL);
            if (hLab6) {
                PostMessageW(hLab6, state->msgObj2Ready, 0, 0);
            }
        }
        break;

    case WM_COPYDATA:
        if (state) {
            auto* pcds = reinterpret_cast<PCOPYDATASTRUCT>(lParam);
            if (pcds && pcds->dwData == 1 && pcds->cbData == sizeof(VectorParams)) {
                const auto* params = reinterpret_cast<const VectorParams*>(pcds->lpData);
                GenerateVector(state, *params);
                WriteVectorToClipboard(hWnd, state->data);
                InvalidateRect(hWnd, NULL, TRUE);

                HWND hLab6 = reinterpret_cast<HWND>(wParam);
                if (!hLab6 || !IsWindow(hLab6)) {
                    hLab6 = FindWindowW(LAB6_CLASS_NAME, NULL);
                }
                if (hLab6) {
                    PostMessageW(hLab6, state->msgObj2Done, 0, 0);
                }
                return 1;
            }
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (state) {
            DrawVectorGrid(hdc, hWnd, state);
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
    Object2State state;
    state.msgObj2Ready = RegisterWindowMessageW(L"LAB6_OBJ2_READY");
    state.msgObj2Done  = RegisterWindowMessageW(L"LAB6_OBJ2_DONE");

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = OBJ2_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowW(
        OBJ2_CLASS_NAME, L"Object2-Генератор вектора(вар3)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 540, 420, NULL, NULL, hInstance, &state
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