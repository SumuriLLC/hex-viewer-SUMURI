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
#include "headers/newtagdialog.h"
#include <QDebug>
#include "headers/tagshandler.h"
#include "headers/tagdialogmodel.h"

#include <QTextStream>
#include <QFileDialog>
#include <windows.h>
#include "headers/windowsdrivedevice.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>
#include <QMessageBox>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

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
    cursorByteOffset(0),
    startBlockOffset(0),
    startBlockSelected(false),
    tagsHandler(nullptr),
    userTagsHandler(nullptr),
    currentTabIndex(0),
    currentSearchIndex(-1),
    loadingDialog(new LoadingDialog(this)),
    file_name("")
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




void HexEditor::setTagsHandler(TagsHandler *tagsHandler)
{
    this->tagsHandler = tagsHandler;
}

void HexEditor::setUserTagsHandler(TagsHandler *userTagsHandler)
{
    this->userTagsHandler = userTagsHandler;
}




void HexEditor::setData(const QString &filePath,int tabIndex)
{


    file_name=filePath;
    currentTabIndex=tabIndex;
    QFileInfo fileInfo(filePath);
    delete device; // Clean up any previously used device

    // Check if the filePath represents a physical drive
    if (filePath.startsWith("\\\\.\\PhysicalDrive")) {

        qDebug() << "Opening to  Windows drive device.";


        WindowsDriveDevice *driveDevice = new WindowsDriveDevice(filePath, this);
        if (!driveDevice->open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open Windows drive device.";
            delete driveDevice;
            device = nullptr;
            return;
        }
        device = driveDevice;
        fileSize = device->size();
        qDebug() << "Windows device file size:" << fileSize;





    } else if (fileInfo.suffix().toUpper() == "E01") {
        EwfDevice *ewfDevice = new EwfDevice(this);
        if (!ewfDevice->openEwf(filePath.toStdString().c_str(), QIODevice::ReadOnly)) {
            qDebug() << "Failed to open EWF device.";
            delete ewfDevice;
            device = nullptr;
            return;
        }
        device = ewfDevice;
        fileSize = device->size(); // This should work for EWF files
        qDebug() << "EWF file size:" << fileSize;
    } else {
        // Handle regular file
        device = new QFile(filePath, this);
        if (!device->open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file.";
            delete device;
            device = nullptr;
            return;
        }
        fileSize = device->size();
        qDebug() << "Regular file size:" << fileSize;
    }

    //File Size is Limited to 32GB due to limits for data structues
    qint64 maxFileSizeLimit=32212254720 ;
    if(fileSize >maxFileSizeLimit){
        fileSize= maxFileSizeLimit;
    }

    m_data.clear();

    updateScrollbar();
    updateVisibleData();

    // Reset selection
    selection.first = -1;
    selection.second = -1;
    selectedOffsets.clear();


    //Set saved tags from disk

    QList<Tag> userTags = userTagsHandler->getUserTagsFromUserDB(tabIndex);
    QList<Tag> templateTags = userTagsHandler->getTemplateTagsFromUserDB(tabIndex);

    for (const Tag &tag : userTags) {
        addTag(tag.offset, tag.length, tag.description, QColor(tag.color), tag.type);
    }

    for (const Tag &tag : templateTags) {
        addTag(tag.offset, tag.length, tag.description, QColor(tag.color), tag.type);
    }
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
    // Calculate total lines
    quint64 lines = (fileSize + bytesPerLine - 1) / bytesPerLine;

    //qDebug() << "Total lines:" << lines << "File size:" << fileSize << "Bytes per line:" << bytesPerLine;

    //Max scrollbar value that can be set
    if(lines >2147483647){
        lines=2147483647;
    }
    // Set vertical scrollbar range and page step
    verticalScrollBar()->setRange(0, lines-1 );
    verticalScrollBar()->setPageStep((viewport()->height() - headerHeight) / charHeight);

    // Calculate content width
    int contentWidth = addressAreaWidth + hexAreaWidth + asciiAreaWidth;

    // Set horizontal scrollbar range and page step
    horizontalScrollBar()->setRange(0, qMax(0, contentWidth - viewport()->width()));
    horizontalScrollBar()->setPageStep(viewport()->width() / charWidth);

   // qDebug() << "Vertical Scrollbar range:" << verticalScrollBar()->minimum() << "-" << verticalScrollBar()->maximum();
   // qDebug() << "Horizontal Scrollbar range:" << horizontalScrollBar()->minimum() << "-" << horizontalScrollBar()->maximum();
}


