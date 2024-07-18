#include "headers/tagshandler.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrent>



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

}

TagsHandler::~TagsHandler()
{
    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
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
        //qDebug() << "Tag" << tag.description;
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


void TagsHandler::createMetadataTable(const QString &caseName, const QString &caseNumber)
{
    ensureDatabaseOpen();

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS Metadata ("
                    "name TEXT, "
                    "value TEXT)")) {
        qDebug() << "Error: unable to create Metadata table" << query.lastError().text();
        return;
    } else {
        qDebug() << "Metadata table created or verified successfully.";
    }

    query.prepare("INSERT INTO Metadata (name, value) VALUES (?, ?)");

    query.addBindValue("case_name");
    query.addBindValue(caseName);
    if (!query.exec()) {
        qDebug() << "Error: unable to insert case name" << query.lastError().text();
        return;
    }

    query.addBindValue("case_number");
    query.addBindValue(caseNumber);
    if (!query.exec()) {
        qDebug() << "Error: unable to insert case number" << query.lastError().text();
        return;
    }



    qDebug() << "Metadata inserted successfully.";
}
void TagsHandler::createTabTable()
{
    ensureDatabaseOpen();

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS Tab ("
                    "TabIndex INTEGER, "
                    "Filename TEXT)")) {
        qDebug() << "Error: unable to create Tab table" << query.lastError().text();
        return;
    } else {
        qDebug() << "Tab table created or verified successfully.";
    }


}

void TagsHandler::addNewTab(const QString &sourcePath)
{
    ensureDatabaseOpen();

    QSqlQuery query(db);

    // Check if the Tab table is empty
    if (!query.exec("SELECT COUNT(*) FROM Tab")) {
        qDebug() << "Error: unable to check if Tab table is empty" << query.lastError().text();
        return;
    }

    int count = 0;
    if (query.next()) {
        count = query.value(0).toInt();
    }

    int newTabIndex = 0;
    if (count > 0) {
        // Get the maximum TabIndex currently in the Tab table
        if (!query.exec("SELECT MAX(TabIndex) FROM Tab")) {
            qDebug() << "Error: unable to fetch maximum TabIndex" << query.lastError().text();
            return;
        }

        if (query.next()) {
            newTabIndex = query.value(0).toInt() + 1;
        }
    }

    // Insert the new tab entry
    query.prepare("INSERT INTO Tab (TabIndex, Filename) VALUES (?, ?)");
    query.addBindValue(newTabIndex);
    query.addBindValue(sourcePath);

    if (!query.exec()) {
        qDebug() << "Error: unable to insert new tab" << query.lastError().text();
        return;
    }

    qDebug() << "New tab inserted successfully with TabIndex:" << newTabIndex;
}



QList<Tab> TagsHandler::getTabs() const
{
    ensureDatabaseOpen();

    QList<Tab> tabs;
    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot fetch tabs.";
        return tabs;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT TabIndex, Filename FROM Tab")) {
        qDebug() << "Error: unable to fetch tabs" << query.lastError().text();
        return tabs;
    }

    while (query.next()) {
        Tab tab;
        tab.index = query.value(0).toInt();
        tab.filename = query.value(1).toString();

        tabs.append(tab);
    }
    return tabs;
}

void TagsHandler::createUserTagsTable()
{
    ensureDatabaseOpen();

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS UserTags ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "offset INTEGER, "
                    "length INTEGER, "
                    "description TEXT, "
                    "color TEXT, "
                    "category TEXT,"
                    "tabID INTEGER)")) {
        qDebug() << "Error: unable to create UserTags table" << query.lastError().text();
    } else {
        qDebug() << "UserTags table created or verified successfully.";
    }
}

void TagsHandler::createTemplateTagsTable()
{
    ensureDatabaseOpen();

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS TemplateTags ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "offset INTEGER, "
                    "length INTEGER, "
                    "description TEXT, "
                    "color TEXT, "
                    "category TEXT,"
                    "tabID INTEGER)")) {
        qDebug() << "Error: unable to create TemplateTags table" << query.lastError().text();
    } else {
        qDebug() << "TemplateTags table created or verified successfully.";
    }
}

