#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <QIODevice>
#include <QAudioFormat>
#include <QByteArray>
#include <QRandomGenerator>

/**
 * Generates white noise suitable for QAudioDevice output.
 *
 * @todo: implement modulation.
 */
class NoiseGenerator final: public QIODevice
{
public:
    NoiseGenerator(const QAudioFormat &format, PyObject *pyGenFunc):
        m_format { format },
        k_sampleInterval { 1.0 / format.sampleRate() },
        m_pyGenFunc { pyGenFunc }
    {
        if (!format.isValid())
            throw std::runtime_error { "incorrect format" };

        if (m_pyGenFunc && PyCallable_Check(m_pyGenFunc))
            m_pyArgs = PyTuple_New(3);
    }

    ~NoiseGenerator()
    {
        Py_XDECREF(m_pyArgs);
        m_pyArgs = nullptr;
    }

    void start();
    void stop();

    int optimalBufferSize() const noexcept;

    qint64 bytesAvailable() const override;
    qint64 writeData(const char* data, qint64 len) override;
    qint64 readData(char* data, qint64 maxlen) override;
    bool isSequential() const override
    {
        return true;
    }

private:
    // Hint about optimal buffer size for the audio playback device, in microseconds
    static constexpr int BUFFER_DUR_MS = 1000;

    const QAudioFormat m_format;
    const double k_sampleInterval;

    QRandomGenerator m_ranGen = QRandomGenerator::securelySeeded();
    double m_time = 0,
           m_prev = 0;

    // Script stuff
    PyObject *m_pyGenFunc = nullptr,
             *m_pyArgs = nullptr;

    void fillBuffer(qint64 length, char *pData);

    /// @return Sample value in range [-1.0, 1.0]
    double generateSample() noexcept;
};

#endif // NOISEGENERATOR_H
