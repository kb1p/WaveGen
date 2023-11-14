#include "wavegen.h"
#include "ui_wavegen.h"

WaveGen::WaveGen(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::WaveGen)
{
    m_ui->setupUi(this);
}

WaveGen::~WaveGen() = default;
