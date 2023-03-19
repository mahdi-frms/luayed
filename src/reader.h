#ifndef READER_H
#define READER_H

#include "virtuals.h"

namespace luayed
{

    class ISourceReader
    {
    public:
        virtual char readch() = 0;
        virtual char peekch() = 0;
    };

    class StringSourceReader : public ISourceReader
    {
    private:
        const char *text;
        size_t pointer;

    public:
        char readch();
        char peekch();
        StringSourceReader(const char *text);
    };
};

#endif