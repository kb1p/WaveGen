#include <Python.h>

#include "wavegen.h"
#include "ui_wavegen.h"

#include <QMessageBox>

WaveGen::WaveGen(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::WaveGen)
{
    m_ui->setupUi(this);
}

WaveGen::~WaveGen() = default;

void WaveGen::on_btnBrowseScript_clicked()
{
    QMessageBox::information(this, "Debug", "Browse");
}

void WaveGen::on_btnPlay_clicked()
{
    if (m_ui->btnPlay->isChecked())
        QMessageBox::information(this, "Debug", "Play ON");
    else
        QMessageBox::information(this, "Debug", "Play OFF");
}

void WaveGen::on_btnReset_clicked()
{
    QMessageBox::information(this, "Debug", "Reset");
}
