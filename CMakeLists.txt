cmake_minimum_required(VERSION 3.16)
project(ulc VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Concurrent)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# ── LZMA SDK (7-zip) — compiled directly into the binary ─────────────────────
file(GLOB LZMA_SOURCES third_party/lzma/*.c)
add_library(lzma_sdk STATIC ${LZMA_SOURCES})
target_include_directories(lzma_sdk PUBLIC third_party/lzma)
target_compile_definitions(lzma_sdk PUBLIC _7ZIP_ST)
if(MSVC)
    target_compile_options(lzma_sdk PRIVATE /W0)
else()
    target_compile_options(lzma_sdk PRIVATE -w)
endif()

# ── Main executable ───────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/aboutdialog.cpp
    src/downloader.cpp
    src/archive_zip.cpp
    src/archive_7z.cpp
    src/updater.cpp
    src/settings.cpp
    src/etag_cache.cpp
    resources/resources.qrc
)

if(WIN32)
    set(SOURCES ${SOURCES} resources/app.rc)
endif()
if(WIN32)
    add_executable(ulc WIN32 ${SOURCES})
else()
    add_executable(ulc ${SOURCES})
endif()

target_link_libraries(ulc PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    OpenSSL::Crypto
    ZLIB::ZLIB
    lzma_sdk
)

target_include_directories(ulc PRIVATE src third_party/lzma)

if(WIN32)
    target_compile_definitions(ulc PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(ulc PRIVATE version)
endif()

if(MSVC)
    target_compile_options(ulc PRIVATE /W3 /utf-8)
else()
    target_compile_options(ulc PRIVATE -Wall -Wextra)
endif()
