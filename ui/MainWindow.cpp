#include "MainWindow.h"

#include "core/HpglEmitter.h"
#include "core/HpglParser.h"
#include "core/transport/FileTransport.h"
#include "core/transport/IPlotterTransport.h"
#include "core/transport/LptTransport.h"
#include "core/transport/SerialTransport.h"
#include "ui/PlotView.h"
#include "ui/SettingsDialog.h"

#include <QAction>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include <memory>

namespace camm3
{

namespace
{

// HPGL plotter units are 1/40 mm; sizes throughout the dock are presented and
// edited in millimetres for the operator's convenience.
constexpr double kUnitsPerMm = 40.0;

// Zoom slider maps integer steps [kZoomSliderMin..kZoomSliderMax] linearly onto
// PlotView zoom factors [1.0 .. kZoomMax].
constexpr int kZoomSliderMin = 1;
constexpr int kZoomSliderMax = 10;
constexpr double kZoomMax = 10.0;

double sliderToZoom(int value)
{
    return static_cast<double>(value);
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("CAMM3"));

    m_plotView = new PlotView(this);
    setCentralWidget(m_plotView);

    buildActions();
    buildMenus();
    buildToolBar();
    buildDock();
    buildStatusBar();

    setControlsEnabled(false);
    resize(1000, 700);
}

MainWindow::~MainWindow() = default;

void MainWindow::buildActions()
{
    m_openAction = new QAction(QIcon(QStringLiteral(":/icons/folder_image.png")), tr("Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openHpgl);

    m_cutAction = new QAction(QIcon(QStringLiteral(":/icons/printer.png")), tr("Cut"), this);
    connect(m_cutAction, &QAction::triggered, this, &MainWindow::cut);

    m_bboxAction = new QAction(QIcon(QStringLiteral(":/icons/shape_handles.png")), tr("Bounding Box"), this);
    connect(m_bboxAction, &QAction::triggered, this, &MainWindow::boundingBox);

    m_quitAction = new QAction(tr("Quit"), this);
    m_quitAction->setShortcut(QKeySequence::Quit);
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::close);

    m_showPenUpAction = new QAction(tr("Show pen-up"), this);
    m_showPenUpAction->setCheckable(true);
    m_showPenUpAction->setChecked(true);
    connect(m_showPenUpAction, &QAction::toggled, this, &MainWindow::toggleShowPenUp);

    m_zoomInAction = new QAction(QIcon(QStringLiteral(":/icons/zoomin.png")), tr("Zoom In"), this);
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(m_zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);

    m_zoomOutAction = new QAction(QIcon(QStringLiteral(":/icons/zoomout.png")), tr("Zoom Out"), this);
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(m_zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);

    m_fitAction = new QAction(QIcon(QStringLiteral(":/icons/page_refresh.png")), tr("Fit"), this);
    connect(m_fitAction, &QAction::triggered, this, &MainWindow::fitView);

    m_rotateLeftAction = new QAction(
        QIcon(QStringLiteral(":/icons/shape_rotate_anticlockwise.png")), tr("Rotate Left"), this);
    connect(m_rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);

    m_rotateRightAction = new QAction(
        QIcon(QStringLiteral(":/icons/shape_rotate_clockwise.png")), tr("Rotate Right"), this);
    connect(m_rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);

    m_settingsAction = new QAction(tr("Settings..."), this);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
}

void MainWindow::buildMenus()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(m_openAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_cutAction);
    fileMenu->addAction(m_bboxAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_quitAction);

    QMenu* viewMenu = menuBar()->addMenu(tr("View"));
    viewMenu->addAction(m_showPenUpAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_zoomInAction);
    viewMenu->addAction(m_zoomOutAction);
    viewMenu->addAction(m_fitAction);

    QMenu* toolsMenu = menuBar()->addMenu(tr("Tools"));
    toolsMenu->addAction(m_settingsAction);
}

void MainWindow::buildToolBar()
{
    QToolBar* toolBar = addToolBar(tr("Main"));
    toolBar->setObjectName(QStringLiteral("mainToolBar"));
    toolBar->addAction(m_openAction);
    toolBar->addSeparator();
    toolBar->addAction(m_cutAction);
    toolBar->addAction(m_bboxAction);
    toolBar->addSeparator();
    toolBar->addAction(m_rotateLeftAction);
    toolBar->addAction(m_rotateRightAction);
    toolBar->addSeparator();
    toolBar->addAction(m_zoomInAction);
    toolBar->addAction(m_zoomOutAction);
    toolBar->addAction(m_fitAction);
}

void MainWindow::buildDock()
{
    QDockWidget* dock = new QDockWidget(tr("Job"), this);
    dock->setObjectName(QStringLiteral("jobDock"));
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* panel = new QWidget(dock);
    QVBoxLayout* layout = new QVBoxLayout(panel);

    // --- Size group (mm) ---
    QGroupBox* sizeGroup = new QGroupBox(tr("Size (mm)"), panel);
    QFormLayout* sizeForm = new QFormLayout(sizeGroup);

    m_widthSpin = new QDoubleSpinBox(sizeGroup);
    m_widthSpin->setRange(0.0, 100000.0);
    m_widthSpin->setDecimals(2);
    m_widthSpin->setSuffix(QStringLiteral(" mm"));
    sizeForm->addRow(tr("Width:"), m_widthSpin);

    m_heightSpin = new QDoubleSpinBox(sizeGroup);
    m_heightSpin->setRange(0.0, 100000.0);
    m_heightSpin->setDecimals(2);
    m_heightSpin->setSuffix(QStringLiteral(" mm"));
    sizeForm->addRow(tr("Height:"), m_heightSpin);

    m_lockButton = new QPushButton(sizeGroup);
    m_lockButton->setCheckable(true);
    m_lockButton->setChecked(m_settings.saveProportions());
    m_lockButton->setText(tr("Lock proportions"));
    m_lockButton->setIcon(QIcon(m_lockButton->isChecked()
                                    ? QStringLiteral(":/icons/lock.png")
                                    : QStringLiteral(":/icons/unlocked.png")));
    sizeForm->addRow(m_lockButton);

    layout->addWidget(sizeGroup);

    // --- Rotation group ---
    QGroupBox* rotGroup = new QGroupBox(tr("Rotation"), panel);
    QVBoxLayout* rotLayout = new QVBoxLayout(rotGroup);

    m_rotationCombo = new QComboBox(rotGroup);
    m_rotationCombo->addItem(QStringLiteral("0°"), 0);
    m_rotationCombo->addItem(QStringLiteral("90°"), 90);
    m_rotationCombo->addItem(QStringLiteral("180°"), 180);
    m_rotationCombo->addItem(QStringLiteral("270°"), 270);
    rotLayout->addWidget(m_rotationCombo);

    QHBoxLayout* rotButtons = new QHBoxLayout();
    m_rotateLeftButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/shape_rotate_anticlockwise.png")), tr("Left"), rotGroup);
    m_rotateRightButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/shape_rotate_clockwise.png")), tr("Right"), rotGroup);
    rotButtons->addWidget(m_rotateLeftButton);
    rotButtons->addWidget(m_rotateRightButton);
    rotLayout->addLayout(rotButtons);

    layout->addWidget(rotGroup);

    // --- Zoom group ---
    QGroupBox* zoomGroup = new QGroupBox(tr("Zoom"), panel);
    QVBoxLayout* zoomLayout = new QVBoxLayout(zoomGroup);
    m_zoomSlider = new QSlider(Qt::Horizontal, zoomGroup);
    m_zoomSlider->setRange(kZoomSliderMin, kZoomSliderMax);
    m_zoomSlider->setValue(kZoomSliderMin);
    zoomLayout->addWidget(m_zoomSlider);
    layout->addWidget(zoomGroup);

    // --- Output buttons ---
    m_cutButton = new QPushButton(QIcon(QStringLiteral(":/icons/printer.png")), tr("Cut"), panel);
    m_bboxButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/shape_handles.png")), tr("Bounding Box"), panel);
    layout->addWidget(m_cutButton);
    layout->addWidget(m_bboxButton);

    layout->addStretch(1);

    dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // Wiring.
    connect(m_widthSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &MainWindow::onWidthChanged);
    connect(m_heightSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &MainWindow::onHeightChanged);
    connect(m_lockButton, &QPushButton::toggled, this, &MainWindow::onProportionToggled);
    connect(m_rotationCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::onRotationComboChanged);
    connect(m_rotateLeftButton, &QPushButton::clicked, this, &MainWindow::rotateLeft);
    connect(m_rotateRightButton, &QPushButton::clicked, this, &MainWindow::rotateRight);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &MainWindow::zoomSliderChanged);
    connect(m_cutButton, &QPushButton::clicked, this, &MainWindow::cut);
    connect(m_bboxButton, &QPushButton::clicked, this, &MainWindow::boundingBox);
}

