cmake_minimum_required(VERSION 3.1)

#set project name and language
project(sec1000d LANGUAGES CXX)

#Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#C++ 11 Flag
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Set project Version
set(sec1000d_VERSION_MAJOR "0")
set(sec1000d_VERSION_MINOR "0")
set(sec1000d_VERSION_PATCH "1")

set(PROJECT_VERSION "${sec1000d_VERSION_MAJOR}.${sec1000d_VERSION_MINOR}.${sec1000d_VERSION_PATCH}")



option(useSystemd "Using libsystemd" ON)



#Adding cmake functions like feature_summary()
include(FeatureSummary)
include(GNUInstallDirs)

#Find dependecies
find_package(Qt5 COMPONENTS Gui Xml Network CONFIG REQUIRED)
find_package(SCPI REQUIRED)
find_package(xiqnet REQUIRED)
find_package(zeramisc REQUIRED)
find_package(zeraxmlconfig REQUIRED)
find_package(zeramath REQUIRED)
find_package(VfProtobuf REQUIRED)
find_package(zeraprotobuf REQUIRED)

if(${useSystemd})
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Systemd REQUIRED libsystemd)
    add_definitions(-DSYSTEMD_NOTIFICATION)
endif()




#sum up project Files 
set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/sec1000d.cpp
    ${PROJECT_SOURCE_DIR}/pcbserver.cpp
    ${PROJECT_SOURCE_DIR}/ethsettings.cpp
    ${PROJECT_SOURCE_DIR}/fpgasettings.cpp
    ${PROJECT_SOURCE_DIR}/scpidelegate.cpp
    ${PROJECT_SOURCE_DIR}/statusinterface.cpp
    ${PROJECT_SOURCE_DIR}/scpiconnection.cpp
    ${PROJECT_SOURCE_DIR}/systeminterface.cpp
    ${PROJECT_SOURCE_DIR}/debugsettings.cpp
    ${PROJECT_SOURCE_DIR}/systeminfo.cpp
    ${PROJECT_SOURCE_DIR}/resource.cpp
    ${PROJECT_SOURCE_DIR}/rmconnection.cpp
    ${PROJECT_SOURCE_DIR}/sec1000dprotobufwrapper.cpp
    ${PROJECT_SOURCE_DIR}/protonetcommand.cpp
    ${PROJECT_SOURCE_DIR}/ecalcsettings.cpp
    ${PROJECT_SOURCE_DIR}/ecalcinterface.cpp
    ${PROJECT_SOURCE_DIR}/ecalcchannel.cpp
    ${PROJECT_SOURCE_DIR}/notificationvalue.cpp
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
    ${PROJECT_SOURCE_DIR}/scpidelegate.h
    ${PROJECT_SOURCE_DIR}/statusinterface.h
    ${PROJECT_SOURCE_DIR}/scpiconnection.h
    ${PROJECT_SOURCE_DIR}/systeminterface.h
    ${PROJECT_SOURCE_DIR}/xmlsettings.h
    ${PROJECT_SOURCE_DIR}/debugsettings.h
    ${PROJECT_SOURCE_DIR}/resource.h
    ${PROJECT_SOURCE_DIR}/systeminfo.h
    ${PROJECT_SOURCE_DIR}/rmconnection.h
    ${PROJECT_SOURCE_DIR}/sec1000dprotobufwrapper.h
    ${PROJECT_SOURCE_DIR}/notificationdata.h
    ${PROJECT_SOURCE_DIR}/protonetcommand.h
    ${PROJECT_SOURCE_DIR}/ecalcsettings.h
    ${PROJECT_SOURCE_DIR}/ecalcinterface.h
    ${PROJECT_SOURCE_DIR}/ecalcchannel.h
    ${PROJECT_SOURCE_DIR}/notificationvalue.h
    ${PROJECT_SOURCE_DIR}/inputsettings.h 
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

set(OTHER_FILES 
    ${PROJECT_SOURCE_DIR}/sec1000d.xsd
    ${PROJECT_SOURCE_DIR}/sec1000d.xml
)

#create executable
add_executable(sec1000d
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )


if(${Systemd_FOUND})
    target_link_libraries(sec1000d
        PRIVATE
        systemd
    )
endif()

#link dependencies
target_link_libraries(sec1000d
    PRIVATE
    Qt5::Xml
    Qt5::Network
    Qt5::Gui
    Zera::SCPI 
    VeinMeta::xiqnet 
    ZeraClasses::zeramisc 
    ZeraClasses::zeraxmlconfig 
    ZeraClasses::zeramath 
    VeinMeta::VfProtobuf 
    VeinMeta::zeraprotobuf 
    )	





# announce headers - target perspective
target_include_directories(sec1000d
    PUBLIC
    $<BUILD_INTERFACE:${MY_LIBRARY_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/sec1000d>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )



#install sec1000dto bin 
install(TARGETS sec1000d
    EXPORT sec1000dExport
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

install(
    FILES ${OTHER_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/zera/sec1000d
    )


# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)






