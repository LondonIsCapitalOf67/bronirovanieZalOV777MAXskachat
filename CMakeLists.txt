cmake_minimum_required(VERSION 3.16)
project(HallBooking LANGUAGES CXX)

set(QT_VERSION 6)
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets Sql Network)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


set(SOURCES
    main.cpp
    mainwindow.cpp
    loginwindow.cpp
    adminwindow.cpp
    bookingdialog.cpp
    database.cpp
)

set(HEADERS
    mainwindow.h
    loginwindow.h
    adminwindow.h
    bookingdialog.h
    database.h
    user.h
    booking.h
    tariff.h
)

set(RESOURCES
    resources.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    calendarwidget.h calendarwidget.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Sql
    Qt${QT_VERSION}::Network
)