void MainWindow::buildStatusBar()
{
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::setControlsEnabled(bool enabled)
{
    m_widthSpin->setEnabled(enabled);
    m_heightSpin->setEnabled(enabled);
    m_lockButton->setEnabled(enabled);
    m_rotationCombo->setEnabled(enabled);
    m_rotateLeftButton->setEnabled(enabled);
    m_rotateRightButton->setEnabled(enabled);
    m_zoomSlider->setEnabled(enabled);
    m_cutButton->setEnabled(enabled);
    m_bboxButton->setEnabled(enabled);

    m_cutAction->setEnabled(enabled);
    m_bboxAction->setEnabled(enabled);
    m_rotateLeftAction->setEnabled(enabled);
    m_rotateRightAction->setEnabled(enabled);
    m_zoomInAction->setEnabled(enabled);
    m_zoomOutAction->setEnabled(enabled);
    m_fitAction->setEnabled(enabled);
}

void MainWindow::openHpgl()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open HPGL"), m_settings.lastDirectory(),
        tr("HPGL files (*.plt *.hgl)"));
    if (path.isEmpty())
    {
        return;
    }

    if (QFileInfo(path).suffix().compare(QStringLiteral("dxf"), Qt::CaseInsensitive) == 0)
    {
        QMessageBox::information(this, tr("Unsupported format"),
                                 tr("DXF import is not supported."));
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("Open failed"),
                             tr("Could not open file:\n%1").arg(path));
        return;
    }

    HpglParser parser;
    HpglParser::Result result = parser.parse(file);
    file.close();

    // Reset transform parameters for the freshly loaded job.
    m_job = PlotJob{};
    m_job.segments = result.segments;
    m_hasJob = true;

    const Bounds raw = m_job.rawBounds();
    m_rawWidthMm = (raw.width() > 0.0) ? raw.width() / kUnitsPerMm : 0.0;
    m_rawHeightMm = (raw.height() > 0.0) ? raw.height() / kUnitsPerMm : 0.0;

    {
        const bool prev = m_updating;
        m_updating = true;
        m_widthSpin->setValue(m_rawWidthMm);
        m_heightSpin->setValue(m_rawHeightMm);
        m_rotationCombo->setCurrentIndex(0);
        m_zoomSlider->setValue(kZoomSliderMin);
        m_updating = prev;
    }

    m_job.scaleX = 1.0;
    m_job.scaleY = 1.0;
    m_job.setRotation(0);

    m_plotView->setJob(m_job);
    m_plotView->setRotation(0);
    m_plotView->fitJob();

    setControlsEnabled(true);

    m_settings.setLastDirectory(QFileInfo(path).absolutePath());
}

