cmake_minimum_required(VERSION 3.0)

project(WaveGen)

set(CMAKE_CXX_STANDARD_REQUIRED 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

if(WIN32)
    add_definitions(-DWIN32)
endif()

# Find the QtWidgets library
find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Python 3.5 REQUIRED COMPONENTS Development)

include_directories(${Python_INCLUDE_DIRS})

set(wavegen_SRC
    "src/main.cpp"
    "src/wavegen.cpp"
    "src/noisegenerator.cpp"
    "src/pymodule.cpp")

# Tell CMake to create the helloworld executable
add_executable(wavegen ${wavegen_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(wavegen Qt5::Widgets Qt5::Multimedia ${Python_LIBRARIES})

# Install the executable
install(TARGETS wavegen DESTINATION "bin")
install(DIRECTORY "modulators" DESTINATION "${CMAKE_INSTALL_PREFIX}")
install(PROGRAMS "start.sh" "start.bat" DESTINATION "${CMAKE_INSTALL_PREFIX}")
