#include "headers/markerstablemodel.h"
#include <QFont>

MarkersTableModel::MarkersTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    headers <<"Offset (DEC)" << "Offset (HEX)" << "Length (Bytes)" << "Description";
}

void MarkersTableModel::setMarkerData(const QList<QStringList> &data)
{
    beginResetModel();
    markerData = data;
    endResetModel();
}

int MarkersTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return markerData.count();
}

int MarkersTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return headers.count();
}

QVariant MarkersTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const QStringList &entry = markerData.at(index.row());
    return entry.at(index.column());
}

QVariant MarkersTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return headers.at(section);
        } else {
            return section + 1;
        }
    } else if (role == Qt::FontRole && orientation == Qt::Horizontal) {
        QFont font;
        font.setBold(true);
        return font;
    } else if (role == Qt::TextAlignmentRole && orientation == Qt::Horizontal) {
        return Qt::AlignCenter;
    }

    return QVariant();
}
