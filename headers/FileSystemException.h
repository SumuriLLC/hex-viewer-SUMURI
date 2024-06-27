#ifndef FILESYSTEMEXCEPTION_H
#define FILESYSTEMEXCEPTION_H

#include <stdexcept>
#include <QString>

class FileSystemException : public std::runtime_error
{
public:
    explicit FileSystemException(const QString& message)
        : std::runtime_error(message.toStdString()), m_message(message) {}

    QString getMessage() const { return m_message; }

private:
    QString m_message;
};

#endif // FILESYSTEMEXCEPTION_H
