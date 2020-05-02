#ifndef LINUX_FILE_CONTROL_H_INCLUDED
#define LINUX_FILE_CONTROL_H_INCLUDED

#include <fcntl.h> // open
#include <sys/ioctl.h> // ioctl
#include <unistd.h> // close

#include "FileControl.h"

class LinuxFileControl : public FileControl
{
public:
    int open(const char* filename, int flags) const override { return ::open(filename, flags); }; //, mode_t mode);
    int close(int filedes) const override { return ::close(filedes); };
    int ioctl(int fd, unsigned long request, void* argp) const override { return ::ioctl(fd, request, argp); };
};

#endif
