#include "headers/openingdialog.h"
#include "qmenu.h"
#include "ui_openingdialog.h"
#include <QFileDialog>

#include "headers/mainwindow.h"
#include "headers/physicaldrivesdialog.h"
#include <QMessageBox>



OpeningDialog::OpeningDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OpeningDialog)
    ,loadingDialog(new LoadingDialog(this))
    ,tagsHandler(nullptr)

{
    ui->setupUi(this);

    //Main Application Database
    tagsHandler = new TagsHandler("tags_database.db", "connection");


    ui->radioButtonForensicImage->setChecked(true); // Default selection


    connect(ui->selectSourceButton, &QPushButton::clicked, this, &OpeningDialog::onSelectSourceClicked);



    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &OpeningDialog::onOkButtonClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &OpeningDialog::reject);


    connect(ui->outputDirectoryPushButton, &QPushButton::clicked, this, &OpeningDialog::onSelectOutputDirectoryClicked);
    connect(ui->loadCasepushButton, &QPushButton::clicked, this, &OpeningDialog::onLoadCaseClicked);


}

void OpeningDialog::onLoadCaseClicked()
{
    QMenu menu;

    QList<QString> recentFolders = tagsHandler->getRecentFolders();

    for (const QString &folder : recentFolders) {
        QAction *folderAction = menu.addAction(folder);
        connect(folderAction, &QAction::triggered, this, &OpeningDialog::onRecentFolderClicked);
    }



    QAction *otherCaseAction = menu.addAction("Other Case");
    connect(otherCaseAction, &QAction::triggered, this, &OpeningDialog::onOtherCaseClicked);





    menu.exec(QCursor::pos());
}

void OpeningDialog::onRecentFolderClicked()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    QString dir = action->text();
    QString dbPath = dir + "/user_database.db";
    TagsHandler *usertagsHandler = new TagsHandler(dbPath, "user_connection");

    // Fetch the tab data from the Tab table
    QList<Tab> tabs = usertagsHandler->getTabs();

    if (!tabs.isEmpty()) {
        MainWindow *w = new MainWindow;
        w->setTagsHandler(tagsHandler);
        w->setUserTagsHandler(usertagsHandler);

        for (const Tab &tab : tabs) {
            // Add tags etc for each tab
            w->createNewTab(tab.filename, false);
        }
        w->show();
        this->hide();
    } else {
        QMessageBox::warning(this, tr("Load Error"), tr("No tabs found in the selected case."));
    }
}

void OpeningDialog::onOtherCaseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Case Directory"), "",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        QString dbPath = dir + "/user_database.db";
        TagsHandler *usertagsHandler = new TagsHandler(dbPath, "user_connection");




        // Fetch the tab data from the Tab table
        QList<Tab> tabs = usertagsHandler->getTabs();

        if (!tabs.isEmpty()) {
            MainWindow *w = new MainWindow;
            w->setTagsHandler(tagsHandler);
            w->setUserTagsHandler(usertagsHandler);

            for (const Tab &tab : tabs) {

                //Add tags etc for each tab
                w->createNewTab(tab.filename,false);
            }
            w->show();
            this->hide();
        } else {
            QMessageBox::warning(this, tr("Load Error"), tr("No tabs found in the selected case."));
        }
    }
}



void OpeningDialog::onSelectSourceClicked()
{
    if (ui->radioButtonPhysicalDrive->isChecked()) {
        PhysicalDrivesDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString selectedDrive = dialog.selectedDrive();
            if (!selectedDrive.isEmpty()) {
                ui->selectSourcelineEdit->setText(selectedDrive);
            }
        }
    } else {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
        if (!filePath.isEmpty()) {
            ui->selectSourcelineEdit->setText(filePath);
        }
    }
}


bool OpeningDialog::onOkButtonClicked()
{

    this->show();

    // Check each field individually and show one error message at a time
    if (ui->caseNamelineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Case Name is required. Please fill in the Case Name."));
        this->show();
        return false;
    }
    if (ui->caseNolineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Case Number is required. Please fill in the Case Number."));
        this->show();

        return false;
    }
    if (ui->selectSourcelineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Source Path is required. Please fill in the Source Path."));
        this->show();

        return false;
    }
    if (ui->outputDirectorylineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Output Directory is required. Please fill in the Output Directory."));
        this->show();

        return false;
    }

    // Fetch values from the UI elements
    QString caseName = ui->caseNamelineEdit->text();
    QString caseNumber = ui->caseNolineEdit->text();
    QString sourcePath = ui->selectSourcelineEdit->text();
    QString outputPath = ui->outputDirectorylineEdit->text();

    // Create folder with current date and time
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString folderName = currentDateTime.toString("yyyy-MM-dd_hh-mm-ss");
    QDir dir(outputPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString fullPath = dir.filePath(folderName);
    dir.mkpath(fullPath);

    QString dbPath = fullPath + "/user_database.db";

    TagsHandler *usertagsHandler = new TagsHandler(dbPath, "user_connection");

    // Create Metadata table and insert values
    usertagsHandler->createMetadataTable(caseName, caseNumber);
    usertagsHandler->createTabTable();
    usertagsHandler->createUserTagsTable();
    usertagsHandler->createTemplateTagsTable();

    //Create tags table

    MainWindow *w = new MainWindow;
    //Add recent folder
    tagsHandler->addRecentFolder(fullPath);
    w->setTagsHandler(tagsHandler);
    w->setUserTagsHandler(usertagsHandler);
    w->createNewTab(ui->selectSourcelineEdit->text());
    w->show();

    accept();

    return true;
}


OpeningDialog::~OpeningDialog()
{
    delete ui;

}

void OpeningDialog::onSelectOutputDirectoryClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), "",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->outputDirectorylineEdit->setText(dir);
    }
}



