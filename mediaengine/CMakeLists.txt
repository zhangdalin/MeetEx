cmake_minimum_required(VERSION 3.16)

add_library(mediaengine STATIC
    media_util.cpp
    media_engine.cpp
    media_mgr.cpp
    media_qmic.cpp
    media_qcam.cpp
    media_qspk.cpp
    media_filewav.cpp
)

target_include_directories(mediaengine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/thirdparty/livekit/include
)

target_link_directories(mediaengine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/thirdparty/livekit/lib/$<IF:$<CONFIG:Debug>,Debug,Release>
)

target_link_libraries(mediaengine
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::Widgets
        livekit
        livekit_ffi
)