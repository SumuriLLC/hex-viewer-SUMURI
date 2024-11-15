#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include "headers/physicaldrivesdialog.h"
#include <QCloseEvent>
#include <QRegularExpression>
#include <QProgressDialog>


//Multiple instances will be created for each tab Form
#include "headers/hexviewerform.h"
//Open Dialog
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,dataTypeViewModel(new DataTypeViewModel(this))
    ,tagsHandler(nullptr)
    ,userTagsHandler(nullptr)
{
    ui->setupUi(this);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::openFile);

    ui->DataTypesView->setModel(dataTypeViewModel);
    ui->DataTypesView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // For Serial Number column
    ui->DataTypesView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // For Data Type column
    QFontMetrics fm(ui->DataTypesView->font());

    int guidWidth = fm.horizontalAdvance("{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}");
    ui->DataTypesView->horizontalHeader()->resizeSection(2, guidWidth);


    connect(ui->bigEndiancheckBox, &QCheckBox::stateChanged, this, &MainWindow::onEndianCheckboxStateChanged);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    dataTypeViewModel->setEndian(false); //Default is little endian


    connect(ui->openDiskButton, &QPushButton::clicked, this, &MainWindow::on_openDiskButton_clicked);

    // Handle tab close request
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, [=](int index) {
        // Remove the tab and clean up maps
        qDebug() << "Closing tab " << index;

        if (userTagsHandler->removeTab(index)) {
            qDebug() << "Tab removed successfully";
            ui->tabWidget->removeTab(index);

        } else {
            qDebug() << "Failed to remove tab";
        }

    });

}


void MainWindow::setTagsHandler(TagsHandler *tagsHandler)
{
    this->tagsHandler = tagsHandler;
}

void MainWindow::setUserTagsHandler(TagsHandler *userTagsHandler)
{
    this->userTagsHandler = userTagsHandler;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty()) {
        createNewTab(fileName);
    }
}

void MainWindow::on_openDiskButton_clicked()
{
    PhysicalDrivesDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedDrive = dialog.selectedDrive();
        if (!selectedDrive.isEmpty()) {
           // QMessageBox::information(this, tr("Selected Drive"), tr("You selected: %1").arg(selectedDrive));
              createNewTab(selectedDrive);
        }
    }
}
void MainWindow::createNewTab(const QString &fileName, bool sync)
{


    QFileInfo fileInfo(fileName);

    QRegularExpression systemDrivePattern(R"(\\\\\.\\(?:PhysicalDrive\d+|[A-Z]:))");
    if (!systemDrivePattern.match(fileName).hasMatch() && !fileInfo.exists()) {
        qDebug() << "File does not exist:" << fileName;
        return;
    }

    HexViewerForm *hexViewerForm = new HexViewerForm(this);
    quint64 index = ui->tabWidget->addTab(hexViewerForm, QFileInfo(fileName).fileName());

    //TagsHandler *tagsHandler = new TagsHandler("tags_database.db", "connection");

    if(sync){
        userTagsHandler->addNewTab(fileName);

    }

    hexViewerForm->setTagsHandler(tagsHandler);
    hexViewerForm->setUserTagsHandler(userTagsHandler);


    hexViewerForm->openFile(fileName,index);

    ui->tabWidget->setCurrentIndex(index);
    ui->tabWidget->setFocus();

    connect(hexViewerForm->hexEditor(), &HexEditor::selectionChanged, dataTypeViewModel, &DataTypeViewModel::updateData);
    connect(hexViewerForm->hexEditor(), &HexEditor::tagNameAndLength, this, &MainWindow::onTagNameAndLength);

}

void MainWindow::onTabChanged(int index)
{
    HexViewerForm *currentHexViewer = qobject_cast<HexViewerForm*>(ui->tabWidget->widget(index));
    if (currentHexViewer) {
        HexEditor *hexEditor = currentHexViewer->hexEditor();
        if (hexEditor) {
            connect(hexEditor, &HexEditor::selectionChanged, dataTypeViewModel, &DataTypeViewModel::updateData);
            connect(hexEditor, &HexEditor::selectionChanged, this, &MainWindow::onSelectionChanged);
            connect(hexEditor, &HexEditor::tagNameAndLength, this, &MainWindow::onTagNameAndLength);

            dataTypeViewModel->updateData(hexEditor->getSelectedBytes());
        }
    }
}

