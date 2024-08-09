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

    connect(ui->eulaPushButton, &QPushButton::clicked, this, &OpeningDialog::showEulaDialog);




}

void OpeningDialog::showEulaDialog()
{
    QDialog eulaDialog(this);
    eulaDialog.setWindowTitle("End User License Agreement");
    eulaDialog.setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(&eulaDialog);

    QTextEdit *eulaText = new QTextEdit(&eulaDialog);
    eulaText->setReadOnly(true);
    eulaText->setHtml("<p><b>SUMURI HEX VIEWER</b><br>"
                      "Copyright - SUMURI LLC<br>"
                      "www.sumuri.com</p>"

                      "<p style='color: red;'><b>IMPORTANT, PLEASE READ CAREFULLY. THIS IS A LICENSE AGREEMENT</b></p>"

                      "<p><b>This SUMURI HEX VIEWER is protected by copyright laws and international copyright treaties, "
                      "as well as other intellectual property laws and treaties. This SUMURI HEX VIEWER is licensed, not sold.</b></p>"

                      "<p><b>End-User License Agreement</b></p>"

                      "<p>This End User License Agreement ('EULA') is a legal agreement between you (either an individual or a single entity) "
                      "and SUMURI LLC with regard to the copyrighted software (herein referred to as SUMURI HEX VIEWER or 'software') "
                      "provided with this EULA. The SUMURI HEX VIEWER includes computer software, the associated media, any printed materials, "
                      "and any 'online' or electronic documentation. Use of any software and related documentation ('software') provided to you "
                      "by SUMURI HEX VIEWER in whatever form or media, will constitute your acceptance of these terms, unless separate terms "
                      "are provided by the software supplier, in which case certain additional or different terms may apply. If you do not agree "
                      "with the terms of this EULA, do not download, install, copy or use the software. By installing, copying or otherwise using "
                      "the SUMURI HEX VIEWER, you agree to be bound by the terms of this EULA. If you do not agree to the terms of this EULA, "
                      "SUMURI LLC is unwilling to license the SUMURI HEX VIEWER to you.</p>"

                      "<p><b>Eligible License</b> - This software is available for license solely to software owners, with no right of duplication "
                      "or further distribution, licensing, or sub-licensing.</p>"

                      "<p><b>License Grant</b> - SUMURI LLC grants to, active International Association of Computer Investigative Specialists (IACIS) "
                      "members a personal, non-transferable and non-exclusive right to use the copy of the software provided with this EULA. You agree "
                      "you will not copy or duplicate the software without permission from SUMURI LLC. You agree that you may not copy the written "
                      "materials accompanying the software. Modifying, translating, renting, copying, transferring or assigning all or part of the "
                      "software, or any rights granted hereunder, to any other persons and removing any proprietary notices, labels or marks from the "
                      "software is strictly prohibited. Furthermore, you hereby agree not to create derivative works based on the software. You may not "
                      "transfer this software. The License Grant will remain in effect for the duration of the agreement between IACIS and SUMURI LLC.</p>"

                      "<p><b>Copyright</b> - The software is licensed, not sold. You acknowledge that no title to the intellectual property in the software "
                      "is transferred to you. You further acknowledge that title and full ownership rights to the software will remain the exclusive property "
                      "of SUMURI LLC and/or its suppliers, and you will not acquire any rights to the software, except as expressly set forth above. All copies "
                      "of the software will contain the same proprietary notices as contained in or on the software. All title and copyrights in and to the "
                      "SUMURI HEX VIEWER (including but not limited to any images, photographs, animations, video, audio, music, text and ''applets,'' "
                      "incorporated into the SUMURI HEX VIEWER), the accompanying printed materials, and any copies of the SUMURI HEX VIEWER, are owned by "
                      "SUMURI LLC. The SUMURI HEX VIEWER is protected by copyright laws and international treaty provisions. You may not copy the printed "
                      "materials accompanying the SUMURI HEX VIEWER without the permission of SUMURI LLC.</p>"

                      "<p><b>Reverse Engineering</b> - You agree that you will not attempt, and if you are a corporation, you will use your best efforts to "
                      "prevent your employees and contractors from attempting to reverse compile, modify, translate or disassemble the software in whole or "
                      "in part. Any failure to comply with the above or any other terms and conditions contained herein will result in the automatic termination "
                      "of this license and the reversion of the rights granted hereunder to SUMURI LLC.</p>"

                      "<p><b>Disclaimer of Warranty</b> - The software is provided 'AS IS' without warranty of any kind. SUMURI LLC and its suppliers disclaim "
                      "and make no express or implied warranties and specifically disclaim the warranties of merchantability, fitness for a particular purpose "
                      "and non-infringement of third-party rights. The entire risk as to the quality and performance of the software is with you. Neither SUMURI "
                      "LLC nor its suppliers warrant that the functions contained in the software will meet your requirements or that the operation of the software "
                      "will be uninterrupted or error-free. SUMURI LLC is not obligated to provide any updates to the software for any user who does not have a "
                      "software maintenance subscription.</p>"

                      "<p><b>Limitation of Liability</b> - SUMURI LLC's entire liability and your exclusive remedy under this EULA shall not exceed the price paid "
                      "for the software, if any. In no event shall SUMURI LLC or its suppliers be liable to you for any consequential, special, incidental or "
                      "indirect damages of any kind arising out of the use or inability to use the software, even if SUMURI LLC or its supplier has been advised "
                      "of the possibility of such damages, or any claim by a third party.</p>"

                      "<p><b>Rental</b> - You may not loan, rent, or lease the software.</p>"

                      "<p><b>Training</b> - Other than IACIS official events, the SUMURI HEX VIEWER may not be used for training by other agencies or persons "
                      "without written permission from IACIS and SUMURI.</p>"

                      "<p><b>Transfer</b> - You may not transfer the software to a third party, without written consent from SUMURI LLC and written acceptance "
                      "of the terms of this Agreement by the transferee. Your license is automatically terminated if you transfer the software without the "
                      "written consent of SUMURI LLC. You are to ensure that the software is not made available in any form to anyone not subject to this Agreement.</p>"

                      "<p><b>Upgrades</b> - If the software is an upgrade from an earlier release or previously released version, you now may use that upgraded "
                      "product only in accordance with this EULA. If the SUMURI HEX VIEWER is an upgrade of a software program that you licensed as a single product, "
                      "the SUMURI HEX VIEWER may be used only as part of that single product package and may not be separated for use on more than one computer.</p>"

                      "<p><b>OEM Product Support</b> - Product support for the SUMURI HEX VIEWER is provided by SUMURI LLC. For product support, please contact "
                      "SUMURI LLC. Should you have any questions concerning this, please refer to the address provided in the documentation.</p>"

                      "<p><b>No Liability for Consequential Damages</b> - In no event shall SUMURI LLC or its suppliers be liable for any damages whatsoever "
                      "(including, without limitation, incidental, direct, indirect special and consequential damages, damages for loss of business profits, "
                      "business interruption, loss of business information, or other pecuniary loss) arising out of the use or inability to use this 'SUMURI LLC' "
                      "product, even if SUMURI LLC has been advised of the possibility of such damages. Because some states/countries do not allow the exclusion "
                      "or limitation of liability for consequential or incidental damages, the above limitation may not apply to you.</p>"

                      "<p><b>Indemnification By You</b> - If you distribute the software in violation of this Agreement, you agree to indemnify, hold harmless "
                      "and defend SUMURI LLC and its suppliers from and against any claims or lawsuits, including attorney's fees that arise or result from the "
                      "use or distribution of the software in violation of this Agreement.</p>"

                      "<p><b>Jurisdiction</b> - The parties consent to the exclusive jurisdiction and venue of the federal and state courts located in the State "
                      "of Delaware, USA, in any action arising out of or relating to this Agreement. The parties waive any other venue to which either party might "
                      "be entitled by domicile or otherwise.</p>");



    layout->addWidget(eulaText);

    eulaDialog.setLayout(layout);
    eulaDialog.resize(800, 600);

    eulaDialog.exec();
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
    tagsHandler->createRecentFoldersTable();
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