void HexEditor::updateVisibleData()
{

    quint64 firstLine = verticalScrollBar()->value();
    quint64 linesVisible = (viewport()->height() - headerHeight) / charHeight;

    visibleStart = firstLine * bytesPerLine;
    visibleEnd = qMin(fileSize, visibleStart + linesVisible * bytesPerLine);



    device->seek(visibleStart);
    data_visible = device->read(visibleEnd - visibleStart);


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

    // Update the set of selected offsets
    selectedOffsets.clear();
    quint64 selectionStart = qMin(selection.first, selection.second) + visibleStart;
    quint64 selectionEnd = qMax(selection.first, selection.second) + visibleStart;
    for (quint64 i = selectionStart; i <= selectionEnd; ++i) {
        selectedOffsets.insert(i);
    }

    cursorPosition = selection.second + visibleStart; // Update cursor position with respect to the file
    QByteArray selectedData = data_visible.mid(qMin(selection.first, selection.second),
                                               qAbs(selection.second - selection.first) + 1);
    emit selectionChanged(selectedData, selectionStart, selectionEnd);

    QString tagName = "";
    quint64 tagLength = 0;
    QString tagColor = "";

    for (const Tag &tag : tags) {
        if (cursorPosition >= tag.offset && cursorPosition < tag.offset + tag.length) {
            tagName = tag.description;
            tagLength = tag.length;
            tagColor = tag.color;
            break;
        }
    }

    emit tagNameAndLength(tagName, tagLength, tagColor);
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
   // qDebug() << "Setting selected byte at offset:" << offset << " (unsigned:" << unsignedOffset << ")";
   // qDebug() << "File size:" << fileSize;

    if (unsignedOffset < fileSize) {
       // qDebug() << "Offset is within the file size range.";
        if (unsignedOffset < visibleStart || unsignedOffset >= visibleEnd) {
          //  qDebug() << "Offset is outside the current visible range. Updating vertical scroll value.";
            verticalScrollBar()->setValue(unsignedOffset / bytesPerLine);
            updateVisibleData();
        } else {
          //  qDebug() << "Offset is within the current visible range.";
        }

        selection.first = unsignedOffset - visibleStart;
        selection.second = selection.first;
        selectedOffsets.clear();
        selectedOffsets.insert(offset);

      //  qDebug() << "Selection updated. First:" << selection.first << "Second:" << selection.second;
    } else {
       // qDebug() << "Offset is out of the file size range. Clearing selection.";
        selection.first = -1;
        selection.second = -1;
        selectedOffsets.clear();
    }

    cursorPosition = selection.first + visibleStart;
    cursorByteOffset = cursorPosition;

    //qDebug() << "Cursor position updated:" << cursorPosition;

    viewport()->update();
    emit selectionChanged(data_visible, selection.first + visibleStart, selection.second + visibleStart);

    //qDebug() << "Emitted selectionChanged signal.";
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



    for (quint64 line = startLine; line <= endLine; ++line) {
        if (line * bytesPerLine >= fileSize) break;

        for (quint64 byte = 0; byte < bytesPerLine; ++byte) {
            quint64 pos = line * bytesPerLine + byte;
            if (pos - visibleStart >= static_cast<quint64>(data_visible.size())) return;

            QColor backgroundColor = Qt::white;

            // Check if the position is within the selected offsets first
            bool isSelected = selectedOffsets.contains(pos);
            bool isHighlighted = highligtedOffsets.contains(pos);

            if (isHighlighted) {
                backgroundColor = Qt::yellow;
            }

            if (isSelected) {
                backgroundColor = Qt::darkBlue;
            } else {
                // If not selected, check for tags
                for (const Tag &tag : tags) {
                    if (pos >= tag.offset && pos < tag.offset + tag.length) {
                        backgroundColor = tag.color;
                        break;
                    }
                }
            }

           // painter.fillRect(addressAreaWidth + byte * 3 * charWidth - horizontalOffset, headerHeight + (line - startLine) * charHeight, 3 * charWidth, charHeight, backgroundColor);

            int x_highlight_offset=-4;
            int y_highlight_offset=3;

            painter.fillRect(
                addressAreaWidth + byte * 3 * charWidth - horizontalOffset + x_highlight_offset,  // Adjust x-position
                headerHeight + (line - startLine) * charHeight +y_highlight_offset ,
                3 * charWidth ,  // Adjust width to center text within
                charHeight,
                backgroundColor
                );



            // Set the font color based on the background color's brightness
            int brightness = (backgroundColor.red() * 299 + backgroundColor.green() * 587 + backgroundColor.blue() * 114) / 1000;
            QColor fontColor = (brightness > 128) ? Qt::black : Qt::white;
            painter.setPen(fontColor);

            if (pos == cursorPosition) {
                painter.setFont(boldFont); // Set bold font for cursor position
            } else {
                painter.setFont(originalFont); // Set original font for other positions
            }

            QString hex = QString("%1").arg((unsigned char)data_visible[pos - visibleStart], 2, 16, QChar('0')).toUpper();
            painter.drawText(addressAreaWidth + byte * 3 * charWidth - horizontalOffset, headerHeight + (line - startLine + 1) * charHeight, hex);
        }


        int totalHeight = headerHeight + (linesVisible + 1) * charHeight; // Ensure total height includes all visible lines

        // Draw vertical lines after every 8 columns
        for (quint64 i = 8; i < bytesPerLine; i += 8) {
            int x = addressAreaWidth + i * 3 * charWidth - horizontalOffset;
            //qDebug() << "Drawing vertical line... i:" << i << "x:" << x << "headerHeight:" << headerHeight << "totalHeight:" << totalHeight;
            painter.drawLine(x-3, headerHeight, x-3, totalHeight);
        }

        // Draw vertical line between hex and ASCII areas
        int separatorX = addressAreaWidth + hexAreaWidth - horizontalOffset-1;
        //qDebug() << "Drawing separator line... separatorX:" << separatorX << "headerHeight:" << headerHeight << "totalHeight:" << totalHeight;
        painter.setPen(Qt::gray);  // Set pen color for separator
        painter.drawLine(separatorX-3, headerHeight, separatorX-3, totalHeight);
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

            QColor backgroundColor = Qt::white;

            // Check if the position is within the selected offsets first
            bool isSelected = selectedOffsets.contains(pos);
            bool isHighlighted=highligtedOffsets.contains(pos);

            if (isHighlighted) {
                backgroundColor = Qt::yellow;
            }

            if (isSelected) {
                backgroundColor = Qt::darkBlue;
            } else {
                // If not selected, check for tags
                for (const Tag &tag : tags) {
                    if (pos >= tag.offset && pos < tag.offset + tag.length) {
                        backgroundColor = tag.color;
                        break;
                    }
                }
            }

            painter.fillRect(addressAreaWidth + hexAreaWidth + byte * charWidth - horizontalOffset, headerHeight + (line - startLine) * charHeight, charWidth, charHeight, backgroundColor);

            // Set the font color based on the background color's brightness
            int brightness = (backgroundColor.red() * 299 + backgroundColor.green() * 587 + backgroundColor.blue() * 114) / 1000;
            QColor fontColor = (brightness > 128) ? Qt::black : Qt::white;
            painter.setPen(fontColor);

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
            painter.drawLine(x-3, 0, x-3, headerHeight);
        }
    }

    // Draw vertical line between hex and ASCII areas
    quint64 separatorX = addressAreaWidth + hexAreaWidth - horizontalOffset-1;
    painter.drawLine(separatorX-3, 0, separatorX-3, headerHeight);
}

void HexEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);

    ////////Copy Menu ////////////////
    QAction *copyAction = new QAction("Copy", this);
    contextMenu.addAction(copyAction);
    connect(copyAction, &QAction::triggered, this, &HexEditor::onCopy);

    ////////Tag Selected Bytes Menu////
    QAction *tagSelectedBytesAction = new QAction("Tag Selected Bytes", this);
    contextMenu.addAction(tagSelectedBytesAction);
    connect(tagSelectedBytesAction, &QAction::triggered, this, &HexEditor::onTagSelectedBytes);

    ////////Start Block Menu///////////
    QAction *startBlocksAction = new QAction("Start Block", this);
    contextMenu.addAction(startBlocksAction);
    connect(startBlocksAction, &QAction::triggered, this, &HexEditor::onStartBlock);


    ////////End Block Action////////////
    QAction *endBlockAction = new QAction("End Block", this);
    contextMenu.addAction(endBlockAction);
    connect(endBlockAction, &QAction::triggered, this, &HexEditor::onEndBlock);


    ////////////Show as Menu/////////////////////

    QMenu *showAsMenu = contextMenu.addMenu("Show as");

        //Disk Structures
        QMenu *diskStructuresMenu = showAsMenu->addMenu("Disk Structures");

            QAction *mbrAction = new QAction("MBR", this);
            diskStructuresMenu->addAction(mbrAction);
            connect(mbrAction, &QAction::triggered, this, [this]() { onShowTags("MBR"); });

            QAction *gptAction = new QAction("GPT", this);
            diskStructuresMenu->addAction(gptAction);
            connect(gptAction, &QAction::triggered, this, [this]() { onShowTags("GPT"); });


        //FAT
        QMenu *showAsFATMenu = showAsMenu->addMenu("FAT");

            QAction *showAsFATVBRAction = new QAction("FAT VBR", this);
            showAsFATMenu->addAction(showAsFATVBRAction);
            connect(showAsFATVBRAction, &QAction::triggered, this, [this]() { onShowTags("FAT VBR"); });

            QAction *showAsFAT32VBRAction = new QAction("FAT32 VBR", this);
            showAsFATMenu->addAction(showAsFAT32VBRAction);
            connect(showAsFAT32VBRAction, &QAction::triggered, this, [this]() { onShowTags("FAT32 VBR"); });

            QAction *showAsFAT32SFNAction = new QAction("FAT32 SFN Directory Entry", this);
            showAsFATMenu->addAction(showAsFAT32SFNAction);
            connect(showAsFAT32SFNAction, &QAction::triggered, this, [this]() { onShowTags("FAT32 SFN Directory Entry"); });




        //exFAT
        QMenu *showAsExFATMenu = showAsMenu->addMenu("exFAT");

            QAction *showAsexFATVBRAction = new QAction("exFAT VBR", this);
            showAsExFATMenu->addAction(showAsexFATVBRAction);
            connect(showAsexFATVBRAction, &QAction::triggered, this, [this]() { onShowTags("exFAT VBR"); });


        //NTFS
        QMenu *showAsNTFSMenu = showAsMenu->addMenu("NTFS");

            QAction *showAsNTFSVBRAction = new QAction("NTFS VBR", this);
            showAsNTFSMenu->addAction(showAsNTFSVBRAction);
            connect(showAsNTFSVBRAction, &QAction::triggered, this, [this]() { onShowTags("NTFS VBR"); });

            QAction *showAsNTFSMFTaction = new QAction("NTFS MFT Entry", this);
            showAsNTFSMenu->addAction(showAsNTFSMFTaction);
            connect(showAsNTFSMFTaction, &QAction::triggered, this, [this]() { onShowTags("File Record Header"); });


            QAction *showAsNTFSRunlistaction = new QAction("NTFS Runlist Entry", this);
            showAsNTFSMenu->addAction(showAsNTFSRunlistaction);
            connect(showAsNTFSRunlistaction, &QAction::triggered, this, [this]() { onShowTags("NTFS Runlist Entry"); });


        /////////////////////////////////////////////////



    // Apply Template Menu
    QMenu *applyTemplateMenu = contextMenu.addMenu("Apply Template");

        // Disk Structures
        QMenu *templateDiskStructuresMenu = applyTemplateMenu->addMenu("Disk Structures");
            QAction *templateMbrAction = new QAction("MBR", this);
            templateDiskStructuresMenu->addAction(templateMbrAction);
            connect(templateMbrAction, &QAction::triggered, this, [this]() { onApplyTags("MBR"); });


            QAction *templateMptAction = new QAction("MPT", this);
            templateDiskStructuresMenu->addAction(templateMptAction);
            connect(templateMptAction, &QAction::triggered, this, [this]() { onApplyTags("MPT"); });



            QMenu *gptMenu = templateDiskStructuresMenu->addMenu("GPT");

                QAction *protectiveMbrAction = new QAction("Protective MBR", this);
                gptMenu->addAction(protectiveMbrAction);
                connect(protectiveMbrAction, &QAction::triggered, this, [this]() { onApplyTags("Protective MBR"); });

                QAction *gptHeaderAction = new QAction("GPT Header", this);
                gptMenu->addAction(gptHeaderAction);
                connect(gptHeaderAction, &QAction::triggered, this, [this]() { onApplyTags("GPT Header"); });


                QAction *gptEntryAction = new QAction("GPT Entry", this);
                gptMenu->addAction(gptEntryAction);
                connect(gptEntryAction, &QAction::triggered, this, [this]() { onApplyTags("GPT Entry"); });



        // FAT
        QMenu *fatMenu = applyTemplateMenu->addMenu("FAT");

            QAction *fatVBRAction = new QAction("FAT VBR", this);
            fatMenu->addAction(fatVBRAction);
            connect(fatVBRAction, &QAction::triggered, this, [this]() { onApplyTags("FAT VBR"); });


            QAction *fat32VBRAction = new QAction("FAT32 VBR", this);
            fatMenu->addAction(fat32VBRAction);
            connect(fat32VBRAction, &QAction::triggered, this, [this]() { onApplyTags("FAT32 VBR"); });

            QAction *fat32FileAllocationAction = new QAction("FAT32 File Allocation Table", this);
            fatMenu->addAction(fat32FileAllocationAction);
            connect(fat32FileAllocationAction, &QAction::triggered, this, [this]() { onApplyTags("FAT32 File Allocation Table"); });

            QAction *fat32SFNDirectoryAction = new QAction("FAT32 SFN Directory Entry", this);
            fatMenu->addAction(fat32SFNDirectoryAction);
            connect(fat32SFNDirectoryAction, &QAction::triggered, this, [this]() { onApplyTags("FAT32 SFN Directory Entry"); });


            QAction *fat32DosAliasLFNAction = new QAction("FAT32 DOS Alias with LFN", this);
            fatMenu->addAction(fat32DosAliasLFNAction);
            connect(fat32DosAliasLFNAction, &QAction::triggered, this, [this]() { onApplyTags("FAT32 DOS Alias with LFN"); });


        // NTFS
        QMenu *ntfsMenu = applyTemplateMenu->addMenu("NTFS");

            QAction *ntfsVBRAction = new QAction("NTFS VBR", this);
            ntfsMenu->addAction(ntfsVBRAction);
            connect(ntfsVBRAction, &QAction::triggered, this, [this]() { onApplyTags("NTFS VBR"); });

            QMenu *MFTEntrySubMenu = ntfsMenu->addMenu("$MFT Entry");

                QAction *fileRecordHeaderAction = new QAction("File Record Header", this);
                MFTEntrySubMenu->addAction(fileRecordHeaderAction);
                connect(fileRecordHeaderAction, &QAction::triggered, this, [this]() { onApplyTags("File Record Header"); });

                QAction *standardAttributerAction = new QAction("Standard Attribute", this);
                MFTEntrySubMenu->addAction(standardAttributerAction);
                connect(standardAttributerAction, &QAction::triggered, this, [this]() { onApplyTags("Standard Attribute"); });


                QAction *filenameAttributerAction = new QAction("Filename Attribute", this);
                MFTEntrySubMenu->addAction(filenameAttributerAction);
                connect(filenameAttributerAction, &QAction::triggered, this, [this]() { onApplyTags("Filename Attribute"); });


                QAction *dataAttributerAction = new QAction("Data Attribute", this);
                MFTEntrySubMenu->addAction(dataAttributerAction);
                connect(dataAttributerAction, &QAction::triggered, this, [this]() { onApplyTags("Data Attribute"); });

            QAction *ntfsRunlistEntryAction = new QAction("NTFS Runlist Entry", this);
            ntfsMenu->addAction(ntfsRunlistEntryAction);
            connect(ntfsRunlistEntryAction, &QAction::triggered, this, [this]() { onApplyTags("NTFS Runlist Entry"); });


        // ExFAT
        QMenu *exfatMenu = applyTemplateMenu->addMenu("ExFAT");

            QAction *exfatVBRAction = new QAction("ExFAT VBR", this);
            exfatMenu->addAction(exfatVBRAction);
            connect(exfatVBRAction, &QAction::triggered, this, [this]() { onApplyTags("exFAT VBR"); });


            QAction *exfatAllocationBitmapTableAction = new QAction("ExFAT Allocation Bitmap Table", this);
            exfatMenu->addAction(exfatAllocationBitmapTableAction);
            connect(exfatAllocationBitmapTableAction, &QAction::triggered, this, [this]() { onApplyTags("ExFAT Allocation Bitmap Table"); });


            QMenu *exfatDirectoryEntryMenu = exfatMenu->addMenu("ExFAT Directory Entry");
            exfatMenu->addAction(exfatAllocationBitmapTableAction);

                QAction *exfatAllocationBitmapDirectoryEntryAction = new QAction("ExFAT Allocation Bitmap Directory Entry", this);
                exfatDirectoryEntryMenu->addAction(exfatAllocationBitmapDirectoryEntryAction);
                connect(exfatAllocationBitmapDirectoryEntryAction, &QAction::triggered, this, [this]() { onApplyTags("ExFAT Allocation Bitmap Directory Entry"); });


                QAction *exfatUpcaseTableDirectoryEntryAction = new QAction("ExFAT Upcase Table Directory Entry", this);
                exfatDirectoryEntryMenu->addAction(exfatUpcaseTableDirectoryEntryAction);
                connect(exfatUpcaseTableDirectoryEntryAction, &QAction::triggered, this, [this]() { onApplyTags("ExFAT Upcase Table Directory Entry"); });

                QAction *exfatVolumrLabelDirectoryEntryAction = new QAction("ExFAT Volume Label Directory Entry", this);
                exfatDirectoryEntryMenu->addAction(exfatVolumrLabelDirectoryEntryAction);
                connect(exfatVolumrLabelDirectoryEntryAction, &QAction::triggered, this, [this]() { onApplyTags("ExFAT Volume Label Directory Entry"); });


                QAction *exfatFileDirectoryEntryAction = new QAction("ExFAT File Directory Entry", this);
                exfatDirectoryEntryMenu->addAction(exfatFileDirectoryEntryAction);
                connect(exfatFileDirectoryEntryAction, &QAction::triggered, this, [this]() { onApplyTags("ExFAT File Directory Entry"); });

                QAction *exfatStreamExtensionDirectoryEntryAction = new QAction("ExFAT Stream Extension Directory Entry", this);
                exfatDirectoryEntryMenu->addAction(exfatStreamExtensionDirectoryEntryAction);
                connect(exfatStreamExtensionDirectoryEntryAction, &QAction::triggered, this, [this]() { onApplyTags("ExFAT Stream Extension Directory Entry"); });

                QAction *exfatFileNameDirectoryEntryAction = new QAction("ExFAT File Name Directory Entry", this);
                exfatDirectoryEntryMenu->addAction(exfatFileNameDirectoryEntryAction);
                connect(exfatFileNameDirectoryEntryAction, &QAction::triggered, this, [this]() { onApplyTags("ExFAT File Name Directory Entry"); });




    contextMenu.exec(event->globalPos());
}

