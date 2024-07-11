#include "headers/openingdialog.h"
#include "ui_openingdialog.h"
#include <QFileDialog>

#include "headers/mainwindow.h"
#include "headers/physicaldrivesdialog.h"


OpeningDialog::OpeningDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OpeningDialog)
    ,loadingDialog(new LoadingDialog(this))
{
    ui->setupUi(this);

    ui->radioButtonForensicImage->setChecked(true); // Default selection


    connect(ui->selectSourceButton, &QPushButton::clicked, this, &OpeningDialog::onSelectSourceClicked);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &OpeningDialog::onDialogButtonClicked);


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

void OpeningDialog::onDialogButtonClicked(QAbstractButton *button)
{


    QDialogButtonBox::StandardButton standardButton = ui->buttonBox->standardButton(button);


    switch (standardButton) {
    case QDialogButtonBox::Ok:


        // Handle OK button clicked
        if (! ui->selectSourcelineEdit->text().isEmpty()) {



            MainWindow *w = new MainWindow;
            w->createNewTab(ui->selectSourcelineEdit->text());
            w->show();

        }
        accept();
        break;
    case QDialogButtonBox::Cancel:
        // Handle Cancel button clicked
        reject();
        break;
    default:
        break;
    }
}

OpeningDialog::~OpeningDialog()
{
    delete ui;

}


