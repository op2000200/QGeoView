set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(main)

# Set the QT version
find_package(Qt6 COMPONENTS Core QUIET)
if (NOT Qt6_FOUND)
    set(QT_VERSION 5 CACHE STRING "Qt version for QGeoView")
else()
    set(QT_VERSION 6 CACHE STRING "Qt version for QGeoView")
endif()

find_package(SQLite3 REQUIRED)

find_package(Qt${QT_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Sql
)

add_executable(test
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(test
    PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Network
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Sql
    QGeoView
    qgeoview-samples-shared
    SQLite::SQLite3
)