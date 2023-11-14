#include "wavegen.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("WaveGen");

    WaveGen w;
    w.show();

    return app.exec();
}

