#include "headers/datatypeviewmodel.h"
#include <QString>
#include <QDateTime>
#include <QtEndian>


DataTypeViewModel::DataTypeViewModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void DataTypeViewModel::updateData(const QByteArray &data, int startOffset, int endOffset)
{
    beginResetModel();
    entries.clear();

    if (startOffset < 0 || endOffset >= data.size() || startOffset >= endOffset) {
        qDebug() << "Invalid offset range..." << startOffset<< " : " << endOffset << ":"  << data.size() ;

        // Invalid offset range
        endResetModel();
        return;
    }

    // Safe extraction and conversion of data
    auto safeMid = [&](int offset, int size) -> QByteArray {
        if (offset + size <= data.size()) {
            return data.mid(offset, size);
        }
        return QByteArray();
    };


    qDebug() << "updating...";

    qint8 val8 = safeMid(startOffset, 1).isEmpty() ? 0 : static_cast<qint8>(safeMid(startOffset, 1).at(0));
    quint8 uval8 = safeMid(startOffset, 1).isEmpty() ? 0 : static_cast<quint8>(safeMid(startOffset, 1).at(0));
    qint16 val16 = safeMid(startOffset, 2).isEmpty() ? 0 : qFromLittleEndian<qint16>(reinterpret_cast<const uchar *>(safeMid(startOffset, 2).constData()));
    quint16 uval16 = safeMid(startOffset, 2).isEmpty() ? 0 : qFromLittleEndian<quint16>(reinterpret_cast<const uchar *>(safeMid(startOffset, 2).constData()));
    qint32 val32 = safeMid(startOffset, 4).isEmpty() ? 0 : qFromLittleEndian<qint32>(reinterpret_cast<const uchar *>(safeMid(startOffset, 4).constData()));
    quint32 uval32 = safeMid(startOffset, 4).isEmpty() ? 0 : qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(safeMid(startOffset, 4).constData()));
    qint64 val64 = safeMid(startOffset, 8).isEmpty() ? 0 : qFromLittleEndian<qint64>(reinterpret_cast<const uchar *>(safeMid(startOffset, 8).constData()));
    quint64 uval64 = safeMid(startOffset, 8).isEmpty() ? 0 : qFromLittleEndian<quint64>(reinterpret_cast<const uchar *>(safeMid(startOffset, 8).constData()));

    QDateTime dosTime = QDateTime::fromSecsSinceEpoch(uval32);
    QDateTime unixTime = QDateTime::fromSecsSinceEpoch(uval32);

    entries.append({"8 bit signed", QString::number(val8)});
    entries.append({"8 bit unsigned", QString::number(uval8)});
    entries.append({"16 bit signed", QString::number(val16)});
    entries.append({"16 bit unsigned", QString::number(uval16)});
    entries.append({"32 bit signed", QString::number(val32)});
    entries.append({"32 bit unsigned", QString::number(uval32)});
    entries.append({"64 bit signed", QString::number(val64)});
    entries.append({"64 bit unsigned", QString::number(uval64)});
    entries.append({"DOS Time", dosTime.toString("hh:mm:ss")});
    entries.append({"DOS Date", dosTime.toString("yyyy-MM-dd")});
    entries.append({"Unix Time", unixTime.toString("yyyy-MM-dd hh:mm:ss")});

   // qint8 val8 = static_cast<qint8>(data.mid(startOffset, 1).at(0));
   // quint8 uval8 = static_cast<quint8>(data.mid(startOffset, 1).at(0));
    //qint16 val16 = static_cast<qint16>(qFromLittleEndian<qint16>(reinterpret_cast<const uchar *>(data.mid(startOffset, 2).constData())));
    //quint16 uval16 = static_cast<quint16>(qFromLittleEndian<quint16>(reinterpret_cast<const uchar *>(data.mid(startOffset, 2).constData())));
    //qint32 val32 = static_cast<qint32>(qFromLittleEndian<qint32>(reinterpret_cast<const uchar *>(data.mid(startOffset, 4).constData())));
    //quint32 uval32 = static_cast<quint32>(qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(data.mid(startOffset, 4).constData())));
    //qint64 val64 = static_cast<qint64>(qFromLittleEndian<qint64>(reinterpret_cast<const uchar *>(data.mid(startOffset, 8).constData())));
    //quint64 uval64 = static_cast<quint64>(qFromLittleEndian<quint64>(reinterpret_cast<const uchar *>(data.mid(startOffset, 8).constData())));

    //QDateTime dosTime = QDateTime::fromSecsSinceEpoch(uval32);
    //QDateTime unixTime = QDateTime::fromSecsSinceEpoch(uval32);

   // entries.append({"8 bit signed", QString::number(val8)});
   // entries.append({"8 bit unsigned", QString::number(uval8)});
   // entries.append({"16 bit signed", QString::number(val16)});
   // entries.append({"16 bit unsigned", QString::number(uval16)});
    //entries.append({"32 bit signed", QString::number(val32)});
   // entries.append({"32 bit unsigned", QString::number(uval32)});
    //entries.append({"64 bit signed", QString::number(val64)});
    //entries.append({"64 bit unsigned", QString::number(uval64)});
    //entries.append({"DOS Time", dosTime.toString("hh:mm:ss")});
    //entries.append({"DOS Date", dosTime.toString("yyyy-MM-dd")});
    //entries.append({"Unix Time", unixTime.toString("yyyy-MM-dd hh:mm:ss")});

    endResetModel();
}

int DataTypeViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return entries.count();
}

int DataTypeViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;  // Data Type and Value columns
}

QVariant DataTypeViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const DataTypeEntry &entry = entries.at(index.row());
        if (index.column() == 0)
            return entry.type;
        else if (index.column() == 1)
            return entry.value;
    }

    return QVariant();
}

QVariant DataTypeViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0)
                return QString("Data Type");
            else if (section == 1)
                return QString("Value");
        }
    }

    return QVariant();
}
