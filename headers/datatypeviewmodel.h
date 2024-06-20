#ifndef DATATYPEVIEWMODEL_H
#define DATATYPEVIEWMODEL_H

#include <QAbstractTableModel>
#include <QByteArray>
#include <QVector>
#include <QDateTime>

class DataTypeViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DataTypeViewModel(QObject *parent = nullptr);

    void updateData(const QByteArray &data);
    void setEndian(bool littleEndian);  // Method to set the endian mode

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    struct DataTypeEntry {
        QString type;
        QString value;
    };

    QVector<DataTypeEntry> entries;
    bool littleEndian;
     QByteArray currentData;
    QDateTime  dosDateTimeToUnixDateTime(uint32_t dosDateTime);
     QDateTime windowsFileTimeToUnixDateTime(uint64_t windowsTime);
    QDateTime macTimeToUnixDateTime(uint32_t macTime);
     QString convertGuid(const QByteArray &bytes);
};

#endif // DATATYPEVIEWMODEL_H
