// tagshandler.h
#ifndef TAGSHANDLER_H
#define TAGSHANDLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "tag.h"

class TagsHandler : public QObject
{
    Q_OBJECT

public:
    explicit TagsHandler(const QString &dbPath, const QString &connectionName = "tagsHandlerConnection", QObject *parent = nullptr);
    ~TagsHandler();

    QList<Tag> getTags() const;
    QList<Tag> getTagsByCategory(const QString &category) const; // Add this declaration
    void addTemporaryTag(qint64 offset, qint64 length, const QString &description, const QString &color, const QString &category);
    void saveTagsToDatabase();
    void updateTag(const Tag &tag);
    void deleteTag(const Tag &tag);

    void insertExFATVBR();
    void insertFAT32SFN();

private:
    void initializeDatabase();
    QSqlDatabase db;
    QString connectionName;
    QList<Tag> temporaryTags;
    void ensureDatabaseOpen() const;

};

#endif // TAGSHANDLER_H
