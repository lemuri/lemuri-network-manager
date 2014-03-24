include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB QML_SRC qml/*.qml qml/*.js)

set(igloobox_SRCS
    ${QML_SRC}
    passworddialog.cpp
    secretagent.cpp
    trayicon.cpp
    interfacemanager.cpp
    DeviceConnectionModel.cpp
    AvailableConnectionsModel.cpp
    AvailableConnectionsSortModel.cpp
    IconProvider.cpp
    main.cpp
)

qt5_wrap_ui(igloobox_SRCS
    passworddialog.ui
)

qt5_add_resources(igloobox_SRCS
    application.qrc
    OPTIONS -compress 3 -threshold 3
)

add_executable(lemuri-nm-agent ${igloobox_SRCS})

qt5_use_modules(lemuri-nm-agent Core Quick DBus Widgets X11Extras)
target_link_libraries(lemuri-nm-agent
    ${NETWORKMANAGERQT_LDFLAGS}
    ${LIBXCB_LIBRARIES}
)

install(TARGETS lemuri-nm-agent DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
