#include "headers/gotooffsetdialog.h"
#include "headers/hexeditor.h"
#include "headers/hexviewerform.h"
#include "ui_gotooffsetdialog.h"
#include "headers/filesystemexception.h"

#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>

GoToOffsetDialog::GoToOffsetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GoToOffsetDialog)
{
    ui->setupUi(this);

    HexEditor *hexEditor = parent->findChild<HexEditor*>();
    if (hexEditor) {
        ui->offsetLineEdit->setText(QString::number(hexEditor->cursorPosition));
    }

    // Set up validators for input fields
    QRegularExpression hexRegExp("^[0-9A-Fa-f]+$");
    QValidator *hexValidator = new QRegularExpressionValidator(hexRegExp, this);
    QValidator *numberValidator = new QRegularExpressionValidator(QRegularExpression("^[0-9]+$"), this);

    ui->offsetLineEdit->setValidator(hexValidator);
    ui->clusterNumberlineEdit->setValidator(numberValidator);

    connect(ui->jumpButton, &QPushButton::clicked, this, &GoToOffsetDialog::onJumpButtonClicked);

    populatePartitionsComboBox();
}

GoToOffsetDialog::~GoToOffsetDialog()
{
    delete ui;
}

void GoToOffsetDialog::populatePartitionsComboBox()
{
    HexViewerForm *hexViewerForm = qobject_cast<HexViewerForm*>(parentWidget());
    if (!hexViewerForm) {
        qDebug() << "HexViewerForm not found";
        return;
    }

    if (!hexViewerForm->fsHandler) {
        qDebug() << "fsHandler not initialized";
        return;
    }

    QList<QStringList> partitions = hexViewerForm->fsHandler->getAvailablePartitions();

    if(partitions.isEmpty()){
        ui->tabWidget->setTabVisible(1,false);
    }else{
        ui->tabWidget->setTabVisible(1,true);
    }

    for (const QStringList &partition : partitions) {
        if (partition.size() >= 2) {
            ui->partitionsComboBox->addItem(partition[1], partition[0]);
        }
    }
}

quint64 GoToOffsetDialog::offset() const
{
    qDebug() << "is cluster tab active" << ui->tabWidget->currentIndex();

    bool ok;
    qint64 baseOffset;

    //Go to hex
    if(ui->tabWidget->currentIndex()==0){
        QString offsetText = ui->offsetLineEdit->text();

        if (ui->formatComboBox->currentText() == "Hex") {
            baseOffset = offsetText.toLongLong(&ok, 16);
        } else {
            baseOffset = offsetText.toLongLong(&ok);
        }
        if (!ok) {
            QMessageBox::warning(const_cast<GoToOffsetDialog*>(this), tr("Invalid Input"), tr("Please enter a valid hexadecimal or decimal offset."));
            return 0;
        }

        qint64 multiplier = ui->multiplierComboBox->currentText().toLongLong(&ok);
        if (!ok) {
            multiplier = 1;
        }

        qint64 offset = baseOffset * multiplier;

        HexEditor *hexEditor = parentWidget()->findChild<HexEditor*>();
        if (hexEditor) {
            if (ui->fromCursorRadioButton->isChecked()) {
                offset += hexEditor->cursorPosition;
            } else if (ui->fromEndRadioButton->isChecked()) {
                offset = (hexEditor->fileSize-1) - offset;
            }
        }

        // Ensure the offset is within the valid range
        if (offset < 0) {
            offset = 0;
        } else if (hexEditor && offset >= hexEditor->fileSize) {
            offset = hexEditor->fileSize - 1;
        }

        qDebug() << "Returned offset" << offset;
        return static_cast<quint64>(offset);

    } else {
        //Go to cluster
        QString clusterText = ui->clusterNumberlineEdit->text();

        baseOffset = clusterText.toLongLong(&ok);
        if (!ok) {
            QMessageBox::warning(const_cast<GoToOffsetDialog*>(this), tr("Invalid Input"), tr("Please enter a valid cluster number."));
            return 0;
        }
        qDebug() << "Go to cluster" << baseOffset;

        HexViewerForm *hexViewerForm = qobject_cast<HexViewerForm*>(parentWidget());
        if (!hexViewerForm) {
            qDebug() << "HexViewerForm not found";
            return 0;
        }

        if (!hexViewerForm->fsHandler) {
            qDebug() << "fsHandler not initialized";
            return 0;
        }

        qDebug() << "File system type is " << hexViewerForm->fsHandler->getAvailablePartitions();



        try {
            qint64 partitionIndex = ui->partitionsComboBox->currentData().toInt();
            qDebug() << "opening partitionIndex " << partitionIndex << " with offset " << baseOffset;

            quint64 clusterOffset = hexViewerForm->fsHandler->calculateClusterOffset(partitionIndex, baseOffset);

            qDebug() << "Cluster offset calculated" << clusterOffset;
            return clusterOffset;
        } catch (FileSystemException &e) {
            QMessageBox::critical(const_cast<GoToOffsetDialog*>(this), tr("Error"), QString::fromStdString(e.what()));

            ui->clusterNumberlineEdit->setText("0");
            qDebug() << "returning to offset 0";

            return 0;
        }
    }

    return 0;
}

void GoToOffsetDialog::onJumpButtonClicked()
{
    quint64 offsetValue = offset();
    qDebug() << "Jump to offset:" << offsetValue;



    // Perform the action needed with the calculated offset
    HexViewerForm *hexViewerForm = qobject_cast<HexViewerForm*>(parentWidget());

    if (hexViewerForm) {
        hexViewerForm->jumpToOffset(offsetValue);
    }

    // Prevent the dialog from closing
    // Do not call accept() or close()
}
