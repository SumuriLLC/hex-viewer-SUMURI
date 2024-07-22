#ifndef FILESYSTEMHANDLER_H
#define FILESYSTEMHANDLER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <tsk/libtsk.h>

class FileSystemHandler : public QObject
{
    Q_OBJECT

public:
    explicit FileSystemHandler(QObject *parent = nullptr);
    ~FileSystemHandler();

    bool openImage(const QString &fileName);
    QList<QStringList> listFilesInDirectory(int partitionIndex, const QString &directoryPath);
    int getPartitionCount() const;
    QString getPartitionDescription(uint partitionIndex) const;
    TSK_FS_INFO* getFileSystem(uint partitionIndex);
    QList<QStringList> getPartitionDetails() const;
    QList<QString> searchFileNamesInDirectory(int partitionIndex, const QString &directoryPath, const QString &searchText);
    void printSupportedFormats() const;
    QString getLastError() const;
    bool openImageAsFileSystem();
    QByteArray readFileContents(int partitionIndex, const QString &filePath);
    void exportFileContents(int partitionIndex, const QString &filePath, const QString &destinationPath);
    quint64 getFileOffset(int partitionIndex, const QString &filePath) ;
    quint64 getPartitionMftFileLocation(int partitionIndex);

    QString fileSystemType;



private:
    TSK_IMG_INFO *img;
    TSK_VS_INFO *vs;
    QString filePath;

    QList<TSK_FS_INFO*> openFileSystems;
    QString formatSize(qint64 size) const;
    void closeImage();
    bool fsOpenedDirectly;
};

#endif // FILESYSTEMHANDLER_H
