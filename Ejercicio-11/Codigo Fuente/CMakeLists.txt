cmake_minimum_required(VERSION 3.16)
project(Ejercicio09_CoordenadasBD VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_add_executable(Ejercicio09_CoordenadasBD
    src/main.cpp
    src/database.cpp
    src/database.h
    src/logindialog.cpp
    src/logindialog.h
    src/logindialog.ui
    src/mainwindow.cpp
    src/mainwindow.h
    src/pintura.cpp
    src/pintura.h
    resources/resources.qrc
)

target_link_libraries(Ejercicio09_CoordenadasBD PRIVATE Qt6::Widgets Qt6::Sql)

if(WIN32)
    set_target_properties(Ejercicio09_CoordenadasBD PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS Ejercicio09_CoordenadasBD
    RUNTIME DESTINATION bin
)
