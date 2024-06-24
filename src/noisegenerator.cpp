#include "noisegenerator.h"

#include <QtMath>
#include <QtEndian>
#include <limits>
#include <type_traits>

void NoiseGenerator::start()
{
    m_time = 0;
    m_prev = 0;
    open(QIODevice::ReadOnly);
}

void NoiseGenerator::stop()
{
    close();
}

template <typename T>
typename std::enable_if<std::is_signed<T>::value, T>::type convertSample(qreal x) noexcept
{
    return static_cast<T>(x * std::numeric_limits<T>::max());
}

template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type convertSample(qreal x) noexcept
{
    return static_cast<T>((1.0 + x) / 2.0 * std::numeric_limits<T>::max());
}

template <typename T>
void convertSample(void *pOut, const qreal x, const QAudioFormat::Endian e) noexcept
{
    if (e == QAudioFormat::LittleEndian)
        qToLittleEndian<T>(convertSample<T>(x), pOut);
    else
        qToBigEndian<T>(convertSample<T>(x), pOut);
}

void NoiseGenerator::fillBuffer(qint64 length, char *pData)
{
    const int channelBytes = m_format.sampleSize() / 8;
    const auto byteOrder = m_format.byteOrder();
    while (length)
    {
        // Produces value in range [-1, 1]
        const qreal x = std::min(1.0, std::max(-1.0, generateSample()));

        // Put sample to buffer
        for (int i = 0; i < m_format.channelCount(); i++)
        {
            switch (m_format.sampleSize())
            {
                case 8:
                    // Only integer samples
                    *reinterpret_cast<qint8*>(pData) = m_format.sampleType() == QAudioFormat::SignedInt ?
                                                       convertSample<qint8>(x) :
                                                       convertSample<quint8>(x);
                    break;
                case 16:
                    // Only integer samples
                    switch (m_format.sampleType())
                    {
                        case QAudioFormat::UnSignedInt:
                            convertSample<quint16>(pData, x, byteOrder);
                            break;
                        case QAudioFormat::SignedInt:
                            convertSample<qint16>(pData, x, byteOrder);
                            break;
                        default:
                            Q_ASSERT(false);
                    }
                    break;
                case 32:
                    // Ints & Floats
                    switch (m_format.sampleType())
                    {
                        case QAudioFormat::UnSignedInt:
                            convertSample<quint32>(pData, x, byteOrder);
                            break;
                        case QAudioFormat::SignedInt:
                            convertSample<qint32>(pData, x, byteOrder);
                            break;
                        case QAudioFormat::Float:
                            if (byteOrder == QAudioFormat::LittleEndian)
                                qToLittleEndian<float>(x, pData);
                            else
                                qToBigEndian<float>(x, pData);
                            break;
                        default:
                            Q_ASSERT(false);
                    }
                    break;
                default:
                    throw std::runtime_error { "24-bit formats support not implemented" };
            }

            pData += channelBytes;
            length -= channelBytes;
        }

        m_time += k_sampleInterval;
    }
}

qreal NoiseGenerator::generateSample() noexcept
{
    auto s = 0.0;
    if (m_pyArgs)
    {
        // We need random in range including both ends, i.e. [-1, 1]
        constexpr double upperBoundX = 1.0 + std::numeric_limits<double>::epsilon();
        PyTuple_SetItem(m_pyArgs, 0, PyFloat_FromDouble(m_time));
        PyTuple_SetItem(m_pyArgs, 1, PyFloat_FromDouble(m_ranGen.bounded(upperBoundX) * 2.0 - 1.0));
        PyTuple_SetItem(m_pyArgs, 2, PyFloat_FromDouble(m_prev));
        auto rv = PyObject_CallObject(m_pyGenFunc, m_pyArgs);
        if (rv)
        {
            m_prev = s = PyFloat_AsDouble(rv);
            Py_DECREF(rv);
        }
    }
    Q_ASSERT(s >= -1.0 && s <= 1.0);

    return s;
}

qint64 NoiseGenerator::readData(char *data, qint64 len)
{
    try
    {
        fillBuffer(len, data);
    }
    catch (const std::exception &ex)
    {
        setErrorString(ex.what());
        len = -1;
    }

    return len;
}

qint64 NoiseGenerator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 NoiseGenerator::bytesAvailable() const
{
    const int channelBytes = m_format.sampleSize() / 8;
    const auto capHint = (m_format.sampleRate() * m_format.channelCount() * channelBytes) * BUFFER_DUR_US / 1000000;
    Q_ASSERT(capHint % (m_format.channelCount() * channelBytes) == 0);

    return capHint;
}
