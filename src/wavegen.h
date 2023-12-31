#ifndef WAVEGEN_H
#define WAVEGEN_H

#include "noisegenerator.h"

#include <QMainWindow>
#include <QScopedPointer>
#include <QAudioDeviceInfo>
#include <QAudioOutput>

namespace Ui
{
    class WaveGen;
}

// Application window
class WaveGen: public QMainWindow
{
    Q_OBJECT

public:
    explicit WaveGen(QWidget *parent = nullptr);
    ~WaveGen() override;

private Q_SLOTS:
    void onAudioDevStateChanged(QAudio::State newState);
    void on_btnPlay_clicked();
    void on_btnReset_clicked();
    void on_btnRefresh_clicked();
    void on_cbxDevice_activated(int idx);
    void on_cbxScript_activated(int idx);
    void on_cbxFormat_activated(int idx);
    void on_cbxFunction_activated(int idx);
    void on_slVolume_valueChanged(int val);
    void on_txtFrequency_valueChanged(double val);
    void on_txtModDepth_valueChanged(double val);

private:
    void loadModule(const QString &name = { });
    void setFunction(const QString &name);
    void setModulationParams(double freqHz, double depth);
    void scanSupportedFormats();
    void initializeAudio();

    QScopedPointer<Ui::WaveGen> m_ui;
    QScopedPointer<NoiseGenerator> m_gen;
    QScopedPointer<QAudioOutput> m_audioOut;

    PyObject *m_pyModule = nullptr,
             *m_pyGenFunc = nullptr;
};

#endif // WAVEGEN_H
