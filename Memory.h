/*
MIT License

Copyright (c) 2022 Tony D

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef _MEMORY_H
#define _MEMORY_H

#define MAX_MEMORY Megabytes(10)
#define MIN_MEMORY Kilobytes(500)

#define ToKilobytes(Value) ((Value) / 1024)
#define ToMegabytes(Value) (ToKilobytes(Value) / 1024)


enum FILETYPE 
{
    F_AUDIO,
    F_IMAGE,
    F_TEXT,
    F_OTHER
};

struct fileData
{
    // files that are not images / audio
    int32 size;
    char extension[12];
    FILETYPE type;
    uint8* baseData;
};

struct memoryBlock
{
    Color color;
    bool32 selected;
    char string[20];
    real32 stringWidth;
    Vector2 stringPos;
    Rectangle stringLine;
    Rectangle rect; // rectangle that visually represents it
    fileData* data; // the data
};

struct memory_arena
{
	m_index Size;
	uint8* Base;
	m_index Used;
};

#endif //_MEMORY_H
