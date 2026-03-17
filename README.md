# libretro Updater — Qt6 Port

Qt6 C++ port of the original C# WinForms libretro updater.
Builds on Windows (MSVC) and Linux (GCC/Clang).

## Dependencies

| Dependency     | How to get it                                              |
|----------------|------------------------------------------------------------|
| Qt 6.7.x       | https://www.qt.io/download-qt-installer (msvc2019_64)      |
| OpenSSL        | vcpkg: `vcpkg install openssl:x64-windows`                 |
| zlib           | vcpkg: `vcpkg install zlib:x64-windows`                    |
| LZMA SDK       | See below — must be copied manually                        |

## LZMA SDK Setup (required — do this first!)

1. Download the LZMA SDK from: https://www.7-zip.org/sdk.html
2. Extract the downloaded archive
3. Inside it, find the `C\` subfolder
4. Copy ALL files from that `C\` folder into:

       ulc-qt/third_party/lzma/

   You should end up with files like:
       third_party/lzma/7z.h
       third_party/lzma/7zAlloc.c
       third_party/lzma/LzmaDec.c
       ... etc.

## Building in Visual Studio 2022

### Prerequisites
- Visual Studio 2022 with "Desktop development with C++" workload
- CMake tools component (included with the workload)
- Qt 6.7.3 installed to C:\Qt\6.7.3\msvc2019_64
- vcpkg installed at C:\vcpkg

### 1. Install vcpkg dependencies
Open a Developer Command Prompt for VS 2022:

    C:\vcpkg\vcpkg install openssl:x64-windows zlib:x64-windows

### 2. Edit CMakeUserPresets.json (if your paths differ)
Open CMakeUserPresets.json and update:
- CMAKE_TOOLCHAIN_FILE  -> path to your vcpkg toolchain file
- CMAKE_PREFIX_PATH     -> path to your Qt6 msvc2019_64 folder

### 3. Open in Visual Studio
- File -> Open -> Folder -> select the ulc-qt folder
- In the configuration dropdown, select "Windows MSVC Release"
- Build -> Build All (Ctrl+Shift+B)

### 4. Deploy Qt DLLs
After building, run from a Developer Command Prompt:

    C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe build\Release\ulc.exe

## Building on Linux

    sudo apt install cmake build-essential qt6-base-dev libssl-dev zlib1g-dev
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j$(nproc)

## Icon
Replace resources/icons/ulc.png with your own 64x64 PNG icon.
The current placeholder is a simple green square.

## Project Structure

    ulc-qt/
    ├── CMakeLists.txt          Build system
    ├── CMakeUserPresets.json   VS2022 preset (edit paths here)
    ├── resources/
    │   ├── resources.qrc       Qt resource file
    │   └── icons/ulc.png       Application icon (replace with yours)
    ├── third_party/
    │   └── lzma/               LZMA SDK goes here (download separately)
    └── src/
        ├── constants.h         URLs and tuning constants
        ├── settings.h/.cpp     JSON settings load/save
        ├── etag_cache.h/.cpp   ETag persistence (SHA256-keyed files)
        ├── downloader.h/.cpp   Async HTTP downloader with ETag support
        ├── archive_zip.h/.cpp  ZIP extractor (zlib, no extra dependency)
        ├── archive_7z.h/.cpp   7z extractor (LZMA SDK)
        ├── updater.h/.cpp      Core update logic (worker thread)
        ├── aboutdialog.h/.cpp  About dialog
        ├── mainwindow.h/.cpp   Main UI
        └── main.cpp            Entry point
