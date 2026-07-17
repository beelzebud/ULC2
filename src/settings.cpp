#include "settings.h"

#include <QFile>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

AppSettings AppSettings::defaults()
{
    AppSettings s;
#ifdef Q_OS_WIN
    s.retroarchPath = R"(D:\Emulators\RetroArch\)";
    s.corePath = R"(D:\Emulators\RetroArch\cores\)";
#else
    const QString ra = QDir::homePath() + "/.config/retroarch/";
    s.retroarchPath = ra;
    s.corePath = ra + "cores/";
#endif
    for (const EmulatorConfig& cfg : allEmulatorConfigs()) {
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

    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();

    auto get = [&](const QString& key, QString& dst) {
        const QString v = root.value(key).toString();
        if (!v.isEmpty()) dst = v;
        };

    get("retroarchPath", s.retroarchPath);
    get("corePath", s.corePath);

    const QJsonObject emuObj = root.value("emulators").toObject();
    for (auto it = emuObj.begin(); it != emuObj.end(); ++it) {
        const QJsonObject obj = it.value().toObject();
        EmulatorSettings es;
        es.installPath = obj.value("installPath").toString();
        es.lastKnownTag = obj.value("lastKnownTag").toString();
        es.channel = (obj.value("channel").toString() == "nightly")
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
    root["retroarchPath"] = s.retroarchPath;
    root["corePath"] = s.corePath;

    QJsonObject emuObj;
    for (auto it = s.emulators.begin(); it != s.emulators.end(); ++it) {
        QJsonObject obj;
        obj["installPath"] = it.value().installPath;
        obj["lastKnownTag"] = it.value().lastKnownTag;
        obj["channel"] = (it.value().channel == ReleaseChannel::Nightly)
            ? "nightly" : "stable";
        emuObj[it.key()] = obj;
    }
    root["emulators"] = emuObj;

    QFile f(m_path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(root).toJson());
}