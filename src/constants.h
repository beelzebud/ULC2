#pragma once
#include <QString>

namespace Constants {

#ifdef Q_OS_WIN
inline const QString PlatformSegment = QStringLiteral("windows/x86_64");
#else
inline const QString PlatformSegment = QStringLiteral("linux/x86_64");
#endif

inline const QString BuildbotBase   = QStringLiteral("https://buildbot.libretro.com/nightly/");
inline const QString RemoteCoreBase = BuildbotBase + PlatformSegment + QStringLiteral("/latest/");

inline const QString AssetsUrl   = QStringLiteral("https://buildbot.libretro.com/assets/frontend/assets.zip");
inline const QString InfoUrl     = QStringLiteral("https://buildbot.libretro.com/assets/frontend/info.zip");
inline const QString DatabaseUrl = QStringLiteral("https://buildbot.libretro.com/assets/frontend/database-rdb.zip");
inline const QString RetroarchUrl = BuildbotBase + PlatformSegment + QStringLiteral("/RetroArch.7z");

inline constexpr int MaxLogLines  = 4000;
inline constexpr int TrimLogLines = 3000;
inline constexpr int CoreParallel = 4;

} // namespace Constants
