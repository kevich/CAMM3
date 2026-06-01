#include "core/PlotJob.h"

namespace camm3
{

namespace
{

// Rotation about the origin, matching the mapping used by HPGL.vb:117-130.
// Working in standard math coordinates (the VB code adds the bounding-box
// offsets and the screen Y-flip separately; those belong to the displace /
// preview layers, not to the pure rotation matrix). The matrices below are the
// standard counter-clockwise rotations that the VB cases implement:
//   0   -> ( x,  y)
//   90  -> (-y,  x)
//   180 -> (-x, -y)
//   270 -> ( y, -x)
Point2D rotatePoint(const Point2D& p, int degrees)
{
    switch (degrees)
    {
    case 90:
        return Point2D{-p.y, p.x};
    case 180:
        return Point2D{-p.x, -p.y};
    case 270:
        return Point2D{p.y, -p.x};
    case 0:
    default:
        return Point2D{p.x, p.y};
    }
}

} // namespace

Bounds PlotJob::rawBounds() const
{
    return boundsOf(segments);
}

std::vector<Segment> PlotJob::applyTransform() const
{
    auto transform = [this](const Point2D& p) -> Point2D
    {
        // (1) scale about origin
        Point2D scaled{p.x * scaleX, p.y * scaleY};
        // (2) rotate about origin
        Point2D rotated = rotatePoint(scaled, rotationDegrees);
        // (3) translate by displace
        return Point2D{rotated.x + displace.x, rotated.y + displace.y};
    };

    std::vector<Segment> out;
    out.reserve(segments.size());
    for (const Segment& s : segments)
    {
        out.push_back(Segment{s.kind, transform(s.a), transform(s.b)});
    }
    return out;
}

Bounds PlotJob::transformedBounds() const
{
    return boundsOf(applyTransform());
}

void PlotJob::setRotation(int degrees)
{
    int normalized = degrees % 360;
    if (normalized < 0)
    {
        normalized += 360;
    }
    // Snap to the nearest valid quadrant; only 0/90/180/270 are meaningful.
    rotationDegrees = (normalized / 90) * 90;
}

} // namespace camm3