void HexEditor::onStartBlock()
{
    startBlockOffset = cursorPosition;
    startBlockSelected = true;
    qDebug() << "Start block selected at offset:" << startBlockOffset;
}

void HexEditor::onEndBlock()
{
    if (!startBlockSelected) {
        qDebug() << "Start block not selected.";
        return;
    }

    quint64 endBlockOffset = cursorPosition;
   // quint64 length = endBlockOffset - startBlockOffset + 1;

    if (startBlockOffset > endBlockOffset) {
        std::swap(startBlockOffset, endBlockOffset);
    }

    NewTagDialog dialog(this);
    dialog.setStartAddress(startBlockOffset);
    dialog.setEndAddress(endBlockOffset);
    QString description = QString("[%1] - [%2]").arg(startBlockOffset).arg(endBlockOffset);
    dialog.setDescription(description);
    connect(&dialog, &NewTagDialog::tagSaved, this, &HexEditor::addTag);
    dialog.exec();

    startBlockSelected = false;
}


void HexEditor::onShowTags(const QString &tagCategory)
{



    if(tagCategory=="GPT"){

        QMap<QString, QList<Tag>> tagsByGroup;
        QList<Tag> headertags = tagsHandler->getTagsByCategory("GPT Header");

        if(!headertags.isEmpty()){
            tagsByGroup.insert("GPT Header", headertags);
        }

        QList<Tag> entrytags= tagsHandler->getTagsByCategory("GPT Entry");

        if(!entrytags.isEmpty()){
            tagsByGroup.insert("GPT Entry", entrytags);
        }

        TagDialogModel dialog(tagCategory, tags, *device,cursorPosition, this,tagsByGroup);
        dialog.exec();
    }

    QList<Tag> tags = tagsHandler->getTagsByCategory(tagCategory);

    if (tags.isEmpty()) {
        qDebug() << "No tags found for category " << tagCategory;
        return;
    }

    TagDialogModel dialog(tagCategory, tags, *device,cursorPosition, this);
    dialog.exec();
}

