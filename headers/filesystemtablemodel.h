#ifndef FILESYSTEMTABLEMODEL_H
#define FILESYSTEMTABLEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

class FileSystemTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit FileSystemTableModel(QObject *parent = nullptr);
    void setFileData(const QList<QStringList> &data);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    static QString formatSize(qint64 size);

signals:
    void rowDoubleClicked(const QString &filePath, const QString &fileType);
private:
    QList<QStringList> fileData;
    QStringList headers;
    QIcon fileIcon;
    QIcon folderIcon;
};

#endif // FILESYSTEMTABLEMODEL_H
