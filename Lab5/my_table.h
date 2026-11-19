#pragma once

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

struct TableRowData {
    std::wstring name;
    long x1, y1, x2, y2;
};

//інтерфейс спостерігача(Observer/Listener) без залежностей від інших модулів
class ITableListener {
public:
    virtual ~ITableListener() {}
    virtual void OnTableRowSelected(int rowIndex) = 0;
    virtual void OnTableRowDeleted(int rowIndex) = 0;
};

class MyTable {
private:
    HWND hDlg;
    HWND hParent;
    HWND hList;
    ITableListener* m_pListener;
    bool m_isUpdating;

    void InitListView();
    void DeleteSelectedRow();
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
    MyTable();
    ~MyTable();

    void SetListener(ITableListener* listener) { m_pListener = listener; }

    void Show(HINSTANCE hInstance, HWND parent);
    void Close();
    bool IsOpen() const { return hDlg != NULL && IsWindow(hDlg); }
    void UpdateData(const std::vector<TableRowData>& data);

    HWND GetDialogHandle() const { return hDlg; }
};