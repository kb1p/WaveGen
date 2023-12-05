#include "wavegen.h"
#include "ui_wavegen.h"

#include <QMessageBox>
#include <QDir>

WaveGen::WaveGen(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::WaveGen)
{
    m_ui->setupUi(this);

    Py_Initialize();
    PySys_SetPath(L"modulators");

    // Enumerate available audio devices and fill combo box
    const auto &defaultDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    m_ui->cbxDevice->addItem(defaultDeviceInfo.deviceName(), QVariant::fromValue(defaultDeviceInfo));
    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        if (deviceInfo != defaultDeviceInfo)
            m_ui->cbxDevice->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }
    scanSupportedFormats();

    // Enumerate available modules
    QDir dir;
    if (dir.cd("modulators"))
    {
        const auto lst = dir.entryInfoList({ "*.py" }, QDir::Files);
        for (const auto &sf: lst)
        {
            m_ui->cbxScript->addItem(sf.baseName());
        }
        loadModule(m_ui->cbxScript->currentText());
    }
    else
        initializeAudio();
}

WaveGen::~WaveGen()
{
    if (m_audioOut)
        m_audioOut->stop();
    if (m_gen)
    {
        m_gen->stop();
        m_gen.reset();
    }

    Py_XDECREF(m_pyGenFunc);
    Py_XDECREF(m_pyModule);
    Py_Finalize();
}

void WaveGen::onAudioDevStateChanged(QAudio::State newState)
{
    if (newState == QAudio::StoppedState && m_audioOut->error() != QAudio::NoError)
    {
        QMessageBox::critical(this,
                              QStringLiteral("Fatal error"),
                              m_gen ? QStringLiteral("Waveform generation aborted: %1").arg(m_gen->errorString()) :
                                      QStringLiteral("Waveform generation aborted, unknown error"));
    }
}

void WaveGen::on_btnPlay_clicked()
{
    if (m_audioOut)
    {
        if (m_ui->btnPlay->isChecked())
            m_audioOut->start(m_gen.data());
        else
            m_audioOut->stop();
    }
}

void WaveGen::on_btnReset_clicked()
{
    QMessageBox::information(this, "Debug", "Reset");
}

void WaveGen::on_btnRefresh_clicked()
{
    // reload
    loadModule();
}

void WaveGen::on_cbxDevice_activated(int idx)
{
    scanSupportedFormats();
    initializeAudio();
}

void WaveGen::on_cbxScript_activated(int idx)
{
    loadModule(m_ui->cbxScript->currentText());
}

void WaveGen::on_cbxFunction_activated(int idx)
{
    setFunction(m_ui->cbxFunction->currentText());
}

void WaveGen::on_cbxFormat_activated(int idx)
{
    initializeAudio();
}

void WaveGen::on_slVolume_valueChanged(int val)
{
    if (m_audioOut)
    {
        m_ui->txtVolume->setText(QString("%1 %").arg(val));

        const auto linearVolume = QAudio::convertVolume(val / 100.0,
                                                        QAudio::LogarithmicVolumeScale,
                                                        QAudio::LinearVolumeScale);

        m_audioOut->setVolume(linearVolume);
    }
}

void WaveGen::on_txtFrequency_valueChanged(double val)
{
    setModulationParams(val, m_ui->txtModDepth->value());
}

void WaveGen::on_txtModDepth_valueChanged(double val)
{
    setModulationParams(m_ui->txtFrequency->value(), val);
}

void WaveGen::loadModule(const QString &name)
{
    const bool reload = name.isEmpty() && m_pyModule != nullptr;

    Py_XDECREF(m_pyGenFunc);
    m_pyGenFunc = nullptr;
    if (!reload)
    {
        Py_XDECREF(m_pyModule);
        m_pyModule = nullptr;
    }

    if (reload)
        m_pyModule = PyImport_ReloadModule(m_pyModule);
    else
    {
        auto pName = PyUnicode_DecodeFSDefault(name.toLocal8Bit().data());
        m_pyModule = PyImport_Import(pName);
        Py_XDECREF(pName);
    }

    // Enumerate functions
    if (m_pyModule)
    {
        m_ui->cbxFunction->clear();
        auto d = PyModule_GetDict(m_pyModule);
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(d, &pos, &key, &value))
        {
            if (PyCallable_Check(value))
            {
                auto name = PyUnicode_AsUTF8(key);
                m_ui->cbxFunction->addItem(name);
            }
        }
        setModulationParams(m_ui->txtFrequency->value(),
                            m_ui->txtModDepth->value());
        setFunction(m_ui->cbxFunction->currentText());
    }
    else
    {
        QMessageBox::warning(this, "Script error", reload ? "Failed to reload module" : "Failed to load module " + name);
        PyErr_Print();
    }
}

void WaveGen::setFunction(const QString& name)
{
    Py_XDECREF(m_pyGenFunc);
    m_pyGenFunc = PyObject_GetAttrString(m_pyModule, name.toLocal8Bit().data());
    if (!m_pyGenFunc)
        QMessageBox::warning(this, "Script error", "Failed to load function " + name);

    initializeAudio();
}

