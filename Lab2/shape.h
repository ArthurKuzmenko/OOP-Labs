#pragma once
#include <windows.h>

class Shape {
protected:
    long xs1, ys1, xs2, ys2;
public:
    virtual ~Shape() {}
    void SetCoord(long x1, long y1, long x2, long y2);
    virtual void Show(HDC hdc) = 0;
};
class PointShape : public Shape {
public:
    void Show(HDC hdc) override;
};
class LineShape : public Shape {
public:
    void Show(HDC hdc) override;
};
class RectangleShape : public Shape {
public:
    void Show(HDC hdc) override;
};
class EllipseShape : public Shape {
public:
    void Show(HDC hdc) override;
};