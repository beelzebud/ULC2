#include "settings.h"
#include "emulator_config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

AppSettings AppSettings::defaults()
{
    AppSettings s;
#ifdef Q_OS_WIN
    s.corePath = R"(C:\RetroArch\cores\)";
    s.assetsPath = R"(C:\RetroArch\assets\)";
    s.infoPath = R"(C:\RetroArch\info\)";
    s.databasePath = R"(C:\RetroArch\database\rdb\)";
    s.retroarchPath = R"(C:\RetroArch\)";
#else
    const QString ra = QDir::homePath() + "/.config/retroarch/";
    s.corePath = ra + "cores/";
    s.assetsPath = ra + "assets/";
    s.infoPath = ra + "cores/";
    s.databasePath = ra + "database/rdb/";
    s.retroarchPath = ra;
#endif
    for (const auto& cfg : allEmulatorConfigs()) {
        EmulatorSettings es;
        es.installPath = cfg.defaultInstallPath;
        es.channel = cfg.defaultChannel;
        s.emulators[cfg.id] = es;
    }
    return s;
}

SettingsManager::SettingsManager(const QString& path) : m_path(path) {}

AppSettings SettingsManager::load() const
{
    AppSettings s = AppSettings::defaults();
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly)) return s;

    const auto root = QJsonDocument::fromJson(f.readAll()).object();

    auto get = [&](const QString& key, QString& dst) {
        const QString v = root.value(key).toString();
        if (!v.isEmpty()) dst = v;
        };

    get("corePath", s.corePath);
    get("assetsPath", s.assetsPath);
    get("infoPath", s.infoPath);
    get("databasePath", s.databasePath);
    get("retroarchPath", s.retroarchPath);

    const auto emuObj = root.value("emulators").toObject();
    for (auto it = emuObj.begin(); it != emuObj.end(); ++it) {
        const auto obj = it.value().toObject();
        EmulatorSettings es;
        es.installPath = obj.value("installPath").toString();
        es.lastKnownTag = obj.value("lastKnownTag").toString();
        es.channel = obj.value("channel").toString() == "nightly"
            ? ReleaseChannel::Nightly
            : ReleaseChannel::Stable;
        if (!es.installPath.isEmpty())
            s.emulators[it.key()] = es;
    }
    return s;
}

void SettingsManager::save(const AppSettings& s) const
{
    QJsonObject root;
    root["corePath"] = s.corePath;
    root["assetsPath"] = s.assetsPath;
    root["infoPath"] = s.infoPath;
    root["databasePath"] = s.databasePath;
    root["retroarchPath"] = s.retroarchPath;

    QJsonObject emuObj;
    for (auto it = s.emulators.begin(); it != s.emulators.end(); ++it) {
        QJsonObject obj;
        obj["installPath"] = it.value().installPath;
        obj["lastKnownTag"] = it.value().lastKnownTag;
        obj["channel"] = it.value().channel == ReleaseChannel::Nightly
            ? "nightly" : "stable";
        emuObj[it.key()] = obj;
    }
    root["emulators"] = emuObj;

    QFile f(m_path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(root).toJson());
}