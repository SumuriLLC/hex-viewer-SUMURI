#include "headers/tagdialogmodel.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>

TagDialogModel::TagDialogModel(const QString &title, const QList<Tag> &tags, QIODevice &file, quint64 currentCursorPos, QWidget *parent, const QMap<QString, QList<Tag>> &tagsListByGroup)
    : QDialog(parent),
    tabWidget(new QTabWidget(this)),
    tableView(new QTableView(this)),
    model(new QStandardItemModel(this)),
    currentCursorPos(currentCursorPos)
{
    setWindowTitle(title);
    setFixedSize(750, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);

    if (!tagsListByGroup.isEmpty()) {
        qDebug() << "Adding tabs";
        tableView->hide();
        populateModelByGroup(tagsListByGroup, file);
    } else {
        qDebug() << "Adding normal layout";
        QWidget *singleTab = new QWidget(this);
        QVBoxLayout *singleTabLayout = new QVBoxLayout(singleTab);

        singleTabLayout->addWidget(tableView);
        tabWidget->addTab(singleTab, title);

        model->setHorizontalHeaderLabels({"Offset (HEX)", "Offset (DEC)", "Length", "Value", "Value (HEX)", "Description"});
        tableView->setModel(model);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        populateModel(tags, file);
    }
}

void TagDialogModel::populateModel(const QList<Tag> &tags, QIODevice &file)
{
    for (const Tag &tag : tags) {
        QList<QStandardItem *> row;
        quint64 adjustedOffset = tag.offset + currentCursorPos;
        row.append(new QStandardItem(QString("0x%1").arg(tag.offset, 0, 16).toUpper()));
        row.append(new QStandardItem(QString::number(tag.offset)));
        row.append(new QStandardItem(QString::number(tag.length)));

        if (file.seek(adjustedOffset)) {
            QByteArray tagData = file.read(tag.length);
            QString value, valueHex;

            // Display hex as-is
            for (int i = 0; i < tagData.size(); ++i) {
                valueHex.append(QString("%1").arg(static_cast<unsigned char>(tagData.at(i)), 2, 16, QChar('0')).toUpper());
                if (i < tagData.size() - 1) {
                    valueHex.append(" ");
                }
            }

            // Calculate the value based on the datatype
            if (tag.datatype == "hex") {
                value = valueHex;
            } else if (tag.datatype == "number") {
                QDataStream stream(tagData);
                stream.setByteOrder(QDataStream::LittleEndian);
                switch (tag.length) {
                case 1: {
                    quint8 numberValue;
                    stream >> numberValue;
                    value = QString::number(numberValue);
                    break;
                }
                case 2: {
                    quint16 numberValue;
                    stream >> numberValue;
                    value = QString::number(numberValue);
                    break;
                }
                case 4: {
                    quint32 numberValue;
                    stream >> numberValue;
                    value = QString::number(numberValue);
                    break;
                }
                case 8: {
                    quint64 numberValue;
                    stream >> numberValue;
                    value = QString::number(numberValue);
                    break;
                }
                default:
                    quint64 numberValue;
                    stream >> numberValue;
                    value = QString::number(numberValue);
                }
            } else if (tag.datatype == "string") {
                value = QString::fromUtf8(tagData);
            } else if (tag.datatype == "date") {
                QDataStream stream(tagData);
                stream.setByteOrder(QDataStream::LittleEndian);
                quint16 year, month, day;
                stream >> year >> month >> day;
                value = QDate(year, month, day).toString(Qt::ISODate);
            } else if (tag.datatype == "time") {
                QDataStream stream(tagData);
                stream.setByteOrder(QDataStream::LittleEndian);
                quint16 hours, minutes, seconds;
                stream >> hours >> minutes >> seconds;
                value = QTime(hours, minutes, seconds).toString(Qt::ISODate);
            }

            row.append(new QStandardItem(value));
            row.append(new QStandardItem(valueHex));
        } else {
            row.append(new QStandardItem("Error reading file"));
            row.append(new QStandardItem("Error reading file"));
        }

        row.append(new QStandardItem(tag.description));

        model->appendRow(row);
    }
}

void TagDialogModel::populateModelByGroup(const QMap<QString, QList<Tag>> &tagsListByGroup, QIODevice &file)
{
    for (const QString &groupName : tagsListByGroup.keys()) {
        QTableView *groupTableView = new QTableView(this);
        QStandardItemModel *groupModel = new QStandardItemModel(this);

        groupModel->setHorizontalHeaderLabels({"Offset (HEX)", "Offset (DEC)", "Length", "Value", "Value (HEX)", "Description"});
        groupTableView->setModel(groupModel);
        groupTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        groupTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        groupTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        groupTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        groupTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        groupTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        const QList<Tag> &tags = tagsListByGroup.value(groupName);
        for (const Tag &tag : tags) {
            QList<QStandardItem *> row;
            quint64 adjustedOffset = tag.offset + currentCursorPos;
            row.append(new QStandardItem(QString("0x%1").arg(tag.offset, 0, 16).toUpper()));
            row.append(new QStandardItem(QString::number(tag.offset)));
            row.append(new QStandardItem(QString::number(tag.length)));

            if (file.seek(adjustedOffset)) {
                QByteArray tagData = file.read(tag.length);
                QString value, valueHex;

                // Display hex as-is
                for (int i = 0; i < tagData.size(); ++i) {
                    valueHex.append(QString("%1").arg(static_cast<unsigned char>(tagData.at(i)), 2, 16, QChar('0')).toUpper());
                    if (i < tagData.size() - 1) {
                        valueHex.append(" ");
                    }
                }

                // Calculate the value based on the datatype
                if (tag.datatype == "hex") {
                    value = valueHex;
                } else if (tag.datatype == "number") {
                    QDataStream stream(tagData);
                    stream.setByteOrder(QDataStream::LittleEndian);
                    switch (tag.length) {
                    case 1: {
                        quint8 numberValue;
                        stream >> numberValue;
                        value = QString::number(numberValue);
                        break;
                    }
                    case 2: {
                        quint16 numberValue;
                        stream >> numberValue;
                        value = QString::number(numberValue);
                        break;
                    }
                    case 4: {
                        quint32 numberValue;
                        stream >> numberValue;
                        value = QString::number(numberValue);
                        break;
                    }
                    case 8: {
                        quint64 numberValue;
                        stream >> numberValue;
                        value = QString::number(numberValue);
                        break;
                    }
                    default:
                        quint64 numberValue;
                        stream >> numberValue;
                        value = QString::number(numberValue);
                    }
                } else if (tag.datatype == "string") {
                    value = QString::fromUtf8(tagData);
                } else if (tag.datatype == "date") {
                    QDataStream stream(tagData);
                    stream.setByteOrder(QDataStream::LittleEndian);
                    quint16 year, month, day;
                    stream >> year >> month >> day;
                    value = QDate(year, month, day).toString(Qt::ISODate);
                } else if (tag.datatype == "time") {
                    QDataStream stream(tagData);
                    stream.setByteOrder(QDataStream::LittleEndian);
                    quint16 hours, minutes, seconds;
                    stream >> hours >> minutes >> seconds;
                    value = QTime(hours, minutes, seconds).toString(Qt::ISODate);
                }

                row.append(new QStandardItem(value));
                row.append(new QStandardItem(valueHex));
            } else {
                row.append(new QStandardItem("Error reading file"));
                row.append(new QStandardItem("Error reading file"));
            }

            row.append(new QStandardItem(tag.description));

            groupModel->appendRow(row);
        }

        tabWidget->addTab(groupTableView, groupName);
    }
}
