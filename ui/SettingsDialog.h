#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace camm3
{

class Settings;

// Modal dialog for editing the persisted application configuration. Reads the
// current values out of the supplied Settings on construction and, on OK,
// writes every control's value back through the matching Settings setter
// (Settings persists itself). Cancel leaves Settings untouched.
//
// The widget tree is built programmatically (no .ui file). The transport-kind
// combo drives the visibility of the Port row and the serial-only sub-group.
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Settings* settings, QWidget* parent = nullptr);

private slots:
    // Writes all controls back to Settings, then QDialog::accept().
    void onAccept();

private:
    // Shows/hides Port + serial sub-group based on the current transport kind.
    void updateTransportVisibility();

    Settings* m_settings = nullptr;

    QComboBox* m_transportCombo = nullptr;

    QLabel* m_portLabel = nullptr;
    QLineEdit* m_portEdit = nullptr;

    QGroupBox* m_serialGroup = nullptr;
    QComboBox* m_baudCombo = nullptr;
    QComboBox* m_flowCombo = nullptr;

    QSpinBox* m_speedSpin = nullptr;
    QSpinBox* m_moveAfterSpin = nullptr;
    QComboBox* m_afterPlotCombo = nullptr;
    QCheckBox* m_saveProportionsCheck = nullptr;
    QCheckBox* m_autoCutCheck = nullptr;
    QComboBox* m_languageCombo = nullptr;
};

} // namespace camm3
