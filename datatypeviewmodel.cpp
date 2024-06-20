#include "headers/datatypeviewmodel.h"
#include <QString>
#include <QDateTime>
#include <QtEndian>
#include <bitset>
#include <QUuid>

DataTypeViewModel::DataTypeViewModel(QObject *parent)
    : QAbstractTableModel(parent), littleEndian(true)
{
}

void DataTypeViewModel::setEndian(bool isLittleEndian)
{
    littleEndian = !isLittleEndian; //Switch endien
    if (!currentData.isEmpty()) {
        updateData(currentData);
    }
}

QDateTime DataTypeViewModel::dosDateTimeToUnixDateTime(uint32_t dosDateTime)
{
    if (dosDateTime == 0) {
        return QDateTime(QDate(1980, 1, 1), QTime(0, 0), Qt::UTC);
    }

    int year = ((dosDateTime & 0xfe000000) >> 25) + 1980;
    int month = ((dosDateTime & 0x1e00000) >> 21)-1;
    int day = (dosDateTime & 0x1f0000) >> 16;
    int hour = (dosDateTime & 0xf800) >> 11;
    int minute = (dosDateTime & 0x7e0) >> 5;
    int second = 2 * (dosDateTime & 0x1f);

    QDate date(year, month, day);
    QTime time(hour, minute, second);
    return QDateTime(date, time, Qt::UTC);
}

QDateTime DataTypeViewModel::windowsFileTimeToUnixDateTime(uint64_t windowsTime)
{
    if (windowsTime == 0) {
        return QDateTime(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC);
    }
    return QDateTime::fromMSecsSinceEpoch((windowsTime - 116444736000000000LL) / 10000, Qt::UTC);
}

QDateTime DataTypeViewModel::macTimeToUnixDateTime(uint32_t macTime)
{
    // Mac HFS+ epoch starts at 2001-01-01 00:00:00 UTC
    QDateTime macEpoch(QDate(1904, 1, 1), QTime(0, 0), Qt::UTC);
    return macEpoch.addSecs(macTime);
}


QString DataTypeViewModel::convertGuid(const QByteArray &bytes)
{
    if (bytes.size() < 16)
        return QString();

    uint32_t data1 = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(bytes.constData()));
    uint16_t data2 = qFromLittleEndian<quint16>(reinterpret_cast<const uchar *>(bytes.constData() + 4));
    uint16_t data3 = qFromLittleEndian<quint16>(reinterpret_cast<const uchar *>(bytes.constData() + 6));
    std::array<uint8_t, 8> data4;
    memcpy(data4.data(), bytes.constData() + 8, 8);

    return QString("{%1-%2-%3-%4%5-%6%7%8%9%10%11}")
        .arg(data1, 8, 16, QChar('0')).toUpper()
        .arg(data2, 4, 16, QChar('0')).toUpper()
        .arg(data3, 4, 16, QChar('0')).toUpper()
        .arg(data4[0], 2, 16, QChar('0')).toUpper()
        .arg(data4[1], 2, 16, QChar('0')).toUpper()
        .arg(data4[2], 2, 16, QChar('0')).toUpper()
        .arg(data4[3], 2, 16, QChar('0')).toUpper()
        .arg(data4[4], 2, 16, QChar('0')).toUpper()
        .arg(data4[5], 2, 16, QChar('0')).toUpper()
        .arg(data4[6], 2, 16, QChar('0')).toUpper()
        .arg(data4[7], 2, 16, QChar('0')).toUpper();
}



