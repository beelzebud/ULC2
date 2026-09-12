#include "launched_process.h"

#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QTimer>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  include <cerrno>
#  include <csignal>
#  include <sys/types.h>
#endif

namespace {

bool isProcessAlive(qint64 pid)
{
    if (pid <= 0) return false;
#ifdef Q_OS_WIN
    HANDLE h = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE, static_cast<DWORD>(pid));
    if (!h) return false;
    const DWORD state = WaitForSingleObject(h, 0);
    CloseHandle(h);
    return state == WAIT_TIMEOUT;
#else
    // Signal 0 performs error checking without actually signalling.
    if (::kill(static_cast<pid_t>(pid), 0) == 0) return true;
    return errno != ESRCH;
#endif
}

} // namespace

LaunchedProcess::LaunchedProcess(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(PollIntervalMs);
    connect(m_timer, &QTimer::timeout, this, &LaunchedProcess::poll);
}

bool LaunchedProcess::launch(const QString& exePath, QString* error)
{
    const QFileInfo fi(exePath);
    if (!fi.exists() || !fi.isFile()) {
        if (error) *error = QStringLiteral("not found: %1").arg(exePath);
        return false;
    }

    qint64 pid = -1;
    if (!QProcess::startDetached(fi.absoluteFilePath(), QStringList(),
            fi.absolutePath(), &pid)
        || pid <= 0) {
        if (error) *error = QStringLiteral("could not start: %1").arg(exePath);
        return false;
    }

    m_pid = pid;
    setRunning(true);
    m_timer->start();
    return true;
}

void LaunchedProcess::poll()
{
    if (!m_running) {
        m_timer->stop();
        return;
    }
    if (!isProcessAlive(m_pid)) {
        m_timer->stop();
        setRunning(false);
    }
}

void LaunchedProcess::setRunning(bool running)
{
    if (m_running == running) return;
    m_running = running;
    emit runningChanged(running);
}
