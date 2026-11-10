#include "module2.h"
#include "resource.h"
static wchar_t* g_pBuffer = nullptr;
static size_t g_maxLen = 0;
//захована callback функція діалогу
static INT_PTR CALLBACK DialogProc2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            if (g_pBuffer) {
                GetDlgItemTextW(hDlg, IDC_EDIT_INPUT, g_pBuffer, (int)g_maxLen);
            }
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        } else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
bool ShowModule2Dialog(HINSTANCE hInstance, HWND hWndParent, wchar_t* outResult, size_t maxLen) {
    g_pBuffer = outResult;
    g_maxLen = maxLen;
    INT_PTR res = DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_DIALOG2), hWndParent, DialogProc2);
    return (res == IDOK);
}