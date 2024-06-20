#ifndef GOTOOFFSETDIALOG_H
#define GOTOOFFSETDIALOG_H

#include <QDialog>

namespace Ui {
class GoToOffsetDialog;
}

class GoToOffsetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoToOffsetDialog(QWidget *parent = nullptr);
    ~GoToOffsetDialog();
    quint64 offset() const;


private:
    Ui::GoToOffsetDialog *ui;
};

#endif // GOTOOFFSETDIALOG_H
