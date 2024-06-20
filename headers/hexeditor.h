#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QMutex>
#include <QPair>
#include <QTimer>


class HexEditor : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit HexEditor(QWidget *parent = nullptr);

    void setData(const QByteArray &data);
    QByteArray getData() const;

     void changeBytesPerLine(quint64 newBytesPerLine);
    QByteArray getSelectedBytes() const;
     void setSelectedBytes(const QByteArray &selectedBytes);
    void setSelectedByte(quint64 offset);

     void setCursorPosition(quint64 position);
     void ensureCursorVisible();
     void clearSelection();

     quint64 cursorPosition;



 signals:
    void selectionChanged(const QByteArray &selectedData, quint64 startOffset, quint64 endOffset);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
     void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QByteArray m_data;
    mutable QMutex m_mutex;  // Ensure the mutex is non-const

    quint64 bytesPerLine;
    quint64 charWidth;
    quint64 charHeight;
    quint64 headerHeight;
    quint64 hexAreaWidth;
    quint64 asciiAreaWidth;
    quint64 addressAreaWidth;

    QPair<qint64, qint64> selection;  // Start and end offsets of the selection
    bool isDragging;  // Flag to indicate if dragging is in progress

    void updateScrollbar();
    void drawAddressArea(QPainter &painter, quint64 startLine, int horizontalOffset);
    void drawHexArea(QPainter &painter, quint64 startLine, int horizontalOffset);
    void drawAsciiArea(QPainter &painter, quint64 startLine, int horizontalOffset);
    void drawHeader(QPainter &painter, int horizontalOffset);
    void updateSelection(const QPoint &pos, bool reset);
    quint64 calculateOffset(const QPoint &pos);

    QTimer cursorBlinkTimer;

    bool cursorVisible;
    void drawCursor(QPainter &painter);


private slots:
    void onCopy();

    void updateCursorBlink();
};





#endif // HEXEDITOR_H