void WaveGen::setModulationParams(double freqHz, double depth)
{
    if (m_pyModule)
    {
        bool ok = false;
        auto pyFreq = PyFloat_FromDouble(freqHz);
        auto pyPeriod = PyFloat_FromDouble(1.0 / freqHz);
        auto pyDepth = PyFloat_FromDouble(depth / 100.0);
        if (pyFreq && pyPeriod && pyDepth)
        {
            ok = PyObject_SetAttrString(m_pyModule, "FREQ_HZ", pyFreq) == 0 &&
                 PyObject_SetAttrString(m_pyModule, "PERIOD", pyPeriod) == 0 &&
                 PyObject_SetAttrString(m_pyModule, "DEPTH", pyDepth) == 0;
        }
        Py_XDECREF(pyPeriod);
        Py_XDECREF(pyFreq);
        Py_XDECREF(pyDepth);

        if (!ok)
            QMessageBox::warning(this, "Script error", "Failed to set modulation frequency");
    }
    else
        statusBar()->showMessage("Failed to set frequency: modulator is not loaded");
}

QString format2str(const QAudioFormat &fmt) noexcept
{
    QString t;
    switch (fmt.sampleType())
    {
        case QAudioFormat::Float:
            t = QStringLiteral("real");
            break;
        case QAudioFormat::SignedInt:
            t = QStringLiteral("signed");
            break;
        case QAudioFormat::UnSignedInt:
            t = QStringLiteral("unsigned");
            break;
        default:
            t = QStringLiteral("unknown");
    }

    return QStringLiteral("%1Hz %2bit %3").arg(fmt.sampleRate()).arg(fmt.sampleSize()).arg(t);
}

void WaveGen::scanSupportedFormats()
{
    m_ui->cbxFormat->clear();
    if (m_ui->cbxDevice->currentIndex() < 0)
        return;

    const auto devInf = m_ui->cbxDevice->currentData().value<QAudioDeviceInfo>();
    const auto freqencies = devInf.supportedSampleRates();
    const auto sizes = devInf.supportedSampleSizes();
    const auto types = devInf.supportedSampleTypes();
    const auto byteOrders = devInf.supportedByteOrders();

    // We limit channel count to 1 (mono) and codec to linear PCM
    QAudioFormat defFmt = devInf.preferredFormat(),
                 curFmt;
    defFmt.setChannelCount(1);
    curFmt.setChannelCount(1);
    curFmt.setByteOrder(QAudioFormat::LittleEndian);
    curFmt.setCodec(QStringLiteral("audio/pcm"));
    int index = 0;
    for (auto f: freqencies)
    {
        curFmt.setSampleRate(f);
        for (auto s: sizes)
        {
            if (s != 8 && s != 16 && s != 32)
                continue;

            curFmt.setSampleSize(s);
            for (auto t: types)
            {
                curFmt.setSampleType(t);
                if (devInf.isFormatSupported(curFmt))
                {
                    m_ui->cbxFormat->addItem(format2str(curFmt), QVariant::fromValue(curFmt));
                    if (curFmt == defFmt)
                        m_ui->cbxFormat->setCurrentIndex(index);
                    index++;
                }
            }
        }
    }
}

void WaveGen::initializeAudio()
{
    if (m_ui->cbxDevice->currentIndex() < 0)
    {
        QMessageBox::critical(this,
                              QStringLiteral("Fatal error"),
                              QStringLiteral("Invalid device is selected. Wave generation aborted."));
        return;
    }
    if (m_ui->cbxFormat->currentIndex() < 0)
    {
        QMessageBox::critical(this,
                              QStringLiteral("Fatal error"),
                              QStringLiteral("Incorrect format is chosen. Wave generation aborted."));
        return;
    }

    const auto devInf = m_ui->cbxDevice->currentData().value<QAudioDeviceInfo>();
    const auto format = m_ui->cbxFormat->currentData().value<QAudioFormat>();

    const QString msg = "Sample rate: %1 Sample size: %2 Channels: %3 Codec: %4";
    statusBar()->showMessage(msg.arg(format.sampleRate()).arg(format.sampleSize()).arg(format.channelCount()).arg(format.codec()));

    // Update maximum frequency limit
    const double maxFreq = format.sampleRate() / 2.0;
    m_ui->txtFrequency->setMaximum(maxFreq);
    m_ui->txtFrequency->setStatusTip(QStringLiteral("Choose modulation frequency. Allowed range: 0 ~ %1").arg(maxFreq));

    try
    {
        if (m_gen)
            m_gen->stop();
        if (m_audioOut)
        {
            m_audioOut->stop();
            disconnect(m_audioOut.data(), nullptr, nullptr, nullptr);
        }
        m_gen.reset(new NoiseGenerator(format, m_pyGenFunc));
        m_audioOut.reset(new QAudioOutput(devInf, format));
        connect(m_audioOut.data(), &QAudioOutput::stateChanged,
                this, &WaveGen::onAudioDevStateChanged);
        m_gen->start();

        const auto linearVolume = QAudio::convertVolume(m_ui->slVolume->value() / 100.0,
                                                        QAudio::LogarithmicVolumeScale,
                                                        QAudio::LinearVolumeScale);

        m_audioOut->setVolume(linearVolume);

        if (m_ui->btnPlay->isChecked())
            m_audioOut->start(m_gen.data());
        else
            m_audioOut->stop();
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this,
                              QStringLiteral("Fatal error"),
                              QStringLiteral("Error initializing audio output: %1").arg(err.what()));
    }
}
