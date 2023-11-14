#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H

#include <QIODevice>
#include <QAudioFormat>
#include <QByteArray>
#include <QRandomGenerator>

/**
 * Generates white noise suitable for QAudioDevice output.
 *
 * @todo: implement modulation.
 */
class NoiseGenerator: public QIODevice
{
public:
    NoiseGenerator(const QAudioFormat &format):
        m_format { format }
    {
        if (!format.isValid())
            throw std::runtime_error { "incorrect format" };
    }

    void start();
    void stop();

    qint64 bytesAvailable() const override;
    qint64 writeData(const char* data, qint64 len) override;
    qint64 readData(char* data, qint64 maxlen) override;

private:
    static constexpr int DURATION_US = 2000000;

    const QAudioFormat m_format;

    // ring buffer
    QByteArray m_buffer;
    qint64 m_pos = 0,
           m_size = 0;
    QRandomGenerator m_ranGen = QRandomGenerator::securelySeeded();

    void fillBuffer();
};

#endif // NOISEGENERATOR_H
