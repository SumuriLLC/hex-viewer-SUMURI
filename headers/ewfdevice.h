#ifndef EWFDEVICE_H
#define EWFDEVICE_H

#include <QIODevice>
#include <libewf.h>

class EwfDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit EwfDevice(QObject *parent = nullptr);
    bool openEwf(const char *ewfFilePath, OpenMode mode);
    ~EwfDevice();

    qint64 size() const override;


protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    libewf_handle_t *m_ewfHandle;
    size64_t m_mediaSize;
    size64_t m_fileSize;

    libewf_error_t *m_error;
};

#endif // EWFDEVICE_H
