#pragma once

#include <QString>
#include <QMap>
#include "emulator_config.h"

struct EmulatorSettings {
    QString        installPath;
    // Path to the emulator executable used by the tab's Launch button.
    // Defaults to installPath + the emulator's exe name, but may point at an
    // existing install elsewhere.
    QString        launchPath;
    QString        lastKnownTag;
    // Human-readable form of lastKnownTag (e.g. "cbe7951 (2026-09-08)").
    // Purely cosmetic; comparisons use lastKnownTag.
    QString        lastKnownTagDisplay;
    ReleaseChannel channel = ReleaseChannel::Stable;
};

struct AppSettings {
    QString retroarchPath;
    QString corePath;
    QMap<QString, EmulatorSettings> emulators;
    static AppSettings defaults();
};

class SettingsManager {
public:
    explicit SettingsManager(const QString& path);
    AppSettings load() const;
    void        save(const AppSettings& s) const;
private:
    QString m_path;
};