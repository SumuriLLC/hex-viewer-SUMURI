#include "headers/tagstablemodel.h"
#include <QColor>

TagsTableModel::TagsTableModel(QObject *parent)
    : QAbstractTableModel(parent), m_filterType("")
{
}

void TagsTableModel::setTags(const QVector<Tag> &tags)
{
    beginResetModel();
    m_tags = tags;
    applyFilter();
    endResetModel();
}

void TagsTableModel::setFilterType(const QString &type)
{
    m_filterType = type;
    applyFilter();
}

void TagsTableModel::applyFilter()
{
    if (m_filterType.isEmpty()) {
        m_filteredTags = m_tags;
    } else {
        m_filteredTags.clear();
        for (const Tag &tag : m_tags) {
            if (tag.type == m_filterType) {
                m_filteredTags.append(tag);
            }
        }
    }
}

int TagsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_filteredTags.size();
}

int TagsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4; // Offset, Length, Description, Color
}

QVariant TagsTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Tag &tag = m_filteredTags.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return QString::number(tag.offset);
        case 1: return QString::number(tag.length);
        case 2: return tag.description;
        case 3: return "";
        default: return QVariant();
        }
    } else if (role == Qt::BackgroundRole && index.column() == 3) {
        return QColor(tag.color);
    }

    return QVariant();
}

QVariant TagsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case 0: return "Offset";
    case 1: return "Length";
    case 2: return "Description";
    case 3: return "Color";
    default: return QVariant();
    }
}