// tagstablemodel.h
#ifndef TAGSTABLEMODEL_H
#define TAGSTABLEMODEL_H

#include <QAbstractTableModel>
#include "tagshandler.h"

class TagsTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TagsTableModel(QObject *parent = nullptr);

    void setTags(const QVector<Tag> &tags);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setFilterType(const QString &type);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void setTags(const QVector<Tag> &tags, int sortColumn = -1, Qt::SortOrder order = Qt::AscendingOrder);


private:
    int m_sortColumn;
    Qt::SortOrder m_sortOrder;
    QVector<Tag> m_tags;
    QVector<Tag> m_filteredTags;
    QString m_filterType;

    void applyFilter();
};

#endif // TAGSTABLEMODEL_H
