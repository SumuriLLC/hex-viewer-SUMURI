#ifndef DATATYPEVIEWMODEL_H
#define DATATYPEVIEWMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

class DataTypeViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    DataTypeViewModel(QObject *parent = nullptr);

    void updateData(const QByteArray &data, int startOffset, int endOffset);
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
};

#endif // DATATYPEVIEWMODEL_H
