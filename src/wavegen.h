#ifndef WAVEGEN_H
#define WAVEGEN_H

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui
{
    class WaveGen;
}

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

private:
    QScopedPointer<Ui::WaveGen> m_ui;
};

#endif // WAVEGEN_H
