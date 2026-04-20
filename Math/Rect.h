#ifndef RECT_H
#define RECT_H

#include "Point.h"
#include <ostream>

class Rect
{
    public:
    Rect() : x(0), y(0), w(0), h(0) {}

    Rect(const int x_, const int y_, const int width_, const int height_)
        : x(x_), y(y_), w(std::max<int>(0, width_)), h(std::max<int>(0, height_))
    {
    }

    Rect(const Rect &r) = default;

    Rect(Rect &&r) noexcept : Rect() { swap(*this, r); }

    Rect &operator=(Rect r)
    {
        swap(*this, r);
        return *this;
    }

    bool operator==(const Rect &r) const
    { return (x == r.x) && (y == r.y) && (w == r.w) && (h == r.h); }

    bool operator!=(const Rect &r) const { return !(*this == r); }

    [[nodiscard]] constexpr bool Isnullptr() const { return ((w == 0) || (h == 0)); }

    void Set(const int px, const int py, int w, int h)
    {
        x       = px;
        y       = py;
        this->w = w;
        this->h = h;
    }

    void SetPosition(int px, int py)
    {
        x = px;
        y = py;
    }

    void SetSize(int w, int h)
    {
        this->w = w;
        this->h = h;
    }

    [[nodiscard]] bool IsInside(const Point2D<float> &p) const
    { return ((x <= p.x) && (x + w > p.x) && (y <= p.y) && (y + h > p.y)); }

    [[nodiscard]] bool IsPointInside(const int x_, const int y_) const
    { return ((x <= x_) && (x + w > x_) && (y <= y_) && (y + h > y_)); }

    [[nodiscard]] bool IsIntersecting(const Rect &r) const
    { return !((r.x > x + w) || (r.x + r.w < x) || (r.y > y + h) || (r.y + r.h < y)); }

    [[nodiscard]] Rect Intersect(const Rect &r) const
    {
        if (IsIntersecting(r)) {
            const Point2D<float> &ul1 = Point2D<float>(x, y);
            const Point2D<float> &ul2 = Point2D<float>(r.x, r.y);
            int xx                    = std::max<int>(ul1.x, ul2.x);
            int yy                    = std::max<int>(ul1.y, ul2.y);
            int ww                    = std::min<int>(ul1.x + w, ul2.x + r.w) - xx;
            int hh                    = std::min<int>(ul1.y + h, ul2.y + r.h) - yy;

            return {xx, yy, ww, hh};
        }

        return {};
    }

    void Expand(int dx, int dy)
    {
        if (!Isnullptr()) {
            x -= dx;
            y -= dy;
            w += 2 * dx;
            h += 2 * dy;
        }
    }

    [[nodiscard]] Rect GetExpand(int dx, int dy) const
    {
        Rect r(x - dx, y - dy, w + 2 * dx, h + 2 * dy);

        if (r.Isnullptr()) {
            return {0, 0, 0, 0};
        }

        return r;
    }

    Rect operator+(Rect const &rhs) const
    { return {this->x + rhs.x, this->y + rhs.y, this->w + rhs.w, this->h + rhs.h}; }

    Rect operator-(Rect const &rhs) const
    { return {this->x - rhs.x, this->y - rhs.y, this->w - rhs.w, this->h - rhs.h}; }

    Rect operator*(const float scalar) const
    {
        return {static_cast<int>(this->x * scalar), static_cast<int>(this->y * scalar),
                static_cast<int>(this->w * scalar), static_cast<int>(this->h * scalar)};
    }

    std::ostream &operator<<(std::ostream &os) const
    {
        return os << "Rect, x: " << this->x << " y: " << this->y << " width: " << this->w
                  << " height: " << this->h;
    }

    [[nodiscard]] int GetWidth() const { return w; }
    [[nodiscard]] int GetHeight() const { return h; }

    [[nodiscard]] Point2D<float> GetCenter() const { return Point2D<float>(x + w / 2, y + h / 2); }

    [[nodiscard]] Point2D<float> GetPosition() const { return Point2D<float>(x, y); }

    void SetWidth(int w) { w = w; }
    void SetHeight(int h) { h = h; }
    void SetX(int px) { x = px; }
    void SetY(int py) { y = py; }

    void OffsetSize(int dw, int dh)
    {
        w += dw;
        h += dh;

        if (w < 0)
            w = 0;

        if (h < 0)
            h = 0;
    }

    void OffsetPosition(int dx, int dy)
    {
        x += dx;
        y += dy;
    }

    friend void swap(Rect &lhs, Rect &rhs) noexcept
    {
        using std::swap;
        swap(lhs.x, rhs.x);
        swap(lhs.y, rhs.y);
        swap(lhs.w, rhs.w);
        swap(lhs.h, rhs.h);
    }

    int x, y;
    int w, h;
};

#endif
