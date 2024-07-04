#include "headers/ewfdevice.h"
#include <QDebug>
#include <QFileInfo>
EwfDevice::EwfDevice(QObject *parent)
    : QIODevice(parent), m_ewfHandle(nullptr), m_mediaSize(0), m_fileSize(0), m_error(nullptr)
{
}

bool EwfDevice::openEwf(const char *ewfFilePath, OpenMode mode)
{
    const char *filenames[] = { ewfFilePath, nullptr };

    if (libewf_handle_initialize(&m_ewfHandle, &m_error) == 0) {
        libewf_error_fprint(m_error, stderr);
        libewf_error_free(&m_error);
        return false;
    }

    if (libewf_handle_open(m_ewfHandle, const_cast<char **>(filenames), 1, LIBEWF_OPEN_READ, &m_error) == 0) {
        libewf_error_fprint(m_error, stderr);
        libewf_error_free(&m_error);
        libewf_handle_free(&m_ewfHandle, nullptr);
        m_ewfHandle = nullptr;
        return false;
    }

    if (libewf_handle_get_media_size(m_ewfHandle, &m_mediaSize, &m_error) == 0) {
        libewf_error_fprint(m_error, stderr);
        libewf_error_free(&m_error);
    }
                QFileInfo fileInfo(ewfFilePath);
                m_fileSize = fileInfo.size();
                qDebug() << "Media size is " << m_mediaSize;

    return QIODevice::open(mode);
}

qint64 EwfDevice::size() const
{
    return m_fileSize;
}

EwfDevice::~EwfDevice()
{
    if (m_ewfHandle) {
        libewf_handle_close(m_ewfHandle, nullptr);
        libewf_handle_free(&m_ewfHandle, nullptr);
        m_ewfHandle = nullptr;
    }
    QIODevice::close();

}

qint64 EwfDevice::readData(char *data, qint64 maxlen)
{
    qint64 pos = QIODevice::pos(); // Get the current position
    //qDebug() << "read start..";
    //qDebug() << "current pos: " << pos << ", maxlen: " << maxlen;

    if (maxlen <= 0 || pos >= m_mediaSize) {
       // qDebug() << "read error: length " << maxlen << " " << m_mediaSize;
        return -1;
    }

    if (libewf_handle_seek_offset(m_ewfHandle, pos, SEEK_SET, &m_error) == -1) {
        //qDebug() << "read error: seek failed with error";
        libewf_error_fprint(m_error, stderr);
        libewf_error_free(&m_error);
        return -1;
    }

    qint64 bytesToRead = qMin(static_cast<qint64>(maxlen), static_cast<qint64>(m_mediaSize - pos));
    //qDebug() << "Reading " << bytesToRead << " bytes";

    int read_count = libewf_handle_read_buffer(m_ewfHandle, reinterpret_cast<uint8_t *>(data), bytesToRead, &m_error);
    if (read_count < 0) {
       // qDebug() << "read error: read buffer failed with error";
        libewf_error_fprint(m_error, stderr);
        libewf_error_free(&m_error);
        return -1;
    }

    //qDebug() << "read complete, new pos: " << QIODevice::pos() + read_count;
    return read_count;
}

qint64 EwfDevice::writeData(const char *data, qint64 len)
{
    // Writing is not supported for this device.
    Q_UNUSED(data);
    Q_UNUSED(len);
    return -1;
}
