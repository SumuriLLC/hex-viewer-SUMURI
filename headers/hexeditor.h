#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QPair>
#include <QSet>
#include "tag.h"
#include "tagshandler.h"
#include "ewfdevice.h"


class HexEditor : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit HexEditor(QWidget *parent = nullptr);
    void setData(const QString &filePath);
    QByteArray getData() const;
    void changeBytesPerLine(quint64 newBytesPerLine);
    void setSelectedBytes(const QByteArray &selectedBytes);
    void setSelectedByte(qint64 offset);
    void setCursorPosition(quint64 position);
    void ensureCursorVisible();
    void clearSelection();
    QByteArray getSelectedBytes() const;
    quint64 cursorPosition;
    quint64 fileSize;
    void addTag(quint64 offset, quint64 length, const QString &description, const QColor &color, const QString &type);
    void removeTag(quint64 offset, int index = -1);
    void clearTags();

    void exportTags(const QString &type);
    void importTags(const QString &tagType);
    void exportTagDataByOffset(quint64 offset, const QString &fileName);
    void setTagsHandler(TagsHandler *tagsHandler);


signals:
    void selectionChanged(const QByteArray &selectedData, quint64 startOffset, quint64 endOffset);
    void tagsUpdated(const QVector<Tag> &tags);
    void tagNameAndLength(const QString &tagName, quint64 length);


protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void scrollContentsBy(int dx, int dy)override;


private slots:
    void onCopy();
    void updateCursorBlink();
    void onTagSelectedBytes();
    void onApplyTags(QString category);
    void onShowTags(const QString &tagCategory);

private:
    void updateScrollbar();
    void updateSelection(const QPoint &pos, bool reset);
    quint64 calculateOffset(const QPoint &pos);
    void drawAddressArea(QPainter &painter, quint64 startLine, int horizontalOffset);
    void drawHexArea(QPainter &painter, quint64 startLine, int horizontalOffset);
    void drawAsciiArea(QPainter &painter, quint64 startLine, int horizontalOffset);
    void drawHeader(QPainter &painter, int horizontalOffset);
    void drawCursor(QPainter &painter);
    void updateVisibleData();

    quint64 bytesPerLine;
    QPair<quint64, quint64> selection;
    bool isDragging;
    bool cursorVisible;
    int charWidth;
    int charHeight;
    int headerHeight;
    int addressAreaWidth;
    int hexAreaWidth;
    int asciiAreaWidth;
    QTimer cursorBlinkTimer;
    mutable QMutex m_mutex;
    bool cursorBlinkState;


    QByteArray m_data;
    QByteArray data_visible;
    //QFile file;
    quint64 visibleStart;
    quint64 visibleEnd;
    QSet<quint64> selectedOffsets;
    quint64 cursorByteOffset;



    QList<Tag> tags;
    QIODevice *device = nullptr;

    quint64 startBlockOffset;
    bool startBlockSelected;

    void onStartBlock();
    void onEndBlock();

     TagsHandler *tagsHandler;

};

#endif // HEXEDITOR_H
