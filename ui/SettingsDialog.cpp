#include "ui/SettingsDialog.h"

#include "core/Settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVariant>

namespace camm3
{

namespace {

// Stable tokens used as combo item userData so that index<->value mapping never
// relies on the visual order of the items.
constexpr int kAfterPlotReturn = 1; // "Return to start"
constexpr int kAfterPlotSlide = 2;  // "Slide"

// Selects the combo entry whose userData equals value, falling back to the
// first item when no match exists.
void selectByData(QComboBox* combo, const QVariant& value)
{
    const int index = combo->findData(value);
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

} // namespace

SettingsDialog::SettingsDialog(Settings* settings, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(tr("Settings"));

    auto* mainLayout = new QVBoxLayout(this);

    // --- Transport group -----------------------------------------------------
    auto* transportGroup = new QGroupBox(tr("Transport"), this);
    auto* transportForm = new QFormLayout(transportGroup);

    m_transportCombo = new QComboBox(transportGroup);
    m_transportCombo->addItem(tr("Serial"),
                              static_cast<int>(Settings::TransportKind::Serial));
    m_transportCombo->addItem(tr("LPT"), static_cast<int>(Settings::TransportKind::Lpt));
    m_transportCombo->addItem(tr("File"), static_cast<int>(Settings::TransportKind::File));
    transportForm->addRow(tr("Kind:"), m_transportCombo);

    m_portLabel = new QLabel(tr("Port:"), transportGroup);
    m_portEdit = new QLineEdit(transportGroup);
    m_portEdit->setToolTip(tr("LPT device (e.g. LPT1) or serial device path."));
    transportForm->addRow(m_portLabel, m_portEdit);

    mainLayout->addWidget(transportGroup);

    // --- Serial sub-group ----------------------------------------------------
    m_serialGroup = new QGroupBox(tr("Serial"), this);
    auto* serialForm = new QFormLayout(m_serialGroup);

    m_baudCombo = new QComboBox(m_serialGroup);
    for (int rate : {9600, 19200, 38400, 57600, 115200}) {
        m_baudCombo->addItem(QString::number(rate), rate);
    }
    serialForm->addRow(tr("Baud:"), m_baudCombo);

    m_flowCombo = new QComboBox(m_serialGroup);
    m_flowCombo->addItem(tr("None"), QStringLiteral("None"));
    m_flowCombo->addItem(tr("Hardware"), QStringLiteral("Hardware"));
    m_flowCombo->addItem(tr("Software"), QStringLiteral("Software"));
    serialForm->addRow(tr("Flow control:"), m_flowCombo);

    mainLayout->addWidget(m_serialGroup);

    // --- Cutting group -------------------------------------------------------
    auto* cuttingGroup = new QGroupBox(tr("Cutting"), this);
    auto* cuttingForm = new QFormLayout(cuttingGroup);

    m_speedSpin = new QSpinBox(cuttingGroup);
    m_speedSpin->setRange(1, 100);
    cuttingForm->addRow(tr("Speed:"), m_speedSpin);

    m_moveAfterSpin = new QSpinBox(cuttingGroup);
    m_moveAfterSpin->setRange(0, 10000);
    cuttingForm->addRow(tr("Move after:"), m_moveAfterSpin);

    m_afterPlotCombo = new QComboBox(cuttingGroup);
    m_afterPlotCombo->addItem(tr("Return to start"), kAfterPlotReturn);
    m_afterPlotCombo->addItem(tr("Slide"), kAfterPlotSlide);
    cuttingForm->addRow(tr("After plot:"), m_afterPlotCombo);

    m_saveProportionsCheck = new QCheckBox(tr("Lock proportions"), cuttingGroup);
    cuttingForm->addRow(QString(), m_saveProportionsCheck);

    m_autoCutCheck = new QCheckBox(tr("Auto cut"), cuttingGroup);
    cuttingForm->addRow(QString(), m_autoCutCheck);

    mainLayout->addWidget(cuttingGroup);

    // --- General group -------------------------------------------------------
    auto* generalGroup = new QGroupBox(tr("General"), this);
    auto* generalForm = new QFormLayout(generalGroup);

    m_languageCombo = new QComboBox(generalGroup);
    m_languageCombo->addItem(tr("System"), QString());
    m_languageCombo->addItem(tr("Russian"), QStringLiteral("ru"));
    m_languageCombo->addItem(tr("English"), QStringLiteral("en"));
    m_languageCombo->setToolTip(tr("Applies at next launch."));
    generalForm->addRow(tr("Language:"), m_languageCombo);

    mainLayout->addWidget(generalGroup);

    // --- Button box ----------------------------------------------------------
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);

    // --- Load current values -------------------------------------------------
    selectByData(m_transportCombo, static_cast<int>(m_settings->transportKind()));
    m_portEdit->setText(m_settings->port());
    selectByData(m_baudCombo, m_settings->baud());
    selectByData(m_flowCombo, m_settings->flowControl());
    m_speedSpin->setValue(m_settings->speed());
    m_moveAfterSpin->setValue(m_settings->moveAfter());
    selectByData(m_afterPlotCombo, m_settings->afterPlot());
    m_saveProportionsCheck->setChecked(m_settings->saveProportions());
    m_autoCutCheck->setChecked(m_settings->autoCut());
    selectByData(m_languageCombo, m_settings->language());

    // --- Wire reveal/hide and apply once -------------------------------------
    connect(m_transportCombo, &QComboBox::currentIndexChanged, this,
            &SettingsDialog::updateTransportVisibility);
    updateTransportVisibility();
}

void SettingsDialog::updateTransportVisibility()
{
    const auto kind = static_cast<Settings::TransportKind>(
        m_transportCombo->currentData().toInt());

    const bool serial = (kind == Settings::TransportKind::Serial);
    const bool lpt = (kind == Settings::TransportKind::Lpt);

    // Port is meaningful for Serial (device path) and LPT (device name) but not
    // for File, where the path is chosen at cut time.
    const bool showPort = serial || lpt;
    m_portLabel->setVisible(showPort);
    m_portEdit->setVisible(showPort);

    // Baud/flow only apply to the serial transport.
    m_serialGroup->setVisible(serial);
}

void SettingsDialog::onAccept()
{
    m_settings->setTransportKind(
        static_cast<Settings::TransportKind>(m_transportCombo->currentData().toInt()));
    m_settings->setPort(m_portEdit->text());
    m_settings->setBaud(m_baudCombo->currentData().toInt());
    m_settings->setFlowControl(m_flowCombo->currentData().toString());
    m_settings->setSpeed(m_speedSpin->value());
    m_settings->setMoveAfter(m_moveAfterSpin->value());
    m_settings->setAfterPlot(m_afterPlotCombo->currentData().toInt());
    m_settings->setSaveProportions(m_saveProportionsCheck->isChecked());
    m_settings->setAutoCut(m_autoCutCheck->isChecked());
    m_settings->setLanguage(m_languageCombo->currentData().toString());

    QDialog::accept();
}

} // namespace camm3
