#include "headers/filesystemhandler.h"
#include "headers/filesystemexception.h"
#include <QDebug>
#include <QDateTime>
#include <tsk/libtsk.h>
#include <tsk/tsk_tools_i.h>
#include <tsk/fs/tsk_fatxxfs.h>
#include <tsk/fs/tsk_ext2fs.h>


#include <QFile>
#include <QIODevice>

FileSystemHandler::FileSystemHandler(QObject *parent)
    : QObject(parent),
    fileSystemType(""),
    img(nullptr),
    vs(nullptr),
    fsOpenedDirectly(false)
{
}

FileSystemHandler::~FileSystemHandler()
{
    closeImage();
}

bool FileSystemHandler::openImage(const QString &fileName)
{

        closeImage();  // Close any previously opened image

#if defined(_MSC_VER) || defined(__MINGW32__)
        std::wstring fileNameW = fileName.toStdWString();
        const TSK_TCHAR *images[] = {reinterpret_cast<const TSK_TCHAR*>(fileNameW.c_str())};
#else
        const TSK_TCHAR *images[] = {fileName.toStdString().c_str()};
#endif


        qDebug()<< "Opening........."<< fileName;
        img = tsk_img_open(1, images, TSK_IMG_TYPE_DETECT, 0);

        if (img == nullptr) {
            throw FileSystemException("Failed to open file: " + getLastError());
        }

        vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);
        if (vs == nullptr) {
            qDebug() << "Failed to open volume system, trying to open as file system...";
            return openImageAsFileSystem();
        }




        return true;

}

bool FileSystemHandler::openImageAsFileSystem()
{

        TSK_FS_INFO *fs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);
        if (fs == nullptr) {
            throw FileSystemException("Failed to open image  as file system: " + getLastError());
        }

        openFileSystems.append(fs);
        fsOpenedDirectly = true;

        return true;

}

void FileSystemHandler::closeImage()
{
    if (vs) {
        tsk_vs_close(vs);
        vs = nullptr;
    }
    if (img) {
        tsk_img_close(img);
        img = nullptr;
    }

    for (TSK_FS_INFO* fs : openFileSystems) {
        tsk_fs_close(fs);
    }
    openFileSystems.clear();

    fsOpenedDirectly = false;
}

int FileSystemHandler::getPartitionCount() const
{
    return vs ? vs->part_count : (fsOpenedDirectly ? 1 : 0);
}

QString FileSystemHandler::getPartitionDescription(uint partitionIndex) const
{
    if (vs) {
        if (partitionIndex >= vs->part_count)
            return QString();

        const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, partitionIndex);
        return part ? QString::fromUtf8(part->desc) : QString();
    } else if (fsOpenedDirectly && partitionIndex == 0) {
        return "Single File System";
    }

    return QString();
}

TSK_FS_INFO* FileSystemHandler::getFileSystem(uint partitionIndex)
{
    if (vs) {
        if (partitionIndex >= vs->part_count)
            throw FileSystemException("Invalid partition index");

        const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, partitionIndex);
        if (!part)
            throw FileSystemException("Failed to get partition");

        TSK_FS_INFO *fs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);
        if (fs) {
            openFileSystems.append(fs);
        } else {

            qDebug()<<"Failed to open file system...";
           // throw FileSystemException("Failed to open file system");
        }

        return fs;
    } else if (fsOpenedDirectly && partitionIndex == 0) {
        return openFileSystems.first();
    }

    throw FileSystemException("Invalid partition index");
}

