#include "core/transport/FileTransport.h"

namespace camm3 {

FileTransport::FileTransport(QObject* parent)
    : IPlotterTransport(parent)
{
}

FileTransport::FileTransport(const QString& filePath, QObject* parent)
    : IPlotterTransport(parent)
{
    m_file.setFileName(filePath);
}

FileTransport::~FileTransport()
{
    close();
}

void FileTransport::setFilePath(const QString& filePath)
{
    m_file.setFileName(filePath);
}

QString FileTransport::filePath() const
{
    return m_file.fileName();
}

bool FileTransport::open()
{
    if (m_file.isOpen())
    {
        return true;
    }

    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        emit errorOccurred(QStringLiteral("Failed to open '%1' for writing: %2")
                               .arg(m_file.fileName(), m_file.errorString()));
        return false;
    }

    return true;
}

qint64 FileTransport::write(const QByteArray& data)
{
    if (!m_file.isOpen())
    {
        emit errorOccurred(QStringLiteral("write() called on a closed transport."));
        return -1;
    }

    const int total = data.size();
    qint64 done = 0;

    while (done < total)
    {
        const qint64 n = m_file.write(data.constData() + done, total - done);
        if (n < 0)
        {
            emit errorOccurred(QStringLiteral("Failed to write to '%1': %2")
                                   .arg(m_file.fileName(), m_file.errorString()));
            return done;
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

void FileTransport::close()
{
    if (m_file.isOpen())
    {
        m_file.flush();
        m_file.close();
    }
}

} // namespace camm3
