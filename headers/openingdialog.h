#ifndef OPENINGDIALOG_H
#define OPENINGDIALOG_H

#include <QDialog>
#include <QAbstractButton>

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
};

#endif // OPENINGDIALOG_H
