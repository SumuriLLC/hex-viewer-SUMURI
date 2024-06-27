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
#include <QFile>

HexEditor::HexEditor(QWidget *parent)
    : QAbstractScrollArea(parent),
    cursorPosition(0),
    bytesPerLine(16),  // Initialize selection as invalid
    selection(qMakePair(-1, -1)),  // Initialize dragging flag
    isDragging(false),
    cursorVisible(true),
 cursorBlinkState(true),
    visibleStart(0),
    visibleEnd(0),
    cursorByteOffset(0)
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

    // Initialize cursor blink timer
    cursorBlinkTimer.setInterval(500);
    connect(&cursorBlinkTimer, &QTimer::timeout, this, &HexEditor::updateCursorBlink);
    cursorBlinkTimer.start();
}

void HexEditor::setData(const QString &filePath)
{
    {
        file.setFileName(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            // Handle error
            return;
        }
        fileSize = file.size();
        m_data.clear();
    } // Unlock the mutex immediately after modifying m_data

    updateScrollbar();
    updateVisibleData();

    // Reset selection
    selection.first = -1;
    selection.second = -1;
    selectedOffsets.clear();
    viewport()->update();
}

QByteArray HexEditor::getData() const
{
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


    drawAddressArea(painter, firstLine, horizontalOffset);
    drawHexArea(painter, firstLine, horizontalOffset);
    drawAsciiArea(painter, firstLine, horizontalOffset);
    drawCursor(painter);

}

void HexEditor::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateScrollbar();
    updateVisibleData();
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
        cursorPosition = selection.second + visibleStart; // Update cursor position with respect to the file
    }
}

void HexEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        updateSelection(event->pos(), false);
        cursorPosition = selection.second + visibleStart; // Update cursor position with respect to the file
    }
}

void HexEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        updateSelection(event->pos(), false);
        cursorPosition = selection.second + visibleStart; // Update cursor position with respect to the file
    }
}

void HexEditor::updateScrollbar()
{

        quint64 lines = (fileSize + bytesPerLine - 1) / bytesPerLine;
        verticalScrollBar()->setRange(0, lines - 1); //1 Line adjustment
        verticalScrollBar()->setPageStep((viewport()->height() - headerHeight) / charHeight);

        int contentWidth = addressAreaWidth + hexAreaWidth + asciiAreaWidth;
        horizontalScrollBar()->setRange(0, qMax(0, contentWidth - viewport()->width()));
        horizontalScrollBar()->setPageStep(viewport()->width() / charWidth);
}

void HexEditor::updateVisibleData()
{

    quint64 firstLine = verticalScrollBar()->value();
    quint64 linesVisible = (viewport()->height() - headerHeight) / charHeight;

    visibleStart = firstLine * bytesPerLine;
    visibleEnd = qMin(fileSize, visibleStart + linesVisible * bytesPerLine);

    file.seek(visibleStart);
    data_visible = file.read(visibleEnd - visibleStart);

    viewport()->update();
}

