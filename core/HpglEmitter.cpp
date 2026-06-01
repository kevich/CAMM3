#include "core/HpglEmitter.h"

#include "core/Geometry.h"

#include <QtGlobal>
#include <cmath>
#include <vector>

namespace camm3
{

namespace
{

// Integer plotter point. The legacy worked in integer plotter units; we round
// the transformed (double) coordinates to the nearest integer with qRound.
struct IPoint
{
    int x = 0;
    int y = 0;
};

bool samePoint(const IPoint& a, const IPoint& b)
{
    return a.x == b.x && a.y == b.y;
}

// Prologue shared by CAMM_Draw and CAMMSquare:
//   "VS{speed}" "PA" "PU0,0" "PR"  -> e.g. "VS50PAPU0,0PR"
QByteArray prologue(int speed)
{
    QByteArray out;
    out += "VS";
    out += QByteArray::number(speed);
    out += "PA";
    out += "PU0,0";
    out += "PR";
    return out;
}

// AfterPlot tail + final SP0, reproducing CAMM_Draw lines 210-216.
//   afterPlot == 1 -> "PAPU0,0"
//   afterPlot == 2 -> "PAPU{width + moveAfter*40},0"
//   (any other)    -> no advance
// then always "SP0".
QByteArray tail(int afterPlot, int width, int moveAfter)
{
    QByteArray out;
    switch (afterPlot)
    {
    case 1:
        out += "PAPU0,0";
        break;
    case 2:
        out += "PAPU";
        out += QByteArray::number(width + moveAfter * 40);
        out += ",0";
        break;
    default:
        break;
    }
    out += "SP0";
    return out;
}

} // namespace

QByteArray HpglEmitter::emitStream(const PlotJob& job)
{
    const std::vector<Segment> transformed = job.applyTransform();

    // Normalize so the minimum corner sits at the origin. Mirrors the legacy
    // CAMM_Command "(X - XMin, Y - YMin)" subtraction.
    const Bounds bounds = boundsOf(transformed);
    const double xMin = bounds.isValid() ? bounds.xMin : 0.0;
    const double yMin = bounds.isValid() ? bounds.yMin : 0.0;

    auto toInt = [xMin, yMin](const Point2D& p) -> IPoint
    {
        return IPoint{qRound(p.x - xMin), qRound(p.y - yMin)};
    };

    QByteArray out = prologue(job.speed);

    // Relative-move body. We track the pen's last integer position (starting at
    // the normalized origin) and the last emitted command token. A new command
    // token (PU/PD) is written only when the command changes; otherwise moves
    // are comma-chained onto the running command (legacy run-on encoding).
    IPoint last{0, 0};
    QByteArray lastCmd; // empty == no command emitted yet

    auto move = [&](const char* cmd, const IPoint& dst)
    {
        const int dx = dst.x - last.x;
        const int dy = dst.y - last.y;
        if (lastCmd == cmd)
        {
            out += ",";
        }
        else
        {
            out += cmd;
            lastCmd = cmd;
        }
        out += QByteArray::number(dx);
        out += ",";
        out += QByteArray::number(dy);
        last = dst;
    };

    for (const Segment& s : transformed)
    {
        const IPoint a = toInt(s.a);
        const IPoint b = toInt(s.b);

        // If the segment does not start where the pen currently is, travel there
        // pen-up first (disconnected polyline / explicit re-positioning).
        if (!samePoint(a, last))
        {
            move("PU", a);
        }

        move(s.kind == PenState::Down ? "PD" : "PU", b);
    }

    const int width = qRound(bounds.isValid() ? bounds.width() : 0.0);
    out += tail(job.afterPlot, width, job.moveAfter);
    return out;
}

QByteArray HpglEmitter::boundingBox(const PlotJob& job)
{
    const Bounds bounds = job.transformedBounds();
    const int w = qRound(bounds.isValid() ? bounds.width() : 0.0);
    const int h = qRound(bounds.isValid() ? bounds.height() : 0.0);

    // CAMMSquare (HPGL.vb:144-158): a single PU with four comma-chained relative
    // moves tracing a closed rectangle, starting/ending at the origin:
    //   up by h, right by w, down by h, left by w.
    QByteArray out = prologue(job.speed);
    out += "PU0,";
    out += QByteArray::number(h);
    out += ",";
    out += QByteArray::number(w);
    out += ",0";
    out += ",0,";
    out += QByteArray::number(-h);
    out += ",";
    out += QByteArray::number(-w);
    out += ",0";
    out += "PAPU0,0";
    return out;
}

} // namespace camm3
