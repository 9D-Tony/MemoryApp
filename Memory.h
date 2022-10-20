#ifndef _MEMORY_H
#define _MEMORY_H

#define MAX_MEMORY Megabytes(10)
#define MIN_MEMORY Kilobytes(500)

#define ToKilobytes(Value) ((Value) / 1024)
#define ToMegabytes(Value) (ToKilobytes(Value) / 1024)


enum FILETYPE 
{
    AUDIO,
    IMAGE,
    TEXT,
    OTHER
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
	memory_index Size;
	uint8* Base;
	memory_index Used;
};


#endif //_MEMORY_H
