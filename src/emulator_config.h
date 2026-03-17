#pragma once
#include <QString>
#include <QList>
#include <QDir>

enum class ArchiveType { Zip, SevenZ, SingleFile };
enum class UpdateSource { GitHub };
enum class ReleaseChannel { Stable, Nightly };

struct EmulatorConfig {
    QString        id;
    QString        displayName;
    UpdateSource   source;
    QString        githubRepo;
    // Two asset patterns — used based on selected channel.
    // If nightlyAssetPattern is empty, stableAssetPattern is used for both.
    QString        stableAssetPattern;
    QString        nightlyAssetPattern;
    ArchiveType    archiveType;
    QString        defaultInstallPath;
    QString        exeName;
    bool           stripTopLevelDir;
    // Which channel to default to when first added
    ReleaseChannel defaultChannel;
};

inline QList<EmulatorConfig> allEmulatorConfigs()
{
#ifdef Q_OS_WIN
#  define EMU_BASE R"(C:\Emulators\)"
#else
#  define EMU_BASE (QDir::homePath() + "/Emulators/").toStdString().c_str()
#endif

    return {
        {
            "pcsx2", "PCSX2  (PS2)",
            UpdateSource::GitHub, "PCSX2/pcsx2",
            R"(PCSX2-.*windows.*64.*\.7z)",          // stable
            R"(PCSX2-.*windows.*64.*\.7z)",          // nightly (same repo, pre-release flag)
            ArchiveType::SevenZ,
#ifdef Q_OS_WIN
            R"(C:\Emulators\PCSX2\)",
#else
            QDir::homePath() + "/Emulators/PCSX2/",
#endif
            "pcsx2-qt.exe", true, ReleaseChannel::Stable
        },
        {
            "rpcs3", "RPCS3  (PS3)",
            UpdateSource::GitHub, "RPCS3/rpcs3-binaries-win",
            R"(rpcs3-.*_win64\.7z)",
            R"(rpcs3-.*_win64\.7z)",
            ArchiveType::SevenZ,
#ifdef Q_OS_WIN
            R"(C:\Emulators\RPCS3\)",
#else
            QDir::homePath() + "/Emulators/RPCS3/",
#endif
            "rpcs3.exe", true, ReleaseChannel::Nightly
        },
        {
            "dolphin", "Dolphin  (GC / Wii)",
            UpdateSource::GitHub, "dolphin-emu/dolphin",
            R"(dolphin-.*-x64\.7z)",
            R"(dolphin-.*-x64\.7z)",
            ArchiveType::SevenZ,
#ifdef Q_OS_WIN
            R"(C:\Emulators\Dolphin\)",
#else
            QDir::homePath() + "/Emulators/Dolphin/",
#endif
            "Dolphin.exe", true, ReleaseChannel::Nightly
        },
        {
            "duckstation", "DuckStation  (PS1)",
            UpdateSource::GitHub, "stenzek/duckstation",
            R"(duckstation-windows-x64-release\.zip)",
            R"(duckstation-windows-x64-release\.zip)",
            ArchiveType::Zip,
#ifdef Q_OS_WIN
            R"(C:\Emulators\DuckStation\)",
#else
            QDir::homePath() + "/Emulators/DuckStation/",
#endif
            "duckstation-qt-x64-ReleaseLTCG.exe", false, ReleaseChannel::Nightly
        },
        {
            "ppsspp", "PPSSPP  (PSP)",
            UpdateSource::GitHub, "hrydgard/ppsspp",
            R"(PPSSPPWindows64\.zip)",
            R"(PPSSPPWindows64\.zip)",
            ArchiveType::Zip,
#ifdef Q_OS_WIN
            R"(C:\Emulators\PPSSPP\)",
#else
            QDir::homePath() + "/Emulators/PPSSPP/",
#endif
            "PPSSPPWindows64.exe", true, ReleaseChannel::Stable
        },
        {
            "cemu", "Cemu  (Wii U)",
            UpdateSource::GitHub, "cemu-project/Cemu",
            R"(cemu-.*-windows-x64\.zip)",
            R"(cemu-.*-windows-x64\.zip)",
            ArchiveType::Zip,
#ifdef Q_OS_WIN
            R"(C:\Emulators\Cemu\)",
#else
            QDir::homePath() + "/Emulators/Cemu/",
#endif
            "Cemu.exe", true, ReleaseChannel::Stable
        },
        {
            "flycast", "Flycast  (Dreamcast)",
            UpdateSource::GitHub, "flyinghead/flycast",
            R"(flycast-win64\.zip)",
            R"(flycast-win64\.zip)",
            ArchiveType::Zip,
#ifdef Q_OS_WIN
            R"(C:\Emulators\Flycast\)",
#else
            QDir::homePath() + "/Emulators/Flycast/",
#endif
            "flycast.exe", false, ReleaseChannel::Nightly
        },
        {
            "mesen", "Mesen  (NES / SNES / GB)",
            UpdateSource::GitHub, "SourMesen/Mesen2",
            R"(Mesen\.exe)",
            R"(Mesen\.exe)",
            ArchiveType::SingleFile,
#ifdef Q_OS_WIN
            R"(C:\Emulators\Mesen\)",
#else
            QDir::homePath() + "/Emulators/Mesen/",
#endif
            "Mesen.exe", false, ReleaseChannel::Stable
        },
    };
}