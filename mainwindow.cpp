#include "headers/mainwindow.h"
#include "ui_mainwindow.h"




//Multiple instances will be created for each tab Form
#include "headers/hexviewerform.h"
//Open Dialog
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::openFile);
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
    int index = ui->tabWidget->addTab(hexViewerForm, QFileInfo(fileName).fileName());
    ui->tabWidget->setCurrentIndex(index);
    ui->tabWidget->setFocus();


   /*HexEditor *hexEditor = new HexEditor(this);

   QFile file(fileName);
   if (!file.open(QIODevice::ReadOnly)) {
       QMessageBox::warning(this, tr("Error"), tr("Failed to open file"));
       delete hexEditor;
       return;
   }

   QByteArray fileData = file.readAll();
   hexEditor->setData(fileData);

   int index = ui->tabWidget->addTab(hexEditor, QFileInfo(fileName).fileName());
   ui->tabWidget->setCurrentIndex(index); // Set the new tab as the current tab
   ui->tabWidget->setFocus(); // Ensure the tab widget gets focus */




}
