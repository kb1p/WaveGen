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
class NoiseGenerator: public QIODevice
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
            m_pyArgs = PyTuple_New(2);
    }

    ~NoiseGenerator()
    {
        Py_XDECREF(m_pyArgs);
        m_pyArgs = nullptr;
    }

    void start();
    void stop();

    qint64 bytesAvailable() const override;
    qint64 writeData(const char* data, qint64 len) override;
    qint64 readData(char* data, qint64 maxlen) override;

private:
    static constexpr int DURATION_US = 2000000;

    const QAudioFormat m_format;
    const double k_sampleInterval;

    // ring buffer
    QByteArray m_buffer;
    qint64 m_pos = 0,
           m_size = 0;
    QRandomGenerator m_ranGen = QRandomGenerator::securelySeeded();
    double m_time = 0;

    // Script stuff
    PyObject *m_pyGenFunc = nullptr,
             *m_pyArgs = nullptr;

    void fillBuffer();
    double generateSample();
};

#endif // NOISEGENERATOR_H
