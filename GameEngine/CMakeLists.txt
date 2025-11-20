cmake_minimum_required(VERSION 3.15)
project(GameEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Automatically find all source and header files
file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

file(GLOB_RECURSE HEADER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Create executable (only SOURCE_FILES are required)
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Framework
    ${CMAKE_CURRENT_SOURCE_DIR}/Object
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene
    ${CMAKE_CURRENT_SOURCE_DIR}/States
)

# DirectX SDK include and library paths (environment variable)
if(DEFINED ENV{DXSDK_DIR})
    target_include_directories(${PROJECT_NAME} PRIVATE "$ENV{DXSDK_DIR}Include")
    target_link_directories(${PROJECT_NAME} PRIVATE "$ENV{DXSDK_DIR}Lib/x86")
endif()

# Preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32
    _UNICODE
    UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W3
        /sdl
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:Release>:/O2>
    )
    # Set subsystem to Windows for GUI application
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS"
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
    )
endif()

# Link DirectX and Windows libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d9
    d3dx9
    dxguid
    dinput8
    dsound
    dxerr
    winmm
    dxgi
    legacy_stdio_definitions
)

# Set working directory for Visual Studio
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

message(STATUS "GameEngine CMake configuration completed")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