QList<QStringList> FileSystemHandler::listFilesInDirectory(int partitionIndex, const QString &directoryPath)
{
    QList<QStringList> fileList;

        TSK_FS_INFO *fs = getFileSystem(partitionIndex);
        if (!fs)
            return fileList;

        TSK_FS_DIR *dir = tsk_fs_dir_open(fs, directoryPath.toStdString().c_str());
        if (!dir) {
            throw FileSystemException("Failed to open directory: " + directoryPath);
        }

        bool dotFound = false;
        bool dotDotFound = false;

        for (size_t i = 0; i < dir->names_used; i++) {
            TSK_FS_NAME *fs_name = &dir->names[i];

            if (QString::fromUtf8(fs_name->name) == ".") {
                dotFound = true;
            }
            if (QString::fromUtf8(fs_name->name) == "..") {
                dotDotFound = true;
            }

            TSK_FS_FILE *file = tsk_fs_file_open_meta(fs, nullptr, fs_name->meta_addr);
            if (!file) {
                qDebug() << "File " << fs_name->name << " could not be read";
                continue;
            }

            QString filePath = directoryPath;
            if (!filePath.endsWith('/')) {
                filePath += '/';
            }
            filePath += QString::fromUtf8(fs_name->name);

            QString fileType = (fs_name->type == TSK_FS_NAME_TYPE_DIR) ? "Directory" : "File";


            int StartingClusterNumber=0;

            if( fs->ftype == TSK_FS_TYPE_NTFS ){
                StartingClusterNumber=file->meta->seq;
            }

             if (fs->ftype == TSK_FS_TYPE_EXFAT || fs->ftype == TSK_FS_TYPE_FAT12 || fs->ftype == TSK_FS_TYPE_FAT16 || fs->ftype == TSK_FS_TYPE_FAT32) {

                TSK_DADDR_T *addr_ptr;
                addr_ptr = (TSK_DADDR_T *) file->meta->content_ptr;


               // qDebug() << "content_ptr " << addr_ptr[0];

                 StartingClusterNumber=addr_ptr[0];
            }


            QStringList fileAttributes;
            fileAttributes << QString::fromUtf8(fs_name->name)
                           << filePath
                           << fileType
                           << formatSize(file->meta->size)
                           << QDateTime::fromSecsSinceEpoch(file->meta->crtime).toLocalTime().toString("yyyy-MM-dd hh:mm:ss")
                           << QDateTime::fromSecsSinceEpoch(file->meta->mtime).toLocalTime().toString("yyyy-MM-dd hh:mm:ss")
                           << QDateTime::fromSecsSinceEpoch(file->meta->atime).toLocalTime().toString("yyyy-MM-dd hh:mm:ss")
                           << QString::number(file->meta->addr)
                           << QString::number(StartingClusterNumber)
                           << fileType;


            fileList.append(fileAttributes);





            tsk_fs_file_close(file);
        }


            if (!dotDotFound) {
                QString parentPath = directoryPath;
                if (parentPath.endsWith('/')) {
                    parentPath.chop(1);
                }
                parentPath = parentPath.left(parentPath.lastIndexOf('/'));
                if (parentPath.isEmpty()) {
                    parentPath = "/";
                }

                QStringList dotDotAttributes;
                dotDotAttributes << ".."
                                 << parentPath
                                 << "Directory"
                                 << ""
                                 << ""
                                 << ""
                                 << ""
                                 << ""
                                 << ""
                                 << "Directory";
                fileList.prepend(dotDotAttributes);
            }

            if (!dotFound) {
                QStringList dotAttributes;
                dotAttributes << "."
                              << directoryPath
                              << "Directory"
                              << ""
                              << ""
                              << ""
                              << ""
                              << ""
                              << ""
                              << "Directory";
                fileList.prepend(dotAttributes);
            }


        // fileSystemType

        if( fs->ftype == TSK_FS_TYPE_EXFAT || fs->ftype == TSK_FS_TYPE_FAT12 || fs->ftype == TSK_FS_TYPE_FAT16 || fs->ftype == TSK_FS_TYPE_FAT32 ){
            fileSystemType="FAT";
        }

        if( fs->ftype == TSK_FS_TYPE_NTFS ){
            fileSystemType="NTFS";
        }

        tsk_fs_dir_close(dir);


    return fileList;
}

QList<QStringList> FileSystemHandler::getPartitionDetails() const
{
    QList<QStringList> partitionDetails;

    if (vs) {
        for (uint32_t i = 0; i < vs->part_count; ++i) {
            const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, i);
            if (!part)
                continue;

            QStringList details;
            details << QString::number(part->start * vs->block_size)  // Offset DEC
                    << QString::number(part->start * vs->block_size, 16)  // Offset HEX
                    << QString::number(part->len * vs->block_size)  // Length (Bytes)
                    << QString::fromUtf8(part->desc);  // Partition Description

            partitionDetails.append(details);
        }
    } else if (fsOpenedDirectly) {
        TSK_FS_INFO *fs = openFileSystems.first();
        QStringList details;
        details << "0"  // Offset DEC
                << "0"  // Offset HEX
                << formatSize(fs->block_count * fs->block_size)  // Length (Bytes)
                << "Single File System";  // Partition Description

        partitionDetails.append(details);
    }

    return partitionDetails;
}

