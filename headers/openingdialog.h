#ifndef OPENINGDIALOG_H
#define OPENINGDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include "loadingdialog.h"
#include "tagshandler.h"


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
    bool onOkButtonClicked();
    void onSelectOutputDirectoryClicked();
    void onLoadCaseClicked();
    void onOtherCaseClicked();
    void onRecentFolderClicked();
    void showEulaDialog();



private:
    Ui::OpeningDialog *ui;
     LoadingDialog *loadingDialog;
    TagsHandler *tagsHandler;


};

#endif // OPENINGDIALOG_H