void MainWindow::onEndianCheckboxStateChanged(quint64 state)
{
    bool isBigEndian = (state == Qt::Checked);
    dataTypeViewModel->setEndian(isBigEndian);
}

void MainWindow::onSelectionChanged(const QByteArray &selectedData, quint64 startOffset, quint64 endOffset)
{
    // Update the selection details
    int selectionCount = selectedData.size();
    QString selectionDetails = QString("Selection : %1 - %2")
                                   .arg(startOffset)
                                   .arg(endOffset);

    ui->labelSelection->setText(selectionDetails);

    //Update selection count
    QString selectionCountText = QString("Selection Count : %1 (0x%2)")
                                   .arg(selectionCount)
                                  .arg(QString::number(selectionCount, 16).toUpper());
    ui->labelSelectionCount->setText(selectionCountText);


    // Update cursor position
    QString cursorPosition = QString("Cursor Position : %1 (0x%2)")
                                 .arg(endOffset)
                                 .arg(QString::number(endOffset, 16).toUpper());
    ui->labelCursorPosition->setText(cursorPosition);



    QString sector= QString("Sector: %1 ")
                         .arg(round(startOffset/512)+1);


    ui->labelSector->setText(sector);
}

void MainWindow::onTagNameAndLength(const QString &tagName, quint64 length, QString tagColor)
{
    // Calculate the brightness of the color
    QColor color(tagColor);
    int brightness = (color.red() * 299 + color.green() * 587 + color.blue() * 114) / 1000;

    QString style;
    if (brightness > 128) {
        // Light color, use grey background
        style = QString("background-color: black; color: %1;").arg(tagColor);
    } else {
        // Dark color, no background needed
        style = QString("color: %1;").arg(tagColor);
    }

    QString tagInfo;
    if (tagName.isEmpty()) {
        tagInfo = "Tag:";
    } else {
        tagInfo = QString("Tag: <span style='%1'>%2 (Length: %3)</span>").arg(style).arg(tagName).arg(length);
    }
    ui->labelTagName->setText(tagInfo);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    static bool isClosing = false;
    if (isClosing) {
        event->accept();
        return;
    }

    event->ignore();
    isClosing = true;

    int tabCount = ui->tabWidget->count();
    if (tabCount == 0) {
        event->accept();
        return;
    }

    QProgressDialog *progress = new QProgressDialog("Saving tags...", "Cancel", 0, tabCount, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setValue(0);

    std::shared_ptr<std::atomic<int>> completedTabs = std::make_shared<std::atomic<int>>(0);
    std::shared_ptr<std::atomic<bool>> allSuccessful = std::make_shared<std::atomic<bool>>(true);

    for (int i = 0; i < tabCount; ++i) {
        HexViewerForm *hexViewerForm = qobject_cast<HexViewerForm*>(ui->tabWidget->widget(i));
        if (hexViewerForm) {
            HexEditor *hexEditor = hexViewerForm->hexEditor();
            if (hexEditor) {
                hexEditor->syncTagsOnClose([this, completedTabs, allSuccessful, progress, tabCount](bool success) {
                    allSuccessful->store(allSuccessful->load() && success);
                    int completed = ++(*completedTabs);
                    progress->setValue(completed);

                    if (completed == tabCount) {
                        progress->close();
                        if (allSuccessful->load()) {
                            QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
                        } else {
                            QMessageBox::StandardButton reply = QMessageBox::warning(
                                this,
                                tr("Save Failed"),
                                tr("Failed to save tags for some tabs. Do you want to close anyway?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No
                                );
                            if (reply == QMessageBox::Yes) {
                                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
                            } else {
                                isClosing = false;
                            }
                        }
                        progress->deleteLater();
                    }
                });
            }
        }
    }
}
