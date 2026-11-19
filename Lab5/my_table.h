#pragma once

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

struct TableRowData {
    std::wstring name;
    long x1, y1, x2, y2;
};

class MyTable {
private:
    HWND hDlg;
    HWND hParent;
    HWND hList;

    void InitListView();
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
    MyTable();
    ~MyTable();

    void Show(HINSTANCE hInstance, HWND parent);
    void Close();
    bool IsOpen() const { return hDlg != NULL && IsWindow(hDlg); }
    void UpdateData(const std::vector<TableRowData>& data);

    HWND GetDialogHandle() const { return hDlg; }
};