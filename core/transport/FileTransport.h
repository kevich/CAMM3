#pragma once

#include "core/transport/IPlotterTransport.h"

#include <QByteArray>
#include <QFile>
#include <QString>

namespace camm3 {

// IPlotterTransport implementation that dumps the byte stream to a regular file.
// Used for development and testing on platforms without a physical plotter port.
class FileTransport : public IPlotterTransport
{
    Q_OBJECT

public:
    explicit FileTransport(QObject* parent = nullptr);
    explicit FileTransport(const QString& filePath, QObject* parent = nullptr);
    ~FileTransport() override;

    void setFilePath(const QString& filePath);
    QString filePath() const;

    // Opens the target file for writing, truncating any existing content.
    // Returns false and emits errorOccurred() if the file cannot be opened.
    bool open() override;

    // Writes data to the file, emitting progressChanged() at least once on
    // completion. Returns the number of bytes written.
    qint64 write(const QByteArray& data) override;

    // Flushes and closes the file.
    void close() override;

private:
    QFile m_file;
};

} // namespace camm3
