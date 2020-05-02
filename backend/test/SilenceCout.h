#ifndef SILENCE_COUT_H
#define SILENCE_COUT_H

#include <iostream>

class SilenceCout
{
public:
    SilenceCout(std::ostream& stream = std::cout) : s(stream), p(stream.rdbuf(nullptr)) {}
    ~SilenceCout()
    {
        s.rdbuf(p);
        // clear error flags
        s.clear();
    }

private:
    std::ostream& s;
    std::streambuf* p = nullptr;
};

#endif