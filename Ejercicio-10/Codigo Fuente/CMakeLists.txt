cmake_minimum_required(VERSION 3.16)
project(Ejercicio08_EditorMultilenguaje LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
    src/main.cpp
    src/appconfig.h
    src/appconfig.cpp
    src/logger.h
    src/logger.cpp
    src/pantalla.h
    src/pantalla.cpp
    src/validadores.h
    src/validadores.cpp
    src/login_screen.h
    src/login_screen.cpp
    src/blocked_screen.h
    src/blocked_screen.cpp
    src/editor_screen.h
    src/editor_screen.cpp
    src/application_controller.h
    src/application_controller.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
