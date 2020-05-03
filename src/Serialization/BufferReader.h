#ifndef _BUFFER_READER_H
#define _BUFFER_READER_H

class BufferReader
{
	const char* _buffer;
	int _length;
	int _postion;
public:
	BufferReader(const char *buffer, int length);
	const char *ReadBuffer(int length);
};

#endif