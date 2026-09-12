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
        es.launchPath = cfg.defaultInstallPath + cfg.exeName;
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

        // Start from the built-in defaults for this emulator so fields the
        // JSON omits (e.g. launchPath) keep their configured defaults.
        EmulatorSettings es;
        const auto def = s.emulators.find(it.key());
        if (def != s.emulators.end())
            es = def.value();

        const QString installPath = obj.value("installPath").toString();
        if (!installPath.isEmpty())
            es.installPath = installPath;
        const QString launchPath = obj.value("launchPath").toString();
        if (!launchPath.isEmpty())
            es.launchPath = launchPath;

        es.lastKnownTag = obj.value("lastKnownTag").toString();
        es.lastKnownTagDisplay = obj.value("lastKnownTagDisplay").toString();
        es.channel = (obj.value("channel").toString() == "nightly")
            ? ReleaseChannel::Nightly
            : ReleaseChannel::Stable;
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
        obj["launchPath"] = it.value().launchPath;
        obj["lastKnownTag"] = it.value().lastKnownTag;
        obj["lastKnownTagDisplay"] = it.value().lastKnownTagDisplay;
        obj["channel"] = (it.value().channel == ReleaseChannel::Nightly)
            ? "nightly" : "stable";
        emuObj[it.key()] = obj;
    }
    root["emulators"] = emuObj;

    QFile f(m_path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(root).toJson());
}