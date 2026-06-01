#pragma once

#include "core/transport/IPlotterTransport.h"

#include <QByteArray>
#include <QObject>
#include <QString>

namespace camm3 {

// IPlotterTransport implementation that writes the byte stream straight to a
// parallel (LPT) port, mirroring the legacy CAMM2/Ports.vb CammRun routine.
//
// The real implementation is Windows-only and relies on the Win32 API
// (CreateFileW/WriteFile/CloseHandle). On non-Windows platforms the class still
// compiles but acts as a stub: open() fails and write() returns -1. The Win32
// header is deliberately kept out of this header so it stays portable; the
// native handle is stored as an opaque void*.
class LptTransport : public IPlotterTransport
{
    Q_OBJECT

public:
    explicit LptTransport(QObject* parent = nullptr);
    explicit LptTransport(const QString& portName, QObject* parent = nullptr);
    ~LptTransport() override;

    // Logical port name, e.g. "LPT1" or "LPT2". Defaults to "LPT1". Mapped
    // internally to the \\.\LPTx device path when opening.
    void setPortName(const QString& name);
    QString portName() const;

    // Opens the LPT device for writing. Returns false and emits errorOccurred()
    // (including the GetLastError() value) when the handle cannot be acquired.
    // Always false on non-Windows platforms.
    bool open() override;

    // Writes all of data to the port in a loop, emitting progressChanged() as it
    // goes. Returns the number of bytes written (== data.size() on success) or -1
    // on failure. Always -1 on non-Windows platforms.
    qint64 write(const QByteArray& data) override;

    // Closes the underlying handle if open. Safe to call when not open.
    void close() override;

private:
    QString m_portName;
    // Opaque native handle (Win32 HANDLE on Windows). nullptr means "not open".
    void* m_handle;
};

} // namespace camm3
