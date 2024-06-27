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
#include "hexeditor.h"
#include "filesystemhandler.h"
#include <QDir>
#include <QModelIndex>
#include <QMap>
#include "markerstablemodel.h"
#include "loadingdialog.h"
//#include "filesystemtabwidget.h"
#include "filesystemtablemodel.h"


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
    HexEditor* hexEditor() const;
    QByteArray getSelectedData() const;

private slots:
    void onShowTablesClicked();

    void onBytesPerLineChanged(int index);
    void onGoToOffsetClicked();

    void on_readAsImageButton_clicked();
    void onFileSystemSearchButtonClicked();
    void onFileSystemClearButtonClicked();
    void onTableRowRightClicked(const QPoint &pos);
    void onExportAction(const QModelIndex &index);
    void onOpenExternalAction(const QModelIndex &index);
    void onRefreshButtonClicked();



private:

    Ui::HexViewerForm *ui;
    MarkersTableModel *markersTableModel;
    FileSystemTableModel *fileSystemTableModel;

    FileSystemHandler *fsHandler;
    void onTableRowDoubleClicked(const QModelIndex &index);
     QMap<int, QString> currentDirMap;
     QMap<int, int> tabPartitionMap;

     void populateMarkersTable();
     void exportDirectoryContents(int partitionIndex, const QString &sourcePath, const QString &destinationPath);

     QModelIndex contextMenuIndex;
     LoadingDialog *loadingDialog;

    QString m_fileName;

};

#endif // HEXVIEWERFORM_H
