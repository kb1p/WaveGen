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
    void on_btnBrowseScript_clicked();
    void on_btnPlay_clicked();
    void on_btnReset_clicked();
    void on_cbxDevice_activated(int idx);
    void on_slVolume_valueChanged(int val);

private:
    void initializeAudio(const QAudioDeviceInfo &devInf);

    QScopedPointer<Ui::WaveGen> m_ui;
    QScopedPointer<NoiseGenerator> m_gen;
    QScopedPointer<QAudioOutput> m_audioOut;
};

#endif // WAVEGEN_H
