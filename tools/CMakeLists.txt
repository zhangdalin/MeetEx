cmake_minimum_required(VERSION 3.16)

add_library(avatar_tool STATIC
    avatar_generator.cpp
    avatar_generator.h
)

add_executable(avatar_generator
    avatar_generator_main.cpp
)

target_include_directories(avatar_tool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(avatar_tool
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
)

target_link_libraries(avatar_generator
    PRIVATE
        avatar_tool
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
)