void HexEditor::onApplyTags(QString category)
{


    QList<Tag> tagsFromDB = tagsHandler->getTagsByCategory(category);

    if (tagsFromDB.isEmpty()) {
        qDebug() << "No tags found for category " << category;
        return;
    }

   // qDebug()<< "cursorPosition:"<< cursorPosition;

    for (const Tag &tag : tagsFromDB) {
        addTag(tag.offset+cursorPosition, tag.length, tag.description, QColor(tag.color),"template");
    }

    viewport()->update(); // Refresh the viewport to apply the new tags

    //qDebug() << category<< " tags applied successfully.";
}





void HexEditor::onTagSelectedBytes()
{
    if (selection.first == -1 || selection.second == -1) {
        // No selection made
        return;
    }

    quint64 startAddr = qMin(selection.first, selection.second) + visibleStart;
    quint64 endAddr = qMax(selection.first, selection.second) + visibleStart;

    NewTagDialog dialog(this);
    dialog.setStartAddress(startAddr);
    dialog.setEndAddress(endAddr);
    QString description = QString("[%1] - [%2]").arg(startAddr).arg(endAddr);
    dialog.setDescription(description);
    connect(&dialog, &NewTagDialog::tagSaved, this, &HexEditor::addTag);
    dialog.exec();
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


void HexEditor::addTag(quint64 offset, quint64 length, const QString &description, const QColor &color, const QString &type)
{

    qDebug() << "Adding tags" << type;

    tags.append(Tag{offset, length, description, color.name(),"",type});

    emit tagsUpdated(tags);

    viewport()->update();


}

void HexEditor::clearTags(){
    tags.clear();

    emit tagsUpdated(tags);


}

void HexEditor::removeTag(quint64 offset, int index, const QString &tagType)
{
    Q_UNUSED(index);

    // Find all tags that start at this offset and match the tagType
    QVector<Tag> tagsToRemove;
    for (const Tag &tag : tags) {
        if (tag.offset == offset && tag.type == tagType) {
            tagsToRemove.append(tag);
        }
    }

    if (tagsToRemove.isEmpty()) {
        QMessageBox::information(this, tr("No Tag Found"),
                                 tr("No %1 tag found at offset %2.").arg(tagType).arg(offset));
        return;
    }

    // If there's more than one tag at this offset, ask the user which one(s) to delete
    if (tagsToRemove.size() > 1) {
        QStringList items;
        for (const Tag &tag : tagsToRemove) {
            items << QString("%1 (Length: %2, Color: %3)").arg(tag.description).arg(tag.length).arg(tag.color);
        }

        bool ok;
        QString item = QInputDialog::getItem(this, tr("Select Tag to Delete"),
                                             tr("Multiple %1 tags found at this offset. Select which to delete:").arg(tagType),
                                             items, 0, false, &ok);
        if (!ok || item.isEmpty()) {
            QMessageBox::information(this, tr("No Tag Selected"),
                                     tr("No tag was selected for deletion."));
            return;
        }

        int selectedIndex = items.indexOf(item);
        tagsToRemove = {tagsToRemove[selectedIndex]}; // Keep only the selected tag
    }

    // Remove the selected tag(s)
    for (const Tag &tagToRemove : tagsToRemove) {
        tags.removeOne(tagToRemove);
    }

    emit tagsUpdated(tags);
    viewport()->update();

    QMessageBox::information(this, tr("Tag Deleted"),
                             tr("%n %1 tag(s) deleted successfully.", "", tagsToRemove.size()).arg(tagType));
}
void HexEditor::importTags(const QString &tagType)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tags"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for reading:\n%1").arg(file.errorString()));
        return;
    }

    int importedTagsCount = 0;
    int failedTagsCount = 0;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() != 4) {
            failedTagsCount++;
            continue;
        }

        bool ok;
        quint64 offset = parts[0].split(" ").first().toULongLong(&ok);
        if (!ok) {
            failedTagsCount++;
            continue;
        }
        quint64 endOffset = parts[1].split(" ").first().toULongLong(&ok);
        if (!ok) {
            failedTagsCount++;
            continue;
        }

        quint64 length = endOffset - offset + 1;
        QString description = parts[2];
        QString color = parts[3];

        addTag(offset, length, description, QColor(color), tagType);
        importedTagsCount++;
    }

    file.close();
    viewport()->update(); // Refresh the viewport to apply the new tags

    QString message;
    if (importedTagsCount > 0) {
        message = tr("Successfully imported %1 tags.").arg(importedTagsCount);
        if (failedTagsCount > 0) {
            message += tr("\n%1 tags failed to import.").arg(failedTagsCount);
        }
        QMessageBox::information(this, tr("Import Successful"), message);
    } else if (failedTagsCount > 0) {
        message = tr("Failed to import any tags.\n%1 tags were invalid or could not be processed.").arg(failedTagsCount);
        QMessageBox::warning(this, tr("Import Failed"), message);
    } else {
        QMessageBox::information(this, tr("No Tags Imported"), tr("The file did not contain any valid tags to import."));
    }
}
void HexEditor::exportTags(const QString &type)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tags"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export Failed"),
                              tr("Could not open file for writing:\n%1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    int exportedTagsCount = 0;

    for (const Tag &tag : tags) {
        if (tag.type == type) {
            out << tag.offset << " (0x" << QString::number(tag.offset, 16).toUpper() << "),"
                << tag.offset + tag.length - 1 << " (0x" << QString::number(tag.offset + tag.length - 1, 16).toUpper() << "),"
                << tag.description << "," << tag.color << "\n";
            exportedTagsCount++;
        }
    }

    file.close();

    if (exportedTagsCount > 0) {
        QMessageBox::information(this, tr("Export Successful"),
                                 tr("Successfully exported %1 tags to:\n%2")
                                     .arg(exportedTagsCount)
                                     .arg(QDir::toNativeSeparators(fileName)));
    } else {
        QMessageBox::warning(this, tr("No Tags Exported"),
                             tr("No tags of type '%1' were found to export.").arg(type));
    }
}

