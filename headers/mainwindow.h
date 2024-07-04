#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datatypeviewmodel.h"
#include "filesystemhandler.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createNewTab(const QString &fileName);

private slots:
    void openFile();
    void onEndianCheckboxStateChanged(quint64 state);
    void onTabChanged(int index);
    void onSelectionChanged(const QByteArray &selectedData, quint64 startOffset, quint64 endOffset);
    void onTagNameAndLength(const QString &tagName, quint64 length);

private:
    Ui::MainWindow *ui;
    DataTypeViewModel *dataTypeViewModel;
    FileSystemHandler *fsHandler;



};

#endif // MAINWINDOW_H
