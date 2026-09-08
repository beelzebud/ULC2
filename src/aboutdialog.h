#pragma once
#include <QDialog>
#include <QPixmap>
#include <QSize>

class QMovie;

class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    QPixmap scaledFrame();

    QMovie* m_movie = nullptr;
    QPixmap m_scaled;
    QSize   m_scaledSize;
};
