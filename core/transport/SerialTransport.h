#pragma once

#include "core/transport/IPlotterTransport.h"

#include <QByteArray>
#include <QString>

// NOTE: This header intentionally does NOT include <QSerialPort> or
// <QSerialPortInfo>. The Qt6 SerialPort module may be absent on the build
// machine, so callers must be able to include this header regardless. The
// configuration uses plain ints and the project's own enums below; these are
// translated to QSerialPort::* values inside the .cpp under the
// CAMM3_HAVE_SERIALPORT guard.

QT_BEGIN_NAMESPACE
class QSerialPort;
QT_END_NAMESPACE

namespace camm3 {

// IPlotterTransport implementation that drives a physical RS-232 serial link.
//
// Serial output is NEW in CAMM3 (legacy CAMM2/Ports.vb supported LPT only). The
// CAMM-2 hardware typically expects 9600 8N1 with hardware (RTS/CTS) flow
// control, but every parameter here is configurable rather than hardcoded.
//
// When the Qt6 SerialPort module is not present at build time
// (CAMM3_HAVE_SERIALPORT == 0), this class still compiles with the same public
// API, but open()/write() fail gracefully via errorOccurred().
class SerialTransport : public IPlotterTransport
{
    Q_OBJECT

public:
    // Mirrors of the QSerialPort enums, kept independent so this header does not
    // depend on the SerialPort module being installed.
    enum class Parity
    {
        None,
        Even,
        Odd,
        Space,
        Mark
    };

    enum class StopBits
    {
        One,
        OneAndHalf,
        Two
    };

    enum class FlowControl
    {
        None,
        Hardware, // RTS/CTS
        Software  // XON/XOFF
    };

    // All serial-line settings in one place. Defaults match the common CAMM-2
    // RS-232 link: 9600 8N1 with hardware flow control.
    struct SerialConfig
    {
        QString portName;
        int baudRate = 9600;
        int dataBits = 8;
        Parity parity = Parity::None;
        StopBits stopBits = StopBits::One;
        FlowControl flowControl = FlowControl::Hardware;
    };

    explicit SerialTransport(QObject* parent = nullptr);
    explicit SerialTransport(const SerialConfig& config, QObject* parent = nullptr);
    ~SerialTransport() override;

    // Replace the full configuration. Takes effect on the next open().
    void setConfig(const SerialConfig& config);
    SerialConfig config() const;

    // Convenience setters for individual fields. All take effect on next open().
    void setPortName(const QString& portName);
    void setBaudRate(int baudRate);
    void setDataBits(int dataBits);
    void setParity(Parity parity);
    void setStopBits(StopBits stopBits);
    void setFlowControl(FlowControl flowControl);

    // Configures and opens the serial port. Returns false and emits
    // errorOccurred() on failure (including when the SerialPort module is
    // unavailable in this build).
    bool open() override;

    // Writes data in fixed-size chunks, waiting for each chunk to drain and
    // emitting progressChanged(done, total) after every chunk. Returns the total
    // number of bytes written, or -1 on error (after emitting errorOccurred()).
    qint64 write(const QByteArray& data) override;

    // Closes the serial port. Safe to call when not open.
    void close() override;

private:
    SerialConfig m_config;

#if CAMM3_HAVE_SERIALPORT
    // The concrete QSerialPort lives only in the .cpp to keep this header free
    // of the SerialPort module; it is forward-declared at global scope above so
    // this resolves to ::QSerialPort, not camm3::QSerialPort.
    QSerialPort* m_port = nullptr;
#endif
};

} // namespace camm3
