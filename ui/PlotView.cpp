#include "ui/PlotView.h"

#include "core/Geometry.h"
#include "core/PlotJob.h"

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPen>
#include <QRectF>
#include <QTransform>

namespace camm3
{

PlotView::PlotView(QWidget* parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing, true);
    setBackgroundBrush(QColor(40, 40, 40));
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    updateTransform();
}

void PlotView::setJob(const PlotJob& job)
{
    m_scene->clear();
    m_penUpItem = nullptr;
    m_penDownItem = nullptr;

    // Build two paths from the RAW segments (HPGL coordinates). Rotation is a
    // view transform, so geometry stays untransformed here.
    QPainterPath penUpPath;
    QPainterPath penDownPath;
    for (const Segment& s : job.segments)
    {
        QPainterPath& path = (s.kind == PenState::Down) ? penDownPath : penUpPath;
        path.moveTo(s.a.x, s.a.y);
        path.lineTo(s.b.x, s.b.y);
    }

    // Cosmetic pens: stroke width stays constant regardless of zoom.
    QPen penUp(QColor(192, 192, 192));
    penUp.setCosmetic(true);
    QPen penDown(QColor(0, 0, 0));
    penDown.setCosmetic(true);

    // Add pen-up first so cuts draw on top of travel moves.
    m_penUpItem = m_scene->addPath(penUpPath, penUp);
    m_penDownItem = m_scene->addPath(penDownPath, penDown);

    const Bounds b = job.rawBounds();
    if (b.isValid())
    {
        m_scene->setSceneRect(QRectF(b.xMin, b.yMin, b.width(), b.height()));
    }
    else
    {
        m_scene->setSceneRect(QRectF());
    }

    updateTransform();
    fitJob();
}

void PlotView::setShowPenUp(bool show)
{
    if (m_penUpItem)
    {
        m_penUpItem->setVisible(show);
    }
}

void PlotView::setRotation(int degrees)
{
    // Normalize to 0/90/180/270.
    int d = degrees % 360;
    if (d < 0)
    {
        d += 360;
    }
    m_rotationDegrees = (d / 90) * 90;
    updateTransform();
}

void PlotView::setZoom(double factor)
{
    if (factor > 0.0)
    {
        m_zoom = factor;
    }
    updateTransform();
}

void PlotView::fitJob()
{
    const QRectF rect = m_scene->sceneRect();
    if (!rect.isEmpty() && rect.width() > 0.0 && rect.height() > 0.0)
    {
        fitInView(rect, Qt::KeepAspectRatio);
    }
}

void PlotView::updateTransform()
{
    // Compose Y-flip x rotation x zoom. The Y-flip turns HPGL's upward Y into
    // screen-down coordinates so the preview appears right-side-up.
    QTransform t;
    t.scale(1.0, -1.0);
    t.rotate(m_rotationDegrees);
    t.scale(m_zoom, m_zoom);
    setTransform(t);
}

} // namespace camm3
