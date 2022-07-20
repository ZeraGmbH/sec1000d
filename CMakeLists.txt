cmake_minimum_required(VERSION 3.1)

project(sec1000d LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(sec1000d_VERSION_MAJOR "0")
set(sec1000d_VERSION_MINOR "0")
set(sec1000d_VERSION_PATCH "1")
set(PROJECT_VERSION "${sec1000d_VERSION_MAJOR}.${sec1000d_VERSION_MINOR}.${sec1000d_VERSION_PATCH}")

option(useSystemd "Using libsystemd" ON)

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 COMPONENTS Gui Xml Network CONFIG REQUIRED)
find_package(SCPI REQUIRED)
find_package(scpi-tcp REQUIRED)
find_package(xiqnet REQUIRED)
find_package(zeramisc REQUIRED)
find_package(zeraxmlconfig REQUIRED)
find_package(zeramath REQUIRED)
find_package(zeraservercommon REQUIRED)
find_package(VfProtobuf REQUIRED)
find_package(zeraprotobuf REQUIRED)

if(${useSystemd})
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Systemd REQUIRED libsystemd)
    add_definitions(-DSYSTEMD_NOTIFICATION)
endif()

set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/sec1000d.cpp
    ${PROJECT_SOURCE_DIR}/pcbserver.cpp
    ${PROJECT_SOURCE_DIR}/ethsettings.cpp
    ${PROJECT_SOURCE_DIR}/fpgasettings.cpp
    ${PROJECT_SOURCE_DIR}/statusinterface.cpp
    ${PROJECT_SOURCE_DIR}/systeminterface.cpp
    ${PROJECT_SOURCE_DIR}/debugsettings.cpp
    ${PROJECT_SOURCE_DIR}/systeminfo.cpp
    ${PROJECT_SOURCE_DIR}/resource.cpp
    ${PROJECT_SOURCE_DIR}/ecalcsettings.cpp
    ${PROJECT_SOURCE_DIR}/ecalcinterface.cpp
    ${PROJECT_SOURCE_DIR}/ecalcchannel.cpp
    ${PROJECT_SOURCE_DIR}/inputsettings.cpp
    )

set(HEADER
    ${PROJECT_SOURCE_DIR}/zeraglobal.h
    ${PROJECT_SOURCE_DIR}/sec1000d.h
    ${PROJECT_SOURCE_DIR}/sec1000dscpi.h
    ${PROJECT_SOURCE_DIR}/pcbserver.h
    ${PROJECT_SOURCE_DIR}/sec1000dglobal.h
    ${PROJECT_SOURCE_DIR}/ethsettings.h
    ${PROJECT_SOURCE_DIR}/fpgasettings.h
    ${PROJECT_SOURCE_DIR}/statusinterface.h
    ${PROJECT_SOURCE_DIR}/systeminterface.h
    ${PROJECT_SOURCE_DIR}/debugsettings.h
    ${PROJECT_SOURCE_DIR}/resource.h
    ${PROJECT_SOURCE_DIR}/systeminfo.h
    ${PROJECT_SOURCE_DIR}/notificationdata.h
    ${PROJECT_SOURCE_DIR}/ecalcsettings.h
    ${PROJECT_SOURCE_DIR}/ecalcinterface.h
    ${PROJECT_SOURCE_DIR}/ecalcchannel.h
    ${PROJECT_SOURCE_DIR}/inputsettings.h 
    )

set(OTHER_FILES 
    ${PROJECT_SOURCE_DIR}/sec1000d.xsd
    ${PROJECT_SOURCE_DIR}/sec1000d.xml
)

add_executable(sec1000d
    ${SOURCES}
    ${HEADER}
    )

if(${Systemd_FOUND})
    target_link_libraries(sec1000d
        PRIVATE
        systemd
    )
endif()

target_link_libraries(sec1000d
    PRIVATE
    Qt5::Xml
    Qt5::Network
    Qt5::Gui
    Zera::SCPI
    VeinMeta::xiqnet
    Zera::scpi-tcp
    ZeraClasses::zeramisc 
    ZeraClasses::zeraxmlconfig 
    ZeraClasses::zeramath 
    ZeraClasses::zeraservercommon
    VeinMeta::VfProtobuf
    VeinMeta::zeraprotobuf 
    )

target_include_directories(sec1000d
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

install(TARGETS sec1000d
    EXPORT sec1000dExport
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

install(
    FILES ${OTHER_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/zera/sec1000d
    )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
