#ifndef OPENINGDIALOG_H
#define OPENINGDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "loadingdialog.h"


namespace Ui {
class OpeningDialog;
}

class OpeningDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpeningDialog(QWidget *parent = nullptr);
    ~OpeningDialog();


private slots:
    void onSelectSourceClicked();
    void onDialogButtonClicked(QAbstractButton *button);


private:
    Ui::OpeningDialog *ui;
     LoadingDialog *loadingDialog;

};

#endif // OPENINGDIALOG_H
