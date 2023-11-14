## WaveGen - scriptable waveform signal generator

### Prerequisites
You need Qt5, CMake 2.8.11 or higher, Python 3.5 or higher.

### Building
```
$ mkdir build ; cd build
$ cmake ..
$ cmake --build . --target install
```

### Running
Simply run `start.sh` (Unix) or `start.bat` (Windows). Or run executable named `waveform[.exe]` from `bin` subdirectory, but make sure `modulators` dir is located (or linked) in the current launch directory.

### Safety warning
Take care of you hearing: don't launch signals at maximum volume, better start at lower values and increase it per demand during signal generation.

### References
Based on Qt's [Audio Output Example](https://doc.qt.io/qt-5/qtmultimedia-multimedia-audiooutput-example.html).

Reference for modulation functions at 220Hz can be found on Wikipedia ("Triangle Wave", "Sawtooth Wave" etc.).
