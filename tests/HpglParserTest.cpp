#include "core/HpglParser.h"

#include "core/Geometry.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QtTest/QtTest>

#include <vector>

using camm3::HpglParser;
using camm3::PenState;
using camm3::Segment;

class HpglParserTest : public QObject
{
    Q_OBJECT

private slots:
    void parsesSampleFile();
    void inlineCommaAndSpaceSeparators();
    void multiCoordinatePolyline();
    void progressReachesHundred();
    void ignoresUnknownCommands();
};

void HpglParserTest::parsesSampleFile()
{
    QFile file(QString::fromUtf8(CAMM3_TEST_DATA_DIR "/Untitled-1.plt"));
    QVERIFY(file.open(QIODevice::ReadOnly));

    HpglParser parser;
    const HpglParser::Result result = parser.parse(file);

    QVERIFY(result.segments.size() > 0);

    QVERIFY(result.bounds.isValid());
    QVERIFY(result.bounds.width() > 0.0);
    QVERIFY(result.bounds.height() > 0.0);

    // The first real command in the file is "PU2146 1876", so the first
    // emitted segment is a pen-up (travel) move starting from the origin.
    const Segment& first = result.segments.front();
    QCOMPARE(first.kind, PenState::Up);
    QCOMPARE(first.a.x, 0.0);
    QCOMPARE(first.a.y, 0.0);
    QCOMPARE(first.b.x, 2146.0);
    QCOMPARE(first.b.y, 1876.0);

    // The second command "PD4625 1876" must be a pen-down (cut) segment that
    // continues from where the pen-up move ended.
    QVERIFY(result.segments.size() > 1);
    const Segment& second = result.segments.at(1);
    QCOMPARE(second.kind, PenState::Down);
    QCOMPARE(second.a.x, 2146.0);
    QCOMPARE(second.a.y, 1876.0);
    QCOMPARE(second.b.x, 4625.0);
    QCOMPARE(second.b.y, 1876.0);
}

void HpglParserTest::inlineCommaAndSpaceSeparators()
{
    // Comma-separated coordinates (first move PU from origin), then PD cuts.
    const QByteArray hpgl("PU0,0;PD10,0;PD10,10;");
    HpglParser parser;
    const HpglParser::Result result = parser.parse(hpgl);

    QCOMPARE(result.segments.size(), static_cast<std::size_t>(3));

    QCOMPARE(result.segments.at(0).kind, PenState::Up);
    QCOMPARE(result.segments.at(0).a.x, 0.0);
    QCOMPARE(result.segments.at(0).a.y, 0.0);
    QCOMPARE(result.segments.at(0).b.x, 0.0);
    QCOMPARE(result.segments.at(0).b.y, 0.0);

    QCOMPARE(result.segments.at(1).kind, PenState::Down);
    QCOMPARE(result.segments.at(1).a.x, 0.0);
    QCOMPARE(result.segments.at(1).a.y, 0.0);
    QCOMPARE(result.segments.at(1).b.x, 10.0);
    QCOMPARE(result.segments.at(1).b.y, 0.0);

    QCOMPARE(result.segments.at(2).kind, PenState::Down);
    QCOMPARE(result.segments.at(2).a.x, 10.0);
    QCOMPARE(result.segments.at(2).a.y, 0.0);
    QCOMPARE(result.segments.at(2).b.x, 10.0);
    QCOMPARE(result.segments.at(2).b.y, 10.0);

    // Space-separated form must yield the identical geometry.
    const QByteArray spaced("PU0 0;PD10 0;PD10 10;");
    const HpglParser::Result spacedResult = parser.parse(spaced);
    QCOMPARE(spacedResult.segments.size(), result.segments.size());
    for (std::size_t i = 0; i < result.segments.size(); ++i)
    {
        QCOMPARE(spacedResult.segments.at(i).b.x, result.segments.at(i).b.x);
        QCOMPARE(spacedResult.segments.at(i).b.y, result.segments.at(i).b.y);
        QCOMPARE(spacedResult.segments.at(i).kind, result.segments.at(i).kind);
    }
}

void HpglParserTest::multiCoordinatePolyline()
{
    // A single PD carrying a polyline of three points produces three connected
    // segments (origin->p1, p1->p2, p2->p3).
    const QByteArray hpgl("PD0,0,10,0,10,10;");
    HpglParser parser;
    const HpglParser::Result result = parser.parse(hpgl);

    QCOMPARE(result.segments.size(), static_cast<std::size_t>(3));
    QCOMPARE(result.segments.at(0).a.x, 0.0);
    QCOMPARE(result.segments.at(0).b.x, 0.0);
    QCOMPARE(result.segments.at(1).b.x, 10.0);
    QCOMPARE(result.segments.at(1).b.y, 0.0);
    QCOMPARE(result.segments.at(2).b.x, 10.0);
    QCOMPARE(result.segments.at(2).b.y, 10.0);
    for (const Segment& s : result.segments)
    {
        QCOMPARE(s.kind, PenState::Down);
    }
}

void HpglParserTest::progressReachesHundred()
{
    HpglParser parser;
    int last = -1;
    parser.setProgressCallback([&last](int percent) { last = percent; });

    const QByteArray hpgl("IN;PU0,0;PD10,0;PD10,10;SP0;");
    parser.parse(hpgl);

    QCOMPARE(last, 100);
}

void HpglParserTest::ignoresUnknownCommands()
{
    // Preamble of ignorable commands followed by a single cut. Only the PU/PD
    // geometry should survive.
    const QByteArray hpgl("IN;VS32,1;PW0.350,1;LT;SP8;PU100,200;PD300,400;");
    HpglParser parser;
    const HpglParser::Result result = parser.parse(hpgl);

    QCOMPARE(result.segments.size(), static_cast<std::size_t>(2));
    QCOMPARE(result.segments.at(0).kind, PenState::Up);
    QCOMPARE(result.segments.at(0).b.x, 100.0);
    QCOMPARE(result.segments.at(0).b.y, 200.0);
    QCOMPARE(result.segments.at(1).kind, PenState::Down);
    QCOMPARE(result.segments.at(1).b.x, 300.0);
    QCOMPARE(result.segments.at(1).b.y, 400.0);
}

QTEST_GUILESS_MAIN(HpglParserTest)
#include "HpglParserTest.moc"
