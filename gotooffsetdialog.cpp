#include "headers/gotooffsetdialog.h"
#include "headers/hexeditor.h"
#include "ui_gotooffsetdialog.h"

GoToOffsetDialog::GoToOffsetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GoToOffsetDialog)
{
    ui->setupUi(this);

    HexEditor *hexEditor = parent->findChild<HexEditor*>();
    if (hexEditor) {
        ui->offsetLineEdit->setText(QString::number(hexEditor->cursorPosition));
    }
}

GoToOffsetDialog::~GoToOffsetDialog()
{
    delete ui;
}

quint64 GoToOffsetDialog::offset() const
{
    bool ok;
    qint64 baseOffset;
    if (ui->formatComboBox->currentText() == "Hex") {
        baseOffset = ui->offsetLineEdit->text().toLongLong(&ok, 16);
    } else {
        baseOffset = ui->offsetLineEdit->text().toLongLong(&ok);
    }
    if (!ok) {
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
}
