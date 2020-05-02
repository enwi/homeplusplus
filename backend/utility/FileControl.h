#ifndef FILE_CONTROL_H_INCLUDED
#define FILE_CONTROL_H_INCLUDED

class FileControl
{
public:
    virtual int open(const char* filename, int flags) const = 0; //, mode_t mode);
    virtual int close(int filedes) const = 0;
    virtual int ioctl(int fd, unsigned long request, void* argp) const = 0;
    virtual ~FileControl() = default;
};

#endif
