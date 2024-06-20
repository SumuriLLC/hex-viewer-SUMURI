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
#include <QInputDialog>

HexEditor::HexEditor(QWidget *parent)
    : QAbstractScrollArea(parent),
    bytesPerLine(16),
    selection(qMakePair(-1, -1)),  // Initialize selection as invalid
    isDragging(false),  // Initialize dragging flag
    cursorVisible(true),
    cursorPosition(0)
{
    setFont(QFont("Courier New", 10));
    QFontMetrics fm(font());
    charWidth = fm.horizontalAdvance('0');
    charHeight = fm.height();
    headerHeight = charHeight + 4;

    addressAreaWidth = charWidth * 18;
    hexAreaWidth = charWidth * 3 * bytesPerLine;
    asciiAreaWidth = charWidth * bytesPerLine;

   // setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   // setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    updateScrollbar();

    // Initialize cursor blink timer
    cursorBlinkTimer.setInterval(500);
    connect(&cursorBlinkTimer, &QTimer::timeout, this, &HexEditor::updateCursorBlink);
    cursorBlinkTimer.start();
}

void HexEditor::setData(const QByteArray &data)
{
    {
        QMutexLocker locker(&m_mutex); // Lock the mutex for thread-safe access
        m_data = data;
    } // Unlock the mutex immediately after modifying m_data
    updateScrollbar();

    // Reset selection
    selection.first = -1;
    selection.second = -1;
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

    quint64 firstLine = verticalScrollBar()->value();
    int horizontalOffset = horizontalScrollBar()->value();

    drawHeader(painter, horizontalOffset);

    {
        QMutexLocker locker(&m_mutex); // Lock the mutex during painting to ensure thread-safe access
        drawAddressArea(painter, firstLine, horizontalOffset);
        drawHexArea(painter, firstLine, horizontalOffset);
        drawAsciiArea(painter, firstLine, horizontalOffset);
        if (cursorVisible) {
            drawCursor(painter);
        }
    } // Unlock the mutex immediately after drawing
}

void HexEditor::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateScrollbar();
    viewport()->update();
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
        updateSelection(event->pos(), true);
        cursorPosition = selection.second;
    }
}

void HexEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        updateSelection(event->pos(), false);
        cursorPosition = selection.second;
    }
}

void HexEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        updateSelection(event->pos(), false);
        cursorPosition = selection.second;
    }
}

void HexEditor::updateScrollbar()
{
    {
        QMutexLocker locker(&m_mutex); // Lock the mutex for thread-safe access
        quint64 lines = (m_data.size() + bytesPerLine - 1) / bytesPerLine;
        verticalScrollBar()->setRange(0, lines-1); //1 Line adjustment
        verticalScrollBar()->setPageStep((viewport()->height() - headerHeight) / charHeight);

        int contentWidth = addressAreaWidth + hexAreaWidth + asciiAreaWidth;
        horizontalScrollBar()->setRange(0, qMax(0, contentWidth - viewport()->width()));
        horizontalScrollBar()->setPageStep(viewport()->width() / charWidth);
    } // Unlock the mutex immediately after updating scrollbar range
}

void HexEditor::updateSelection(const QPoint &pos, bool reset)
{
    quint64 offset = calculateOffset(pos);
    if (offset == -1 || offset >= m_data.size()) {
        return;  // Click outside the data range
    }

    if (reset) {
        selection.first = selection.second = offset;
    } else {
        selection.second = offset;
    }

    if (selection.first > selection.second) {
        std::swap(selection.first, selection.second);
    }

    cursorPosition = selection.second; // Update cursor position
    QByteArray selectedData = m_data.mid(selection.first, selection.second - selection.first + 1);
    emit selectionChanged(selectedData, selection.first, selection.second);

    viewport()->update();
}

QByteArray HexEditor::getSelectedBytes() const
{
    if (selection.first == -1 || selection.second == -1 || selection.first >= m_data.size()) {
        return QByteArray();
    }

    quint64 start = qMin(selection.first, selection.second);
    quint64 end = qMin(qMax(selection.first, selection.second), m_data.size() - 1);

    return m_data.mid(start, end - start + 1);
}

