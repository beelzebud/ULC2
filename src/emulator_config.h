#pragma once

#include <QString>
#include <QList>
#include <QDir>

enum class ArchiveType { Zip, SevenZ, SingleFile };
enum class UpdateSource { GitHub, DolphinBuildbot, Rpcs3Net, Gitea, DirectUrl };
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

    return QList<EmulatorConfig>({
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
            "eden", "Eden  (Switch)",
            UpdateSource::Gitea, {},
            "https://git.eden-emu.dev/api/v1/repos/eden-emu/eden/releases/latest",
            R"(Eden-Windows-.*-amd64-msvc-standard\.zip)",
            R"(Eden-Windows-.*-amd64-msvc-standard\.zip)",
            ArchiveType::Zip,
            base + "Eden/",
            "eden.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/eden.png"
        },
        {
            "hypseus_singe", "Hypseus Singe  (Laserdisc)",
            UpdateSource::GitHub, "DirtBagXon/hypseus-singe", {},
            R"((?i)hypseus.*win.*\.zip)",
            R"((?i)hypseus.*win.*\.zip)",
            ArchiveType::Zip,
            base + "HypseusSinge/",
            "hypseus.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/hypseus_singe.png"
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
            "mesen", "Mesen  (NES / SNES / GB)",
            UpdateSource::GitHub, "nesdev-org/MesenCE", {},
            R"(Mesen_.*_Windows\.zip)",
            R"(Mesen_.*_Windows\.zip)",
            ArchiveType::Zip,
            base + "Mesen/",
            "Mesen.exe", false, ReleaseChannel::Stable,
            ":/icons/emulators/mesen.png"
        },
        {
            "mgba", "mGBA  (GBA / GB)",
            UpdateSource::DirectUrl, {},
            "https://s3.amazonaws.com/mgba/mGBA-build-latest-win64.7z",
            R"(mGBA-build-latest-win64\.7z)",
            R"(mGBA-build-latest-win64\.7z)",
            ArchiveType::SevenZ,
            base + "mGBA/",
            "mGBA.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/mgba.png"
        },
        {
            "pcsx2", "PCSX2  (PS2)",
            UpdateSource::GitHub, "PCSX2/pcsx2", {},
            R"(PCSX2-(?!.*symbols).*windows.*64.*\.7z)",
            R"(PCSX2-(?!.*symbols).*windows.*64.*\.7z)",
            ArchiveType::SevenZ,
            base + "PCSX2/",
            "pcsx2-qt.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/pcsx2.png"
        },
        {
            "pcsx2x6", "PCSX2X6 (Namco 246/256)",
            UpdateSource::GitHub, "PS2Homebrew-arcade/pcsx2x6", {},
            R"(PCSX2X6-(?!.*symbols).*windows.*64.*\.7z)",
            R"(PCSX2X6-(?!.*symbols).*windows.*64.*\.7z)",
            ArchiveType::SevenZ,
            base + "PCSX2X6/",
            "pcsx2-qt.exe", true, ReleaseChannel::Stable,
            ":/icons/emulators/pcsx2x6.png"
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
            "rpcs3", "RPCS3  (PS3)",
            UpdateSource::Rpcs3Net, {},
            "https://update.rpcs3.net/?api=v2&c=0000000",
            R"(rpcs3-.*_win64.*\.7z)",
            R"(rpcs3-.*_win64.*\.7z)",
            ArchiveType::SevenZ,
            base + "RPCS3/",
            "rpcs3.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/rpcs3.png"
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
            "vita3k", "Vita3K  (PS Vita)",
            UpdateSource::GitHub, "Vita3K/Vita3K-builds", {},
            R"(vita3k-.*-windows-x86_64\.7z)",
            R"(vita3k-.*-windows-x86_64\.7z)",
            ArchiveType::SevenZ,
            base + "Vita3K/",
            "Vita3K.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/vita3k.png"
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
            "ymir", "Ymir  (Sega Saturn)",
            UpdateSource::GitHub, "StrikerX3/Ymir", {},
            R"(ymir-windows-x86_64-AVX2-.*\.zip)",
            R"(ymir-windows-x86_64-AVX2-.*\.zip)",
            ArchiveType::Zip,
            base + "Ymir/",
            "ymir.exe", true, ReleaseChannel::Nightly,
            ":/icons/emulators/ymir.png"
        },
        });
}