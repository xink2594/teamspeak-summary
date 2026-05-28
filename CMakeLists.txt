cmake_minimum_required(VERSION 3.10)
project(VoiceMonitorPlugin LANGUAGES C)

# SDK path - relative to this CMakeLists.txt
set(SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ts3client-pluginsdk-master")

# Plugin source files
set(PLUGIN_SOURCES
    src/plugin.c
)

# Create shared library (DLL/SO)
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

# Include SDK headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDK_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific settings
if(WIN32)
    # Windows: output as .dll
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        SUFFIX "_win64.dll"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32
        _WIN32
    )
elseif(UNIX AND NOT APPLE)
    # Linux: output as .so
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        SUFFIX "_linux_amd64.so"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    )
elseif(APPLE)
    # macOS: output as .dylib
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        SUFFIX "_mac.dylib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    )
endif()

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -O2
    -fPIC
)
