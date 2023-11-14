#include "wavegen.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WaveGen w;
    w.show();

    return app.exec();
}

