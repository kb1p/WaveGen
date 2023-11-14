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
    m_audioOut->stop();
    m_gen->stop();
    m_gen.reset();

    Py_XDECREF(m_pyGenFunc);
    Py_XDECREF(m_pyModule);
    Py_Finalize();
}

void WaveGen::on_btnPlay_clicked()
{
    if (m_ui->btnPlay->isChecked())
        m_audioOut->start(m_gen.data());
    else
        m_audioOut->stop();
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

void WaveGen::on_slVolume_valueChanged(int val)
{
    m_ui->txtVolume->setText(QString("%1 %").arg(val));

    const auto linearVolume = QAudio::convertVolume(val / 100.0,
                                                    QAudio::LogarithmicVolumeScale,
                                                    QAudio::LinearVolumeScale);

    m_audioOut->setVolume(linearVolume);
}

void WaveGen::on_txtFrequency_valueChanged(double val)
{
    setFrequency(val);
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
        setFrequency(m_ui->txtFrequency->value());
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

void WaveGen::setFrequency(double freqHz)
{
    if (m_pyModule)
    {
        bool ok = false;
        auto pyFreq = PyFloat_FromDouble(freqHz);
        auto pyPeriod = PyFloat_FromDouble(1.0 / freqHz);
        if (pyFreq && pyPeriod)
        {
            ok = PyObject_SetAttrString(m_pyModule, "freqHz", pyFreq) == 0 &&
                 PyObject_SetAttrString(m_pyModule, "period", pyPeriod) == 0;
        }
        Py_XDECREF(pyPeriod);
        Py_XDECREF(pyFreq);

        if (!ok)
            QMessageBox::warning(this, "Script error", "Failed to set modulation frequency");
    }
    else
        statusBar()->showMessage("Failed to set frequency: modulator is not loaded");
}

void WaveGen::initializeAudio()
{
    const auto devInf = m_ui->cbxDevice->currentData().value<QAudioDeviceInfo>();
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    if (!devInf.isFormatSupported(format))
        format = devInf.nearestFormat(format);

    const QString msg = "Sample rate: %1 Sample size: %2 Channels: %3 Codec: %4";
    statusBar()->showMessage(msg.arg(format.sampleRate()).arg(format.sampleSize()).arg(format.channelCount()).arg(format.codec()));

    // Update maximum frequency limit
    const double maxFreq = format.sampleRate() / 2.0;
    m_ui->txtFrequency->setMaximum(maxFreq);
    m_ui->txtFrequency->setStatusTip(QStringLiteral("Choose modulation frequency. Allowed range: 0 ~ %1").arg(maxFreq));

    if (m_gen)
        m_gen->stop();
    if (m_audioOut)
        m_audioOut->stop();
    m_gen.reset(new NoiseGenerator(format, m_pyGenFunc));
    m_audioOut.reset(new QAudioOutput(devInf, format));
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
