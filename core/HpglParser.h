#pragma once

#include "core/Geometry.h"

#include <QByteArray>
#include <QIODevice>
#include <functional>
#include <vector>

namespace camm3
{

// Parser for the subset of HP-GL emitted by the legacy CAMM2 application.
//
// Only the PU (pen up / travel) and PD (pen down / cut) commands carry
// geometry; every other mnemonic (IN, VS, PW, WU, LT, SP, PA, PR, ...) is
// ignored. A single PU/PD may carry one coordinate pair or a whole polyline,
// with coordinates separated by spaces and/or commas and the command
// terminated by ';' and/or a newline.
//
// Start position: the pen starts at the origin (0, 0), mirroring the legacy
// loop which initialised LastCmd = "PU" and an explicit LastPosition before
// reading the file. The very first PU/PD therefore emits a segment from the
// origin to its first coordinate. Because the first real command in CAMM2
// output is always a PU, the first emitted segment is a pen-up (travel) move.
//
// Bounds: the bounding box covers every visited point, both pen-up and
// pen-down (the origin start point is only included once it is connected by an
// emitted segment).
class HpglParser
{
public:
    struct Result
    {
        std::vector<Segment> segments;
        Bounds bounds;
    };

    using ProgressCallback = std::function<void(int)>;

    HpglParser() = default;

    // Optional progress reporting. The callback receives an integer percentage
    // in the range [0, 100] as parsing advances through the device. Defaults to
    // a no-op.
    void setProgressCallback(ProgressCallback callback);

    // Streaming parse from an arbitrary QIODevice (already opened for reading,
    // or openable). Tolerant of space/comma separators and ';'/newline
    // terminators; robust against unknown commands and malformed input.
    Result parse(QIODevice& device);

    // Convenience overload: parse an in-memory buffer.
    Result parse(const QByteArray& bytes);

private:
    void reportProgress(int percent);

    ProgressCallback m_progress;
};

} // namespace camm3
