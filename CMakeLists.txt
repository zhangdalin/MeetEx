cmake_minimum_required(VERSION 3.16)

project(MeetEx VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES
    i18n/MeetEx_zh_CN.ts)

set(PROJECT_FORMS
    ui/home.ui
    ui/login.ui
    ui/bookmeeting.ui
    ui/joinmeeting.ui
    ui/inmeeting.ui
    ui/settings.ui
    ui/settingaudio.ui
    ui/settingvideo.ui
    ui/settingrecording.ui
    ui/settingcommon.ui
    ui/sharescreen.ui
    ui/myprofile.ui
)

set(PROJECT_HEADERS
    src/home.h
    src/login.h
    src/bookmeeting.h
    src/joinmeeting.h
    src/inmeeting.h
    src/settings.h
    src/settingaudio.h
    src/settingvideo.h
    src/settingrecording.h
    src/settingcommon.h
    src/sharescreen.h
    src/myprofile.h
)

set(PROJECT_SOURCES
    src/main.cpp
    src/home.cpp
    src/login.cpp
    src/bookmeeting.cpp
    src/joinmeeting.cpp
    src/inmeeting.cpp
    src/settings.cpp
    src/settingaudio.cpp
    src/settingvideo.cpp
    src/settingrecording.cpp
    src/settingcommon.cpp
    src/myprofile.cpp
    src/sharescreen.cpp
    ${TS_FILES}
)

set(PROJECT_RESOURCES
    res/home.qrc
)

add_subdirectory(mediaengine)
add_subdirectory(meetingengine)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MeetEx
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${PROJECT_FORMS}
        ${PROJECT_RESOURCES}

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MeetEx APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(MeetEx SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MeetEx
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(MeetEx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/meetingengine
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/livekit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL3-3.4.2/include
)

target_link_libraries(MeetEx PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    mediaengine
    meetingengine
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MeetEx)
endif()
set_target_properties(MeetEx PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MeetEx
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MeetEx)
endif()
