#include "core/transport/FileTransport.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using camm3::FileTransport;

class FileTransportTest : public QObject
{
    Q_OBJECT

private slots:
    void roundTrip();
    void progressReachesTotal();
    void openUnwritablePathFails();
};

void FileTransportTest::roundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("out.plt"));

    // Known payload including binary / non-ASCII bytes.
    QByteArray payload;
    payload.append("IN;PU0,0;\x1b\x00\x7f");
    payload.append(static_cast<char>(0xC3));
    payload.append(static_cast<char>(0x28));
    payload.append("SP1;");
    payload.append('\0');
    payload.append(static_cast<char>(0xFF));

    FileTransport transport(path);
    QVERIFY(transport.open());
    QCOMPARE(transport.write(payload), static_cast<qint64>(payload.size()));
    transport.close();

    QFile readBack(path);
    QVERIFY(readBack.open(QIODevice::ReadOnly));
    const QByteArray contents = readBack.readAll();
    readBack.close();

    QCOMPARE(contents, payload);
}

void FileTransportTest::progressReachesTotal()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("progress.plt"));
    const QByteArray payload("hello plotter world");

    FileTransport transport(path);
    QSignalSpy spy(&transport, &FileTransport::progressChanged);

    QVERIFY(transport.open());
    QCOMPARE(transport.write(payload), static_cast<qint64>(payload.size()));
    transport.close();

    QVERIFY(!spy.isEmpty());

    const QList<QVariant> last = spy.takeLast();
    const int done = last.at(0).toInt();
    const int total = last.at(1).toInt();
    QCOMPARE(done, total);
    QCOMPARE(total, payload.size());
}

void FileTransportTest::openUnwritablePathFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // A path inside a directory that does not exist cannot be opened.
    const QString path =
        dir.filePath(QStringLiteral("no_such_subdir/cannot/create/out.plt"));

    FileTransport transport(path);
    QSignalSpy errorSpy(&transport, &FileTransport::errorOccurred);

    QVERIFY(!transport.open());
    QCOMPARE(errorSpy.count(), 1);

    const QString message = errorSpy.takeFirst().at(0).toString();
    QVERIFY(!message.isEmpty());
}

QTEST_GUILESS_MAIN(FileTransportTest)
#include "FileTransportTest.moc"
