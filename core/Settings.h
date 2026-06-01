#pragma once

#include <QSettings>
#include <QString>

namespace camm3
{

// Type-safe wrapper around QSettings holding the persisted application
// configuration. Mirrors the six legacy camm2.ini keys (same defaults) and adds
// the new keys needed by CAMM3's serial/file transports and i18n.
//
// Storage notes:
//   * Backed by QSettings. With the default ctor the org/app store
//     QSettings("kevich", "CAMM3") is used; passing an explicit ini path uses
//     QSettings::IniFormat against that file (used by tests via QTemporaryDir so
//     the real user store is never touched).
//   * Booleans are stored as native QVariant bool (NOT the legacy "0"/"1"
//     strings) since this is a fresh CAMM3 store, not the legacy camm2.ini.
//   * TransportKind is stored as a stable string token ("Serial"/"Lpt"/"File").
//   * Setters write through immediately and call sync().
class Settings
{
public:
    // How plotter output is delivered.
    enum class TransportKind {
        Serial,
        Lpt,
        File,
    };

    // Empty path -> org/app store QSettings("kevich", "CAMM3").
    // Non-empty path -> QSettings(path, QSettings::IniFormat).
    explicit Settings(const QString& iniFilePath = QString());

    // --- Legacy keys (defaults match camm2.ini) ---

    QString port() const;            // default "LPT2"
    void setPort(const QString& value);

    int speed() const;               // default 50
    void setSpeed(int value);

    int moveAfter() const;           // default 0
    void setMoveAfter(int value);

    bool saveProportions() const;    // default true
    void setSaveProportions(bool value);

    int afterPlot() const;           // default 2
    void setAfterPlot(int value);

    bool autoCut() const;            // default false
    void setAutoCut(bool value);

    // --- New keys ---

    TransportKind transportKind() const; // default Lpt (legacy behavior)
    void setTransportKind(TransportKind value);

    int baud() const;                // default 9600
    void setBaud(int value);

    // Serial flow control token; default "None".
    QString flowControl() const;
    void setFlowControl(const QString& value);

    QString lastDirectory() const;   // default "" (empty)
    void setLastDirectory(const QString& value);

    QString language() const;        // default "" (system locale)
    void setLanguage(const QString& value);

private:
    mutable QSettings m_settings;
};

} // namespace camm3
