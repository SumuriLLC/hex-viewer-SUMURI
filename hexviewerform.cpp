#include "headers/hexviewerform.h"
#include "ui_hexviewerform.h"
#include <QDateTime>
#include "headers/hexeditor.h"

#include "headers/gotooffsetdialog.h"


HexViewerForm::HexViewerForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HexViewerForm)


{
    qDebug() << "setup.";

    ui->setupUi(this);
    /*
    // Example usage of TSK library functions
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("All Files (*)"));
    if (fileName.isEmpty())
        return;

#if defined(_MSC_VER) || defined(__MINGW32__)
    // Convert QString to TSK_TCHAR for Windows (assuming TSK_TCHAR is wchar_t)
    std::wstring fileNameW = fileName.toStdWString();
    const TSK_TCHAR* filePath = fileNameW.c_str();
#else
    // For non-Windows platforms (assuming TSK_TCHAR is char)
    const TSK_TCHAR* filePath = fileName.toStdString().c_str();
#endif

    qDebug()  <<  "file" << filePath;
    TSK_IMG_INFO *img = tsk_img_open_sing(filePath, TSK_IMG_TYPE_DETECT, 0);
    if (img == nullptr) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open image"));
        return;
    }

    logImgInfo(img);


    // Close the image when done
    tsk_img_close(img);


*/

    ui->tagstabWidget->setVisible(false);
    connect(ui->showTablesButton, &QPushButton::clicked, this, &HexViewerForm::onShowTablesClicked);

    //Select 16 as default value
    ui->bytesPerLinecomboBox->setCurrentIndex(3);
    // Connect the bytes per line dropdown
    connect(ui->bytesPerLinecomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HexViewerForm::onBytesPerLineChanged);

    connect(ui->jumpToOffsetButton, &QPushButton::clicked, this, &HexViewerForm::onGoToOffsetClicked);


}

/*

void HexViewerForm::logImgInfo(TSK_IMG_INFO *img) {
    if (img == nullptr) {
        qDebug() << "TSK_IMG_INFO is null.";
        return;
    }

    qDebug() << "TSK_IMG_INFO details:";
    qDebug() << "Image type:" << img->itype;
    qDebug() << "Sector size:" << img->sector_size;
    qDebug() << "Number of sectors:" << img->sector_size;

    for (int i = 0; i < img->num_img; ++i) {
        qDebug() << "Image file" << i << ":" << img->images[i];
    }

}

*/

void HexViewerForm::openFile(const QString &fileName)
{

    HexEditor *hexEditor = ui->hexEditorWidget;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open file"));
        delete hexEditor;
        return;
    }

    QByteArray fileData = file.readAll();
    hexEditor->setData(fileData);
    hexEditor->setSelectedByte(0);


}

HexEditor* HexViewerForm::hexEditor() const
{
    return ui->hexEditorWidget;
}



void HexViewerForm::onShowTablesClicked()
{
    bool isVisible = ui->tagstabWidget->isVisible();
    ui->tagstabWidget->setVisible(!isVisible);

    if (isVisible) {
        ui->showTablesButton->setText("Show Tables");
    } else {
        ui->showTablesButton->setText("Hide Tables");
    }

}

void HexViewerForm::onBytesPerLineChanged(int index)
{
    quint64 bytesPerLine = ui->bytesPerLinecomboBox->itemText(index).toInt();
    HexEditor *hexEditor = ui->hexEditorWidget;
    hexEditor->changeBytesPerLine(bytesPerLine);
}

void HexViewerForm::onGoToOffsetClicked()
{

    GoToOffsetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {

        qDebug() << "onGoToOffsetClicked accepted";

        quint64 newOffset = dialog.offset();
        if (newOffset < ui->hexEditorWidget->getData().size()) {
            ui->hexEditorWidget->clearSelection();
            ui->hexEditorWidget->setCursorPosition(newOffset);
            ui->hexEditorWidget->ensureCursorVisible();
        }
    }

}


HexViewerForm::~HexViewerForm()
{
    delete ui;
}