void HexEditor::setSelectedBytes(const QByteArray &selectedBytes)
{
    if (selectedBytes.isEmpty()) {
        selection.first = -1;
        selection.second = -1;
    } else {
        quint64 startOffset = m_data.indexOf(selectedBytes);
        if (startOffset != -1) {
            selection.first = startOffset;
            selection.second = startOffset + selectedBytes.size() - 1;
        } else {
            selection.first = -1;
            selection.second = -1;
        }
    }

    viewport()->update();
}

void HexEditor::setSelectedByte(quint64 offset)
{
    if (offset >= 0 && offset < m_data.size()) {
        selection.first = offset;
        selection.second = offset;
    } else {
        selection.first = -1;
        selection.second = -1;
    }

    viewport()->update();
    emit selectionChanged(m_data, selection.first, selection.second);  // Emit the signal
}

quint64 HexEditor::calculateOffset(const QPoint &pos)
{
    int x = pos.x() + horizontalScrollBar()->value();
    int y = pos.y() - headerHeight; // Adjust for header height

    if (y < 0) {
        return -1;  // Click in the header area
    }

    quint64 row = verticalScrollBar()->value() + y / charHeight;
    int col;

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

void HexEditor::drawAddressArea(QPainter &painter, quint64 startLine, int horizontalOffset)
{
    painter.setPen(Qt::darkMagenta);
    for (quint64 line = startLine; line < startLine + (viewport()->height() - headerHeight) / charHeight; ++line) {

        if (line * bytesPerLine >= m_data.size()) break;
        QString address = QString("%1").arg(line * bytesPerLine, 16, 16, QChar('0')).toUpper();
        painter.drawText(-horizontalOffset, headerHeight + (line - startLine + 1) * charHeight, address);
    }
}

void HexEditor::drawHexArea(QPainter &painter, quint64 startLine, int horizontalOffset)
{
    painter.setPen(Qt::black);
    QFont originalFont = painter.font();
    QFont boldFont = originalFont;
    boldFont.setBold(true);


    int linesVisible = (viewport()->height() - headerHeight) / charHeight;
    quint64 endLine = startLine + linesVisible;


    int totalHeight = headerHeight + (linesVisible + 1) * charHeight; // Ensure total height includes all visible lines

    // Draw vertical lines after every 8 columns
    for (quint64 i = 8; i < bytesPerLine; i += 8) {

        quint64 x = addressAreaWidth + i * 3 * charWidth - horizontalOffset;
        painter.drawLine(x, headerHeight, x, totalHeight);
    }

    // Draw vertical line between hex and ASCII areas
    quint64 separatorX = addressAreaWidth + hexAreaWidth - horizontalOffset;
    painter.drawLine(separatorX, headerHeight, separatorX, totalHeight);


    for (quint64 line = startLine; line <= endLine; ++line) {


         if (line * bytesPerLine >= m_data.size()) break;

        for (quint64 byte = 0; byte < bytesPerLine; ++byte) {
            quint64 pos = line * bytesPerLine + byte;
            if (pos >= m_data.size())
                return;

            if (selection.first != -1 && pos >= selection.first && pos <= selection.second) {
                painter.fillRect(addressAreaWidth + byte * 3 * charWidth - horizontalOffset, headerHeight + (line - startLine) * charHeight, 3 * charWidth, charHeight, Qt::darkBlue);
                painter.setPen(Qt::white);
            } else {
                painter.setPen(Qt::black);
            }

            if (pos == cursorPosition) {
                painter.setFont(boldFont); // Set bold font for cursor position
            } else {
                painter.setFont(originalFont); // Set original font for other positions
            }

            QString hex = QString("%1").arg((unsigned char)m_data[pos], 2, 16, QChar('0')).toUpper();
            painter.drawText(addressAreaWidth + byte * 3 * charWidth - horizontalOffset, headerHeight + (line - startLine + 1) * charHeight, hex);
        }
    }


}

void HexEditor::drawAsciiArea(QPainter &painter, quint64 startLine, int horizontalOffset)
{
    int linesVisible = (viewport()->height() - headerHeight) / charHeight;
    quint64 endLine = startLine + linesVisible;

    for (quint64 line = startLine; line <= endLine; ++line) {
        for (quint64 byte = 0; byte < bytesPerLine; ++byte) {
            quint64 pos = line * bytesPerLine + byte;
            if (pos >= m_data.size())
                return;

            if (selection.first != -1 && pos >= selection.first && pos <= selection.second) {
                painter.fillRect(addressAreaWidth + hexAreaWidth + byte * charWidth - horizontalOffset, headerHeight + (line - startLine) * charHeight, charWidth, charHeight, Qt::darkBlue);
                painter.setPen(Qt::white);
            } else {
                painter.setPen(Qt::black);
            }

            char ch = m_data[pos];
            if ((ch < 32) || (ch > 126))
                ch = '.';



            painter.drawText(addressAreaWidth + hexAreaWidth + byte * charWidth - horizontalOffset, headerHeight + (line - startLine + 1) * charHeight, QChar(ch));
        }
    }
}

void HexEditor::drawHeader(QPainter &painter, int horizontalOffset)
{
    painter.setFont(font());
    painter.setBrush(Qt::white);
    painter.setPen(Qt::darkMagenta);

    painter.drawRect(0, 0, viewport()->width(), headerHeight);

    painter.drawText(-horizontalOffset, charHeight, " Offset");

    for (quint64 i = 0; i < bytesPerLine; ++i) {
        QString hexHeader = QString("%1").arg(i, 2, 16, QChar('0')).toUpper();
        painter.drawText(addressAreaWidth + i * 3 * charWidth - horizontalOffset, charHeight, hexHeader);

        // Draw vertical line after every 8 columns
        if (i > 0 && i % 8 == 0) {
            quint64 x = addressAreaWidth + i * 3 * charWidth - horizontalOffset;
            painter.drawLine(x, 0, x, headerHeight);
        }
    }

    // Draw vertical line between hex and ASCII areas
    quint64 separatorX = addressAreaWidth + hexAreaWidth - horizontalOffset;
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

    quint64 start = qMin(selection.first, selection.second);
    quint64 end = qMax(selection.first, selection.second);

    QString selectedText;
    for (quint64 i = start; i <= end; ++i) {
        selectedText.append(QString("%1").arg((unsigned char)m_data[i], 2, 16, QChar('0')).toUpper()).append(' ');
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(selectedText.trimmed());
}

void HexEditor::changeBytesPerLine(quint64 newBytesPerLine)
{
    bytesPerLine = newBytesPerLine;
    hexAreaWidth = charWidth * 3 * bytesPerLine;
    asciiAreaWidth = charWidth * bytesPerLine;
    updateScrollbar();
    viewport()->update();
}

void HexEditor::updateCursorBlink()
{
    cursorVisible = !cursorVisible;
    viewport()->update();
}

void HexEditor::clearSelection()
{
    selection.first = -1;
    selection.second = -1;
    viewport()->update();
}

void HexEditor::drawCursor(QPainter &painter)
{
    quint64 row = cursorPosition / bytesPerLine;
    quint64 col = cursorPosition % bytesPerLine;
    int x = addressAreaWidth + col * 3 * charWidth - horizontalScrollBar()->value();
    int y = headerHeight + (row - verticalScrollBar()->value()) * charHeight;

    painter.setPen(QPen(Qt::black, 2)); // Set the pen to black with a bold width
    painter.drawLine(x, y + charHeight + 2, x + charWidth, y + charHeight + 2); // Draw a line below the character
}

void HexEditor::setCursorPosition(quint64 position)
{
    cursorPosition = position;
    clearSelection(); // Clear the selection when the cursor position changes
    emit selectionChanged(QByteArray(), cursorPosition, cursorPosition); // Emit the signal with an empty QByteArray for selection
    viewport()->update();
}

void HexEditor::ensureCursorVisible()
{
    quint64 lines = (cursorPosition / bytesPerLine);
    verticalScrollBar()->setValue(lines);
    viewport()->update();
}
