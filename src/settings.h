#pragma once
#include <QString>
#include <QDir>

struct AppSettings {
    QString corePath;
    QString assetsPath;
    QString infoPath;
    QString databasePath;
    QString retroarchPath;

    static AppSettings defaults()
    {
        AppSettings s;
#ifdef Q_OS_WIN
        s.corePath      = R"(C:\RetroArch\cores\)";
        s.assetsPath    = R"(C:\RetroArch\assets\)";
        s.infoPath      = R"(C:\RetroArch\info\)";
        s.databasePath  = R"(C:\RetroArch\database\rdb\)";
        s.retroarchPath = R"(C:\RetroArch\)";
#else
        const QString ra = QDir::homePath() + "/.config/retroarch/";
        s.corePath      = ra + "cores/";
        s.assetsPath    = ra + "assets/";
        s.infoPath      = ra + "cores/";
        s.databasePath  = ra + "database/rdb/";
        s.retroarchPath = ra;
#endif
        return s;
    }
};

class SettingsManager {
public:
    explicit SettingsManager(const QString &path);
    AppSettings load() const;
    void        save(const AppSettings &s) const;
private:
    QString m_path;
};
