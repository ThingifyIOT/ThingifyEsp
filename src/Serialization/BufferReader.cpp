#include "BufferReader.h"

BufferReader::BufferReader(const char *buffer, int length):
_buffer(buffer),
_length(length),
_postion(0)
{
    
}

const char* BufferReader::ReadBuffer(int length)
{
    if (_postion + length > _length)
    {
        return nullptr;
    }
    const char* ret = _buffer + _postion;
    _postion += length;
    return ret;
}