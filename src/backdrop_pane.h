#pragma once

#include <QTextEdit>
#include <QTreeWidget>
#include <QMovie>
#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QHideEvent>

// Mixin for scroll-area widgets (QTextEdit, QTreeWidget, ...) that paints a
// dimmed, animated GIF behind the widget's own content.
//
// The backdrop is an activity indicator: it is only painted while
// setBusy(true), and it only animates while busy AND shown. Falls back to
// plain black when idle or if the movie fails to load.
template <typename Base>
class BackdropPane : public Base
{
public:
    explicit BackdropPane(QWidget* parent = nullptr)
        : Base(parent)
    {
        QObject::connect(m_movie, &QMovie::frameChanged, this, [this](int) {
            m_scaled = QPixmap();
            this->viewport()->update();
            });

        this->viewport()->setAutoFillBackground(false);

        // Clear the styled background so the painted backdrop shows through.
        // Without Q_OBJECT, className() is the nearest Q_OBJECT base
        // ("QTextEdit" / "QTreeWidget") — exactly the selector we need.
        this->setStyleSheet(QStringLiteral("%1 { background: transparent; }")
            .arg(this->metaObject()->className()));
    }

    // Play the backdrop while work is running (and the pane is shown).
    void setBusy(bool busy)
    {
        m_busy = busy;
        applyPlayback();
        this->viewport()->update();
    }

protected:
    void showEvent(QShowEvent* event) override
    {
        applyPlayback();
        Base::showEvent(event);
    }

    void hideEvent(QHideEvent* event) override
    {
        applyPlayback(); // pauses — page not visible
        Base::hideEvent(event);
    }

    void paintEvent(QPaintEvent* event) override
    {
        QPainter p(this->viewport());
        p.fillRect(this->viewport()->rect(), Qt::black);
        if (m_busy) {
            const QPixmap frame = scaledFrame();
            if (!frame.isNull()) {
                p.setOpacity(kDim);
                const QSize vs = this->viewport()->size();
                p.drawPixmap((vs.width() - frame.width()) / 2,
                    (vs.height() - frame.height()) / 2, frame);
                p.setOpacity(1.0);
            }
        }
        p.end();
        Base::paintEvent(event);
    }

private:
    void applyPlayback()
    {
        const bool play = m_busy && this->isVisible();
        if (play) {
            const QMovie::MovieState st = m_movie->state();
            if (st == QMovie::NotRunning)
                m_movie->start();
            else if (st == QMovie::Paused)
                m_movie->setPaused(false);
        }
        else {
            m_movie->setPaused(true); // don't decode while idle/hidden
        }
    }

    QPixmap scaledFrame()
    {
        const QSize vs = this->viewport()->size();
        if (!m_scaled.isNull() && m_scaledSize == vs)
            return m_scaled;
        m_scaled = m_movie->currentPixmap().scaled(
            vs, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_scaledSize = vs;
        return m_scaled;
    }

    static constexpr double kDim = 0.22;

    QMovie* m_movie = new QMovie(QStringLiteral(":/icons/background.gif"),
        QByteArray(), this);
    bool    m_busy = false;
    QPixmap m_scaled;
    QSize   m_scaledSize;
};

using LogView    = BackdropPane<QTextEdit>;
using StatusTree = BackdropPane<QTreeWidget>;
