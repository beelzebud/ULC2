#pragma once
#include <QString>
#include <QMap>
#include <QDir>
#include "emulator_config.h"

struct EmulatorSettings {
    QString        installPath;
    QString        lastKnownTag;
    ReleaseChannel channel = ReleaseChannel::Stable;
};

struct AppSettings {
    QString corePath;
    QString assetsPath;
    QString infoPath;
    QString databasePath;
    QString retroarchPath;
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