void HexEditor::exportTagDataByOffset(quint64 offset, const QString &fileName)
{
    // Find the tag by offset
    auto it = std::find_if(tags.begin(), tags.end(), [offset](const Tag &tag) {
        return tag.offset == offset;
    });

    if (it == tags.end()) {
        qDebug() << "Tag not found at offset:" << offset;
        return;
    }

    const Tag &tag = *it;

    // Read the data from the file at the specified offset and length
    QByteArray data;
    if (device->isOpen()) {
        device->seek(tag.offset);
        data = device->read(tag.length);
    } else {
        qDebug() << "File is not open.";
        return;
    }

    // Export the data to a file
    QFile outFile(fileName);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open file for writing:" << outFile.errorString();
        return;
    }

    outFile.write(data);
    outFile.close();

   // qDebug() << "Exported data for tag at offset:" << offset;
}



void HexEditor::search(const QString &pattern, SearchType type)
{
    currentSearchPattern = pattern;
    currentSearchType = type;

    switch (type) {
    case SearchType::Hex:
        searchInHex(QByteArray::fromHex(pattern.toUtf8()));
        break;
    case SearchType::Ascii:
        searchInAscii(pattern);
        break;
    case SearchType::Utf16:
        searchInUtf16(pattern);
        break;
    }
}


