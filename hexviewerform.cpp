#include "headers/hexviewerform.h"
#include "ui_hexviewerform.h"
#include <QDateTime>
#include "headers/hexeditor.h"

#include "headers/gotooffsetdialog.h"

//#include "headers/filesystemtabwidget.h"
#include "headers/filesystemexception.h"

#include <QMenu>
#include <QAction>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDesktopServices>
#include <QProcess>
#include <QTemporaryDir>
#include <QDir>

HexViewerForm::HexViewerForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HexViewerForm)
    , markersTableModel(new MarkersTableModel(this))
    , fsHandler(new FileSystemHandler(this))
    ,loadingDialog(new LoadingDialog(this))

{
    qDebug() << "setup.";

    ui->setupUi(this);

    ui->markersTableView->setModel(markersTableModel);
    ui->markersTableView->resizeColumnsToContents();
    int descriptionColumnIndex = 3;
    int currentWidth = ui->markersTableView->columnWidth(descriptionColumnIndex);
    ui->markersTableView->setColumnWidth(descriptionColumnIndex, currentWidth * 2);


    ui->tagstabWidget->setVisible(false);
    connect(ui->showTablesButton, &QPushButton::clicked, this, &HexViewerForm::onShowTablesClicked);

    //Select 16 as default value
    ui->bytesPerLinecomboBox->setCurrentIndex(3);
    // Connect the bytes per line dropdown
    connect(ui->bytesPerLinecomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HexViewerForm::onBytesPerLineChanged);

    connect(ui->jumpToOffsetButton, &QPushButton::clicked, this, &HexViewerForm::onGoToOffsetClicked);

    connect(ui->FileSystemSearchpushButton, &QPushButton::clicked, this, &HexViewerForm::onFileSystemSearchButtonClicked);
    connect(ui->FileSystemShowAllpushButton, &QPushButton::clicked, this, &HexViewerForm::onFileSystemClearButtonClicked);

    ui->FileSystemTabWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->refreshButton, &QPushButton::clicked, this, &HexViewerForm::onRefreshButtonClicked);


}


void HexViewerForm::openFile(const QString &fileName)
{

    loadingDialog->setMessage("Loading , please wait...");
    loadingDialog->show();
    qApp->processEvents();

    HexEditor *hexEditor = ui->hexEditorWidget;

  m_fileName = QDir::toNativeSeparators(fileName);
    hexEditor->setData(m_fileName);



/*
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open file"));
        delete hexEditor;
        return;
    }



    QByteArray fileData = file.readAll();
    hexEditor->setData(fileData);

    */

    hexEditor->setSelectedByte(0);

    loadingDialog->hide();

}

HexEditor* HexViewerForm::hexEditor() const
{
    return ui->hexEditorWidget;
}



void HexViewerForm::onShowTablesClicked()
{
    bool isVisible = ui->tagstabWidget->isVisible();
    ui->tagstabWidget->setVisible(!isVisible);

    if (isVisible) {
        ui->showTablesButton->setText("Show Tables");
    } else {
        ui->showTablesButton->setText("Hide Tables");
    }

}

void HexViewerForm::onBytesPerLineChanged(int index)
{
    quint64 bytesPerLine = ui->bytesPerLinecomboBox->itemText(index).toInt();
    HexEditor *hexEditor = ui->hexEditorWidget;
    hexEditor->changeBytesPerLine(bytesPerLine);
}

void HexViewerForm::onGoToOffsetClicked()
{
    GoToOffsetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        qDebug() << "onGoToOffsetClicked accepted";

        quint64 newOffset = dialog.offset();
        if (newOffset < ui->hexEditorWidget->fileSize) {
            ui->hexEditorWidget->clearSelection();
            ui->hexEditorWidget->setSelectedByte(newOffset);
          //  ui->hexEditorWidget->ensureCursorVisible();
        }
    }
}


HexViewerForm::~HexViewerForm()
{
    delete ui;
}

