cmake_minimum_required(VERSION 3.16)

project(ulc VERSION 1.0 LANGUAGES CXX C)

# Set C++ standard early and with proper MSVC flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CRITICAL: Enable MSVC standards conformance for Qt6 compatibility
if(MSVC)
    # /permissive- enables standards conformance (required for Qt6 templates)
    # /Zc:inline enables inline function removal (C++17 feature)
    string(APPEND CMAKE_CXX_FLAGS " /permissive- /Zc:inline")
endif()

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)  # Disable unless using .ui files

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Network 
    Concurrent
)

find_package(OpenSSL)
if(NOT OPENSSL_FOUND)
    message(WARNING "OpenSSL not found. Install via vcpkg or set OPENSSL_ROOT_DIR")
    if(NOT DEFINED OPENSSL_ROOT_DIR)
        if(EXISTS "C:/Program Files/OpenSSL-Win64")
            set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
        elseif(EXISTS "C:/Program Files (x86)/OpenSSL-Win32")
            set(OPENSSL_ROOT_DIR "C:/Program Files (x86)/OpenSSL-Win32")
        endif()
    endif()
    find_package(OpenSSL REQUIRED)
endif()

find_package(ZLIB REQUIRED)

# LZMA SDK library (C library, separate from C++ app)
file(GLOB LZMA_SOURCES third_party/lzma/*.c)
add_library(lzma_sdk STATIC ${LZMA_SOURCES})
target_include_directories(lzma_sdk PUBLIC third_party/lzma)
target_compile_definitions(lzma_sdk PUBLIC _7ZIP_ST)

if(MSVC)
    target_compile_options(lzma_sdk PRIVATE /W0)
else()
    target_compile_options(lzma_sdk PRIVATE -w)
endif()

# Main application
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/dashboard_tab.cpp
    src/dashboard_tab.h
    src/sidebar_delegate.cpp
    src/sidebar_delegate.h
    src/retroarch_tab.cpp
    src/retroarch_tab.h
    src/emulator_tab.cpp
    src/emulator_tab.h
    src/github_updater.cpp
    src/github_updater.h
    src/aboutdialog.cpp
    src/aboutdialog.h
    src/downloader.cpp
    src/downloader.h
    src/archive_zip.cpp
    src/archive_zip.h
    src/archive_7z.cpp
    src/archive_7z.h
    src/updater.cpp
    src/updater.h
    src/settings.cpp
    src/settings.h
    src/etag_cache.cpp
    src/etag_cache.h
    resources/resources.qrc
)

if(WIN32)
    list(APPEND SOURCES resources/app.rc)
    add_executable(emulator_updater WIN32 ${SOURCES})
else()
    add_executable(emulator_updater ${SOURCES})
endif()

# Link libraries
target_link_libraries(emulator_updater PRIVATE
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network 
    Qt6::Concurrent
    OpenSSL::Crypto 
    ZLIB::ZLIB 
    lzma_sdk
)

# Include directories
target_include_directories(emulator_updater PRIVATE 
    src 
    third_party/lzma
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(emulator_updater PRIVATE 
        NOMINMAX 
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
    target_link_libraries(emulator_updater PRIVATE version)
endif()

# Compiler-specific flags
if(MSVC)
    # Apply C++17 standards conformance to main executable as well
    target_compile_options(emulator_updater PRIVATE 
        /W3          # Warning level 3
        /utf-8       # UTF-8 source/execution charset
        /permissive- # Standards conformance
        /Zc:inline   # C++17 inline removal
    )
else()
    target_compile_options(emulator_updater PRIVATE 
        -Wall 
        -Wextra
    )
endif()