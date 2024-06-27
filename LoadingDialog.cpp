#include "headers/loadingdialog.h"
#include <QVBoxLayout>
#include <QScreen>
#include <QApplication>

LoadingDialog::LoadingDialog(QWidget *parent)
    : QDialog(parent),
    loadingLabel(new QLabel(this)),
    messageLabel(new QLabel(this)),
    loadingMovie(new QMovie(":/icons/loading.gif", QByteArray(), this))
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setFixedSize(200, 50);

   /* loadingLabel->setMovie(loadingMovie);
    loadingLabel->setFixedSize(32, 32);  // Set the size of the loading GIF
    loadingLabel->setScaledContents(true);  // Scale the contents to fit the fixed size
    loadingMovie->start(); */

    QVBoxLayout *layout = new QVBoxLayout(this);
   // layout->addWidget(loadingLabel, 0, Qt::AlignCenter);
    layout->addWidget(messageLabel, 0, Qt::AlignCenter);

    setLayout(layout);
}

LoadingDialog::~LoadingDialog()
{
    delete loadingMovie;
    delete loadingLabel;
    delete messageLabel;
}

void LoadingDialog::setMessage(const QString &message)
{
    messageLabel->setText(message);
}

void LoadingDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    // Center the dialog on the parent
    if (parentWidget())
    {
        QRect parentRect = parentWidget()->geometry();
        int x = parentRect.x() + (parentRect.width() - width()) / 2;
        int y = parentRect.y() + (parentRect.height() - height()) / 2;
        move(x, y);
    }
    else
    {
        // Fallback to screen center if no parent
        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
}
