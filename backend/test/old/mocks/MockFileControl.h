#ifndef MOCK_FILE_CONTROL_H
#define MOCK_FILE_CONTROL_H

#include <gmock/gmock.h>

#include "utility/FileControl.h"

class MockFileControl : public FileControl
{
public:
    MOCK_CONST_METHOD2(open, int(const char* filename, int flags));
    MOCK_CONST_METHOD1(close, int(int filedes));
    MOCK_CONST_METHOD3(ioctl, int(int fd, unsigned long request, void* argp));
};

#endif
