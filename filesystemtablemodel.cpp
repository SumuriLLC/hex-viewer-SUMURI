#include "headers/filesystemtablemodel.h"
#include <QFileIconProvider>
#include <QDebug>

FileSystemTableModel::FileSystemTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    QFileIconProvider iconProvider;
    fileIcon = iconProvider.icon(QFileIconProvider::File);
    folderIcon = iconProvider.icon(QFileIconProvider::Folder);
    headers << "Name" << "File Path" << "File Size" << "Creation Date" << "Modification Date" << "Last Access Date" << "Inode No." << "Cluster Number" << "Type";
}

void FileSystemTableModel::setFileData(const QList<QStringList> &data)
{
    beginResetModel();
    fileData = data;
    endResetModel();
}

int FileSystemTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fileData.count();
}

int FileSystemTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return headers.count();
}

QVariant FileSystemTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QStringList &entry = fileData.at(index.row());



    if (role == Qt::DisplayRole) {
        int column = index.column();
        if (column >= 2) // Adjusting for the missing "Type" column
            column++;
        return entry.at(column);
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        return entry.at(2) == "Directory" ? folderIcon : fileIcon;
    }else if (role == Qt::UserRole) {
        return  entry.at(2); // Store the file type in a custom role
    }

    return QVariant();
}

QVariant FileSystemTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return headers.at(section);
    } else {
        return section + 1;
    }
}

QString FileSystemTableModel::formatSize(qint64 size)
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