QList<QString> FileSystemHandler::searchFileNamesInDirectory(int partitionIndex, const QString &directoryPath, const QString &searchText)
{
    QList<QString> searchResults;

        TSK_FS_INFO *fs = getFileSystem(partitionIndex);
        if (!fs)
            return searchResults;

        TSK_FS_DIR *dir = tsk_fs_dir_open(fs, directoryPath.toStdString().c_str());
        if (!dir) {
            throw FileSystemException("Failed to open directory: " + directoryPath);
        }

        for (size_t i = 0; i < dir->names_used; i++) {
            TSK_FS_NAME *fs_name = &dir->names[i];
            QString fileName = QString::fromUtf8(fs_name->name);

            if (fileName.contains(searchText, Qt::CaseInsensitive)) {
                searchResults.append(fileName);
            }
        }

        tsk_fs_dir_close(dir);


    return searchResults;
}

QString FileSystemHandler::formatSize(qint64 size) const
{
    if (size < 1024)
        return QString::number(size) + " B";
    else if (size < 1024 * 1024)
        return QString::number(size / 1024.0, 'f', 2) + " KB";
    else if (size < 1024 * 1024 * 1024)
        return QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
    else
        return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

QString FileSystemHandler::getLastError() const
{
    return QString::fromUtf8(tsk_error_get());
}

void FileSystemHandler::printSupportedFormats() const
{
    qDebug() << "Supported Formats:";
    tsk_img_type_print(stdout);
}

QByteArray FileSystemHandler::readFileContents(int partitionIndex, const QString &filePath)
{
    TSK_FS_INFO *fs = getFileSystem(partitionIndex);
    if (!fs) {
        throw FileSystemException("Invalid file system");
    }

   // qDebug() << "TSK_FS_INFO ok";

    TSK_FS_FILE *file = tsk_fs_file_open(fs, nullptr, filePath.toStdString().c_str());
    if (!file) {
        throw FileSystemException("Failed to open file: " + filePath);
    }

  //  qDebug() << "TSK_FS_FILE ok";

    QByteArray fileData;
    char buffer[8192];
    ssize_t bytesRead;
    TSK_OFF_T offset = 0;

    while ((bytesRead = tsk_fs_file_read(file, offset, buffer, sizeof(buffer), TSK_FS_FILE_READ_FLAG_NONE)) > 0) {
        fileData.append(buffer, bytesRead);
        offset += bytesRead;
    }

    if (bytesRead < 0) {
        qDebug() << "Error reading file: " << getLastError();
    }

    tsk_fs_file_close(file);

    return fileData;
}


void FileSystemHandler::exportFileContents(int partitionIndex, const QString &filePath, const QString &destinationPath)
{
    QByteArray fileData = readFileContents(partitionIndex, filePath);

    QFile outFile(destinationPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        throw FileSystemException("Failed to open destination file: " + destinationPath);
    }

    outFile.write(fileData);
    outFile.close();
}

quint64 FileSystemHandler::getFileOffset(int partitionIndex, const QString &filePath)
{
    qDebug() << "Getting file offset for partitionIndex" << partitionIndex << " and file path " << filePath;

    TSK_FS_INFO *fs = getFileSystem(partitionIndex);
    if (!fs) {
        throw FileSystemException("Invalid file system");
    }

    TSK_FS_FILE *file = tsk_fs_file_open(fs, nullptr, filePath.toStdString().c_str());
    if (!file) {
        throw FileSystemException("Failed to open file: " + filePath);
    }





    quint64 offset = 0;

    qDebug() << " filePath..." << filePath  ;

    //MBR is always at offset 0;
    if(filePath=="/$MBR"){

        return 0;
    }

    quint64 partitionOffset = fs->offset;

    if (file->meta) {
        if (fs->ftype == TSK_FS_TYPE_NTFS) {
            qDebug() << "NTFS Detected..."  ;

            quint64 mftLocation = getPartitionMftFileLocation(partitionIndex);


            offset = partitionOffset + mftLocation + (file->meta->addr * 1024);  // NTFS entry chunk size is 1024


        } else if (fs->ftype == TSK_FS_TYPE_EXFAT || fs->ftype == TSK_FS_TYPE_FAT12 || fs->ftype == TSK_FS_TYPE_FAT16 || fs->ftype == TSK_FS_TYPE_FAT32) {
            qDebug() << "FAT Detected..."  ;

            FATFS_INFO *fatfs = (FATFS_INFO *) fs;

            const quint64 FAT_DIRECTORY_ENTRY_SIZE=32;



            if(filePath=="/$FAT1"){


                qDebug() << "fs->firstfatsect:" << fatfs->firstfatsect;
                qDebug() << "fs->sectperfat:" << fatfs->sectperfat;

                const quint64 fatStart= fatfs->firstfatsect;

                const quint64 FATAreaOffset=fatStart * fs->block_size;

                offset = partitionOffset+ FATAreaOffset ;

                qDebug() << "FAT offset:" << offset;

                return offset;
            }

            if(filePath=="/$FAT2"){


                qDebug() << "fs->firstfatsect:" << fatfs->firstfatsect;
                qDebug() << "fs->sectperfat:" << fatfs->sectperfat;

                const quint64 fatStart= fatfs->firstfatsect+ 1 * (fatfs->sectperfat);

                const quint64 FATAreaOffset=fatStart * fs->block_size;

                offset = partitionOffset+ FATAreaOffset ;

                qDebug() << "FAT offset:" << offset;

                return offset;
            }







            const quint64 dataStart= fatfs->firstdatasect;
            const quint64 dataAreaOffset=dataStart * fs->block_size;

             int offset_adjutment=3;

            if(fs->ftype == TSK_FS_TYPE_EXFAT ){
                 offset_adjutment= 1;
             }


            //The cluster starts with at 3 data units offset
            offset = partitionOffset+ dataAreaOffset+ ((file->meta->addr -offset_adjutment ) * FAT_DIRECTORY_ENTRY_SIZE) ;

            //qDebug() << "clusterStart:" <<clusterStart ;

           /* qDebug() << "dataStart:" <<dataStart ;
            qDebug() << "fs->block_size:" << fs->block_size;

            qDebug() << "-------------------------------------------"  ;

            qDebug() << "partitionOffset:" <<partitionOffset ;
            qDebug() << "dataAreaOffset:" <<dataAreaOffset ;
            qDebug() << "file->meta->addr:" <<file->meta->addr ;

            qDebug() << "-------------------------------------------"  ; */



        }
        else {
            //Ignore unsupported file systems
            qDebug() << "Unsupported file system type: " + QString::number(fs->ftype);

            tsk_fs_file_close(file);

        }

        qDebug() << "Partition offset:" << partitionOffset;
        //qDebug() << "File meta address:" << file->meta->addr;
        qDebug() << "Calculated offset:" << offset;
    }

    tsk_fs_file_close(file);
    return offset;
}


quint64 FileSystemHandler::getPartitionMftFileLocation(int partitionIndex)
{
    TSK_FS_INFO *fs = getFileSystem(partitionIndex);
    if (!fs) {
        throw FileSystemException("Invalid file system");
    }

    // Ensure the file system is NTFS
    if (fs->ftype != TSK_FS_TYPE_NTFS) {
        throw FileSystemException("Not an NTFS file system");
    }

    // Open the MFT file, which is the first file in the NTFS file system (inode 0)
    TSK_FS_FILE *mftFile = tsk_fs_file_open_meta(fs, nullptr, 0);
    if (!mftFile) {
        throw FileSystemException("Failed to open MFT file: " + getLastError());
    }

    // Get the location of the MFT file
    quint64 mftLocation = 0;
    if (mftFile->meta && mftFile->meta->attr) {
        TSK_FS_ATTR *attr;
        for (attr = mftFile->meta->attr->head; attr; attr = attr->next) {
            if (attr->type == TSK_FS_ATTR_TYPE_NTFS_DATA) {
                if (attr->nrd.run) {
                    mftLocation = attr->nrd.run->addr * fs->block_size;
                    break;
                }
            }
        }
    }

    tsk_fs_file_close(mftFile);

    if (mftLocation == 0) {
        throw FileSystemException("Failed to determine MFT file location");
    }

    return mftLocation;
}

QList<QStringList> FileSystemHandler::getAvailablePartitions() const
{
    QList<QStringList> partitions;

    int partitionCounter=1;

    if (vs) {
        for (uint32_t i = 0; i < vs->part_count; ++i) {
            const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, i);
            if (!part)
                continue;

            // Open the filesystem to determine its type
            TSK_FS_INFO *fs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);
            if (!fs)
                continue;

            // Check if the file system type is NTFS, FAT, or exFAT
            if (fs->ftype == TSK_FS_TYPE_NTFS ||
                fs->ftype == TSK_FS_TYPE_FAT12 ||
                fs->ftype == TSK_FS_TYPE_FAT16 ||
                fs->ftype == TSK_FS_TYPE_FAT32 ||
                fs->ftype == TSK_FS_TYPE_EXFAT) {

                QStringList partitionDetails;
                partitionDetails << QString::number(i)
                                 << QString("Partition %1").arg(partitionCounter);  // Partition name in format "Partition 1", "Partition 2", etc.

                partitions.append(partitionDetails);
                partitionCounter++;
            }

            // Close the filesystem info to prevent leaks
            tsk_fs_close(fs);
        }
    } else if (fsOpenedDirectly) {
        TSK_FS_INFO *fs = openFileSystems.first();
        // Check if the directly opened filesystem is NTFS, FAT, or exFAT
        if (fs->ftype == TSK_FS_TYPE_NTFS ||
            fs->ftype == TSK_FS_TYPE_FAT12 ||
            fs->ftype == TSK_FS_TYPE_FAT16 ||
            fs->ftype == TSK_FS_TYPE_FAT32 ||
            fs->ftype == TSK_FS_TYPE_EXFAT) {

            QStringList partitionDetails;
            partitionDetails << "0"  // Partition Index
                             << "Partition 1";  // Partition name for single file system

            partitions.append(partitionDetails);
        }
    }

    return partitions;
}

