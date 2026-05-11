cmake_minimum_required(VERSION 3.16)

add_library(meetingengine STATIC
    meeting_session.cpp
    meeting_participant.cpp
)

target_include_directories(meetingengine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/thirdparty/livekit/include
)

# target_link_directories(meetingengine
#     PUBLIC
#         ${CMAKE_SOURCE_DIR}/thirdparty/livekit/lib/$<IF:$<CONFIG:Debug>,Debug,Release>
# )

target_link_libraries(meetingengine
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        mediaengine
)