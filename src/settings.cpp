#include "settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

SettingsManager::SettingsManager(const QString &path) : m_path(path) {}

AppSettings SettingsManager::load() const
{
    AppSettings s = AppSettings::defaults();
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly)) return s;

    const auto root = QJsonDocument::fromJson(f.readAll()).object();

    auto get = [&](const QString &key, QString &dst) {
        const QString v = root.value(key).toString();
        if (!v.isEmpty()) dst = v;
    };

    get("corePath",      s.corePath);
    get("assetsPath",    s.assetsPath);
    get("infoPath",      s.infoPath);
    get("databasePath",  s.databasePath);
    get("retroarchPath", s.retroarchPath);
    return s;
}

void SettingsManager::save(const AppSettings &s) const
{
    QJsonObject o;
    o["corePath"]      = s.corePath;
    o["assetsPath"]    = s.assetsPath;
    o["infoPath"]      = s.infoPath;
    o["databasePath"]  = s.databasePath;
    o["retroarchPath"] = s.retroarchPath;

    QFile f(m_path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(o).toJson());
}
