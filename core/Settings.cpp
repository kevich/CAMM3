#include "core/Settings.h"

namespace camm3
{

namespace {

constexpr auto kOrg = "kevich";
constexpr auto kApp = "CAMM3";

// Storage keys.
constexpr auto kPort = "Port";
constexpr auto kSpeed = "Speed";
constexpr auto kMoveAfter = "MoveAfter";
constexpr auto kSaveProportions = "SaveProportions";
constexpr auto kAfterPlot = "AfterPlot";
constexpr auto kAutoCut = "AutoCut";
constexpr auto kTransportKind = "TransportKind";
constexpr auto kBaud = "Baud";
constexpr auto kFlowControl = "FlowControl";
constexpr auto kLastDirectory = "LastDirectory";
constexpr auto kLanguage = "Language";

QString transportKindToToken(Settings::TransportKind kind)
{
    switch (kind) {
    case Settings::TransportKind::Serial:
        return QStringLiteral("Serial");
    case Settings::TransportKind::File:
        return QStringLiteral("File");
    case Settings::TransportKind::Lpt:
        break;
    }
    return QStringLiteral("Lpt");
}

Settings::TransportKind tokenToTransportKind(const QString& token)
{
    if (token == QLatin1String("Serial"))
        return Settings::TransportKind::Serial;
    if (token == QLatin1String("File"))
        return Settings::TransportKind::File;
    return Settings::TransportKind::Lpt;
}

} // namespace

Settings::Settings(const QString& iniFilePath)
    : m_settings(iniFilePath.isEmpty()
                     ? QSettings(QString::fromLatin1(kOrg), QString::fromLatin1(kApp))
                     : QSettings(iniFilePath, QSettings::IniFormat))
{
}

QString Settings::port() const
{
    return m_settings.value(QString::fromLatin1(kPort), QStringLiteral("LPT2")).toString();
}

void Settings::setPort(const QString& value)
{
    m_settings.setValue(QString::fromLatin1(kPort), value);
    m_settings.sync();
}

int Settings::speed() const
{
    return m_settings.value(QString::fromLatin1(kSpeed), 50).toInt();
}

void Settings::setSpeed(int value)
{
    m_settings.setValue(QString::fromLatin1(kSpeed), value);
    m_settings.sync();
}

int Settings::moveAfter() const
{
    return m_settings.value(QString::fromLatin1(kMoveAfter), 0).toInt();
}

void Settings::setMoveAfter(int value)
{
    m_settings.setValue(QString::fromLatin1(kMoveAfter), value);
    m_settings.sync();
}

bool Settings::saveProportions() const
{
    return m_settings.value(QString::fromLatin1(kSaveProportions), true).toBool();
}

void Settings::setSaveProportions(bool value)
{
    m_settings.setValue(QString::fromLatin1(kSaveProportions), value);
    m_settings.sync();
}

int Settings::afterPlot() const
{
    return m_settings.value(QString::fromLatin1(kAfterPlot), 2).toInt();
}

void Settings::setAfterPlot(int value)
{
    m_settings.setValue(QString::fromLatin1(kAfterPlot), value);
    m_settings.sync();
}

bool Settings::autoCut() const
{
    return m_settings.value(QString::fromLatin1(kAutoCut), false).toBool();
}

void Settings::setAutoCut(bool value)
{
    m_settings.setValue(QString::fromLatin1(kAutoCut), value);
    m_settings.sync();
}

Settings::TransportKind Settings::transportKind() const
{
    const QString token =
        m_settings.value(QString::fromLatin1(kTransportKind), QStringLiteral("Lpt")).toString();
    return tokenToTransportKind(token);
}

void Settings::setTransportKind(TransportKind value)
{
    m_settings.setValue(QString::fromLatin1(kTransportKind), transportKindToToken(value));
    m_settings.sync();
}

int Settings::baud() const
{
    return m_settings.value(QString::fromLatin1(kBaud), 9600).toInt();
}

void Settings::setBaud(int value)
{
    m_settings.setValue(QString::fromLatin1(kBaud), value);
    m_settings.sync();
}

QString Settings::flowControl() const
{
    return m_settings.value(QString::fromLatin1(kFlowControl), QStringLiteral("None")).toString();
}

void Settings::setFlowControl(const QString& value)
{
    m_settings.setValue(QString::fromLatin1(kFlowControl), value);
    m_settings.sync();
}

QString Settings::lastDirectory() const
{
    return m_settings.value(QString::fromLatin1(kLastDirectory), QString()).toString();
}

void Settings::setLastDirectory(const QString& value)
{
    m_settings.setValue(QString::fromLatin1(kLastDirectory), value);
    m_settings.sync();
}

QString Settings::language() const
{
    return m_settings.value(QString::fromLatin1(kLanguage), QString()).toString();
}

void Settings::setLanguage(const QString& value)
{
    m_settings.setValue(QString::fromLatin1(kLanguage), value);
    m_settings.sync();
}

} // namespace camm3
