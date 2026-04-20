#ifndef POINT2D_INL_H
#define POINT2D_INL_H

template <typename T> class Point2D
{
    public:
    T x, y;

    Point2D() : x(0), y(0) {}

    Point2D(T const &X, T const &Y) : x(X), y(Y) {}

    bool operator==(const Point2D<T> &rhs) { return (this->x == rhs.x) && (this->y == rhs.y); }

    bool operator!=(const Point2D<T> &rhs) { return !((*this) == rhs); }

    Point2D<T> operator-(const Point2D<T> &rhs)
    { return Point2D<T>(this->x - rhs.x, this->y - rhs.y); }

    Point2D<T> operator+(const Point2D<T> &rhs)
    { return Point2D<T>(this->x + rhs.x, this->y + rhs.y); }

    template <typename S> Point2D<T> operator*(S rhs)
    { return Point2D<T>(this->x * rhs, this->y * rhs); }
};

template <typename T> class Point3D
{
    public:
    T x, y, z;
    Point3D() : x(0), y(0), z(0) {}

    ~Point3D() = default;
    Point3D(const Point3D &Pt)
    {
        x = Pt.x;
        y = Pt.y;
        z = Pt.z;
    }

    Point3D(T X, T Y, T Z)
    {
        x = X;
        y = Y;
        z = Z;
    }

    void Set(T X, T Y, T Z)
    {
        x = X;
        y = Y;
        z = Z;
    }

    bool operator==(const Point3D<T> &Pt) const
    { return ((x == Pt.x) && (y == Pt.y) && (z == Pt.z)); }

    bool operator!=(const Point3D<T> &Pt) const { return !((*this) == Pt); }
};

#endif