void HexEditor::searchInHex(const QByteArray &pattern)
{
    searchResults.clear();

    searchInHexFromPosition(pattern, 0);

    if (!searchResults.isEmpty()) {
        setSelectedByte(searchResults.first().first);
        quint64 searchStart = searchResults.first().first;

        highligtedOffsets.clear();
        for (int i = 0; i < pattern.size(); ++i) {
            highligtedOffsets.insert(searchStart + i);
        }
    }

    viewport()->update();
}

void HexEditor::searchInAscii(const QString &pattern)
{

    searchResults.clear();

    searchInAsciiFromPosition(pattern, 0);

    if (!searchResults.isEmpty()) {
        setSelectedByte(searchResults.first().first);
        quint64 searchStart = searchResults.first().first;

        highligtedOffsets.clear();
        for (int i = 0; i < pattern.size(); ++i) {
            highligtedOffsets.insert(searchStart + i);
        }
    }

    viewport()->update();

}

void HexEditor::searchInUtf16(const QString &pattern)
{
    searchResults.clear();

    searchInUtf16FromPosition(pattern, 0);

    if (!searchResults.isEmpty()) {
        setSelectedByte(searchResults.first().first);
        quint64 searchStart = searchResults.first().first;

        highligtedOffsets.clear();
        for (int i = 0; i < pattern.size(); ++i) {
            highligtedOffsets.insert(searchStart + i);
        }
    }

    viewport()->update();
}


void HexEditor::nextSearch()
{
    if (searchResults.isEmpty())
    {

        return;
    }



    quint64 startPosition = searchResults.last().second + 1;
    searchResults.clear();

    switch (currentSearchType) {
    case SearchType::Hex:
       searchInHexFromPosition(QByteArray::fromHex(currentSearchPattern.toUtf8()), startPosition);
        break;
    case SearchType::Ascii:
        searchInAsciiFromPosition(currentSearchPattern, startPosition);
        break;
    case SearchType::Utf16:
        searchInUtf16FromPosition(currentSearchPattern, startPosition);
        break;
    }

    if (!searchResults.isEmpty()) {
        currentSearchIndex = 0;

        highligtedOffsets.clear();
        quint64 searchStart = searchResults.first().first;


        if(currentSearchType==SearchType::Hex){
           // qDebug() << "pattern Size : " << QByteArray::fromHex(currentSearchPattern.toUtf8()).size();

            for (int i = 0; i < QByteArray::fromHex(currentSearchPattern.toUtf8()).size(); ++i) {
                highligtedOffsets.insert(searchStart + i);
            }
        }else if(currentSearchType==SearchType::Utf16){


            const char16_t *patternUtf16 = reinterpret_cast<const char16_t *>(currentSearchPattern.utf16());
            QByteArray patternBytes(reinterpret_cast<const char *>(patternUtf16), currentSearchPattern.size() * 2);

           // qDebug() << "patternBytes Hex : " << patternBytes;
            for (int i = 0; i < patternBytes.size(); ++i) {
                highligtedOffsets.insert(searchStart + i);
            }

        }

        else{

            QByteArray patternBytes = currentSearchPattern.toUtf8();
           // qDebug() << "patternBytes : " << patternBytes;
            for (int i = 0; i < patternBytes.size(); ++i) {
                highligtedOffsets.insert(searchStart + i);
            }


        }

        setSelectedByte(searchResults.first().first);
    }
    viewport()->update();
}


