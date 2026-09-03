cmake_minimum_required(VERSION 3.16)

project(WiFiCardGenerator
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    qrcodegen
    GIT_REPOSITORY https://github.com/nayuki/QR-Code-generator.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(qrcodegen)

add_executable(WiFiCardGenerator WIN32
    src/main.cpp
    src/WifiQrGenerator.cpp
    src/WifiQrGenerator.h
    src/WifiCardWindow.cpp
    src/WifiCardWindow.h
    ${qrcodegen_SOURCE_DIR}/cpp/qrcodegen.cpp
    ${qrcodegen_SOURCE_DIR}/cpp/qrcodegen.hpp
)

target_include_directories(WiFiCardGenerator PRIVATE
    src
    ${qrcodegen_SOURCE_DIR}/cpp
)

if(MSVC)
    target_compile_options(WiFiCardGenerator PRIVATE /W4 /utf-8)
endif()

target_link_libraries(WiFiCardGenerator PRIVATE
    user32
    gdi32
    comdlg32
)
