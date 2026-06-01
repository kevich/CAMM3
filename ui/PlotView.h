#pragma once

#include <QGraphicsView>

class QGraphicsScene;
class QGraphicsPathItem;

namespace camm3
{

class PlotJob;

// Scene-graph preview of a PlotJob. Renders the raw HPGL geometry; rotation and
// zoom are applied purely through the view transform (geometry is never
// re-rotated). The HPGL->screen Y inversion is likewise a view transform, since
// HPGL Y grows up while screen Y grows down.
class PlotView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit PlotView(QWidget* parent = nullptr);

    // Rebuilds the scene from the job's RAW segments (no transform baked in).
    void setJob(const PlotJob& job);

    // Shows/hides the pen-up (travel) item.
    void setShowPenUp(bool show);

    // Sets the view rotation; normalized to 0/90/180/270.
    void setRotation(int degrees);

    // Sets the zoom factor (1.0 == native).
    void setZoom(double factor);

    // Fits the whole job in the view, keeping aspect ratio (reset zoom).
    void fitJob();

private:
    // Builds the view transform as Y-flip x rotation x zoom and applies it.
    void updateTransform();

    QGraphicsScene* m_scene = nullptr;
    QGraphicsPathItem* m_penUpItem = nullptr;
    QGraphicsPathItem* m_penDownItem = nullptr;

    int m_rotationDegrees = 0;
    double m_zoom = 1.0;
};

} // namespace camm3
