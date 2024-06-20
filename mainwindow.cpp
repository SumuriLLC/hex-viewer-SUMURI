#include "headers/mainwindow.h"
#include "ui_mainwindow.h"




//Multiple instances will be created for each tab Form
#include "headers/hexviewerform.h"
//Open Dialog
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,dataTypeViewModel(new DataTypeViewModel(this))
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

//A new instance
void MainWindow::createNewTab(const QString &fileName)
{
    HexViewerForm *hexViewerForm = new HexViewerForm(this);



    hexViewerForm->openFile(fileName);
    quint64 index = ui->tabWidget->addTab(hexViewerForm, QFileInfo(fileName).fileName());
    ui->tabWidget->setCurrentIndex(index);
    ui->tabWidget->setFocus();

    connect(hexViewerForm->hexEditor(), &HexEditor::selectionChanged, dataTypeViewModel, &DataTypeViewModel::updateData);

}

void MainWindow::onTabChanged(int index)
{
    HexViewerForm *currentHexViewer = qobject_cast<HexViewerForm*>(ui->tabWidget->widget(index));
    if (currentHexViewer) {
        HexEditor *hexEditor = currentHexViewer->hexEditor();
        if (hexEditor) {
            connect(hexEditor, &HexEditor::selectionChanged, dataTypeViewModel, &DataTypeViewModel::updateData);
            connect(hexEditor, &HexEditor::selectionChanged, this, &MainWindow::onSelectionChanged);

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
}

