#include "module1.h"
#include "resource.h"
static wchar_t* g_pBuffer = nullptr;
static size_t g_maxLen = 0;
//захована callback функція діалогу
static INT_PTR CALLBACK DialogProc1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG: {
        HWND hList = GetDlgItem(hDlg, IDC_LIST_GROUPS);
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"ІМ-55");
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"ІП-51");
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"ІО-52");
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"ІК-53");
        SendMessageW(hList, LB_SETCURSEL, 0, 0);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            HWND hList = GetDlgItem(hDlg, IDC_LIST_GROUPS);
            int sel = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR && g_pBuffer) {
                SendMessageW(hList, LB_GETTEXT, sel, (LPARAM)g_pBuffer);
                EndDialog(hDlg, IDOK);
            } else {
                EndDialog(hDlg, IDCANCEL);
            }
            return (INT_PTR)TRUE;
        } else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
bool ShowModule1Dialog(HINSTANCE hInstance, HWND hWndParent, wchar_t* outResult, size_t maxLen) {
    g_pBuffer = outResult;
    g_maxLen = maxLen;
    INT_PTR res = DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_DIALOG1), hWndParent, DialogProc1);
    return (res == IDOK);
}