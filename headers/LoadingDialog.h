#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QMovie>

class LoadingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingDialog(QWidget *parent = nullptr);
    ~LoadingDialog();

    void setMessage(const QString &message);

private:
    QLabel *loadingLabel;
    QLabel *messageLabel;
    QMovie *loadingMovie;


protected:
    void showEvent(QShowEvent *event) override;
};

#endif // LOADINGDIALOG_H