quint64 FileSystemHandler::calculateClusterOffset(int partitionIndex, qint64 clusterNumber)
{
    TSK_FS_INFO *fs = getFileSystem(partitionIndex);
    if (!fs) {
        throw FileSystemException("Invalid file system");
    }

    const quint64 partitionOffset = fs->offset;
    quint64 clusterOffset = 0;

    if (fs->ftype == TSK_FS_TYPE_NTFS) {
        // For NTFS, clusters are referred to as sectors
        clusterOffset = partitionOffset + (clusterNumber * fs->block_size);
    } else if (fs->ftype == TSK_FS_TYPE_EXFAT || fs->ftype == TSK_FS_TYPE_FAT12 || fs->ftype == TSK_FS_TYPE_FAT16 || fs->ftype == TSK_FS_TYPE_FAT32) {
        const FATFS_INFO *fatfs = (FATFS_INFO *) fs;

        qDebug() << "CLUSTER count clustcnt" << fatfs->clustcnt;

        if(clusterNumber > fatfs->clustcnt){
            QString errorMessage = QString("Maximum Cluster count is %1").arg(fatfs->clustcnt);
            throw FileSystemException(errorMessage);
        }

        const quint64 dataStart = fatfs->firstdatasect;
        const quint64 CLUSTER_SIZE=(uint32_t) fatfs->csize << fatfs->ssize_sh;
        //Cluster 0-2 are empty
        const quint64 ZeroToTwoOffset= 2 * CLUSTER_SIZE;
        clusterOffset = partitionOffset +  (dataStart * fs->block_size)+  (clusterNumber * CLUSTER_SIZE) - ZeroToTwoOffset;
        qDebug() << "CLUSTER_SIZE:" << CLUSTER_SIZE;







    } else {
        throw FileSystemException("Unsupported file system type");
    }

    return clusterOffset;
}
