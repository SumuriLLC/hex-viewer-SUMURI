// tagshandler.h
#ifndef TAGSHANDLER_H
#define TAGSHANDLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "tag.h"
#include <QFuture>
#include <QMutex>


struct Tab {
    int index;
    QString filename;
};

class TagsHandler : public QObject
{
    Q_OBJECT

public:
    explicit TagsHandler(const QString &dbPath, const QString &connectionName = "tagsHandlerConnection", QObject *parent = nullptr);
    ~TagsHandler();

    QList<Tag> getTags() const;
    QList<Tag> getTagsByCategory(const QString &category) const; // Add this declaration
    void addTemporaryTag(qint64 offset, qint64 length, const QString &description, const QString &color, const QString &category);
    void createMetadataTable(const QString &caseName, const QString &caseNumber);
    void createTabTable();
    void addNewTab(const QString &sourcePath);

    void createUserTagsTable();
    void createTemplateTagsTable();
    void syncTags(const QList<Tag> &tags,int tabID);
       QFuture<void> syncTagsAsync(const QList<Tag> &tags,int tabID);
    QList<Tab> getTabs() const;
       QList<Tag> getUserTagsFromUserDB(int tabID) const;
       QList<Tag> getTemplateTagsFromUserDB(int tabID) const;

       void addRecentFolder(const QString &path);
       QList<QString> getRecentFolders() const;

private:
    QSqlDatabase db;
    QString connectionName;
    QList<Tag> temporaryTags;
    void ensureDatabaseOpen() const;
     QMutex syncMutex;

};

#endif // TAGSHANDLER_H
