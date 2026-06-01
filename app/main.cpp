#include "MainWindow.h"
#include "core/Settings.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QStringList>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Runtime window/taskbar icon (embedded via resources.qrc). On macOS the Dock
    // uses the bundle's .icns instead, but this is harmless there.
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/CAMM3.png")));

    // Language: an explicit choice in Settings wins; otherwise follow the system
    // locale. The .qm files are embedded by qt_add_translations under :/i18n/.
    camm3::Settings settings;
    QStringList candidates;
    const QString configured = settings.language();
    if (!configured.isEmpty())
        candidates << QStringLiteral("CAMM3_") + configured;
    for (const QString &locale : QLocale::system().uiLanguages())
        candidates << QStringLiteral("CAMM3_") + QLocale(locale).name();

    QTranslator translator;
    for (const QString &baseName : candidates) {
        if (translator.load(QStringLiteral(":/i18n/") + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    camm3::MainWindow w;
    w.show();
    return QApplication::exec();
}
