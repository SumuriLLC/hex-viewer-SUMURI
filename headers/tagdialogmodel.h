// tagdialogmodel.h
#ifndef TAGDIALOGMODEL_H
#define TAGDIALOGMODEL_H

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QFile>
#include "tag.h"

class TagDialogModel : public QDialog
{
    Q_OBJECT

public:
    explicit TagDialogModel(const QString &title, const QList<Tag> &tags, QIODevice &file, quint64 currentCursorPos, QWidget *parent = nullptr);

private:
    QTableView *tableView;
    QStandardItemModel *model;
    void populateModel(const QList<Tag> &tags, QIODevice &file);
    quint64 currentCursorPos;

};

#endif // TAGDIALOGMODEL_H
