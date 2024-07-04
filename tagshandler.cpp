#include "headers/tagshandler.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRandomGenerator>


TagsHandler::TagsHandler(const QString &dbPath, const QString &connectionName, QObject *parent)
    : QObject(parent),
    db(QSqlDatabase::addDatabase("QSQLITE", connectionName)),
    connectionName(connectionName)
{
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qDebug() << "Error: unable to open database" << db.lastError().text();
    } else {
        qDebug() << "Database opened successfully with connection name" << db.connectionName();
    }

    //initializeDatabase(); //We are manually initializing the database
}

TagsHandler::~TagsHandler()
{
    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
}


void TagsHandler::initializeDatabase()
{
    ensureDatabaseOpen();

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS tags ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "offset INTEGER, "
                    "length INTEGER, "
                    "description TEXT, "
                    "color TEXT, "
                    "category TEXT)")) {
        qDebug() << "Error: unable to create table" << query.lastError().text();
    } else {
        qDebug() << "Table created or verified successfully.";
    }


    insertExFATVBR();
    insertFAT32SFN();

}

void TagsHandler::ensureDatabaseOpen() const
{
    if (!db.isOpen()) {
        if (!const_cast<QSqlDatabase&>(db).open()) {
            qDebug() << "Error: unable to open database" << db.lastError().text();
        } else {
            qDebug() << "Database reopened successfully with connection name" << db.connectionName();
        }
    }
}

QList<Tag> TagsHandler::getTags() const
{
    ensureDatabaseOpen();

    QList<Tag> tags;
    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot fetch tags.";
        return tags;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT offset, length, description, color, category,datatype FROM tags")) {
        qDebug() << "Error: unable to fetch tags" << query.lastError().text();
        return tags;
    }

    while (query.next()) {
        Tag tag;
        tag.offset = query.value(0).toLongLong();
        tag.length = query.value(1).toLongLong();
        tag.description = query.value(2).toString();
        tag.color = query.value(3).toString();
        tag.category = query.value(4).toString();
        tag.datatype = query.value(5).toString();

        tags.append(tag);
    }
    return tags;
}

QList<Tag> TagsHandler::getTagsByCategory(const QString &category) const
{
    ensureDatabaseOpen();

    QList<Tag> tags;
    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot fetch tags by category.";
        return tags;
    }

    QSqlQuery query(db);
    query.prepare("SELECT offset, length, description, color, category,datatype FROM tags WHERE category = ?");
    query.addBindValue(category);
    if (!query.exec()) {
        qDebug() << "Error: unable to fetch tags by category" << query.lastError().text();
        return tags;
    }

    while (query.next()) {
        Tag tag;
        tag.offset = query.value(0).toLongLong();
        tag.length = query.value(1).toLongLong();
        tag.description = query.value(2).toString();
        tag.color = query.value(3).toString();
        tag.category = query.value(4).toString();
        tag.datatype = query.value(5).toString();
        tags.append(tag);
    }
    return tags;
}

void TagsHandler::addTemporaryTag(qint64 offset, qint64 length, const QString &description, const QString &color, const QString &category)
{
    Tag tag;
    tag.offset = offset;
    tag.length = length;
    tag.description = description;
    tag.color = color;
    tag.category = category;
    temporaryTags.append(tag);
}

void TagsHandler::saveTagsToDatabase()
{
    ensureDatabaseOpen();

    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot save tags.";
        return;
    }

    QSqlQuery query(db);
    for (const Tag &tag : temporaryTags) {
        query.prepare("INSERT INTO tags (offset, length, description, color, category) VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(tag.offset);
        query.addBindValue(tag.length);
        query.addBindValue(tag.description);
        query.addBindValue(tag.color);
        query.addBindValue(tag.category);
        if (!query.exec()) {
            qDebug() << "Error: unable to insert tag" << query.lastError().text();
        }
    }
    temporaryTags.clear();
}

void TagsHandler::updateTag(const Tag &tag)
{
    ensureDatabaseOpen();

    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot update tag.";
        return;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE tags SET length = ?, description = ?, color = ?, category = ? WHERE offset = ?");
    query.addBindValue(tag.length);
    query.addBindValue(tag.description);
    query.addBindValue(tag.color);
    query.addBindValue(tag.category);
    query.addBindValue(tag.offset);
    if (!query.exec()) {
        qDebug() << "Error: unable to update tag" << query.lastError().text();
    }
}

