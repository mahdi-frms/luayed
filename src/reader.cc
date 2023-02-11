#include "reader.h"

using namespace luayed;

char StringSourceReader::readch()
{
    char c = this->text[this->pointer];
    if (c)
        this->pointer++;
    return c;
}
char StringSourceReader::peekch()
{
    return this->text[this->pointer];
}
StringSourceReader::StringSourceReader(const char *text) : text(text), pointer(0)
{
}
