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
    quint64 baseOffset;
    if (ui->formatComboBox->currentText() == "Hex") {
        baseOffset = ui->offsetLineEdit->text().toULongLong(&ok, 16);
    } else {
        baseOffset = ui->offsetLineEdit->text().toULongLong(&ok);
    }
    if (!ok) {
        return 0;
    }

    quint64 multiplier = ui->multiplierComboBox->currentText().toULongLong(&ok);
    if (!ok) {
        multiplier = 1;
    }

    quint64 offset = baseOffset * multiplier;

    HexEditor *hexEditor = parentWidget()->findChild<HexEditor*>();
    if (hexEditor) {
        if (ui->fromCursorRadioButton->isChecked()) {
            offset += hexEditor->cursorPosition;
        } else if (ui->fromEndRadioButton->isChecked()) {
            offset = (hexEditor->fileSize-1) - offset;
        }
    }

    qDebug() << "Returned offfset" << offset;
    return offset;
}
