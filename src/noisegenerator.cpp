#include "noisegenerator.h"

#include <QtMath>
#include <QtEndian>
#include <limits>

void NoiseGenerator::start()
{
    m_time = 0;
    fillBuffer();
    open(QIODevice::ReadOnly);
}

void NoiseGenerator::stop()
{
    close();
    m_pos = m_size = 0;
}

void NoiseGenerator::fillBuffer()
{
    const int channelBytes = m_format.sampleSize() / 8;
    const int sampleBytes = m_format.channelCount() * channelBytes;
    Q_UNUSED(sampleBytes) // suppress warning in release builds

    if (m_buffer.isEmpty())
    {
        const auto newCap = (m_format.sampleRate() * m_format.channelCount() * (m_format.sampleSize() / 8)) * DURATION_US / 1000000;
        Q_ASSERT(newCap % sampleBytes == 0);
        m_buffer.resize(newCap);
    }

    const auto cap = m_buffer.size();
    qint64 length = cap - m_size;
    while (length)
    {
        // Produces value in range [-1, 1]
        const qreal x = generateSample() * 2.0 - 1.0;
        m_time += k_sampleInterval;

        // Put sample to buffer
        auto ptr = reinterpret_cast<quint8*>(m_buffer.data() + (m_pos + m_size) % cap);
        for (int i = 0; i < m_format.channelCount(); i++)
        {
            switch (m_format.sampleSize())
            {
                case 8:
                    // Only integer samples
                    *reinterpret_cast<qint8*>(ptr) = m_format.sampleType() == QAudioFormat::SignedInt ?
                                                     static_cast<qint8>(x * 127) :
                                                     static_cast<quint8>((1.0 + x) / 2 * 255);
                    break;
                case 16:
                    // Only integer samples
                    switch (m_format.sampleType())
                    {
                        case QAudioFormat::UnSignedInt:
                        {
                            const auto value = static_cast<quint16>((1.0 + x) / 2 * 65535);
                            if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                                qToLittleEndian<quint16>(value, ptr);
                            else
                                qToBigEndian<quint16>(value, ptr);
                            break;
                        }
                        case QAudioFormat::SignedInt:
                        {
                            qint16 value = static_cast<qint16>(x * 32767);
                            if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                                qToLittleEndian<qint16>(value, ptr);
                            else
                                qToBigEndian<qint16>(value, ptr);
                            break;
                        }
                        default:
                            Q_ASSERT(false);
                    }
                    break;
                case 32:
                    // Ints & Floats
                    throw std::runtime_error { "not implemented" };
            }

            ptr += channelBytes;
            length -= channelBytes;
            m_size += channelBytes;
        }
    }
    Q_ASSERT(m_size == cap);
}

qreal NoiseGenerator::generateSample() noexcept
{
    // We need randon in range including both ends, i.e. [0, 1]
    constexpr double upperBoundX = 1.0 + std::numeric_limits<double>::epsilon();
    double s = m_ranGen.bounded(upperBoundX);
    if (m_pyArgs)
    {
        PyTuple_SetItem(m_pyArgs, 0, PyFloat_FromDouble(m_time));
        PyTuple_SetItem(m_pyArgs, 1, PyFloat_FromDouble(s));
        auto rv = PyObject_CallObject(m_pyGenFunc, m_pyArgs);
        if (rv)
        {
            s = PyFloat_AsDouble(rv);
            Py_DECREF(rv);
        }
    }

    return s;
}

qint64 NoiseGenerator::readData(char *data, qint64 len)
{
    const auto cap = m_buffer.size();
    len = qMin(len, m_size);

    qint64 total = 0;
    while (len - total > 0)
    {
        const qint64 chunkSize = qMin((cap - m_pos), len - total);
        memcpy(data + total, m_buffer.constData() + m_pos, chunkSize);
        m_pos = (m_pos + chunkSize) % cap;
        m_size -= chunkSize;
        total += chunkSize;
    }

    // Fill the area of the buffer that have just been sent to playback
    fillBuffer();

    return total;
}

qint64 NoiseGenerator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 NoiseGenerator::bytesAvailable() const
{
    return m_size;
}
