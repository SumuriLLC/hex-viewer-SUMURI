#include "headers/hexviewerform.h"
#include "ui_hexviewerform.h"
#include <QDateTime>
#include "headers/hexeditor.h"



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
   // delete file; // Delete the old file object if any

    HexEditor *hexEditor = ui->hexEditorWidget;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open file"));
        delete hexEditor;
        return;
    }

    QByteArray fileData = file.readAll();
    hexEditor->setData(fileData);


   /* fileSize = file->size();
    fileData.resize(fileSize);

    // Initially read the first chunk of data
    readFileChunk(0, chunkSize);

    updateScrollBar();  */

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



HexViewerForm::~HexViewerForm()
{
    delete ui;
}
