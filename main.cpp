#include "headers/openingdialog.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QDateTime>
#include <QMessageBox>
#include <QIcon>

#include "headers/testfilesystemhandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/app_logo/hex_viewer_icon.ico"));
    a.setStyle(QStyleFactory::create("windowsvista"));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SamuriHexViewerV1_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    //////////////////////////Testing Sandbox/////////////////////////////////
    TestFileSystemHandler testHandler;
    testHandler.runTests();
    //////////////////////////////////////////////////////////////////////////
    OpeningDialog o;
    o.show();
    return a.exec();
}
