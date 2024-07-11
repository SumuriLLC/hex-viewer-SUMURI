#include "headers/testfilesystemhandler.h"
#include <QDebug>
#include <QMessageBox>
#include "headers/filesystemexception.h"
#include "headers/tagshandler.h"
#include "headers/ewfdevice.h"
#include "headers/windowsdrivedevice.h"

TestFileSystemHandler::TestFileSystemHandler()
{
    fsHandler = new FileSystemHandler();
    // tagsHandler = new TagsHandler("tags_database.db","initial_connection"); // Use your actual database path here
}

void TestFileSystemHandler::runTests()
{
   /* QString drivePath = "\\\\.\\PhysicalDrive1";
    WindowsDriveDevice physicalDrive(drivePath);

    if (!physicalDrive.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open physical drive.";
        return;
    }

    qint64 seekOffset = 4;  // Example: seek to the second sector

    if (!physicalDrive.seek(seekOffset)) {
        qCritical() << "Failed to seek to offset" << seekOffset << "on the drive.";
        return;
    }
    QByteArray data = physicalDrive.read(1000); // Read the first 1000 bytes
    if (data.isEmpty()) {
        qCritical() << "Failed to read data from the drive.";
        return;
    }

    QString hexData = data.toHex(' ').toUpper();
    qDebug() << hexData;

    physicalDrive.close();

*/
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

void TestFileSystemHandler::testAddTag()
{
    qDebug() << "Testing adding a tag";
    tagsHandler->addTemporaryTag(0, 100, "Test Tag", "#FF5733", "FAT32");
    tagsHandler->saveTagsToDatabase();
    qDebug() << "Tag added successfully";
}

void TestFileSystemHandler::testGetTags()
{
    qDebug() << "Testing retrieving tags";
    QList<Tag> tags = tagsHandler->getTags();
    for (const Tag &tag : tags) {
        qDebug() << "Tag:" << tag.offset << tag.length << tag.description << tag.color << tag.category;
    }
}

void TestFileSystemHandler::testUpdateTag()
{
    qDebug() << "Testing updating a tag";
    QList<Tag> tags = tagsHandler->getTags();
    if (!tags.isEmpty()) {
        Tag tag = tags.first();
        tag.description = "Updated Tag";
        tag.color = "#123456";
        tagsHandler->updateTag(tag);
        tagsHandler->saveTagsToDatabase();
        qDebug() << "Tag updated successfully";
    } else {
        qDebug() << "No tags found to update";
    }
}

void TestFileSystemHandler::testDeleteTag()
{
    qDebug() << "Testing deleting a tag";
    QList<Tag> tags = tagsHandler->getTags();
    if (!tags.isEmpty()) {
        Tag tag = tags.first();
        tagsHandler->deleteTag(tag);
        tagsHandler->saveTagsToDatabase();
        qDebug() << "Tag deleted successfully";
    } else {
        qDebug() << "No tags found to delete";
    }
}