void HexViewerForm::on_readAsImageButton_clicked()
{

    loadingDialog->setMessage("Loading , please wait...");
    loadingDialog->show();
    qApp->processEvents();


   //  QString fileName = "O:\\test_disk_images\\debian-12.5.0-amd64-netinst.iso";  //OK

  // m_fileName = "O:\\test_disk_images\\ntfs1-gen0.E01";  // OK
  // m_fileName = "O:\\test_disk_images\ntfs1-gen0.E01";  // OK

    // QString fileName = "O:\\test_disk_images\\dd_example.dd";

    try{




        fsHandler->openImage(m_fileName);
        ui->FileSystemTabWidget->clear();
        currentDirMap.clear();
        tabPartitionMap.clear();

        int tabCounter=1;
        for (int i = 0; i < fsHandler->getPartitionCount(); ++i) {
            QString partitionDescription = fsHandler->getPartitionDescription(i);
            QList<QStringList> fileList = fsHandler->listFilesInDirectory(i, "/");

            qDebug() << "Partition Found:" << partitionDescription;

            if(fileList.empty()){
                continue;
            }

            QString tabTitle = QString("Partition %1").arg(tabCounter);
            tabCounter++;
            QWidget *tab = new QWidget;
            QVBoxLayout *tabLayout = new QVBoxLayout(tab);
            QTableView *tableView = new QTableView(tab);
            FileSystemTableModel *fileSystemTableModel = new FileSystemTableModel(this);
            fileSystemTableModel->setFileData(fileList);

            tableView->setModel(fileSystemTableModel);
            tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

            connect(tableView, &QTableView::doubleClicked, this, &HexViewerForm::onTableRowDoubleClicked);

            connect(tableView, &QTableView::customContextMenuRequested, this, &HexViewerForm::onTableRowRightClicked);

            tableView->setContextMenuPolicy(Qt::CustomContextMenu);

            tabLayout->addWidget(tableView);
            tab->setLayout(tabLayout);


            int tabIndex = ui->FileSystemTabWidget->addTab(tab, tabTitle);
            currentDirMap[tabIndex] = "/";
            tabPartitionMap[tabIndex] = i; // Map tab index to partition index



        }

        // Populate the markers table once after opening the image
        populateMarkersTable();


        //Set offset as the current tab
        ui->tagstabWidget->setCurrentIndex(2);
        ui->readAsImageButton->setDisabled(true);


    } catch (const FileSystemException& e) {
        loadingDialog->hide();

        qDebug() << "Exception:" << e.getMessage() << "filename" << m_fileName;
        QMessageBox::critical(nullptr, "Error", e.getMessage());

    }

    loadingDialog->hide();

}



void HexViewerForm::onTableRowDoubleClicked(const QModelIndex &index)
{

    loadingDialog->setMessage("Loading , please wait...");
    loadingDialog->show();
    qApp->processEvents();

    qDebug() << "Double-clicked on index" << index;
    if (!index.isValid())
        return;

    QTableView *tableView = qobject_cast<QTableView *>(sender());
    if (!tableView)
        return;

   // qDebug() << "Double-clicked tableView ok :" << tableView;

    FileSystemTableModel *model = qobject_cast<FileSystemTableModel *>(tableView->model());
    if (!model)
        return;

   // qDebug() << "Double-clicked model ok :" << model;

    QStringList fileAttributes;
    int columnCount = model->columnCount();
    for (int col = 0; col < columnCount; ++col) {
        QModelIndex colIndex = model->index(index.row(), col);
        fileAttributes << model->data(colIndex, Qt::DisplayRole).toString();
    }

   // qDebug() << "Double-clicked fileAttributes ok :" << fileAttributes;



    QString filePath = fileAttributes.at(1);
    QString fileType = model->data(index, Qt::UserRole).toString();

    int tabIndex = ui->FileSystemTabWidget->currentIndex();
    int partitionIndex = tabPartitionMap.value(tabIndex, -1);

    if (partitionIndex == -1)
        return;

    QString currentDir = currentDirMap.value(tabIndex, "/");

    if (fileType == "Directory") {
        if (filePath.endsWith("/..")) {
            // Navigate to the parent directory
            if (currentDir != "/") {
                int lastSlashIndex = currentDir.lastIndexOf('/');
                if (lastSlashIndex != -1) {
                    currentDir = currentDir.left(lastSlashIndex);
                    if (currentDir.isEmpty()) {
                        currentDir = "/";
                    }
                }
            }
            qDebug() << "using absolute :" << currentDir;
        } else if (!filePath.endsWith("/.") && !filePath.endsWith("/..")) {
            // Navigate to the clicked directory
            currentDir = filePath;
            qDebug() << "using filepath :" << currentDir;
        }
        qDebug() << "Listing for dir :" << currentDir;


        QList<QStringList> newFileList = fsHandler->listFilesInDirectory(partitionIndex, currentDir);

        if (!newFileList.isEmpty()) {
            model->setFileData(newFileList);
            currentDirMap[tabIndex] = currentDir;
        }
    }else{
        qDebug() << "not processing dir:" << filePath << "as its a " << fileType;

    }

    loadingDialog->hide();
}


