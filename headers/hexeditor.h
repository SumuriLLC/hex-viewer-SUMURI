#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QMutex>
#include <QPair>

class HexEditor : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit HexEditor(QWidget *parent = nullptr);

    void setData(const QByteArray &data);
    QByteArray getData() const;

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

    int bytesPerLine;
    int charWidth;
    int charHeight;
    int headerHeight;
    int hexAreaWidth;
    int asciiAreaWidth;
    int addressAreaWidth;

    QPair<int, int> selection;  // Start and end offsets of the selection
    bool isDragging;  // Flag to indicate if dragging is in progress

    void updateScrollbar();
    void drawAddressArea(QPainter &painter, int startLine);
    void drawHexArea(QPainter &painter, int startLine);
    void drawAsciiArea(QPainter &painter, int startLine);
    void drawHeader(QPainter &painter);
    void updateSelection(const QPoint &pos);
    int calculateOffset(const QPoint &pos);


private slots:
    void onCopy();
};

#endif // HEXEDITOR_H
