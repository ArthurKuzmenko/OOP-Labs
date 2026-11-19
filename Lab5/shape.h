#pragma once
#include <windows.h>

class Shape {
protected:
    long xs1, ys1, xs2, ys2;
public:
    Shape() : xs1(0), ys1(0), xs2(0), ys2(0) {}
    virtual ~Shape() {}

    void SetCoord(long x1, long y1, long x2, long y2);
    long GetX1() const { return xs1; }
    long GetY1() const { return ys1; }
    long GetX2() const { return xs2; }
    long GetY2() const { return ys2; }

    virtual const wchar_t* GetName() const = 0;
    virtual const char* GetTypeTag() const = 0;

    virtual void Show(HDC hdc) = 0;
    virtual void DrawRubberBand(HDC hdc) = 0;
};

class PointShape : virtual public Shape {
public:
    const wchar_t* GetName() const override { return L"Крапка"; }
    const char* GetTypeTag() const override { return "POINT"; }
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

class LineShape : virtual public Shape {
public:
    const wchar_t* GetName() const override { return L"Лінія"; }
    const char* GetTypeTag() const override { return "LINE"; }
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

class RectShape : virtual public Shape {
protected:
    bool m_isFilled;
public:
    RectShape() : m_isFilled(true) {}
    const wchar_t* GetName() const override { return L"Прямокутник"; }
    const char* GetTypeTag() const override { return "RECT"; }
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

class EllipseShape : virtual public Shape {
public:
    const wchar_t* GetName() const override { return L"Еліпс"; }
    const char* GetTypeTag() const override { return "ELLIPSE"; }
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

//лінія з кружечками(множинне успадкування від LineShape та EllipseShape)
class LineWithCirclesShape : public LineShape, public EllipseShape {
private:
    static const long RADIUS = 6;
public:
    const wchar_t* GetName() const override { return L"Лінія з кружечками"; }
    const char* GetTypeTag() const override { return "LINEOO"; }
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};

//каркас куба(множинне успадкування від RectShape та LineShape)
class CubeShape : public RectShape, public LineShape {
public:
    CubeShape() { m_isFilled = false; }
    const wchar_t* GetName() const override { return L"Каркас куба"; }
    const char* GetTypeTag() const override { return "CUBE"; }
    void Show(HDC hdc) override;
    void DrawRubberBand(HDC hdc) override;
};