#include "headers/testfilesystemhandler.h"
#include <QDebug>
#include <QMessageBox>
#include "headers/filesystemexception.h"

TestFileSystemHandler::TestFileSystemHandler()
{
    fsHandler = new FileSystemHandler();
}

void TestFileSystemHandler::runTests()
{
    // Add test cases here
    testOpenImage("O:\\test_disk_images\\ntfs1-gen0.E01");
}

void TestFileSystemHandler::testOpenImage(const QString& fileName)
{
    try {
        qDebug() << "Testing opening image:" << fileName;

        fsHandler->printSupportedFormats();

        if (fsHandler->openImage(fileName)) {
            qDebug() << "Image opened successfully.";

            int partitionCount = fsHandler->getPartitionCount();
            qDebug() << "Total partitions found:" << partitionCount;

            for (int i = 0; i < partitionCount; ++i) {
                QString partitionDescription = fsHandler->getPartitionDescription(i);
                qDebug() << "Partition " << i << " Description:" << partitionDescription;

                QList<QStringList> fileList = fsHandler->listFilesInDirectory(i, "/");

                if (fileList.isEmpty()) {
                    qDebug() << "Partition " << i << " is empty or cannot be read.";
                    continue;
                }

                qDebug() << "Files in Partition " << i << ":";
                for (const QStringList &fileAttributes : fileList) {
                    qDebug() << "  Name:" << fileAttributes.at(0);
                    qDebug() << "  File Path:" << fileAttributes.at(1);
                    qDebug() << "  Size:" << fileAttributes.at(2);
                    qDebug() << "  Creation Date:" << fileAttributes.at(3);
                    qDebug() << "  Modification Date:" << fileAttributes.at(4);
                    qDebug() << "  Last Access Date:" << fileAttributes.at(5);
                    qDebug() << "  Inode No.:" << fileAttributes.at(6);
                    qDebug() << "  Cluster Number:" << fileAttributes.at(7);
                }
            }

            QList<QStringList> partitionDetails = fsHandler->getPartitionDetails();
            for (const QStringList &details : partitionDetails) {
                qDebug() << "Partition Details:" << details;
            }
        } else {
            qDebug() << "Failed to open image.";
        }
    } catch (const FileSystemException& e) {
        qDebug() << "Exception:" << e.getMessage();
        QMessageBox::critical(nullptr, "Error", e.getMessage());
    }
}
