#include "headers/windowsdrivedevice.h"
#include <QDebug>
#include <QFileInfo>

WindowsDriveDevice::WindowsDriveDevice(const QString &drivePath, QObject *parent)
    : QIODevice(parent), hDevice(INVALID_HANDLE_VALUE), m_fileSize(0), bufferStartPos(0), bufferEndPos(0)
{
    hDevice = CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        qCritical() << "Failed to open handle to physical drive.";
        return;
    }

    DISK_GEOMETRY_EX diskGeometry;
    DWORD bytesReturned;
    if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL)) {
        m_fileSize = diskGeometry.DiskSize.QuadPart;
    } else {
        qCritical() << "Failed to get physical drive size.";
        CloseHandle(hDevice);
        hDevice = INVALID_HANDLE_VALUE;
    }

    buffer.resize(1024); // Set buffer size to 1024 bytes
}

qint64 WindowsDriveDevice::size() const
{
    return m_fileSize;
}

WindowsDriveDevice::~WindowsDriveDevice()
{
    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
    }
}

qint64 WindowsDriveDevice::readData(char *data, qint64 maxlen)
{
    qint64 pos = QIODevice::pos(); // Get the current position
    qint64 bytesRead = 0;

    //qDebug() << "Reading " << maxlen << " bytes from position " << pos;

    while (maxlen > 0) {
        // If the position is outside the current buffer, refill the buffer
        if (pos < bufferStartPos || pos >= bufferEndPos) {
            if (!fillBuffer(pos)) {
                break; // If buffer fill fails, break the loop
            }
        }

        // Calculate how many bytes can be read from the current buffer
        qint64 bufferOffset = pos - bufferStartPos;
        qint64 bytesToRead = qMin(maxlen, bufferEndPos - pos);

        if (bytesToRead <= 0) break; // Prevent infinite loop if bytesToRead is not positive

        memcpy(data + bytesRead, buffer.constData() + bufferOffset, bytesToRead);
        bytesRead += bytesToRead;
        maxlen -= bytesToRead;
    }

    //qDebug() << "Read " << bytesRead << " bytes from position " << pos;

    return bytesRead;
}

bool WindowsDriveDevice::fillBuffer(qint64 position)
{
    qint64 alignedPosition = (position / 512) * 512;

    qDebug() << "filling Buffer  from " << position;


    LARGE_INTEGER li;
    li.QuadPart = alignedPosition;
    if (!SetFilePointerEx(hDevice, li, NULL, FILE_BEGIN)) {
        qCritical() << "Failed to seek to position" << alignedPosition;
        return false;
    }

    DWORD bytesRead;
    if (!ReadFile(hDevice, buffer.data(), buffer.size(), &bytesRead, NULL) || bytesRead == 0) {
        qCritical() << "Failed to read from the drive at position" << alignedPosition;
        return false;
    }

    bufferStartPos = alignedPosition;
    bufferEndPos = bufferStartPos + bytesRead;

    //qDebug() << "Buffer filled from " << bufferStartPos << " to " << bufferEndPos;

    return true;
}

qint64 WindowsDriveDevice::writeData(const char *data, qint64 len)
{
    // Writing is not supported for this device.
    Q_UNUSED(data);
    Q_UNUSED(len);
    return -1;
}
