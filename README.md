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
#### Linux
1. Make sure Python 3.5+ package is installed.
2. Run `start.sh`

#### Windows
1. Make sure Python 3.5+ interpreter's location is added to the system PATH variable **or** extract contents of the [Python embedded archive](https://www.python.org/ftp/python/3.8.10/python-3.8.10-embed-amd64.zip) into `bin` subdirectory of WaveGen installation directory.
2. Run`start.bat`.

### Safety warning
Take care of you hearing: don't launch signals at maximum volume, better start at lower values and increase it per demand during signal generation.

### References
Based on Qt's [Audio Output Example](https://doc.qt.io/qt-5/qtmultimedia-multimedia-audiooutput-example.html).

Reference for modulation functions at 220Hz can be found on Wikipedia ("Triangle Wave", "Sawtooth Wave" etc.).