void DataTypeViewModel::updateData(const QByteArray &data)
{
    beginResetModel();
    entries.clear();

    currentData = data;

    if (data.isEmpty()) {
        // Invalid data range
        endResetModel();
        return;
    }

    // Safe extraction and conversion of data
    auto safeMid = [&](quint64 offset, quint64 size) -> QByteArray {
        if (offset + size <= data.size()) {
            return data.mid(offset, size);
        }
        return QByteArray();
    };

    auto convert16 = [&](const QByteArray &bytes) -> qint16 {
        return littleEndian ? qFromLittleEndian<qint16>(reinterpret_cast<const uchar *>(bytes.constData())) :
                   qFromBigEndian<qint16>(reinterpret_cast<const uchar *>(bytes.constData()));
    };

    auto convert32 = [&](const QByteArray &bytes) -> qint32 {
        return littleEndian ? qFromLittleEndian<qint32>(reinterpret_cast<const uchar *>(bytes.constData())) :
                   qFromBigEndian<qint32>(reinterpret_cast<const uchar *>(bytes.constData()));
    };

    auto convert64 = [&](const QByteArray &bytes) -> qint64 {
        return littleEndian ? qFromLittleEndian<qint64>(reinterpret_cast<const uchar *>(bytes.constData())) :
                   qFromBigEndian<qint64>(reinterpret_cast<const uchar *>(bytes.constData()));
    };

    qint8 val8 = safeMid(0, 1).isEmpty() ? 0 : static_cast<qint8>(safeMid(0, 1).at(0));
    quint8 uval8 = safeMid(0, 1).isEmpty() ? 0 : static_cast<quint8>(safeMid(0, 1).at(0));
    qint16 val16 = safeMid(0, 2).isEmpty() ? 0 : convert16(safeMid(0, 2));
    quint16 uval16 = safeMid(0, 2).isEmpty() ? 0 : convert16(safeMid(0, 2));
    qint32 val32 = safeMid(0, 4).isEmpty() ? 0 : convert32(safeMid(0, 4));
    quint32 uval32 = safeMid(0, 4).isEmpty() ? 0 : convert32(safeMid(0, 4));
    qint64 val64 = safeMid(0, 8).isEmpty() ? 0 : convert64(safeMid(0, 8));
    quint64 uval64 = safeMid(0, 8).isEmpty() ? 0 : convert64(safeMid(0, 8));

    qint32 val24 = 0;
    if (!safeMid(0, 3).isEmpty()) {
        QByteArray bytes24 = safeMid(0, 3);
        if (littleEndian) {
            val24 = (static_cast<qint32>(static_cast<quint8>(bytes24[0])) |
                     (static_cast<qint32>(static_cast<quint8>(bytes24[1])) << 8) |
                     (static_cast<qint32>(static_cast<quint8>(bytes24[2])) << 16));
        } else {
            val24 = (static_cast<qint32>(static_cast<quint8>(bytes24[2])) |
                     (static_cast<qint32>(static_cast<quint8>(bytes24[1])) << 8) |
                     (static_cast<qint32>(static_cast<quint8>(bytes24[0])) << 16));
        }
        // Handle the sign bit
        if (val24 & 0x800000) {
            val24 |= 0xFF000000;
        }
    }
    // Ensure proper endianness for time values (independent of littleEndian setting)
    quint32 dosDateTime = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(safeMid(0, 4).constData()));
    QDateTime dosTime = dosDateTimeToUnixDateTime(dosDateTime);

    quint32 unixTimestamp = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(safeMid(0, 4).constData()));
    QDateTime unixTime = QDateTime::fromSecsSinceEpoch(unixTimestamp, Qt::UTC);
   QDateTime localTime = unixTime.toLocalTime();


    quint64 windowsFileTime = qFromLittleEndian<quint64>(reinterpret_cast<const uchar *>(safeMid(0, 8).constData()));
    QDateTime windowsTime = windowsFileTimeToUnixDateTime(windowsFileTime);

   // quint32 macTimeValue = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(safeMid(0, 4).constData()));



    quint32 macTimeInt = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(safeMid(0, 4).constData()));
    QDateTime macTime = macTimeToUnixDateTime(macTimeInt);
    // GUID is a 16-byte value that doesn't change with endianness
    QString guid = convertGuid(safeMid(0, 16));

    entries.append(DataTypeEntry{"8 bit binary", QString::fromStdString(std::bitset<8>(uval8).to_string())});
    entries.append(DataTypeEntry{"8 bit signed", QString::number(val8)});
    entries.append(DataTypeEntry{"8 bit unsigned", QString::number(uval8)});
    entries.append(DataTypeEntry{"16 bit signed", QString::number(val16)});
    entries.append(DataTypeEntry{"16 bit unsigned", QString::number(uval16)});
    entries.append(DataTypeEntry{"24 bit signed", QString::number(val24)});
    entries.append(DataTypeEntry{"32 bit signed", QString::number(val32)});
    entries.append(DataTypeEntry{"32 bit unsigned", QString::number(uval32)});
    entries.append(DataTypeEntry{"64 bit signed", QString::number(val64)});
    entries.append(DataTypeEntry{"64 bit unsigned", QString::number(uval64)});
    entries.append(DataTypeEntry{"DOS Time", dosTime.toString("hh:mm:ss")});
    entries.append(DataTypeEntry{"DOS Date", dosTime.toString("yyyy-MM-dd")});
    entries.append(DataTypeEntry{"Unix Time", localTime.toString("yyyy-MM-dd hh:mm:ss")});
    entries.append(DataTypeEntry{"Windows Time", windowsTime.toString("yyyy-MM-dd hh:mm:ss")});
    entries.append(DataTypeEntry{"Mac Time", macTime.toString("yyyy-MM-dd hh:mm:ss")});
    entries.append(DataTypeEntry{"GUID", guid});

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
    return 3;  // Serial Number, Data Type, and Value columns
}

QVariant DataTypeViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const DataTypeEntry &entry = entries.at(index.row());
        if (index.column() == 0)
            return QString::number(index.row() + 1);  // Serial number starts from 1
        else if (index.column() == 1)
            return entry.type;
        else if (index.column() == 2)
            return entry.value;
    }

    return QVariant();
}

QVariant DataTypeViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0)
                return QString("");
            else if (section == 1)
                return QString("Data Type");
            else if (section == 2)
                return QString("Value");
        }
    }

    return QVariant();
}
