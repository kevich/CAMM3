#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

namespace camm3 {

// Abstract output sink for plotter byte streams.
//
// The legacy app (CAMM2/Ports.vb) wrote raw bytes straight to an LPT port via
// WriteFile. This interface decouples the rest of the app from the concrete
// destination so that real ports, serial ports, or a plain file (for dev/test)
// can all be driven the same way.
class IPlotterTransport : public QObject
{
    Q_OBJECT

public:
    explicit IPlotterTransport(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IPlotterTransport() override = default;

    // Prepare the destination for writing. Returns false on failure and emits
    // errorOccurred() with a human-readable message.
    virtual bool open() = 0;

    // Write all of data, reporting progress via progressChanged(). Returns the
    // number of bytes actually written (== data.size() on success).
    virtual qint64 write(const QByteArray& data) = 0;

    // Flush and release the destination. Safe to call when not open.
    virtual void close() = 0;

signals:
    void progressChanged(int done, int total);
    void errorOccurred(const QString& message);
};

} // namespace camm3