void HexViewerForm::populateMarkersTable()
{
    QList<QStringList> partitionDetails = fsHandler->getPartitionDetails();
    markersTableModel->setMarkerData(partitionDetails);
}


void HexViewerForm::onFileSystemSearchButtonClicked()
{
    QString searchText = ui->FileSystenSearchlineEdit->text();
    int currentTabIndex = ui->FileSystemTabWidget->currentIndex();
    if (currentTabIndex < 0)
        return;

    QWidget *currentTab = ui->FileSystemTabWidget->widget(currentTabIndex);
    QTableView *tableView = currentTab->findChild<QTableView *>();
    if (!tableView)
        return;

    FileSystemTableModel *model = qobject_cast<FileSystemTableModel *>(tableView->model());
    if (!model)
        return;

    int tabIndex = ui->FileSystemTabWidget->currentIndex();
    int partitionIndex = tabPartitionMap.value(tabIndex, -1);

    QString currentDir = currentDirMap.value(tabIndex, "/");



    QList<QStringList> fileList = fsHandler->listFilesInDirectory(partitionIndex,currentDir);
    QList<QStringList> filteredList;

    for (const QStringList &fileAttributes : fileList) {
        QString fileName = fileAttributes.at(0);  // Assuming the file name is in the first column
        if (fileName.contains(searchText, Qt::CaseInsensitive)) {
            filteredList.append(fileAttributes);
        }
    }

    model->setFileData(filteredList);
}

void HexViewerForm::onFileSystemClearButtonClicked()
{
    int currentTabIndex = ui->FileSystemTabWidget->currentIndex();
    if (currentTabIndex < 0)
        return;

    QWidget *currentTab = ui->FileSystemTabWidget->widget(currentTabIndex);
    QTableView *tableView = currentTab->findChild<QTableView *>();
    if (!tableView)
        return;

    FileSystemTableModel *model = qobject_cast<FileSystemTableModel *>(tableView->model());
    if (!model)
        return;

    int tabIndex = ui->FileSystemTabWidget->currentIndex();
    int partitionIndex = tabPartitionMap.value(tabIndex, -1);


    QString currentDir = currentDirMap.value(tabIndex, "/");


    QList<QStringList> fileList = fsHandler->listFilesInDirectory(partitionIndex, currentDir);

    model->setFileData(fileList);

    ui->FileSystenSearchlineEdit->clear();
}

void HexViewerForm::onTableRowRightClicked(const QPoint &pos)
{
    QTableView *tableView = qobject_cast<QTableView *>(sender());
    if (!tableView)
        return;

    contextMenuIndex = tableView->indexAt(pos);
    if (!contextMenuIndex.isValid())
        return;


    QModelIndex  index = tableView->indexAt(pos);
    if (!index.isValid())
        return;


    QString fileName = tableView->model()->data(tableView->model()->index(index.row(), 0), Qt::DisplayRole).toString();


    QMenu contextMenu;
    QAction *exportAction = contextMenu.addAction("Export");
    QAction *openExternalAction = contextMenu.addAction("Open File as External Viewer");
   // QAction *exportZipAction = contextMenu.addAction("Export as zip");

    if (fileName == "." || fileName == "..") {
        exportAction->setEnabled(false);
        openExternalAction->setEnabled(false);
    } else {
        connect(exportAction, &QAction::triggered, [this, index] { onExportAction(index); });
        connect(openExternalAction, &QAction::triggered, [this, index] { onOpenExternalAction(index); });
    }
    contextMenu.exec(tableView->viewport()->mapToGlobal(pos));
}
void HexViewerForm::onExportAction(const QModelIndex &index)
{

    loadingDialog->setMessage("Loading , please wait...");
    loadingDialog->show();
    qApp->processEvents();


    qDebug() << "QModelIndex" << index;

    if (!contextMenuIndex.isValid())
        return;

    qDebug() << "contextMenuIndex" << contextMenuIndex;

    int tabIndex = ui->FileSystemTabWidget->currentIndex();
    int partitionIndex = tabPartitionMap.value(tabIndex, -1);
    if (partitionIndex == -1)
        return;

    qDebug() << "partitionIndex" << partitionIndex;

    QTableView *tableView = qobject_cast<QTableView *>(ui->FileSystemTabWidget->currentWidget()->findChild<QTableView *>());
    if (!tableView)
        return;

    FileSystemTableModel *model = qobject_cast<FileSystemTableModel *>(tableView->model());
    if (!model)
        return;

    QString filePath = model->data(model->index(contextMenuIndex.row(), 1), Qt::DisplayRole).toString();
    QString fileName = model->data(model->index(contextMenuIndex.row(), 0), Qt::DisplayRole).toString();  // Assuming file name is in the first column
    QString fileType = model->data(contextMenuIndex, Qt::UserRole).toString();

    QString destinationPath;

    if (fileType == "Directory") {
        destinationPath = QFileDialog::getExistingDirectory(this, tr("Save Directory As"), fileName);
    } else {
        QString fileExtension = QFileInfo(filePath).suffix();
        QString filter = QString("Files (*.%1)").arg(fileExtension);
        QString suggestedFileName = QString("%1").arg(fileName);
        destinationPath = QFileDialog::getSaveFileName(this, tr("Save File As"), suggestedFileName, filter);
    }

    qDebug() << "destinationPath:" << destinationPath;

    if (destinationPath.isEmpty())
        return;

    try {
        qDebug() << "exportFileContents" << partitionIndex << "filePath" << filePath << "destinationPath" << destinationPath;

        if (fileType == "Directory") {
            // Recursively copy the directory contents
            exportDirectoryContents(partitionIndex, filePath, destinationPath);
        } else {
            fsHandler->exportFileContents(partitionIndex, filePath, destinationPath);
        }

        QMessageBox::information(this, tr("Success"), tr("File exported successfully"));
    } catch (const FileSystemException &e) {
        loadingDialog->hide();

        QMessageBox::critical(this, tr("Error"), e.getMessage());
    }

    loadingDialog->hide();

}

