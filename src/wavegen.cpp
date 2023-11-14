#include "wavegen.h"
#include "ui_wavegen.h"

#include <QMessageBox>
#include <QDir>

WaveGen::WaveGen(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::WaveGen)
{
    m_ui->setupUi(this);

    // Enumerate available modules
    QDir dir;
    if (dir.cd("modulators"))
    {
        const auto lst = dir.entryInfoList({ "*.py" }, QDir::Files);
        for (const auto &sf: lst)
        {
            m_ui->cbxScript->addItem(sf.baseName());
        }
        m_modModule = m_ui->cbxScript->currentText();
    }

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

    initializeAudio(QAudioDeviceInfo::defaultOutputDevice());
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

void WaveGen::on_btnBrowseScript_clicked()
{
    QMessageBox::information(this, "Debug", "Browse");
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

void WaveGen::on_cbxDevice_activated(int idx)
{
    const auto inf = m_ui->cbxDevice->itemData(idx).value<QAudioDeviceInfo>();
    m_gen->stop();
    m_audioOut->stop();
    initializeAudio(inf);
}

void WaveGen::on_cbxScript_activated(int idx)
{
    m_modModule = m_ui->cbxScript->currentText();
    on_cbxDevice_activated(m_ui->cbxDevice->currentIndex());
}

void WaveGen::on_slVolume_valueChanged(int val)
{
    m_ui->txtVolume->setText(QString("%1 %").arg(val));

    const auto linearVolume = QAudio::convertVolume(val / qreal(100),
                                                    QAudio::LogarithmicVolumeScale,
                                                    QAudio::LinearVolumeScale);

    m_audioOut->setVolume(linearVolume);
}

void WaveGen::initializeAudio(const QAudioDeviceInfo &devInf)
{
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    if (!devInf.isFormatSupported(format))
    {
        QMessageBox::warning(this, "Notice", "Default format not supported - trying to use nearest");
        format = devInf.nearestFormat(format);
    }

    const QString msg = "Sample rate: %1 Sample size: %2 Channels: %3 Codec: %4";
    statusBar()->showMessage(msg.arg(format.sampleRate()).arg(format.sampleSize()).arg(format.channelCount()).arg(format.codec()));

    Py_XDECREF(m_pyGenFunc);
    Py_XDECREF(m_pyModule);
    m_pyModule = m_pyGenFunc = nullptr;
    if (!m_modModule.isEmpty())
    {
        auto pName = PyUnicode_DecodeFSDefault(m_modModule.toLocal8Bit().data());
        m_pyModule = PyImport_Import(pName);
        Py_DECREF(pName);
        if (m_pyModule)
        {
            m_pyGenFunc = PyObject_GetAttrString(m_pyModule, "modulate");
            if (!m_pyGenFunc)
                QMessageBox::warning(this, "Script error", "Failed to load function");
        }
        else
            QMessageBox::warning(this, "Script error", "Failed to load module");
    }

    m_gen.reset(new NoiseGenerator(format, m_pyGenFunc));
    m_audioOut.reset(new QAudioOutput(devInf, format));
    m_gen->start();

    const auto initialVolume = QAudio::convertVolume(m_audioOut->volume(),
                                                     QAudio::LinearVolumeScale,
                                                     QAudio::LogarithmicVolumeScale);
    m_ui->slVolume->setValue(qRound(initialVolume * 100));

    if (m_ui->btnPlay->isChecked())
        m_audioOut->start(m_gen.data());
    else
        m_audioOut->stop();
}
