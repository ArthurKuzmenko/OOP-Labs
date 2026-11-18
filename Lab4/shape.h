#pragma once
#include <windows.h>

class Shape {
protected:
    long xs1, ys1, xs2, ys2;
public:
    virtual ~Shape() {}
    void SetCoord(long x1, long y1, long x2, long y2);
    virtual void Show(HDC hdc) = 0;
    virtual void DrawRubberBand(HDC hdc) = 0;
};

class PointShape : virtual public Shape {
public:
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

class LineShape : virtual public Shape {
public:
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

class RectShape : virtual public Shape {
protected:
    bool m_isFilled;
public:
    RectShape() : m_isFilled(true) {}
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

class EllipseShape : virtual public Shape {
public:
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

//нова фігура 1:лінія з кружечками(множинне успадкування від LineShape та EllipseShape)
class LineWithCirclesShape : public LineShape, public EllipseShape {
private:
    static const long RADIUS = 6;
public:
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

//нова фігура 2:каркас куба(множинне успадкування від RectShape та LineShape)
class CubeShape : public RectShape, public LineShape {
public:
    CubeShape() { m_isFilled = false; }
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};