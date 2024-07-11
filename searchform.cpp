#include "headers/searchform.h"
#include "ui_searchform.h"

searchform::searchform(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::searchform)
{
    ui->setupUi(this);
}

searchform::~searchform()
{
    delete ui;
}

QString searchform::getSearchPattern() const {
    return ui->searchLineEdit->text();
}

QString searchform::getSearchType() const {
    return ui->typeSelectComboBox->currentText();
}

QPushButton* searchform::getSearchButton() const {
    return ui->searchButton;
}
