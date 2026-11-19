#pragma once
#include <windows.h>
#include <commctrl.h>
#include "shape.h"
#include "my_table.h"

class MyEditor : public ITableListener {
private:
    HWND m_hWnd;
    HWND m_hToolBar;
    HINSTANCE m_hInstance;
    int m_currentShape;
    bool m_isDrawing;
    long m_xStart, m_yStart, m_xEnd, m_yEnd;
    Shape** m_pcshape;
    Shape* m_pDraftShape;
    int m_shapeCount;
    int m_selectedIndex;
    static const int m_maxShapes = 111;

    MyTable m_table;

    //Singleton Меєрса(вар11)
    MyEditor();
    ~MyEditor();
    MyEditor(const MyEditor&) = delete;
    MyEditor& operator=(const MyEditor&) = delete;

    void CreateToolbar();
    void SelectShape(int shapeId);
    Shape* CreateSelectedShape();
    void DrawRubberBand();
    void SyncTable();
    void SaveToFile();

public:
    static MyEditor& getInstance() {
        static MyEditor instance;
        return instance;
    }

    void Init(HINSTANCE hInstance);
    bool Create(PCWSTR lpWindowName, DWORD dwStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int nWidth = 800, int nHeight = 600);
    HWND Window() const { return m_hWnd; }
    bool IsDialogMsg(MSG* pMsg);

    //реалізація ITableListener(Observer/Listener)
    void OnTableRowSelected(int rowIndex) override;
    void OnTableRowDeleted(int rowIndex) override;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnCreate();
    void OnSize();
    void OnNotify(LPARAM lParam);
    void OnCommand(WPARAM wParam, LPARAM lParam);
    void OnLButtonDown(LPARAM lParam);
    void OnMouseMove(LPARAM lParam);
    void OnLButtonUp(LPARAM lParam);
    void OnPaint(HDC hdc);
    void OnInitMenuPopup(HMENU hMenu);
};