#pragma once

#include <QObject>
#include <QString>

class QTimer;

// Tracks a process this app launched with QProcess::startDetached.
//
// The emulator must survive Emu-Manager closing, so it is deliberately started
// detached and we cannot hold a QProcess on it. Instead we remember the PID and
// poll it, which lets the UI show live running state.
//
// Only the most recent launch is tracked; starting a second instance replaces
// the tracked PID.
class LaunchedProcess : public QObject
{
    Q_OBJECT
public:
    explicit LaunchedProcess(QObject* parent = nullptr);

    // Starts exePath detached. Returns false and sets *error on failure.
    bool launch(const QString& exePath, QString* error = nullptr);

    bool isRunning() const { return m_running; }
    qint64 pid() const { return m_pid; }

signals:
    void runningChanged(bool running);

private slots:
    void poll();

private:
    void setRunning(bool running);

    static constexpr int PollIntervalMs = 2000;

    QTimer* m_timer = nullptr;
    qint64  m_pid = -1;
    bool    m_running = false;
};
