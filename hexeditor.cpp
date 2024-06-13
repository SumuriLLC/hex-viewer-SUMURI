#include "headers/hexeditor.h"
#include <QPainter>
#include <QFontMetrics>
#include <QScrollBar>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QClipboard>

HexEditor::HexEditor(QWidget *parent)
    : QAbstractScrollArea(parent),
    bytesPerLine(16),
    selection(qMakePair(-1, -1)),  // Initialize selection as invalid
    isDragging(false)  // Initialize dragging flag
{
    setFont(QFont("Courier New", 10));
    QFontMetrics fm(font());
    charWidth = fm.horizontalAdvance('0');
    charHeight = fm.height();
    headerHeight = charHeight + 4;

    addressAreaWidth = charWidth * 18;
    hexAreaWidth = charWidth * 3 * bytesPerLine;
    asciiAreaWidth = charWidth * bytesPerLine;

    updateScrollbar();
}

void HexEditor::setData(const QByteArray &data)
{
    {
        QMutexLocker locker(&m_mutex); // Lock the mutex for thread-safe access
        m_data = data;
    } // Unlock the mutex immediately after modifying m_data
    updateScrollbar();
    viewport()->update();
}

QByteArray HexEditor::getData() const
{
    QMutexLocker locker(&m_mutex); // Lock the mutex for thread-safe access
    return m_data;
}

void HexEditor::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(viewport());
    painter.setFont(font());

    int firstLine = verticalScrollBar()->value();
    int lastLine = firstLine + (viewport()->height() - headerHeight) / charHeight;

    drawHeader(painter);

    {
        QMutexLocker locker(&m_mutex); // Lock the mutex during painting to ensure thread-safe access
        drawAddressArea(painter, firstLine);
        drawHexArea(painter, firstLine);
        drawAsciiArea(painter, firstLine);
    } // Unlock the mutex immediately after drawing
}

void HexEditor::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateScrollbar();
}

void HexEditor::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    // Implement key press handling if needed
}

void HexEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        updateSelection(event->pos());
    }
}

void HexEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        updateSelection(event->pos());
    }
}

void HexEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        updateSelection(event->pos());
    }
}

void HexEditor::updateScrollbar()
{
    {
        QMutexLocker locker(&m_mutex); // Lock the mutex for thread-safe access
        int lines = (m_data.size() + bytesPerLine - 1) / bytesPerLine;
        verticalScrollBar()->setRange(0, lines);
        verticalScrollBar()->setPageStep((viewport()->height() - headerHeight) / charHeight);
    } // Unlock the mutex immediately after updating scrollbar range
}

