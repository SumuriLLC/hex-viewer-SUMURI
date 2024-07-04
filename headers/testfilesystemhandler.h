#ifndef TESTFILESYSTEMHANDLER_H
#define TESTFILESYSTEMHANDLER_H

#include "filesystemhandler.h"
#include "tagshandler.h"

class TestFileSystemHandler
{
public:
    TestFileSystemHandler();
    void runTests();

private:
    void testOpenImage(const QString& fileName);
    void testAddTag();
    void testGetTags();
    void testUpdateTag();
    void testDeleteTag();

    FileSystemHandler *fsHandler;
    TagsHandler *tagsHandler;
};

#endif // TESTFILESYSTEMHANDLER_H
