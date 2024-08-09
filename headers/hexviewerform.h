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
#include "tagstablemodel.h"

#include "searchform.h"

namespace Ui {
class HexViewerForm;
}

class HexViewerForm : public QWidget
{
    Q_OBJECT

public:
    explicit HexViewerForm(QWidget *parent = nullptr);
    ~HexViewerForm();
    void openFile(const QString &fileName,int tabIndex);
    HexEditor* hexEditor() const;
    QByteArray getSelectedData() const;
    void setTagsHandler(TagsHandler *tagsHandler);
    void setUserTagsHandler(TagsHandler *userTagsHandler);
    FileSystemHandler *fsHandler;

    void jumpToOffset(quint64 offset);
private slots:
    void onShowTablesClicked();
    void onGoToOffsetClicked();
    void onBytesPerLineChanged(int index);

    void on_readAsImageButton_clicked();
    void onFileSystemSearchButtonClicked();
    void onFileSystemClearButtonClicked();
    void onTableRowRightClicked(const QPoint &pos);
    void onExportAction(const QModelIndex &index);
    void onOpenExternalAction(const QModelIndex &index);
    void onGoToMFTAction(const QModelIndex &index);
    void onRefreshButtonClicked();
    void onMarkersTableDoubleClicked(const QModelIndex &index);
    void removeSelectedTag();
    void removeSelectedTemplateTag();
    void onExportSelectedTagData();
    void onExportSelectedTemplateTagData();

    void onOpenSearchForm();
    void onSearchButtonClicked();
    void onSearchNextButtonClicked();
    void onSaveButtonClicked();



private:

    Ui::HexViewerForm *ui;
    MarkersTableModel *markersTableModel;
    FileSystemTableModel *fileSystemTableModel;

    void onTableRowDoubleClicked(const QModelIndex &index);
     QMap<int, QString> currentDirMap;
     QMap<int, int> tabPartitionMap;

     void populateMarkersTable();
     void exportDirectoryContents(int partitionIndex, const QString &sourcePath, const QString &destinationPath);

     QModelIndex contextMenuIndex;
     LoadingDialog *loadingDialog;

    QString m_fileName;

    TagsTableModel *tagsTableModel;
    TagsTableModel *templateTagsTableModel;


    void updateTagsTable(const QVector<Tag> &tags);
     void onTagTableDoubleClicked(const QModelIndex &index);
    void onTemplateTagTableDoubleClicked(const QModelIndex &index);

     searchform *searchForm;
    TagsHandler *tagsHandler;
     TagsHandler *userTagsHandler;



};

#endif // HEXVIEWERFORM_H