void HexEditor::updateSelection(const QPoint &pos)
{
    int offset = calculateOffset(pos);
    if (offset == -1 || offset >= m_data.size()) {
        return;  // Click outside the data range
    }

    if (selection.first == -1 || !(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        selection.first = selection.second = offset;
    } else {
        selection.second = offset;
    }

    if (selection.first > selection.second) {
        std::swap(selection.first, selection.second);
    }

    viewport()->update();
}

int HexEditor::calculateOffset(const QPoint &pos)
{
    int x = pos.x();
    int y = pos.y() - headerHeight; // Adjust for header height

    if (y < 0) {
        return -1;  // Click in the header area
    }

    int row = verticalScrollBar()->value() + y / charHeight;
    int col = (x - addressAreaWidth) / (3 * charWidth);

    if (x < addressAreaWidth) {
        return -1;  // Click in the address area
    } else if (x < addressAreaWidth + hexAreaWidth) {
        col = (x - addressAreaWidth) / (3 * charWidth);
    } else if (x < addressAreaWidth + hexAreaWidth + asciiAreaWidth) {
        col = (x - addressAreaWidth - hexAreaWidth) / charWidth;
    } else {
        return -1;  // Click outside the hex or ASCII area
    }

    return row * bytesPerLine + col;
}

void HexEditor::drawAddressArea(QPainter &painter, int startLine)
{

       painter.setPen(Qt::darkMagenta);
    for (int line = startLine; line < startLine + (viewport()->height() - headerHeight) / charHeight; ++line)
    {
        QString address = QString("%1").arg(line * bytesPerLine, 16, 16, QChar('0')).toUpper();
        painter.drawText(0, headerHeight + (line - startLine + 1) * charHeight, address);
    }
}

void HexEditor::drawHexArea(QPainter &painter, int startLine)
{
    painter.setPen(Qt::black);

    for (int line = startLine; line < startLine + (viewport()->height() - headerHeight) / charHeight; ++line)
    {
        for (int byte = 0; byte < bytesPerLine; ++byte)
        {
            int pos = line * bytesPerLine + byte;
            if (pos >= m_data.size())
                return;

            if (selection.first != -1 && pos >= selection.first && pos <= selection.second) {
                painter.setPen(Qt::white);
                painter.fillRect(addressAreaWidth + byte * 3 * charWidth, headerHeight + (line - startLine) * charHeight, 3 * charWidth, charHeight, QBrush(Qt::blue, Qt::SolidPattern));
                painter.setPen(Qt::black);
            }

            QString hex = QString("%1").arg((unsigned char)m_data[pos], 2, 16, QChar('0')).toUpper();
            painter.drawText(addressAreaWidth + byte * 3 * charWidth, headerHeight + (line - startLine + 1) * charHeight, hex);
        }
    }

    // Draw vertical lines after every 8 columns
    for (int i = 8; i < bytesPerLine; i += 8) {
        int x = addressAreaWidth + i * 3 * charWidth;
        painter.drawLine(x, headerHeight, x, viewport()->height());
    }

    // Draw vertical line between hex and ASCII areas
    int separatorX = addressAreaWidth + hexAreaWidth;
    painter.drawLine(separatorX, headerHeight, separatorX, viewport()->height());
}

void HexEditor::drawAsciiArea(QPainter &painter, int startLine)
{
    for (int line = startLine; line < startLine + (viewport()->height() - headerHeight) / charHeight; ++line)
    {
        for (int byte = 0; byte < bytesPerLine; ++byte)
        {
            int pos = line * bytesPerLine + byte;
            if (pos >= m_data.size())
                return;

            if (selection.first != -1 && pos >= selection.first && pos <= selection.second) {

                painter.fillRect(addressAreaWidth + hexAreaWidth + byte * charWidth, headerHeight + (line - startLine) * charHeight, charWidth, charHeight, QBrush(Qt::blue, Qt::SolidPattern));


            }

            char ch = m_data[pos];
            if (ch < 32 || ch > 126)
                ch = '.';
            painter.drawText(addressAreaWidth + hexAreaWidth + byte * charWidth, headerHeight + (line - startLine + 1) * charHeight, QChar(ch));
        }
    }
}

void HexEditor::drawHeader(QPainter &painter)
{
    painter.setFont(font());
    painter.setBrush(Qt::white);
    painter.setPen(Qt::darkMagenta);

    painter.drawRect(0, 0, viewport()->width(), headerHeight);

    painter.drawText(0, charHeight, " Offset");


    for (int i = 0; i < bytesPerLine; ++i) {
        QString hexHeader = QString("%1").arg(i, 2, 16, QChar('0')).toUpper();
        painter.drawText(addressAreaWidth + i * 3 * charWidth, charHeight, hexHeader);

        // Draw vertical line after every 8 columns
        if (i > 0 && i % 8 == 0) {
            int x = addressAreaWidth + i * 3 * charWidth;
            painter.drawLine(x, 0, x, headerHeight);
        }
    }

    // Draw vertical line between hex and ASCII areas
    int separatorX = addressAreaWidth + hexAreaWidth;
    painter.drawLine(separatorX, 0, separatorX, headerHeight);
}

void HexEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);

    QAction *copyAction = new QAction("Copy", this);
    connect(copyAction, &QAction::triggered, this, &HexEditor::onCopy);


    QAction *tagSelectedBytesAction = new QAction("Tag Selected Bytes", this);
    QAction *startBlocksAction = new QAction("Start Block", this);

    QAction *endBlockAction = new QAction("End Block", this);
    QAction *applyTemplateAction = new QAction("Apply Template", this);


    contextMenu.addAction(copyAction);
    contextMenu.addAction(tagSelectedBytesAction);
    contextMenu.addAction(startBlocksAction);


    QMenu *showAsMenu = contextMenu.addMenu("Show as");
    QMenu *diskStructuresMenu = showAsMenu->addMenu("Disk Structures");

    QAction *mbrAction = new QAction("MBR", this);
    QAction *gptAction = new QAction("GPT", this);


    diskStructuresMenu->addAction(mbrAction);
    diskStructuresMenu->addAction(gptAction);

    contextMenu.addAction(endBlockAction);
    contextMenu.addAction(applyTemplateAction);




    contextMenu.exec(event->globalPos());
}

void HexEditor::onCopy()
{
    if (selection.first == -1 || selection.second == -1) {
        return;
    }

    int start = qMin(selection.first, selection.second);
    int end = qMax(selection.first, selection.second);

    QString selectedText;
    for (int i = start; i <= end; ++i) {
        selectedText.append(QString("%1").arg((unsigned char)m_data[i], 2, 16, QChar('0')).toUpper()).append(' ');
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(selectedText.trimmed());
}
