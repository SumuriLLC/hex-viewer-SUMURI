// tag.h
#ifndef TAG_H
#define TAG_H

#include <QString>

struct Tag {
    quint64 offset;
    quint64 length;
    QString description;
    QString color;
    QString category;
    QString type;
    QString datatype;
};

#endif // TAG_H
