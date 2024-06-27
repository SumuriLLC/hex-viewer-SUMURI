#ifndef TESTFILESYSTEMHANDLER_H
#define TESTFILESYSTEMHANDLER_H

#include "filesystemhandler.h"

class TestFileSystemHandler
{
public:
    TestFileSystemHandler();
    void runTests();

private:
    void testOpenImage(const QString& fileName);
    FileSystemHandler* fsHandler;
};

#endif // TESTFILESYSTEMHANDLER_H
