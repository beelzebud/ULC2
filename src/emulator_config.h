#pragma once
#include <QString>
#include <QList>
#include <QDir>

enum class ArchiveType { Zip, SevenZ, SingleFile };
enum class UpdateSource { GitHub, DolphinBuildbot };
enum class ReleaseChannel { Stable, Nightly };

struct EmulatorConfig {
    QString        id;
    QString        displayName;
    UpdateSource   source;
    QString        githubRepo;
    QString        buildbotApiUrl;
    QString        stableAssetPattern;
    QString        nightlyAssetPattern;
    ArchiveType    archiveType;
    QString        defaultInstallPath;
    QString        exeName;
    bool           stripTopLevelDir;
    ReleaseChannel defaultChannel;
    QString        iconResource;
};

inline QList<EmulatorConfig> allEmulatorConfigs()
{
#ifdef Q_OS_WIN
    const QString base = R"(C:\Emulators\)";
#else
    const QString base = QDir::homePath() + "/Emulators/";
#endif

    return {
        {
            "pcsx2", "PCSX2  (PS2)",
            UpdateSource::GitHub, "PCSX2/pcsx2", {},
            R"(PCSX2-.*windows.*64.*\.7z)",
            R"(PCSX2-.*windows.*64.*\.7z)",
            ArchiveType::SevenZ,
            base + "PCSX2/",
            "pcsx2-qt.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/pcsx2.png"
        },
        {
            "rpcs3", "RPCS3  (PS3)",
            UpdateSource::GitHub, "RPCS3/rpcs3-binaries-win", {},
            R"(rpcs3-.*_win64\.7z)",
            R"(rpcs3-.*_win64\.7z)",
            ArchiveType::SevenZ,
            base + "RPCS3/",
            "rpcs3.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/rpcs3.png"
        },
        {
            // Dolphin uses its own buildbot — no GitHub releases exist.
            // API returns HTML; we scrape the first x64 build link.
            // Dolphin only publishes development builds, no separate stable.
            "dolphin", "Dolphin  (GC / Wii)",
            UpdateSource::DolphinBuildbot, {},
            "https://api.dolphin-emu.org/download/list/master/1/",
            R"(dolphin-master-.*-x64\.7z)",
            R"(dolphin-master-.*-x64\.7z)",
            ArchiveType::SevenZ,
            base + "Dolphin/",
            "Dolphin.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/dolphin.png"
        },
        {
            "duckstation", "DuckStation  (PS1)",
            UpdateSource::GitHub, "stenzek/duckstation", {},
            R"(duckstation-windows-x64-release\.zip)",
            R"(duckstation-windows-x64-release\.zip)",
            ArchiveType::Zip,
            base + "DuckStation/",
            "duckstation-qt-x64-ReleaseLTCG.exe", false, ReleaseChannel::Nightly,
            ":/icons/emulators/duckstation.png"
        },
        {
            "ppsspp", "PPSSPP  (PSP)",
            UpdateSource::GitHub, "hrydgard/ppsspp", {},
            R"(PPSSPP-.*-Windows-x64\.zip)",
            R"(PPSSPP-.*-Windows-x64\.zip)",
            ArchiveType::Zip,
            base + "PPSSPP/",
            "PPSSPPWindows64.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/ppsspp.png"
        },
        {
            "cemu", "Cemu  (Wii U)",
            UpdateSource::GitHub, "cemu-project/Cemu", {},
            R"(cemu-.*-windows-x64\.zip)",
            R"(cemu-.*-windows-x64\.zip)",
            ArchiveType::Zip,
            base + "Cemu/",
            "Cemu.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/cemu.png"
        },
        {
            "flycast", "Flycast  (Dreamcast)",
            UpdateSource::GitHub, "flyinghead/flycast", {},
            R"(flycast-win64-.*\.zip)",
            R"(flycast-win64-.*\.zip)",
            ArchiveType::Zip,
            base + "Flycast/",
            "flycast.exe", false, ReleaseChannel::Stable,
            ":/icons/emulators/flycast.png"
        },
        {
            "mesen", "Mesen  (NES / SNES / GB)",
            UpdateSource::GitHub, "SourMesen/Mesen2", {},
            R"(Mesen_.*_Windows\.zip)",
            R"(Mesen_.*_Windows\.zip)",
            ArchiveType::Zip,
            base + "Mesen/",
            "Mesen.exe", false, ReleaseChannel::Stable,
            ":/icons/emulators/mesen.png"
        },
        {
            "supermodel", "Supermodel  (Sega Model 3)",
            UpdateSource::GitHub, "trzy/Supermodel", {},
            R"((?i)supermodel.*windows.*\.zip|(?i).*win.*supermodel.*\.zip)",
            R"((?i)supermodel.*windows.*\.zip|(?i).*win.*supermodel.*\.zip)",
            ArchiveType::Zip,
            base + "Supermodel/",
            "supermodel.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/supermodel.png"
        },
        {
            "xemu", "xemu  (Xbox)",
            UpdateSource::GitHub, "xemu-project/xemu", {},
            R"(xemu-.*-windows-x86_64\.zip)",
            R"(xemu-.*-windows-x86_64\.zip)",
            ArchiveType::Zip,
            base + "xemu/",
            "xemu.exe", false, ReleaseChannel::Nightly,
            ":/icons/emulators/xemu.png"
        },
        {
            // Note: separate id from "xemu" to avoid settings collision
            "xemu_chihiro", "xemu  (Chihiro)",
            UpdateSource::GitHub, "xemu-project/xemu", {},
            R"(xemu-.*-windows-x86_64\.zip)",
            R"(xemu-.*-windows-x86_64\.zip)",
            ArchiveType::Zip,
            base + "xemu-chihiro/",
            "xemu.exe", false, ReleaseChannel::Nightly,
            ":/icons/emulators/xemu_chihiro.png"
        },
        {
            "ymir", "Ymir  (Sega Saturn)",
            UpdateSource::GitHub, "StrikerX3/Ymir", {},
            R"(ymir-windows-x86_64-AVX2-.*\.zip)",
            R"(ymir-windows-x86_64-AVX2-.*\.zip)",
            ArchiveType::Zip,
            base + "Ymir/",
            "ymir.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/ymir.png"
        },
        {
            "xenia_edge", "Xenia Edge  (Xbox 360)",
            UpdateSource::GitHub, "has207/xenia-edge", {},
            R"(xenia_edge_windows\.zip)",
            R"(xenia_edge_windows\.zip)",
            ArchiveType::Zip,
            base + "XeniaEdge/",
            "xenia_canary.exe", false, ReleaseChannel::Nightly,
            ":/icons/emulators/xenia_edge.png"
        },
        {
            "mame", "MAME  (Arcade / Multi-system)",
            UpdateSource::GitHub, "mamedev/mame", {},
            R"(mame\d+b_x64\.exe)",
            R"(mame\d+b_x64\.exe)",
            ArchiveType::SingleFile,
            base + "MAME/",
            "mame.exe", false, ReleaseChannel::Stable,
            ":/icons/emulators/mame.png"
        },
        {
            "hypseus_singe", "Hypseus Singe  (Laserdisc)",
            UpdateSource::GitHub, "DirtBagXon/hypseus-singe", {},
            R"((?i)hypseus.*win.*\.zip)",
            R"((?i)hypseus.*win.*\.zip)",
            ArchiveType::Zip,
            base + "HypseusSinge/",
            "hypseus.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/hypseus.png"
        },
    };
}