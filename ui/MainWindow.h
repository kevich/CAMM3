#pragma once

#include "core/PlotJob.h"
#include "core/Settings.h"

#include <QMainWindow>

class QAction;
class QByteArray;
class QComboBox;
class QDoubleSpinBox;
class QProgressBar;
class QPushButton;
class QSlider;

namespace camm3
{

class PlotView;

// Main application window for CAMM3. Builds its entire widget tree
// programmatically (there is no .ui file): a menu bar, a tool bar, a central
// PlotView preview and a right-side dock with the size/rotation/zoom and cut
// controls. Wires the file-open -> parse -> preview pipeline, the proportional
// size lock, rotation/zoom of the view and the cut/bounding-box output through
// the transport configured in Settings.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void openHpgl();
    void cut();
    void boundingBox();
    void openSettings();

    void onWidthChanged(double value);
    void onHeightChanged(double value);
    void onProportionToggled(bool locked);
    void onRotationComboChanged(int index);
    void rotateLeft();
    void rotateRight();
    void zoomIn();
    void zoomOut();
    void zoomSliderChanged(int value);
    void fitView();
    void toggleShowPenUp(bool show);

private:
    // Widget construction helpers.
    void buildActions();
    void buildMenus();
    void buildToolBar();
    void buildDock();
    void buildStatusBar();

    // Recomputes job.scaleX/scaleY from the size spin boxes (mm) and refreshes
    // the preview.
    void applySizeToJob();

    // Pushes the given rotation (degrees) to both the job and the view, and
    // keeps the rotation combo in sync. All under the re-entrancy guard.
    void setRotation(int degrees);

    // Enables/disables every job-dependent control.
    void setControlsEnabled(bool enabled);

    // Builds the configured transport, opens it, writes the stream and closes
    // it; drives the status-bar progress bar and surfaces errors via a message
    // box. Synchronous.
    void sendToPlotter(const QByteArray& stream);

    Settings m_settings;
    PlotJob m_job;
    bool m_hasJob = false;

    // Raw (untransformed) job size in mm, cached so the proportional lock can
    // preserve the aspect ratio independently of rounding in the spin boxes.
    double m_rawWidthMm = 0.0;
    double m_rawHeightMm = 0.0;

    // Re-entrancy guard: suppresses valueChanged/currentIndexChanged feedback
    // while we are programmatically updating controls (mirrors the legacy
    // Drawing/Rotating booleans).
    bool m_updating = false;

    PlotView* m_plotView = nullptr;

    // Dock controls.
    QDoubleSpinBox* m_widthSpin = nullptr;
    QDoubleSpinBox* m_heightSpin = nullptr;
    QPushButton* m_lockButton = nullptr;
    QComboBox* m_rotationCombo = nullptr;
    QPushButton* m_rotateLeftButton = nullptr;
    QPushButton* m_rotateRightButton = nullptr;
    QSlider* m_zoomSlider = nullptr;
    QPushButton* m_cutButton = nullptr;
    QPushButton* m_bboxButton = nullptr;

    // Status bar.
    QProgressBar* m_progressBar = nullptr;

    // Actions.
    QAction* m_openAction = nullptr;
    QAction* m_cutAction = nullptr;
    QAction* m_bboxAction = nullptr;
    QAction* m_quitAction = nullptr;
    QAction* m_showPenUpAction = nullptr;
    QAction* m_zoomInAction = nullptr;
    QAction* m_zoomOutAction = nullptr;
    QAction* m_fitAction = nullptr;
    QAction* m_rotateLeftAction = nullptr;
    QAction* m_rotateRightAction = nullptr;
    QAction* m_settingsAction = nullptr;
};

} // namespace camm3
