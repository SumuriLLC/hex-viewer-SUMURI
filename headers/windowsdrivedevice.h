#ifndef WINDOWSDRIVEDEVICE_H
#define WINDOWSDRIVEDEVICE_H
#include <windows.h>


#include <QIODevice>

class WindowsDriveDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit WindowsDriveDevice(const QString &drivePath, QObject *parent = nullptr);
    ~WindowsDriveDevice();

    qint64 size() const override;


protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    HANDLE hDevice;
    qint64 m_fileSize;
    QByteArray buffer;
    qint64 bufferStartPos;
    qint64 bufferEndPos;

    bool fillBuffer(qint64 position);
};

#endif // WINDOWSDRIVEDEVICE_H