void TagsHandler::syncTags(const QList<Tag> &tags, int tabID)
{
     QMutexLocker locker(&syncMutex);
    ensureDatabaseOpen();


    QSqlQuery query(db);

    if (!db.isOpen()) {
        qDebug() << "Error: Database is not open";
        return;
    }

    if (!query.prepare("DELETE FROM UserTags WHERE tabID = ?")) {
        qDebug() << "Error: unable to prepare deletion from UserTags table" << query.lastError().text();
        return;
    }
    query.addBindValue(tabID);
    if (!query.exec()) {
        qDebug() << "Error: unable to clear UserTags table" << query.lastError().text();
        return;
    }

    if (!query.prepare("DELETE FROM TemplateTags WHERE tabID = ?")) {
        qDebug() << "Error: unable to prepare deletion from TemplateTags table" << query.lastError().text();
        return;
    }
    query.addBindValue(tabID);
    if (!query.exec()) {
        qDebug() << "Error: unable to clear TemplateTags table" << query.lastError().text();
        return;
    }



    QSqlQuery userQuery(db);
    QSqlQuery templateQuery(db);



    userQuery.prepare("INSERT INTO UserTags (offset, length, description, color, category, tabID) VALUES (?, ?, ?, ?, ?,?)");
    templateQuery.prepare("INSERT INTO TemplateTags (offset, length, description, color, category, tabID) VALUES (?, ?, ?, ?, ?,?)");


    for (const Tag &tag : tags) {
      //  qDebug() << "Syncing..." << tag.description << " of type" << tag.type;

        if (tag.type == "user") {
            userQuery.addBindValue(tag.offset);
            userQuery.addBindValue(tag.length);
            userQuery.addBindValue(tag.description);
            userQuery.addBindValue(tag.color);
            userQuery.addBindValue(tag.category);
            userQuery.addBindValue(tabID);

            if (!userQuery.exec()) {
                qDebug() << "Error: unable to insert user tag" << userQuery.lastError().text();
            }
        } else if (tag.type == "template") {
            templateQuery.addBindValue(tag.offset);
            templateQuery.addBindValue(tag.length);
            templateQuery.addBindValue(tag.description);
            templateQuery.addBindValue(tag.color);
            templateQuery.addBindValue(tag.category);
            templateQuery.addBindValue(tabID);

            if (!templateQuery.exec()) {
                qDebug() << "Error: unable to insert template tag" << templateQuery.lastError().text();
            }
        }
    }

    qDebug() << "Tags synced successfully.";
}

QList<Tag> TagsHandler::getUserTagsFromUserDB(int tabID) const
{
    ensureDatabaseOpen();

    QList<Tag> tags;
    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot fetch user tags.";
        return tags;
    }

    QSqlQuery query(db);
    query.prepare("SELECT offset, length, description, color, category FROM UserTags WHERE tabID = ?");
    query.addBindValue(tabID);
    if (!query.exec()) {
        qDebug() << "Error: unable to fetch user tags" << query.lastError().text();
        return tags;
    }

    while (query.next()) {
        Tag tag;
        tag.offset = query.value(0).toLongLong();
        tag.length = query.value(1).toLongLong();
        tag.description = query.value(2).toString();
        tag.color = query.value(3).toString();
        tag.category = query.value(4).toString();
        tag.type = "user";

        tags.append(tag);
    }
    return tags;
}

QList<Tag> TagsHandler::getTemplateTagsFromUserDB(int tabID) const
{
    ensureDatabaseOpen();

    QList<Tag> tags;
    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot fetch template tags.";
        return tags;
    }

    QSqlQuery query(db);
    query.prepare("SELECT offset, length, description, color, category FROM TemplateTags WHERE tabID = ?");
    query.addBindValue(tabID);
    if (!query.exec()) {
        qDebug() << "Error: unable to fetch template tags" << query.lastError().text();
        return tags;
    }

    while (query.next()) {
        Tag tag;
        tag.offset = query.value(0).toLongLong();
        tag.length = query.value(1).toLongLong();
        tag.description = query.value(2).toString();
        tag.color = query.value(3).toString();
        tag.category = query.value(4).toString();
        tag.type = "template";

        tags.append(tag);
    }
    return tags;
}

void TagsHandler::addRecentFolder(const QString &path)
{
    ensureDatabaseOpen();

    QSqlQuery query(db);

    // Insert the new folder path
    query.prepare("INSERT INTO RecentFolders (path) VALUES (?)");
    query.addBindValue(path);

    if (!query.exec()) {
        qDebug() << "Error: unable to insert recent folder" << query.lastError().text();
        return;
    }

    // Check the total number of entries
    if (!query.exec("SELECT COUNT(*) FROM RecentFolders")) {
        qDebug() << "Error: unable to count recent folders" << query.lastError().text();
        return;
    }

    int count = 0;
    if (query.next()) {
        count = query.value(0).toInt();
    }

    // Delete the oldest entries if count exceeds 10
   if (count > 10) {
        if (!query.exec("DELETE FROM RecentFolders WHERE id IN (SELECT id FROM RecentFolders ORDER BY id ASC LIMIT " + QString::number(count - 10) + ")")) {
            qDebug() << "Error: unable to delete old recent folders" << query.lastError().text();
            return;
        }
    }

    qDebug() << "Recent folder inserted successfully: " << path;
}

QList<QString> TagsHandler::getRecentFolders() const
{
    ensureDatabaseOpen();

    QList<QString> folders;
    if (!db.isOpen()) {
        qDebug() << "Database is not open, cannot fetch recent folders.";
        return folders;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT path FROM RecentFolders ORDER BY id DESC LIMIT 10")) {
        qDebug() << "Error: unable to fetch recent folders" << query.lastError().text();
        return folders;
    }

    while (query.next()) {
        folders.append(query.value(0).toString());
    }
    return folders;
}




QFuture<void> TagsHandler::syncTagsAsync(const QList<Tag> &tags, int tabID)
{
    return QtConcurrent::run([this, tags, tabID]() { syncTags(tags, tabID); });
}
