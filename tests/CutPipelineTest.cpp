// End-to-end integration test of the core cut pipeline (no GUI):
//   sample HPGL -> HpglParser -> PlotJob -> HpglEmitter -> FileTransport -> file
// This is the automatable portion of the plan's E2E check #3. The byte-for-byte
// comparison against a legacy-VB capture is a separate manual/hardware step.

#include "core/HpglEmitter.h"
#include "core/HpglParser.h"
#include "core/PlotJob.h"
#include "core/transport/FileTransport.h"

#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using namespace camm3;

namespace {
QByteArray readSample()
{
    QFile f(QStringLiteral(CAMM3_TEST_DATA_DIR "/Untitled-1.plt"));
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return f.readAll();
}
} // namespace

class CutPipelineTest : public QObject
{
    Q_OBJECT

private slots:
    void parsesAndEmitsToFile()
    {
        const QByteArray sample = readSample();
        QVERIFY(!sample.isEmpty());

        HpglParser parser;
        const HpglParser::Result parsed = parser.parse(sample);
        QVERIFY(parsed.segments.size() > 0);
        QVERIFY(parsed.bounds.isValid());

        PlotJob job;
        job.segments = parsed.segments;
        job.speed = 50;
        job.afterPlot = 1;

        HpglEmitter emitter;
        const QByteArray stream = emitter.emitStream(job);
        QVERIFY(!stream.isEmpty());
        // Legacy prologue and pen commands must be present.
        QVERIFY(stream.startsWith("VS"));
        QVERIFY(stream.contains("PU"));
        QVERIFY(stream.contains("PD"));
        QVERIFY(stream.endsWith("SP0"));

        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QString outPath = tmp.filePath(QStringLiteral("out.plt"));

        FileTransport transport(outPath);
        QSignalSpy progress(&transport, &IPlotterTransport::progressChanged);
        QVERIFY(transport.open());
        const qint64 written = transport.write(stream);
        transport.close();
        QCOMPARE(written, static_cast<qint64>(stream.size()));
        QVERIFY(progress.count() > 0);

        // File on disk equals the emitted stream byte-for-byte.
        QFile out(outPath);
        QVERIFY(out.open(QIODevice::ReadOnly));
        QCOMPARE(out.readAll(), stream);
    }

    void boundingBoxIsNonEmpty()
    {
        const QByteArray sample = readSample();
        HpglParser parser;
        PlotJob job;
        job.segments = parser.parse(sample).segments;

        HpglEmitter emitter;
        const QByteArray bbox = emitter.boundingBox(job);
        QVERIFY(!bbox.isEmpty());
        QVERIFY(bbox.startsWith("VS"));
        QVERIFY(bbox.contains("PU"));
    }
};

QTEST_GUILESS_MAIN(CutPipelineTest)
#include "CutPipelineTest.moc"
