#pragma once

#include "core/PlotJob.h"

#include <QByteArray>

namespace camm3
{

// Generates the HP-GL cut stream sent to the vinyl cutter, reproducing the
// behaviour of the legacy CAMM2 module HPGL.vb (CAMM_Draw / CAMMSquare /
// CAMM_Command).
//
// The emitter consumes PlotJob::applyTransform() as its single source of
// transform truth: scale + rotation (and displace) are already baked into the
// transformed points, so this class never re-rotates or swaps axes. It only
//   (1) normalizes the transformed geometry so its minimum corner sits at the
//       origin (0,0) -- matching the legacy "(coord - XMin/YMin)" subtraction
//       that keeps all plotter coordinates non-negative, and which is required
//       because the very first relative move starts from (0,0); and
//   (2) rounds every coordinate to the nearest integer plotter unit (qRound),
//       since HP-GL plotter units are integers. The legacy used VB Int()
//       (truncation); for the non-negative normalized coordinates we use
//       round-to-nearest for deterministic, symmetric behaviour.
//
// All moves are emitted as RELATIVE deltas (PR mode), so any global translation
// cancels out; only the normalized min corner and the inter-point deltas
// matter.
class HpglEmitter
{
public:
    // Full cut stream: prologue + body (from job.applyTransform(), normalized as
    // documented) + AfterPlot tail + SP0.
    QByteArray emitStream(const PlotJob& job);

    // CAMMSquare-style rectangular outline of the job's transformed bounds, with
    // the same prologue/tail conventions.
    QByteArray boundingBox(const PlotJob& job);
};

} // namespace camm3
