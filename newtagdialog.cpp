#include "headers/newtagdialog.h"
#include "ui_newtagdialog.h"
#include <QColorDialog>

NewTagDialog::NewTagDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewTagDialog)
    ,selectedColor("#666666")
{
    ui->setupUi(this);

    connect(ui->pushButtonChangeColor, &QPushButton::clicked, this, &NewTagDialog::openColorPicker);
    connect(ui->tagButtonBox, &QDialogButtonBox::accepted, this, &NewTagDialog::onSaveButtonClicked);

    ui->labelColorShow->setStyleSheet(QString("background-color: %1").arg(selectedColor));
    connect(ui->lineEditStartAddr, &QLineEdit::textChanged, this, &NewTagDialog::updateHexLabels);
    connect(ui->lineEditEndAddr, &QLineEdit::textChanged, this, &NewTagDialog::updateHexLabels);


}

void NewTagDialog::openColorPicker()
{
    QColor color = QColorDialog::getColor(QColor(selectedColor), this, "Select Color");
    if (color.isValid()) {
        // Handle the selected color
        qDebug() << "Selected color:" << color.name();
        selectedColor=color.name();
        //ui->labelColorShow->setText(selectedColor);
        ui->labelColorShow->setStyleSheet(QString("background-color: %1").arg(selectedColor));

    }
}

void NewTagDialog::setStartAddress(quint64 address)
{
    ui->lineEditStartAddr->setText(QString::number(address).toUpper());
}

void NewTagDialog::setEndAddress(quint64 address)
{
    ui->lineEditEndAddr->setText(QString::number(address).toUpper());
}

void NewTagDialog::setDescription(QString description)
{
    ui->lineEditDescription->setText(description);
}


void NewTagDialog::onSaveButtonClicked()
{
    bool ok;
    quint64 startAddr = ui->lineEditStartAddr->text().toULongLong(&ok);
    if (!ok) {
        // Handle the error
        qDebug() << "Invalid start address";
        return;
    }

    quint64 endAddr = ui->lineEditEndAddr->text().toULongLong(&ok);
    if (!ok) {
        // Handle the error
        qDebug() << "Invalid end address";
        return;
    }

    if (endAddr < startAddr) {
        // Handle the error
        qDebug() << "End address is less than start address";
        return;
    }

    quint64 length = (endAddr - startAddr)+1;
    QString description = ui->lineEditDescription->text();

    qDebug() << "emitting event";

    emit tagSaved(startAddr, length, description, selectedColor,"user");
    accept(); // Close the dialog with QDialog::Accepted
}


void NewTagDialog::updateHexLabels()
{
    bool ok;
    quint64 startAddr = ui->lineEditStartAddr->text().toULongLong(&ok);
    if (ok) {
        ui->labelStartAddressHex->setText(QString("0x%1").arg(QString::number(startAddr, 16).toUpper()));
    } else {
        ui->labelStartAddressHex->setText("Invalid");
    }

    quint64 endAddr = ui->lineEditEndAddr->text().toULongLong(&ok);
    if (ok) {
        ui->labelEndAddressHex->setText(QString("0x%1").arg(QString::number(endAddr, 16).toUpper()));
    } else {
        ui->labelEndAddressHex->setText("Invalid");
    }
}

NewTagDialog::~NewTagDialog()
{
    delete ui;
}
