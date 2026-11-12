#pragma once
#include <windows.h>
#include "shape.h"

class EditorWindow {
private:
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    int m_currentShape;
    bool m_isDrawing;
    long m_xStart, m_yStart, m_xEnd, m_yEnd;
    Shape** m_pcshape;
    int m_shapeCount;
    const int m_maxShapes = 112;

    void DrawRubberBand(long x1, long y1, long x2, long y2, int shapeType);

public:
    EditorWindow(HINSTANCE hInstance);
    ~EditorWindow();

    bool Create(PCWSTR lpWindowName, DWORD dwStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int nWidth = 800, int nHeight = 600);
    HWND Window() const { return m_hWnd; }

    //статична процедура вікна для перенаправлення подій у клас
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    //методи-обробники подій
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnCommand(WPARAM wParam, LPARAM lParam);
    void OnLButtonDown(LPARAM lParam);
    void OnMouseMove(LPARAM lParam);
    void OnLButtonUp(LPARAM lParam);
    void OnPaint(HDC hdc);
    void OnInitMenuPopup(HMENU hMenu);
};