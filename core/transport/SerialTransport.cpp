#include "core/transport/SerialTransport.h"

#if CAMM3_HAVE_SERIALPORT
#include <QSerialPort>
#endif

namespace camm3 {

namespace {

// Maximum payload pushed to the port between progress notifications / drains.
constexpr int kChunkSize = 256;

// Milliseconds to wait for each chunk to leave the OS buffer.
constexpr int kWriteTimeoutMs = 3000;

} // namespace

#if CAMM3_HAVE_SERIALPORT

// ---------------------------------------------------------------------------
// Real implementation (Qt6 SerialPort module present).
// ---------------------------------------------------------------------------

namespace {

QSerialPort::Parity toQtParity(SerialTransport::Parity parity)
{
    switch (parity)
    {
    case SerialTransport::Parity::None:
        return QSerialPort::NoParity;
    case SerialTransport::Parity::Even:
        return QSerialPort::EvenParity;
    case SerialTransport::Parity::Odd:
        return QSerialPort::OddParity;
    case SerialTransport::Parity::Space:
        return QSerialPort::SpaceParity;
    case SerialTransport::Parity::Mark:
        return QSerialPort::MarkParity;
    }
    return QSerialPort::NoParity;
}

QSerialPort::StopBits toQtStopBits(SerialTransport::StopBits stopBits)
{
    switch (stopBits)
    {
    case SerialTransport::StopBits::One:
        return QSerialPort::OneStop;
    case SerialTransport::StopBits::OneAndHalf:
        return QSerialPort::OneAndHalfStop;
    case SerialTransport::StopBits::Two:
        return QSerialPort::TwoStop;
    }
    return QSerialPort::OneStop;
}

QSerialPort::FlowControl toQtFlowControl(SerialTransport::FlowControl flowControl)
{
    switch (flowControl)
    {
    case SerialTransport::FlowControl::None:
        return QSerialPort::NoFlowControl;
    case SerialTransport::FlowControl::Hardware:
        return QSerialPort::HardwareControl;
    case SerialTransport::FlowControl::Software:
        return QSerialPort::SoftwareControl;
    }
    return QSerialPort::NoFlowControl;
}

QSerialPort::DataBits toQtDataBits(int dataBits)
{
    switch (dataBits)
    {
    case 5:
        return QSerialPort::Data5;
    case 6:
        return QSerialPort::Data6;
    case 7:
        return QSerialPort::Data7;
    case 8:
    default:
        return QSerialPort::Data8;
    }
}

} // namespace

SerialTransport::SerialTransport(QObject* parent)
    : IPlotterTransport(parent)
{
}

SerialTransport::SerialTransport(const SerialConfig& config, QObject* parent)
    : IPlotterTransport(parent)
    , m_config(config)
{
}

SerialTransport::~SerialTransport()
{
    close();
    delete m_port;
    m_port = nullptr;
}

bool SerialTransport::open()
{
    if (m_port == nullptr)
    {
        m_port = new QSerialPort(this);
    }

    if (m_port->isOpen())
    {
        return true;
    }

    m_port->setPortName(m_config.portName);
    m_port->setBaudRate(m_config.baudRate);
    m_port->setDataBits(toQtDataBits(m_config.dataBits));
    m_port->setParity(toQtParity(m_config.parity));
    m_port->setStopBits(toQtStopBits(m_config.stopBits));
    m_port->setFlowControl(toQtFlowControl(m_config.flowControl));

    if (!m_port->open(QIODevice::WriteOnly))
    {
        emit errorOccurred(QStringLiteral("Failed to open serial port '%1': %2")
                               .arg(m_config.portName, m_port->errorString()));
        return false;
    }

    return true;
}

qint64 SerialTransport::write(const QByteArray& data)
{
    if (m_port == nullptr || !m_port->isOpen())
    {
        emit errorOccurred(QStringLiteral("write() called on a closed serial transport."));
        return -1;
    }

    const int total = data.size();
    qint64 done = 0;

    while (done < total)
    {
        const int chunk = qMin(kChunkSize, total - static_cast<int>(done));
        const qint64 n = m_port->write(data.constData() + done, chunk);
        if (n < 0)
        {
            emit errorOccurred(QStringLiteral("Failed to write to serial port '%1': %2")
                                   .arg(m_config.portName, m_port->errorString()));
            return -1;
        }

        if (!m_port->waitForBytesWritten(kWriteTimeoutMs))
        {
            emit errorOccurred(QStringLiteral("Timed out writing to serial port '%1': %2")
                                   .arg(m_config.portName, m_port->errorString()));
            return -1;
        }

        done += n;
        emit progressChanged(static_cast<int>(done), total);
    }

    // Guarantee at least one progress signal even for an empty payload.
    if (total == 0)
    {
        emit progressChanged(0, 0);
    }

    return done;
}

void SerialTransport::close()
{
    if (m_port != nullptr && m_port->isOpen())
    {
        m_port->close();
    }
}

#else

// ---------------------------------------------------------------------------
// Stub implementation (Qt6 SerialPort module absent). No QSerialPort symbols
// are referenced here; the transport always reports unavailability.
// ---------------------------------------------------------------------------

SerialTransport::SerialTransport(QObject* parent)
    : IPlotterTransport(parent)
{
}

SerialTransport::SerialTransport(const SerialConfig& config, QObject* parent)
    : IPlotterTransport(parent)
    , m_config(config)
{
}

SerialTransport::~SerialTransport() = default;

bool SerialTransport::open()
{
    emit errorOccurred(QStringLiteral("Qt SerialPort module not available in this build."));
    return false;
}

qint64 SerialTransport::write(const QByteArray&)
{
    emit errorOccurred(QStringLiteral("Qt SerialPort module not available in this build."));
    return -1;
}

void SerialTransport::close()
{
    // No-op: nothing was ever opened.
}

#endif // CAMM3_HAVE_SERIALPORT

// ---------------------------------------------------------------------------
// Configuration accessors (identical in both branches).
// ---------------------------------------------------------------------------

void SerialTransport::setConfig(const SerialConfig& config)
{
    m_config = config;
}

SerialTransport::SerialConfig SerialTransport::config() const
{
    return m_config;
}

void SerialTransport::setPortName(const QString& portName)
{
    m_config.portName = portName;
}

void SerialTransport::setBaudRate(int baudRate)
{
    m_config.baudRate = baudRate;
}

void SerialTransport::setDataBits(int dataBits)
{
    m_config.dataBits = dataBits;
}

void SerialTransport::setParity(Parity parity)
{
    m_config.parity = parity;
}

void SerialTransport::setStopBits(StopBits stopBits)
{
    m_config.stopBits = stopBits;
}

void SerialTransport::setFlowControl(FlowControl flowControl)
{
    m_config.flowControl = flowControl;
}

} // namespace camm3