void HexEditor::updateSelection(const QPoint &pos, bool reset)
{
    qint64 offset = calculateOffset(pos);
    if (offset == -1 || offset >= data_visible.size()) {
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

    // Update the set of selected offsets
    selectedOffsets.clear();
    for (quint64 i = selection.first + visibleStart; i <= selection.second + visibleStart; ++i) {
        selectedOffsets.insert(i);
    }

    cursorPosition = selection.second + visibleStart; // Update cursor position with respect to the file
    QByteArray selectedData = data_visible.mid(selection.first, selection.second - selection.first + 1);
    emit selectionChanged(selectedData, selection.first + visibleStart, selection.second + visibleStart);

    viewport()->update();
}

QByteArray HexEditor::getSelectedBytes() const
{
    QByteArray selectedBytes;
    for (quint64 offset : selectedOffsets) {
        if (offset >= visibleStart && offset < visibleEnd) {
            selectedBytes.append(data_visible[offset - visibleStart]);
        }
    }
    return selectedBytes;
}

void HexEditor::setSelectedBytes(const QByteArray &selectedBytes)
{
    if (selectedBytes.isEmpty()) {
        selection.first = -1;
        selection.second = -1;
        selectedOffsets.clear();
    } else {
        quint64 startOffset = data_visible.indexOf(selectedBytes);
        if (startOffset != -1) {
            selection.first = startOffset;
            selection.second = startOffset + selectedBytes.size() - 1;
            for (quint64 i = selection.first + visibleStart; i <= selection.second + visibleStart; ++i) {
                selectedOffsets.insert(i);
            }
        } else {
            selection.first = -1;
            selection.second = -1;
            selectedOffsets.clear();
        }
    }

    viewport()->update();
}

void HexEditor::setSelectedByte(qint64 offset)
{

    quint64 unsignedOffset = static_cast<quint64>(offset);

    if (unsignedOffset >= 0 && unsignedOffset < fileSize) {
        if (unsignedOffset < visibleStart || unsignedOffset >= visibleEnd) {
            verticalScrollBar()->setValue(unsignedOffset / bytesPerLine);
            updateVisibleData();
        }
        selection.first = unsignedOffset - visibleStart;
        selection.second = selection.first;
        selectedOffsets.clear();
        selectedOffsets.insert(offset);
    } else {
        selection.first = -1;
        selection.second = -1;
        selectedOffsets.clear();
    }

    cursorPosition =selection.first + visibleStart; //Update cursor too
    cursorByteOffset=cursorPosition;

    viewport()->update();
    emit selectionChanged(data_visible, selection.first + visibleStart, selection.second + visibleStart);  // Emit the signal
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

    return row * bytesPerLine + col - visibleStart;
}

void HexEditor::drawAddressArea(QPainter &painter, quint64 startLine, int horizontalOffset)
{
    painter.setPen(Qt::darkMagenta);
    for (quint64 line = startLine; line < startLine + (viewport()->height() - headerHeight) / charHeight; ++line) {

        if (line * bytesPerLine >= fileSize) break;
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
        if (line * bytesPerLine >= fileSize) break;

        for (quint64 byte = 0; byte < bytesPerLine; ++byte) {
            quint64 pos = line * bytesPerLine + byte;
            if (pos - visibleStart >= static_cast<quint64>(data_visible.size())) return;

            if (selectedOffsets.contains(pos)) {
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

            QString hex = QString("%1").arg((unsigned char)data_visible[pos - visibleStart], 2, 16, QChar('0')).toUpper();
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
            if (pos - visibleStart >= static_cast<quint64>(data_visible.size())) return;

            if (selectedOffsets.contains(pos)) {
                painter.fillRect(addressAreaWidth + hexAreaWidth + byte * charWidth - horizontalOffset, headerHeight + (line - startLine) * charHeight, charWidth, charHeight, Qt::darkBlue);
                painter.setPen(Qt::white);
            } else {
                painter.setPen(Qt::black);
            }

            char ch = data_visible[pos - visibleStart];
            if ((ch < 32) || (ch > 126)) ch = '.';

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
        selectedText.append(QString("%1").arg((unsigned char)data_visible[i], 2, 16, QChar('0')).toUpper()).append(' ');
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
    updateVisibleData();
    viewport()->update();
}

void HexEditor::updateCursorBlink()
{
    cursorBlinkState = !cursorBlinkState;
    viewport()->update();  // Repaint the viewport to update the cursor blink state
}

void HexEditor::clearSelection()
{
    selection.first = -1;
    selection.second = -1;
    selectedOffsets.clear();
    viewport()->update();
}
void HexEditor::drawCursor(QPainter &painter)
{
    if (!cursorBlinkState) {
        return;  // Skip drawing the cursor if the blink state is off
    }

    // Calculate row and column based on the cursor position
    quint64 row = cursorPosition / bytesPerLine;
    quint64 col = cursorPosition % bytesPerLine;

    // Calculate x and y positions based on row and column
    int x = addressAreaWidth + col * 3 * charWidth - horizontalScrollBar()->value();
    int y = headerHeight + (row - verticalScrollBar()->value()) * charHeight;

    painter.setPen(QPen(Qt::black, 2)); // Set the pen to black with a bold width
    painter.drawLine(x, y + charHeight + 2, x + charWidth, y + charHeight + 2); // Draw a line below the character
}





void HexEditor::setCursorPosition(quint64 position)
{
    cursorPosition = position;
    cursorByteOffset = position; // Store the cursor byte offset
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

void HexEditor::scrollContentsBy(int dx, int dy)
{
    QAbstractScrollArea::scrollContentsBy(dx, dy);
    updateVisibleData();
}
