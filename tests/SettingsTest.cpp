#include "core/Settings.h"

#include <QString>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using camm3::Settings;

class SettingsTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultsWhenUnset();
    void persistsAcrossInstances();
    void transportKindRoundTrips();
};

void SettingsTest::defaultsWhenUnset()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Settings settings(dir.filePath(QStringLiteral("camm3.ini")));

    // Legacy defaults must match camm2.ini exactly.
    QCOMPARE(settings.port(), QStringLiteral("LPT2"));
    QCOMPARE(settings.speed(), 50);
    QCOMPARE(settings.moveAfter(), 0);
    QCOMPARE(settings.saveProportions(), true);
    QCOMPARE(settings.afterPlot(), 2);
    QCOMPARE(settings.autoCut(), false);

    // New keys.
    QCOMPARE(settings.transportKind(), Settings::TransportKind::Lpt);
    QCOMPARE(settings.baud(), 9600);
    QCOMPARE(settings.flowControl(), QStringLiteral("None"));
    QCOMPARE(settings.lastDirectory(), QString());
    QCOMPARE(settings.language(), QString());
}

void SettingsTest::persistsAcrossInstances()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString iniPath = dir.filePath(QStringLiteral("camm3.ini"));

    {
        Settings settings(iniPath);
        settings.setPort(QStringLiteral("COM3"));
        settings.setSpeed(120);
        settings.setMoveAfter(5);
        settings.setSaveProportions(false);
        settings.setAfterPlot(1);
        settings.setAutoCut(true);
        settings.setTransportKind(Settings::TransportKind::Serial);
        settings.setBaud(19200);
        settings.setFlowControl(QStringLiteral("Hardware"));
        settings.setLastDirectory(QStringLiteral("/tmp/plots"));
        settings.setLanguage(QStringLiteral("ru_RU"));
    }

    // A fresh instance on the same file must read back everything.
    Settings reopened(iniPath);
    QCOMPARE(reopened.port(), QStringLiteral("COM3"));
    QCOMPARE(reopened.speed(), 120);
    QCOMPARE(reopened.moveAfter(), 5);
    QCOMPARE(reopened.saveProportions(), false);
    QCOMPARE(reopened.afterPlot(), 1);
    QCOMPARE(reopened.autoCut(), true);
    QCOMPARE(reopened.transportKind(), Settings::TransportKind::Serial);
    QCOMPARE(reopened.baud(), 19200);
    QCOMPARE(reopened.flowControl(), QStringLiteral("Hardware"));
    QCOMPARE(reopened.lastDirectory(), QStringLiteral("/tmp/plots"));
    QCOMPARE(reopened.language(), QStringLiteral("ru_RU"));
}

void SettingsTest::transportKindRoundTrips()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString iniPath = dir.filePath(QStringLiteral("camm3.ini"));

    const Settings::TransportKind kinds[] = {
        Settings::TransportKind::Serial,
        Settings::TransportKind::Lpt,
        Settings::TransportKind::File,
    };

    for (const Settings::TransportKind kind : kinds) {
        {
            Settings settings(iniPath);
            settings.setTransportKind(kind);
        }
        Settings reopened(iniPath);
        QCOMPARE(reopened.transportKind(), kind);
    }
}

QTEST_GUILESS_MAIN(SettingsTest)
#include "SettingsTest.moc"
