#include "headers/filesystemtabwidget.h"
#include <QHeaderView>
#include "headers/filesystemtablemodel.h"

FileSystemTabWidget::FileSystemTabWidget(QWidget *parent)
    : QWidget(parent), tabWidget(new QTabWidget(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}

void FileSystemTabWidget::addFileSystemTab(QWidget *tab,const QString &title, const QList<QStringList> &fileData)
{
    QVBoxLayout *tabLayout = new QVBoxLayout(tab);
    QTableView *tableView = new QTableView(tab);
    FileSystemTableModel *model = new FileSystemTableModel(this);
    model->setFileData(fileData);

    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    tabLayout->addWidget(tableView);
    tab->setLayout(tabLayout);

    tabWidget->addTab(tab, title);
}
