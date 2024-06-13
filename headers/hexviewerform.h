#ifndef HEXVIEWERFORM_H
#define HEXVIEWERFORM_H

#include <QWidget>
#include <QTimer>
#include <tsk/libtsk.h>
#include <QItemSelectionModel>
#include <QTableView>
#include <QScrollBar>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QIODevice>
#include <QFile>
#include <QTimer>
#include <QScrollBar>
#include <QVector>

#include "datatypeviewmodel.h"


namespace Ui {
class HexViewerForm;
}

class HexViewerForm : public QWidget
{
    Q_OBJECT

public:
    explicit HexViewerForm(QWidget *parent = nullptr);
    ~HexViewerForm();
    void openFile(const QString &fileName);


private slots:
    void onShowTablesClicked();



private:
    Ui::HexViewerForm *ui;


};

#endif // HEXVIEWERFORM_H
