#pragma once

#include <cmath>
#include <limits>
#include <vector>

namespace camm3
{

struct Point2D
{
    double x = 0.0;
    double y = 0.0;
};

// Up = travel move (PU, pen up), Down = cutting move (PD, pen down).
enum class PenState
{
    Up,
    Down
};

struct Segment
{
    PenState kind = PenState::Up;
    Point2D a;
    Point2D b;
};

// Axis-aligned bounding box. Default-constructed to an empty/invalid state
// (xMin = +inf, xMax = -inf, ...) so the first expand() seeds real values.
struct Bounds
{
    double xMin = std::numeric_limits<double>::infinity();
    double xMax = -std::numeric_limits<double>::infinity();
    double yMin = std::numeric_limits<double>::infinity();
    double yMax = -std::numeric_limits<double>::infinity();

    // An empty box has never been expanded, so xMin > xMax.
    bool isValid() const
    {
        return xMin <= xMax && yMin <= yMax;
    }

    double width() const
    {
        return isValid() ? (xMax - xMin) : 0.0;
    }

    double height() const
    {
        return isValid() ? (yMax - yMin) : 0.0;
    }

    void expand(const Point2D& p)
    {
        if (p.x < xMin)
        {
            xMin = p.x;
        }
        if (p.x > xMax)
        {
            xMax = p.x;
        }
        if (p.y < yMin)
        {
            yMin = p.y;
        }
        if (p.y > yMax)
        {
            yMax = p.y;
        }
    }

    // Explicit factory for the empty box (equivalent to the default ctor).
    static Bounds empty()
    {
        return Bounds{};
    }
};

inline Bounds boundsOf(const std::vector<Segment>& segments)
{
    Bounds b;
    for (const Segment& s : segments)
    {
        b.expand(s.a);
        b.expand(s.b);
    }
    return b;
}

} // namespace camm3