void MainWindow::applySizeToJob()
{
    if (!m_hasJob)
    {
        return;
    }

    const double desiredWidthMm = m_widthSpin->value();
    const double desiredHeightMm = m_heightSpin->value();

    m_job.scaleX = (m_rawWidthMm > 0.0) ? (desiredWidthMm / m_rawWidthMm) : 1.0;
    m_job.scaleY = (m_rawHeightMm > 0.0) ? (desiredHeightMm / m_rawHeightMm) : 1.0;

    m_plotView->setJob(m_job);
}

void MainWindow::onWidthChanged(double value)
{
    if (m_updating || !m_hasJob)
    {
        return;
    }

    if (m_lockButton->isChecked() && m_rawWidthMm > 0.0 && m_rawHeightMm > 0.0)
    {
        const double aspect = m_rawHeightMm / m_rawWidthMm;
        m_updating = true;
        m_heightSpin->setValue(value * aspect);
        m_updating = false;
    }

    applySizeToJob();
}

void MainWindow::onHeightChanged(double value)
{
    if (m_updating || !m_hasJob)
    {
        return;
    }

    if (m_lockButton->isChecked() && m_rawWidthMm > 0.0 && m_rawHeightMm > 0.0)
    {
        const double aspect = m_rawWidthMm / m_rawHeightMm;
        m_updating = true;
        m_widthSpin->setValue(value * aspect);
        m_updating = false;
    }

    applySizeToJob();
}

void MainWindow::onProportionToggled(bool locked)
{
    m_lockButton->setIcon(QIcon(locked ? QStringLiteral(":/icons/lock.png")
                                       : QStringLiteral(":/icons/unlocked.png")));
    m_settings.setSaveProportions(locked);

    // Re-establish the proportion immediately when (re)locking.
    if (locked && !m_updating && m_hasJob && m_rawWidthMm > 0.0 && m_rawHeightMm > 0.0)
    {
        const double aspect = m_rawHeightMm / m_rawWidthMm;
        m_updating = true;
        m_heightSpin->setValue(m_widthSpin->value() * aspect);
        m_updating = false;
        applySizeToJob();
    }
}

void MainWindow::setRotation(int degrees)
{
    m_job.setRotation(degrees);
    const int normalized = m_job.rotationDegrees;

    m_plotView->setRotation(normalized);

    const bool prev = m_updating;
    m_updating = true;
    const int index = m_rotationCombo->findData(normalized);
    if (index >= 0)
    {
        m_rotationCombo->setCurrentIndex(index);
    }
    m_updating = prev;
}