void TagsHandler::deleteTag(const Tag &tag)
{
    ensureDatabaseOpen();

    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot delete tag.";
        return;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM tags WHERE offset = ?");
    query.addBindValue(tag.offset);
    if (!query.exec()) {
        qDebug() << "Error: unable to delete tag" << query.lastError().text();
    }
}


void TagsHandler::insertExFATVBR()
{
    ensureDatabaseOpen();

    // Check if the ExFAT VBR data already exists in the database
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM tags WHERE category = ?");
    checkQuery.addBindValue("exFAT VBR");
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() > 0) {
        qDebug() << "ExFAT VBR data already exists in the database.";
        return;
    }

    struct ExFATVBRData {
        quint64 offset;
        quint64 length;
        QString description;
    };

    QList<ExFATVBRData> data = {
        {0x00, 3, "Jump Instruction"},
        {0x03, 8, "Volume Label"},
        {0x40, 8, "Partition Start Address"},
        {0x48, 8, "Volume Length"},
        {0x50, 4, "FAT Offset"},
        {0x54, 4, "FAT Length"},
        {0x58, 4, "Cluster Heap Offset"},
        {0x5C, 4, "Cluster Count"},
        {0x60, 4, "First Cluster Of Root Count"},
        {0x64, 4, "Volume Serial Number"},
        {0x68, 2, "File System Revision"},
        {0x6A, 2, "Volume Flags"},
        {0x6C, 1, "Bytes Per Sector Shift"},
        {0x6D, 1, "Sectors Per Cluster Shift"},
        {0x6E, 1, "Number Of FATs"},
        {0x70, 1, "Percent In Use"},
        {0x1FE, 2, "Boot Signature"}
    };

    QSqlQuery query(db);
    query.prepare("INSERT INTO tags (offset, length, description, color, category) VALUES (?, ?, ?, ?, ?)");
    for (const ExFATVBRData &entry : data) {
        QString randomColor = QString("#%1%2%3")
                                  .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
                                  .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
                                  .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0')).toUpper();
        query.addBindValue(entry.offset);
        query.addBindValue(entry.length);
        query.addBindValue(entry.description);
        query.addBindValue(randomColor); // Random color
        query.addBindValue("exFAT VBR"); // Default category
        if (!query.exec()) {
            qDebug() << "Error: unable to insert exFAT VBR data" << query.lastError().text();
        }
    }
}

void TagsHandler::insertFAT32SFN()
{
    ensureDatabaseOpen();

    // Check if the FAT32 SFN data already exists in the database
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM tags WHERE category = ?");
    checkQuery.addBindValue("FAT32 SFN");
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() > 0) {
        qDebug() << "FAT32 SFN data already exists in the database.";
        return;
    }

    struct FAT32SFNData {
        quint64 offset;
        quint64 length;
        QString description;
    };

    QList<FAT32SFNData> data = {
        {0x00, 8, "Filename"},
        {0x08, 3, "File Extension"},
        {0x0B, 1, "File Attribute"},
        {0x0C, 1, "Millisecond stamp"},
        {0x0E, 2, "File Creation Time"},
        {0x10, 2, "File Creation Date"},
        {0x12, 2, "Last Accessed Date"},
        {0x14, 2, "Cluster high word"},
        {0x16, 2, "Last write time"},
        {0x18, 2, "Last write date"},
        {0x1A, 2, "Cluster low word"},
        {0x1C, 4, "File Size"}
    };

    QSqlQuery query(db);
    query.prepare("INSERT INTO tags (offset, length, description, color, category) VALUES (?, ?, ?, ?, ?)");
    for (const FAT32SFNData &entry : data) {
        QString randomColor = QString("#%1%2%3")
                                  .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
                                  .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
                                  .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0')).toUpper();
        query.addBindValue(entry.offset);
        query.addBindValue(entry.length);
        query.addBindValue(entry.description);
        query.addBindValue(randomColor); // Random color
        query.addBindValue("FAT32 SFN"); // Default category
        if (!query.exec()) {
            qDebug() << "Error: unable to insert FAT32 SFN data" << query.lastError().text();
        }
    }
}
