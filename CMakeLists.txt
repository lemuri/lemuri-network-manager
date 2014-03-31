project(lemuri-network-manager)

cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules;${CMAKE_MODULE_PATH}")

include(FindPkgConfig)

find_package(Qt5 COMPONENTS
    Core
    DBus
    Quick
    Widgets
    Qml
    X11Extras
)

# XCB
find_package(XCB REQUIRED)

set(CMAKE_AUTOMOC ON)

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Core_DEFINITIONS})

# Add compiler flags for building executables (-fPIE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"" -std=gnu++11)

if (DISABLE_MODEMMANAGER_SUPPORT)
    message(STATUS "Disabling ModemManager support")
    set(WITH_MODEMMANAGER_SUPPORT 0)
else()
    pkg_check_modules(MODEMMANAGERQT ModemManagerQt5)
    if (MODEMMANAGERQT_FOUND)
        message(STATUS "Enabling ModemManager support")
        set(WITH_MODEMMANAGER_SUPPORT 1)

        if (${MODEMMANAGERQT_VERSION} VERSION_GREATER 0.7.990)
            set(MODEMMANAGERQT_ONE 1)
        endif()
    else()
        message(STATUS "ModemManagerQt not found")
        set(WITH_MODEMMANAGER_SUPPORT 0)
    endif()
endif()

pkg_check_modules(NETWORKMANAGERQT REQUIRED NetworkManagerQt5)
if (NOT NETWORKMANAGERQT_FOUND)
    message(FATAL_ERROR "ERROR: NetworkManagerQt5 not found. http://projects.kde.org/libnm-qt")
endif (NOT NETWORKMANAGERQT_FOUND)

set(LEMURI_NM_VERSION "0.1.0")

configure_file(config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h )

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})

pkg_check_modules(NETWORKMANAGER REQUIRED NetworkManager)

add_definitions(
    -DWITH_MODEMMANAGER_SUPPORT=${WITH_MODEMMANAGER_SUPPORT}
    ${NETWORKMANAGERQT_CFLAGS_OTHER}
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${MODEMMANAGER_INCLUDE_DIRS}
    if (MODEMMANAGERQT_FOUND)
        ${MODEMMANAGERQT_INCLUDE_DIRS}
    endif()
    ${NETWORKMANAGERQT_INCLUDE_DIRS}
    ${NETWORKMANAGER_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(session-agent)
