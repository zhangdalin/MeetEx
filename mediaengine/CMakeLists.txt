cmake_minimum_required(VERSION 3.16)

add_library(mediaengine STATIC
    media_engine.cpp
    media_mgr.cpp
    media.cpp
    video_renderer.cpp
    wav_audio_source.cpp
    fallback_capture.cpp
)

target_include_directories(mediaengine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/thirdparty/livekit/include
        ${CMAKE_SOURCE_DIR}/thirdparty/SDL3-3.4.2/include
)

target_link_directories(mediaengine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/thirdparty/livekit/lib/$<IF:$<CONFIG:Debug>,Debug,Release>
        ${CMAKE_SOURCE_DIR}/thirdparty/SDL3-3.4.2/lib/x64
)

target_link_libraries(mediaengine
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        livekit
        livekit_ffi
        SDL3
)