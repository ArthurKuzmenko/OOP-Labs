#pragma once
#include <windows.h>
#include <commctrl.h>
#include "shape.h"

class MyEditor {
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
    static const int m_maxShapes = 111; //вар11

    Shape* CreateSelectedShape();
    void DrawRubberBand();

public:
    MyEditor();
    ~MyEditor();

    void Init(HINSTANCE hInstance);
    bool Create(PCWSTR lpWindowName, DWORD dwStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int nWidth = 800, int nHeight = 600);
    HWND Window() const { return m_hWnd; }

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