void HexEditor::searchInHexFromPosition(const QByteArray &pattern, quint64 startPosition)
{
    const int chunkSize = 4096; // Read in chunks of 4 KB
    std::ifstream is(file_name.toStdString(), std::ios::binary);
    if (!is) return;

    is.seekg(startPosition);
    std::vector<char> buffer(chunkSize + pattern.size() - 1);
    quint64 currentPos = startPosition;

    while (is) {
        is.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = is.gcount();
        if (bytesRead < pattern.size()) break;

        auto res = std::search(buffer.begin(), buffer.begin() + bytesRead, pattern.begin(), pattern.end());
        if (res != buffer.begin() + bytesRead) {
            quint64 matchPos = currentPos + std::distance(buffer.begin(), res);
            searchResults.append(qMakePair(matchPos, matchPos + pattern.size() - 1));
            break;
        }

        currentPos += chunkSize;
        is.seekg(currentPos);
    }
}


void HexEditor::searchInAsciiFromPosition(const QString &pattern, quint64 startPosition)
{
    qDebug() << "next searching " << pattern << "from pos" << startPosition;
    QByteArray patternBytes = pattern.toUtf8();
    const int chunkSize = 4096; // Read in chunks of 4 KB
    std::ifstream is(file_name.toStdString(), std::ios::binary);
    if (!is) return;

    is.seekg(startPosition);
    std::vector<char> buffer(chunkSize + patternBytes.size() - 1);
    quint64 currentPos = startPosition;

    while (is) {
        is.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = is.gcount();
        if (bytesRead < patternBytes.size()) break;

        auto res = std::search(buffer.begin(), buffer.begin() + bytesRead, patternBytes.begin(), patternBytes.end());
        if (res != buffer.begin() + bytesRead) {
            quint64 matchPos = currentPos + std::distance(buffer.begin(), res);
            searchResults.append(qMakePair(matchPos, matchPos + patternBytes.size() - 1));
            qDebug() << "searchResults " << searchResults;
            return;
        }

        currentPos += chunkSize;
        is.seekg(currentPos);
    }

    qDebug() << "searchResults " << searchResults;

}

void HexEditor::searchInUtf16FromPosition(const QString &pattern, quint64 startPosition)
{
    const char16_t *patternUtf16 = reinterpret_cast<const char16_t *>(pattern.utf16());
    QByteArray patternBytes(reinterpret_cast<const char *>(patternUtf16), pattern.size() * 2);
    const int chunkSize = 4096; // Read in chunks of 4 KB
    std::ifstream is(file_name.toStdString(), std::ios::binary);
    if (!is) return;

    is.seekg(startPosition);
    std::vector<char> buffer(chunkSize + patternBytes.size() - 1);
    quint64 currentPos = startPosition;

    while (is) {
        is.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = is.gcount();
        if (bytesRead < patternBytes.size()) break;

        auto res = std::search(buffer.begin(), buffer.begin() + bytesRead, patternBytes.begin(), patternBytes.end());
        if (res != buffer.begin() + bytesRead) {
            quint64 matchPos = currentPos + std::distance(buffer.begin(), res);
            searchResults.append(qMakePair(matchPos, matchPos + patternBytes.size() - 1));
            break;
        }

        currentPos += chunkSize;
        is.seekg(currentPos);
    }
}



void  HexEditor::clearSearchResults(){

   // qDebug() << "clear searchResults " ;

    highligtedOffsets.clear();
    searchResults.clear();
    viewport()->update();

}

void HexEditor::syncTagsOnClose(std::function<void(bool)> callback)
{
    if (userTagsHandler) {
        QFuture<void> future = QtConcurrent::run([this]() {
            userTagsHandler->syncTags(tags, currentTabIndex);
        });

        QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
        connect(watcher, &QFutureWatcher<void>::finished, [watcher, callback]() {
            qDebug() << "Tags synced successfully on close.";
            callback(true);
            watcher->deleteLater();
        });

        connect(watcher, &QFutureWatcher<void>::canceled, [watcher, callback]() {
            qDebug() << "Tag syncing was canceled.";
            callback(false);
            watcher->deleteLater();
        });

        watcher->setFuture(future);
    } else {
        callback(false);
    }
}