void HexViewerForm::exportDirectoryContents(int partitionIndex, const QString &sourcePath, const QString &destinationPath)
{
    qDebug() << "Exporting directory:" << sourcePath << "to" << destinationPath;

    QDir destDir(destinationPath);
    if (!destDir.exists()) {
        if (!destDir.mkpath(".")) {
            throw FileSystemException("Failed to create directory: " + destinationPath);
        }
    }

    QList<QStringList> fileList = fsHandler->listFilesInDirectory(partitionIndex, sourcePath);
    for (const QStringList &fileAttributes : fileList) {
        QString fileName = fileAttributes.at(0);
        QString fileType = fileAttributes.at(2);
        QString sourceFilePath = sourcePath + (sourcePath.endsWith("/") ? "" : "/") + fileName;
        QString destFilePath = destinationPath + "/" + fileName;

        if (fileType == "Directory" && fileName != "." && fileName != "..") {
            exportDirectoryContents(partitionIndex, sourceFilePath, destFilePath);
        } else if (fileType == "File") {
            fsHandler->exportFileContents(partitionIndex, sourceFilePath, destFilePath);
        }
    }
}



void HexViewerForm::onOpenExternalAction(const QModelIndex &index)
{

    loadingDialog->setMessage("Loading , please wait...");
    loadingDialog->show();
    qApp->processEvents();


    if (!contextMenuIndex.isValid())
        return;

    int tabIndex = ui->FileSystemTabWidget->currentIndex();
    int partitionIndex = tabPartitionMap.value(tabIndex, -1);
    if (partitionIndex == -1)
        return;

    QTableView *tableView = qobject_cast<QTableView *>(ui->FileSystemTabWidget->currentWidget()->findChild<QTableView *>());
    if (!tableView)
        return;

    FileSystemTableModel *model = qobject_cast<FileSystemTableModel *>(tableView->model());
    if (!model)
        return;

    QString fileName = model->data(model->index(contextMenuIndex.row(), 0), Qt::DisplayRole).toString();
    QString filePath = model->data(model->index(contextMenuIndex.row(), 1), Qt::DisplayRole).toString();

    // Ensure the file path is correct, even for subdirectories
    QString currentDir = currentDirMap.value(tabIndex, "/");
    if (!currentDir.endsWith('/')) {
        currentDir += '/';
    }
    filePath = currentDir + fileName;

    QString destinationPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + QFileInfo(fileName).fileName();

    try {
        fsHandler->exportFileContents(partitionIndex, filePath, destinationPath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(destinationPath));
    } catch (const FileSystemException &e) {
        loadingDialog->hide();

        QMessageBox::critical(this, tr("Error"), e.getMessage());
    }

    loadingDialog->hide();
}

void HexViewerForm::onRefreshButtonClicked()
{
    ui->readAsImageButton->setDisabled(false);

}