void MainWindow::onRotationComboChanged(int index)
{
    if (m_updating || !m_hasJob || index < 0)
    {
        return;
    }
    setRotation(m_rotationCombo->itemData(index).toInt());
}

void MainWindow::rotateLeft()
{
    if (!m_hasJob)
    {
        return;
    }
    // Counter-clockwise cycle: 0 -> 90 -> 180 -> 270 -> 0.
    setRotation(m_job.rotationDegrees + 90);
}

void MainWindow::rotateRight()
{
    if (!m_hasJob)
    {
        return;
    }
    // Clockwise cycle: 0 -> 270 -> 180 -> 90 -> 0.
    setRotation(m_job.rotationDegrees + 270);
}

void MainWindow::zoomIn()
{
    m_zoomSlider->setValue(qMin(m_zoomSlider->value() + 1, kZoomSliderMax));
}

void MainWindow::zoomOut()
{
    m_zoomSlider->setValue(qMax(m_zoomSlider->value() - 1, kZoomSliderMin));
}

void MainWindow::zoomSliderChanged(int value)
{
    if (m_updating)
    {
        return;
    }
    m_plotView->setZoom(sliderToZoom(value));
}

void MainWindow::fitView()
{
    const bool prev = m_updating;
    m_updating = true;
    m_zoomSlider->setValue(kZoomSliderMin);
    m_updating = prev;
    m_plotView->fitJob();
}

void MainWindow::toggleShowPenUp(bool show)
{
    m_plotView->setShowPenUp(show);
}

void MainWindow::cut()
{
    if (!m_hasJob)
    {
        return;
    }

    m_job.speed = m_settings.speed();
    m_job.moveAfter = m_settings.moveAfter();
    m_job.afterPlot = m_settings.afterPlot();

    HpglEmitter emitter;
    sendToPlotter(emitter.emitStream(m_job));
}

void MainWindow::boundingBox()
{
    if (!m_hasJob)
    {
        return;
    }

    m_job.speed = m_settings.speed();
    m_job.moveAfter = m_settings.moveAfter();
    m_job.afterPlot = m_settings.afterPlot();

    HpglEmitter emitter;
    sendToPlotter(emitter.boundingBox(m_job));
}

void MainWindow::sendToPlotter(const QByteArray& stream)
{
    std::unique_ptr<IPlotterTransport> transport;

    switch (m_settings.transportKind())
    {
    case Settings::TransportKind::File:
    {
        const QString path = QFileDialog::getSaveFileName(
            this, tr("Save plot stream"), m_settings.lastDirectory(),
            tr("HPGL files (*.plt)"));
        if (path.isEmpty())
        {
            return;
        }
        transport = std::make_unique<FileTransport>(path);
        break;
    }
    case Settings::TransportKind::Lpt:
    {
        auto lpt = std::make_unique<LptTransport>();
        lpt->setPortName(m_settings.port());
        transport = std::move(lpt);
        break;
    }
    case Settings::TransportKind::Serial:
    {
        SerialTransport::SerialConfig config;
        config.portName = m_settings.port();
        config.baudRate = m_settings.baud();

        const QString flow = m_settings.flowControl();
        if (flow.compare(QStringLiteral("Hardware"), Qt::CaseInsensitive) == 0)
        {
            config.flowControl = SerialTransport::FlowControl::Hardware;
        }
        else if (flow.compare(QStringLiteral("Software"), Qt::CaseInsensitive) == 0)
        {
            config.flowControl = SerialTransport::FlowControl::Software;
        }
        else
        {
            config.flowControl = SerialTransport::FlowControl::None;
        }

        transport = std::make_unique<SerialTransport>(config);
        break;
    }
    }

    if (!transport)
    {
        return;
    }

    m_progressBar->setValue(0);
    m_progressBar->setVisible(true);

    connect(transport.get(), &IPlotterTransport::progressChanged, this,
            [this](int done, int total) {
                if (total > 0)
                {
                    m_progressBar->setRange(0, total);
                    m_progressBar->setValue(done);
                }
            });
    connect(transport.get(), &IPlotterTransport::errorOccurred, this,
            [this](const QString& message) {
                QMessageBox::warning(this, tr("Plotter error"), message);
            });

    if (!transport->open())
    {
        m_progressBar->setVisible(false);
        return;
    }

    transport->write(stream);
    transport->close();

    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(&m_settings, this);
    dialog.exec();

    // Reflect any proportional-lock default change back into the toggle.
    const bool prev = m_updating;
    m_updating = true;
    m_lockButton->setChecked(m_settings.saveProportions());
    m_updating = prev;
}

} // namespace camm3
