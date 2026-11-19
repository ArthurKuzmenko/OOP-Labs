#include "shape.h"
#include <cmath>

void Shape::SetCoord(long x1, long y1, long x2, long y2) {
    xs1 = x1;
    ys1 = y1;
    xs2 = x2;
    ys2 = y2;
}

//крапка
void PointShape::Show(HDC hdc) {
    if (m_isSelected) {
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, xs1 - 4, ys1 - 4, xs1 + 4, ys1 + 4);
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
    SetPixel(hdc, xs1, ys1, GetStrokeColor());
}

void PointShape::DrawRubberBand(HDC hdc) {
    MoveToEx(hdc, xs1, ys1, NULL);
    LineTo(hdc, xs1 + 1, ys1 + 1);
}

//лінія
void LineShape::Show(HDC hdc) {
    HPEN hPen = CreatePen(PS_SOLID, GetStrokeWidth(), GetStrokeColor());
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    MoveToEx(hdc, xs1, ys1, NULL);
    LineTo(hdc, xs2, ys2);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void LineShape::DrawRubberBand(HDC hdc) {
    MoveToEx(hdc, xs1, ys1, NULL);
    LineTo(hdc, xs2, ys2);
}

//прямокутник(ввід по 2 протилежних кутах, жовте заповнення)
void RectShape::Show(HDC hdc) {
    HPEN hPen = CreatePen(PS_SOLID, GetStrokeWidth(), GetStrokeColor());
    HBRUSH hBrush = m_isFilled
        ? CreateSolidBrush(RGB(255, 255, 0))
        : (HBRUSH)GetStockObject(NULL_BRUSH);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    Rectangle(hdc, xs1, ys1, xs2, ys2);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
    if (m_isFilled) {
        DeleteObject(hBrush);
    }
}

void RectShape::DrawRubberBand(HDC hdc) {
    Rectangle(hdc, xs1, ys1, xs2, ys2);
}

//еліпс(ввід від центру до кута, без заповнення)
void EllipseShape::Show(HDC hdc) {
    HPEN hPen = CreatePen(PS_SOLID, GetStrokeWidth(), GetStrokeColor());
    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    long dx = std::abs(xs2 - xs1);
    long dy = std::abs(ys2 - ys1);
    Ellipse(hdc, xs1 - dx, ys1 - dy, xs1 + dx, ys1 + dy);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
}

void EllipseShape::DrawRubberBand(HDC hdc) {
    long dx = std::abs(xs2 - xs1);
    long dy = std::abs(ys2 - ys1);
    Ellipse(hdc, xs1 - dx, ys1 - dy, xs1 + dx, ys1 + dy);
}

//лінія з кружечками(використовує Show із LineShape та EllipseShape)
void LineWithCirclesShape::Show(HDC hdc) {
    long origX1 = xs1, origY1 = ys1, origX2 = xs2, origY2 = ys2;

    LineShape::Show(hdc);

    SetCoord(origX1, origY1, origX1 + RADIUS, origY1 + RADIUS);
    EllipseShape::Show(hdc);

    SetCoord(origX2, origY2, origX2 + RADIUS, origY2 + RADIUS);
    EllipseShape::Show(hdc);

    SetCoord(origX1, origY1, origX2, origY2);
}

void LineWithCirclesShape::DrawRubberBand(HDC hdc) {
    long origX1 = xs1, origY1 = ys1, origX2 = xs2, origY2 = ys2;

    LineShape::DrawRubberBand(hdc);

    SetCoord(origX1, origY1, origX1 + RADIUS, origY1 + RADIUS);
    EllipseShape::DrawRubberBand(hdc);

    SetCoord(origX2, origY2, origX2 + RADIUS, origY2 + RADIUS);
    EllipseShape::DrawRubberBand(hdc);

    SetCoord(origX1, origY1, origX2, origY2);
}

//каркас куба(використовує Show із RectShape та LineShape)
void CubeShape::Show(HDC hdc) {
    long origX1 = xs1, origY1 = ys1, origX2 = xs2, origY2 = ys2;
    long shiftX = (origX2 - origX1) / 3;
    long shiftY = (origY2 - origY1) / 3;

    //задня грань
    SetCoord(origX1 + shiftX, origY1 - shiftY, origX2 + shiftX, origY2 - shiftY);
    RectShape::Show(hdc);

    //передня грань
    SetCoord(origX1, origY1, origX2, origY2);
    RectShape::Show(hdc);

    //чотири ребра між гранями
    SetCoord(origX1, origY1, origX1 + shiftX, origY1 - shiftY);
    LineShape::Show(hdc);

    SetCoord(origX2, origY1, origX2 + shiftX, origY1 - shiftY);
    LineShape::Show(hdc);

    SetCoord(origX1, origY2, origX1 + shiftX, origY2 - shiftY);
    LineShape::Show(hdc);

    SetCoord(origX2, origY2, origX2 + shiftX, origY2 - shiftY);
    LineShape::Show(hdc);

    SetCoord(origX1, origY1, origX2, origY2);
}

void CubeShape::DrawRubberBand(HDC hdc) {
    long origX1 = xs1, origY1 = ys1, origX2 = xs2, origY2 = ys2;
    long shiftX = (origX2 - origX1) / 3;
    long shiftY = (origY2 - origY1) / 3;

    SetCoord(origX1 + shiftX, origY1 - shiftY, origX2 + shiftX, origY2 - shiftY);
    RectShape::DrawRubberBand(hdc);

    SetCoord(origX1, origY1, origX2, origY2);
    RectShape::DrawRubberBand(hdc);

    SetCoord(origX1, origY1, origX1 + shiftX, origY1 - shiftY);
    LineShape::DrawRubberBand(hdc);

    SetCoord(origX2, origY1, origX2 + shiftX, origY1 - shiftY);
    LineShape::DrawRubberBand(hdc);

    SetCoord(origX1, origY2, origX1 + shiftX, origY2 - shiftY);
    LineShape::DrawRubberBand(hdc);

    SetCoord(origX2, origY2, origX2 + shiftX, origY2 - shiftY);
    LineShape::DrawRubberBand(hdc);

    SetCoord(origX1, origY1, origX2, origY2);
}