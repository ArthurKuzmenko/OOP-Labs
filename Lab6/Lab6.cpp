#include <windows.h>
#include <string>
#include <sstream>
#include <locale>
#include "resource.h"

constexpr wchar_t LAB6_CLASS_NAME[] = L"Lab6ManagerClass";
constexpr wchar_t OBJ2_CLASS_NAME[] = L"Object2VectorClass";
constexpr wchar_t OBJ3_CLASS_NAME[] = L"Object3GraphClass";

struct VectorParams {
    int n;
    double minVal;
    double maxVal;
};

struct ManagerState {
    HINSTANCE hInst = NULL;
    HWND hWnd = NULL;
    VectorParams params{ 12, -10.0, 25.0 };
    std::wstring status = L"Натисни «Робота->Ввести параметри...» для запуску.";

    UINT msgObj2Ready = 0;
    UINT msgObj2Done = 0;
    UINT msgObj3Ready = 0;
    UINT msgObj3Draw = 0;
};

std::wstring GetModuleDir() {
    wchar_t buf[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    std::wstring path(buf);
    size_t pos = path.find_last_of(L"\\/");
    return (pos != std::wstring::npos) ? path.substr(0, pos + 1) : L"";
}

bool LaunchChildApp(const wchar_t* exeName) {
    std::wstring fullPath = GetModuleDir() + exeName;
    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };

    if (CreateProcessW(fullPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

void SendParamsToObject2(HWND hLab6, const VectorParams& params) {
    HWND hObj2 = FindWindowW(OBJ2_CLASS_NAME, NULL);
    if (!hObj2) return;

    if (IsIconic(hObj2)) ShowWindow(hObj2, SW_RESTORE);

    COPYDATASTRUCT cds = { 0 };
    cds.dwData = 1;
    cds.cbData = sizeof(VectorParams);
    cds.lpData = const_cast<VectorParams*>(&params);

    SendMessageW(hObj2, WM_COPYDATA, (WPARAM)hLab6, (LPARAM)&cds);
}

void TileAllWindows(HWND hLab6) {
    RECT wa = { 0 };
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);

    int totalW = wa.right - wa.left;
    int totalH = wa.bottom - wa.top;
    int topH = 180;
    int halfW = totalW / 2;

    SetWindowPos(hLab6, NULL, wa.left, wa.top, halfW, topH, SWP_NOZORDER | SWP_NOACTIVATE);

    HWND hObj2 = FindWindowW(OBJ2_CLASS_NAME, NULL);
    if (hObj2) {
        if (IsIconic(hObj2)) ShowWindow(hObj2, SW_RESTORE);
        SetWindowPos(hObj2, NULL, wa.left, wa.top + topH, halfW, totalH - topH, SWP_NOZORDER | SWP_SHOWWINDOW);
    }

    HWND hObj3 = FindWindowW(OBJ3_CLASS_NAME, NULL);
    if (hObj3) {
        if (IsIconic(hObj3)) ShowWindow(hObj3, SW_RESTORE);
        SetWindowPos(hObj3, NULL, wa.left + halfW, wa.top + topH, totalW - halfW, totalH - topH, SWP_NOZORDER | SWP_SHOWWINDOW);
    }

    SetForegroundWindow(hLab6);
}

INT_PTR CALLBACK ParamsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    ManagerState* state = nullptr;

    if (message == WM_INITDIALOG) {
        state = reinterpret_cast<ManagerState*>(lParam);
        SetWindowLongPtrW(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(state));

        SetDlgItemInt(hDlg, IDC_EDIT_N, static_cast<UINT>(state->params.n), TRUE);

        std::wostringstream sMin, sMax;
        sMin.imbue(std::locale::classic());
        sMax.imbue(std::locale::classic());
        sMin << state->params.minVal;
        sMax << state->params.maxVal;

        SetDlgItemTextW(hDlg, IDC_EDIT_MIN, sMin.str().c_str());
        SetDlgItemTextW(hDlg, IDC_EDIT_MAX, sMax.str().c_str());
        return TRUE;
    }

    state = reinterpret_cast<ManagerState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
    if (!state) return FALSE;

    if (message == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK) {
            BOOL okN = FALSE;
            int n = static_cast<int>(GetDlgItemInt(hDlg, IDC_EDIT_N, &okN, TRUE));

            wchar_t minBuf[64] = { 0 }, maxBuf[64] = { 0 };
            GetDlgItemTextW(hDlg, IDC_EDIT_MIN, minBuf, 64);
            GetDlgItemTextW(hDlg, IDC_EDIT_MAX, maxBuf, 64);

            for (wchar_t* p = minBuf; *p; ++p) if (*p == L',') *p = L'.';
            for (wchar_t* p = maxBuf; *p; ++p) if (*p == L',') *p = L'.';

            std::wistringstream issMin(minBuf), issMax(maxBuf);
            issMin.imbue(std::locale::classic());
            issMax.imbue(std::locale::classic());

            double minVal = 0.0, maxVal = 0.0;
            bool okMin = (issMin >> minVal) && (issMin >> std::ws).eof();
            bool okMax = (issMax >> maxVal) && (issMax >> std::ws).eof();

            if (!okN || n < 2 || n > 500) {
                MessageBoxW(hDlg, L"Введи ціле n від 2 до 500.", L"Помилка вводу", MB_OK | MB_ICONWARNING);
                return TRUE;
            }
            if (!okMin || !okMax || minVal >= maxVal) {
                MessageBoxW(hDlg, L"Перевір діапазон: Min<Max.", L"Помилка вводу", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            state->params = { n, minVal, maxVal };
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    ManagerState* state = nullptr;

    if (message == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        state = reinterpret_cast<ManagerState*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        state->hWnd = hWnd;
    } else {
        state = reinterpret_cast<ManagerState*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (state) {
        if (message == state->msgObj2Ready) {
            SendParamsToObject2(hWnd, state->params);
            return 0;
        }
        if (message == state->msgObj2Done) {
            HWND hObj3 = FindWindowW(OBJ3_CLASS_NAME, NULL);
            if (!hObj3) {
                if (!LaunchChildApp(L"Object3.exe")) {
                    MessageBoxW(hWnd, L"Не вдалось запустити Object3.exe.", L"Помилка", MB_OK | MB_ICONERROR);
                }
            } else {
                TileAllWindows(hWnd);
                PostMessageW(hObj3, state->msgObj3Draw, 0, 0);
            }
            return 0;
        }
        if (message == state->msgObj3Ready) {
            HWND hObj3 = FindWindowW(OBJ3_CLASS_NAME, NULL);
            if (hObj3) {
                TileAllWindows(hWnd);
                PostMessageW(hObj3, state->msgObj3Draw, 0, 0);
            }
            return 0;
        }
    }

    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDM_WORK_PARAMS && state) {
            if (DialogBoxParamW(state->hInst, MAKEINTRESOURCEW(IDD_PARAMS_DIALOG),
                                hWnd, ParamsDlgProc, reinterpret_cast<LPARAM>(state)) == IDOK) {
                std::wostringstream wss;
                wss.imbue(std::locale::classic());
                wss << L"Параметри: n = " << state->params.n
                    << L", Min = " << state->params.minVal
                    << L", Max = " << state->params.maxVal;
                state->status = wss.str();
                InvalidateRect(hWnd, NULL, TRUE);

                HWND hObj2 = FindWindowW(OBJ2_CLASS_NAME, NULL);
                if (hObj2) {
                    SendParamsToObject2(hWnd, state->params);
                } else if (!LaunchChildApp(L"Object2.exe")) {
                    MessageBoxW(hWnd, L"Не вдалось запустити Object2.exe.", L"Помилка", MB_OK | MB_ICONERROR);
                }
            }
        } else if (LOWORD(wParam) == IDM_FILE_EXIT) {
            DestroyWindow(hWnd);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (state) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            rc.left += 14;
            rc.top += 14;
            HFONT hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            HFONT hOld = static_cast<HFONT>(SelectObject(hdc, hFont));
            SetBkMode(hdc, TRANSPARENT);
            DrawTextW(hdc, state->status.c_str(), -1, &rc, DT_LEFT | DT_WORDBREAK);
            SelectObject(hdc, hOld);
        }
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY: {
        HWND hObj2 = FindWindowW(OBJ2_CLASS_NAME, NULL);
        HWND hObj3 = FindWindowW(OBJ3_CLASS_NAME, NULL);
        if (hObj2) PostMessageW(hObj2, WM_CLOSE, 0, 0);
        if (hObj3) PostMessageW(hObj3, WM_CLOSE, 0, 0);
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    ManagerState state;
    state.hInst = hInstance;
    state.msgObj2Ready = RegisterWindowMessageW(L"LAB6_OBJ2_READY");
    state.msgObj2Done  = RegisterWindowMessageW(L"LAB6_OBJ2_DONE");
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
    wc.lpszMenuName = MAKEINTRESOURCEW(IDR_LAB6_MENU);
    wc.lpszClassName = LAB6_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowW(
        LAB6_CLASS_NAME, L"Lab6-Менеджер(вар3)", WS_OVERLAPPEDWINDOW,
        40, 40, 480, 180, NULL, NULL, hInstance, &state
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