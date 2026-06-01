#pragma once

#include "core/Geometry.h"

#include <vector>

namespace camm3
{

// Pure-data plot job: the raw HPGL geometry plus the transform parameters that
// position/scale/rotate it for both on-screen preview and HPGL emission.
// No I/O, no Qt Widgets, no global state.
class PlotJob
{
public:
    // Raw, untransformed segments exactly as parsed from the HPGL source.
    std::vector<Segment> segments;

    // Transform parameters (see applyTransform for the exact order).
    double scaleX = 1.0;
    double scaleY = 1.0;
    int rotationDegrees = 0; // only 0/90/180/270 are valid
    Point2D displace{0.0, 0.0};
    int moveAfter = 0;
    int afterPlot = 0;
    int speed = 50;

    // Bounds of the raw, untransformed segments.
    Bounds rawBounds() const;

    // Returns NEW segments with the transform applied in this deterministic
    // order:
    //   (1) scale each coordinate by (scaleX, scaleY) about the origin,
    //   (2) rotate about the origin by rotationDegrees,
    //   (3) translate by displace.
    // Single source of transform truth for preview and HPGL emitter.
    std::vector<Segment> applyTransform() const;

    // Bounds of the transformed segments.
    Bounds transformedBounds() const;

    // Normalizes any integer degree value to one of {0, 90, 180, 270}.
    void setRotation(int degrees);
};

} // namespace camm3
