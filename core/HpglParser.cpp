#include "core/HpglParser.h"

#include <QBuffer>

#include <cctype>
#include <cstdlib>

namespace camm3
{

namespace
{

// True for HP-GL coordinate separators: whitespace or comma.
bool isSeparator(char c)
{
    return c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// True for characters that may begin / continue a numeric token.
bool isNumberChar(char c)
{
    return (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.';
}

} // namespace

void HpglParser::setProgressCallback(ProgressCallback callback)
{
    m_progress = std::move(callback);
}

void HpglParser::reportProgress(int percent)
{
    if (m_progress)
    {
        if (percent < 0)
        {
            percent = 0;
        }
        else if (percent > 100)
        {
            percent = 100;
        }
        m_progress(percent);
    }
}

HpglParser::Result HpglParser::parse(const QByteArray& bytes)
{
    QBuffer buffer;
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    return parse(buffer);
}

HpglParser::Result HpglParser::parse(QIODevice& device)
{
    Result result;

    if (!device.isOpen())
    {
        if (!device.open(QIODevice::ReadOnly))
        {
            return result;
        }
    }

    const QByteArray data = device.readAll();
    const int size = data.size();

    // Current pen position. The legacy loop seeds an explicit start position
    // and treats the initial command as a PU, so we begin at the origin.
    Point2D current{0.0, 0.0};

    reportProgress(0);
    int lastReported = 0;

    int i = 0;
    while (i < size)
    {
        const char c = data.at(i);

        // Look for a two-letter mnemonic. Commands are upper-case ASCII in the
        // legacy output, but we accept either case defensively.
        if (std::isalpha(static_cast<unsigned char>(c)) && i + 1 < size
            && std::isalpha(static_cast<unsigned char>(data.at(i + 1))))
        {
            const char a = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            const char b =
                static_cast<char>(std::toupper(static_cast<unsigned char>(data.at(i + 1))));
            const bool isPenUp = (a == 'P' && b == 'U');
            const bool isPenDown = (a == 'P' && b == 'D');

            i += 2;

            if (!isPenUp && !isPenDown)
            {
                // Unknown command: skip its parameters up to the next
                // terminator so we don't misread digits as a new command.
                while (i < size && data.at(i) != ';')
                {
                    // Stop early if another mnemonic begins (no terminator).
                    if (std::isalpha(static_cast<unsigned char>(data.at(i))))
                    {
                        break;
                    }
                    ++i;
                }
                if (i < size && data.at(i) == ';')
                {
                    ++i;
                }
                continue;
            }

            const PenState pen = isPenDown ? PenState::Down : PenState::Up;

            // Consume the coordinate list until the terminating ';', a newline,
            // or the start of the next mnemonic. Each consecutive pair forms a
            // segment from the running current position.
            while (i < size)
            {
                const char p = data.at(i);
                if (p == ';')
                {
                    ++i;
                    break;
                }
                if (std::isalpha(static_cast<unsigned char>(p)))
                {
                    // Next command without an explicit terminator.
                    break;
                }
                if (isSeparator(p))
                {
                    ++i;
                    continue;
                }

                // Parse an X token.
                int start = i;
                while (i < size && isNumberChar(data.at(i)))
                {
                    ++i;
                }
                if (i == start)
                {
                    // Not numeric and not a separator/terminator: skip it.
                    ++i;
                    continue;
                }
                const double x = std::strtod(data.mid(start, i - start).constData(), nullptr);

                // Skip separators between X and Y.
                while (i < size && isSeparator(data.at(i)))
                {
                    ++i;
                }

                // Parse a Y token.
                start = i;
                while (i < size && isNumberChar(data.at(i)))
                {
                    ++i;
                }
                if (i == start)
                {
                    // Dangling X without a Y: ignore the incomplete pair.
                    break;
                }
                const double y = std::strtod(data.mid(start, i - start).constData(), nullptr);

                const Point2D next{x, y};
                Segment seg{pen, current, next};
                result.segments.push_back(seg);
                result.bounds.expand(current);
                result.bounds.expand(next);
                current = next;
            }
        }
        else
        {
            ++i;
        }

        const int percent = size > 0 ? static_cast<int>(static_cast<qint64>(i) * 100 / size) : 100;
        if (percent != lastReported)
        {
            reportProgress(percent);
            lastReported = percent;
        }
    }

    reportProgress(100);
    return result;
}

} // namespace camm3
