#ifndef FILESYSTEMTABWIDGET_H
#define FILESYSTEMTABWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>
//#include "filesystemtablemodel.h"

class FileSystemTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileSystemTabWidget(QWidget *parent = nullptr);
    void addFileSystemTab( QWidget *tab,const QString &title, const QList<QStringList> &fileData);

private:
    QTabWidget *tabWidget;
};

#endif // FILESYSTEMTABWIDGET_H
