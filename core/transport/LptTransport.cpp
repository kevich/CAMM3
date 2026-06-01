#include "core/transport/LptTransport.h"

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace camm3 {

LptTransport::LptTransport(QObject* parent)
    : IPlotterTransport(parent)
    , m_portName(QStringLiteral("LPT1"))
    , m_handle(nullptr)
{
}

LptTransport::LptTransport(const QString& portName, QObject* parent)
    : IPlotterTransport(parent)
    , m_portName(portName)
    , m_handle(nullptr)
{
}

LptTransport::~LptTransport()
{
    close();
}

void LptTransport::setPortName(const QString& name)
{
    m_portName = name;
}

QString LptTransport::portName() const
{
    return m_portName;
}

#ifdef Q_OS_WIN

bool LptTransport::open()
{
    if (m_handle != nullptr)
    {
        return true;
    }

    // Map the logical port name (e.g. "LPT1") to its Win32 device path.
    const std::wstring devicePath = (QStringLiteral("\\\\.\\") + m_portName).toStdWString();

    HANDLE handle = ::CreateFileW(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (handle == INVALID_HANDLE_VALUE)
    {
        const DWORD err = ::GetLastError();
        emit errorOccurred(QStringLiteral("Failed to open port '%1' (error %2).")
                               .arg(m_portName)
                               .arg(static_cast<qulonglong>(err)));
        return false;
    }

    m_handle = handle;
    return true;
}

qint64 LptTransport::write(const QByteArray& data)
{
    if (m_handle == nullptr)
    {
        emit errorOccurred(QStringLiteral("write() called on a closed transport."));
        return -1;
    }

    HANDLE handle = static_cast<HANDLE>(m_handle);
    const int total = data.size();
    qint64 done = 0;

    while (done < total)
    {
        DWORD written = 0;
        const DWORD chunk = static_cast<DWORD>(total - done);
        const BOOL ok = ::WriteFile(handle, data.constData() + done, chunk, &written, nullptr);
        if (!ok)
        {
            const DWORD err = ::GetLastError();
            emit errorOccurred(QStringLiteral("Failed to write to port '%1' (error %2).")
                                   .arg(m_portName)
                                   .arg(static_cast<qulonglong>(err)));
            return -1;
        }
        done += written;
        emit progressChanged(static_cast<int>(done), total);
    }

    // Guarantee at least one progress signal even for an empty payload.
    if (total == 0)
    {
        emit progressChanged(0, 0);
    }

    return done;
}

void LptTransport::close()
{
    if (m_handle != nullptr)
    {
        ::CloseHandle(static_cast<HANDLE>(m_handle));
        m_handle = nullptr;
    }
}

#else // !Q_OS_WIN

bool LptTransport::open()
{
    emit errorOccurred(QStringLiteral("LPT output is only supported on Windows"));
    return false;
}

qint64 LptTransport::write(const QByteArray& data)
{
    Q_UNUSED(data);
    emit errorOccurred(QStringLiteral("LPT output is only supported on Windows"));
    return -1;
}

void LptTransport::close()
{
    // No native handle is ever acquired off Windows.
}

#endif // Q_OS_WIN

} // namespace camm3
