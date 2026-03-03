cmake_minimum_required(VERSION 3.16)
project(NetworkScanner VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Concurrent)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/ipscanner.cpp
    src/ipscannerthread.cpp
    src/scannermodel.cpp
)

set(PROJECT_HEADERS
    src/mainwindow.h
    src/ipscanner.h
    src/ipscannerthread.h
    src/scannermodel.h
    src/deviceinfo.h
)

set(PROJECT_UI
    src/mainwindow.ui
)

qt_add_executable(NetworkScanner
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
    resources.qrc
)

target_link_libraries(NetworkScanner PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

install(TARGETS NetworkScanner
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    install(FILES assets/network-scanner.desktop
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    )
    install(FILES assets/icon.svg
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
        RENAME network-scanner.svg
    )
endif()
