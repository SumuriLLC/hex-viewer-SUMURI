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

    bool operator==(const Tag& other) const {
        return offset == other.offset &&
               length == other.length &&
               description == other.description &&
               color == other.color &&
               category == other.category &&
               type == other.type &&
               datatype == other.datatype;
    }
};

#endif // TAG_H
