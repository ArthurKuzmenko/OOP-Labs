#include "my_table.h"
#include "resource.h"

MyTable::MyTable()
    : hDlg(NULL), hParent(NULL), hList(NULL), m_pListener(nullptr), m_isUpdating(false) {}

MyTable::~MyTable() {
    Close();
}

void MyTable::InitListView() {
    if (!hList) return;

    SendMessageW(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                 LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    const wchar_t* headers[] = { L"#", L"Фігура", L"x1", L"y1", L"x2", L"y2" };
    int widths[] = { 40, 155, 60, 60, 60, 60 };

    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    for (int i = 0; i < 6; i++) {
        lvc.iSubItem = i;
        lvc.cx = widths[i];
        lvc.pszText = const_cast<LPWSTR>(headers[i]);
        SendMessageW(hList, LVM_INSERTCOLUMNW, i, (LPARAM)&lvc);
    }
}

void MyTable::Show(HINSTANCE hInstance, HWND parent) {
    hParent = parent;
    if (IsOpen()) {
        ShowWindow(hDlg, SW_SHOW);
        SetForegroundWindow(hDlg);
        return;
    }

    INITCOMMONCONTROLSEX icex = { 0 };
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    hDlg = CreateDialogParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_TABLE_DIALOG),
        parent,
        MyTable::DlgProc,
        (LPARAM)this
    );

    if (hDlg) {
        ShowWindow(hDlg, SW_SHOW);
    }
}

void MyTable::Close() {
    if (m_pListener) {
        m_pListener->OnTableRowSelected(-1);
    }
    if (hDlg && IsWindow(hDlg)) {
        DestroyWindow(hDlg);
    }
    hDlg = NULL;
    hList = NULL;
}

void MyTable::UpdateData(const std::vector<TableRowData>& data) {
    if (!IsOpen() || !hList) return;

    m_isUpdating = true;
    SendMessageW(hList, LVM_DELETEALLITEMS, 0, 0);

    for (size_t i = 0; i < data.size(); i++) {
        std::wstring numStr = std::to_wstring(i + 1);
        std::wstring x1Str  = std::to_wstring(data[i].x1);
        std::wstring y1Str  = std::to_wstring(data[i].y1);
        std::wstring x2Str  = std::to_wstring(data[i].x2);
        std::wstring y2Str  = std::to_wstring(data[i].y2);

        LVITEMW lvi = { 0 };
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.iSubItem = 0;
        lvi.pszText = const_cast<LPWSTR>(numStr.c_str());
        SendMessageW(hList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

        auto setSubItem = [&](int subItem, const std::wstring& text) {
            LVITEMW sub = { 0 };
            sub.iSubItem = subItem;
            sub.pszText = const_cast<LPWSTR>(text.c_str());
            SendMessageW(hList, LVM_SETITEMTEXTW, (WPARAM)i, (LPARAM)&sub);
        };

        setSubItem(1, data[i].name);
        setSubItem(2, x1Str);
        setSubItem(3, y1Str);
        setSubItem(4, x2Str);
        setSubItem(5, y2Str);
    }

    m_isUpdating = false;
}

INT_PTR CALLBACK MyTable::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    MyTable* pThis = nullptr;

    if (message == WM_INITDIALOG) {
        pThis = (MyTable*)lParam;
        SetWindowLongPtrW(hDlg, DWLP_USER, (LONG_PTR)pThis);
        pThis->hDlg = hDlg;
        pThis->hList = GetDlgItem(hDlg, IDC_TABLE_LIST);
        pThis->InitListView();
        return TRUE;
    }

    pThis = (MyTable*)GetWindowLongPtrW(hDlg, DWLP_USER);
    if (!pThis) return FALSE;

    switch (message) {
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if (pThis->hList && width > 14 && height > 14) {
            MoveWindow(pThis->hList, 7, 7, width - 14, height - 14, TRUE);
        }
        return TRUE;
    }
    case WM_NOTIFY: {
        LPNMHDR lpnm = (LPNMHDR)lParam;
        if (lpnm->idFrom == IDC_TABLE_LIST && lpnm->code == LVN_ITEMCHANGED && !pThis->m_isUpdating) {
            LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
            if (pnmv->uChanged & LVIF_STATE) {
                if (pnmv->uNewState & LVIS_SELECTED) {
                    if (pThis->m_pListener) {
                        pThis->m_pListener->OnTableRowSelected(pnmv->iItem);
                    }
                } else if ((pnmv->uOldState & LVIS_SELECTED) && !(pnmv->uNewState & LVIS_SELECTED)) {
                    int sel = (int)SendMessageW(pThis->hList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
                    if (sel == -1 && pThis->m_pListener) {
                        pThis->m_pListener->OnTableRowSelected(-1);
                    }
                }
            }
        }
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) {
            pThis->Close();
            return TRUE;
        }
        break;
    case WM_CLOSE:
        pThis->Close();
        return TRUE;
    case WM_DESTROY:
        pThis->hDlg = NULL;
        pThis->hList = NULL;
        return TRUE;
    }

    return FALSE;
}