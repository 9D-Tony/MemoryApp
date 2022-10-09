#ifndef MEMORY_TEST_APP
#define MEMORY_TEST_APP

struct programState {
    int FPSTarget;
    char windowName[64];
    int32 screenWidth;
    int32 screenHeight;
    bool hasMemAllocated;
    void* memoryBase;
    int32 size;
    int32 totalUsed;
};

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
    FILETYPE type;
    uint8* baseData;
};

struct memoryBlock
{
    Color color;
    fileData* data; // the data
    Rectangle rect; // rectangle that visually represents it
};

struct memory_arena
{
	memory_index Size;
	uint8* Base;
	memory_index Used;
};

char supportedTxtFiles[] = { 'txt'};
char supportedAudioFiles[] = { 'wav','mp3','ogg', 'flac'};
char supportedImagefiles[] = { 'gif', 'png', 'jpg'};

internal void AllocateBaseMemory(programState& data, memory_arena *arena, int32 memSize)
{
    //really want this as array of memory_arenas
    data.memoryBase = Win32VirtualAlloc(memSize);
    
    arena->Size = memSize;
	arena->Base = (uint8*)data.memoryBase;
	arena->Used = 0;
    
    ZeroMemory(data.memoryBase,memSize);
    data.size = memSize;
    data.totalUsed = 0;
}


internal
void Copy(void* source,uint32 size, void* destination)
{
	uint8 *start = (uint8*)source;
	uint8 *dest = (uint8*)destination;
    
	while (size--)
	{
		*start++ = *dest++;
	}
}

#define pushStruct(Arena, type) (type *)pushSize_(Arena,sizeof(type))
#define pushArray(Arena, Count, type) (type *)pushSize_(Arena,(Count) * sizeof(type))
void* pushSize_(memory_arena *Arena, memory_index size) // the size can be a type then we get the size of the type to push
{
	assert(Arena->Used + size < Arena->Size);
	void *Result = Arena->Base + Arena->Used;
	Arena->Used += size;
	return (Result);
}

internal void arena_align(memory_arena *arena, memory_index boundary)
{
	memory_index p =(arena->Used + (boundary - 1));
	arena->Used = p - p % boundary;
}

internal Color GetRandomColor()
{
    uint32 randomNum = rand()/255;
    uint32 randomNum1 = rand()/255;
    uint32 randomNum2 = rand()/255;
    
    Color blockColor = {randomNum,randomNum1,randomNum2, 255};
    return blockColor;
}

internal inline Rectangle SetMemoryBlockPos(Rectangle baseMemoryRect, memory_arena& programMemory, uint32 beforeUsedMemory)
{
    Rectangle Result = {};
    //(NOTE): to get starting point we need to get memory_arena before allocation happened
    //calculate position based on memory
    // memory range 0 - 128MB
    // memory rentangle range X - x + 950
    //memory_arena.Used
    //memory_arena.Size
    real32 oldRange = (real32)programMemory.Size;
    real32 newRange = (real32)(baseMemoryRect.width - baseMemoryRect.x);
    Result.height = baseMemoryRect.height / 1.1;
    
    Result.y = baseMemoryRect.y + (baseMemoryRect.height / 16);
    Result.x = beforeUsedMemory * newRange / oldRange + baseMemoryRect.x;
    
    Result.width = ((real32)programMemory.Used * newRange / oldRange + baseMemoryRect.x) - Result.x; 
    
    
    
    return Result;
}

internal fileData* LoadDataIntoMemory(memory_arena& programMemory, char* filename)
{
    // NOTE: can strlen fail here?
    //fileData 
    fileData* fileResult = {};
    fileInfo fileLoadResult = DebugWin32LoadFile(filename);
    
    //Error could not get file
    if(fileLoadResult.data == NULL) return NULL;
    int32 filenameLength = strlen(filename);
    
    int32 foundChar = strcspn(filename, ".");
    char extensionString[12]; // most extensions will be < 4
    
    // get  memory for the file
    fileResult = pushStruct(&programMemory,fileData);
    fileResult->size = fileLoadResult.size;
    fileResult->baseData = pushArray(&programMemory,fileLoadResult.size,uint8);
    
    //works if the filename only has one "." in it,  
    //(TODO): search from the back of the string to get the extension.
    if(foundChar != filenameLength)
    {
        // found the "."
        int32 extensionLength = filenameLength - foundChar;
        strcpy_s(extensionString, filename + foundChar);
        printf("extension is: %s \n",extensionString);
        
        if(strcmp(extensionString, ".txt") == 0)
        {
            fileResult->type = TEXT;
            printf("file is a text file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
        }
    }
    
    Win32VirtualFree(fileLoadResult.data);
    return(fileResult);
}

internal void setMemoryData(memory_arena& programMemory,int32 size, void* data,FILETYPE type)
{
    //grab potion of memory and set it to a certain struct
    //data.totalUsed = data.totalUsed + size;
    
    //find the type, load and copy data into memory
    
    fileData* tempFile = {};
    tempFile = pushStruct(&programMemory,fileData);
    
    switch (type)
    {
        case TEXT:
        uint8* stringData = (uint8*)data; 
        
        tempFile->size = size;
        tempFile->type = TEXT;
        
        tempFile->baseData = pushArray(&programMemory,size,uint8);
        Copy(tempFile->baseData,size,stringData);
        
        break;
        
        /*case AUDIO:
        break;
        
        case IMAGE:
        break;
        
        case OTHER:
        break;*/
    }
}

internal void ClearMemory(memory_arena *Arena, void *baseAddress,  memory_index size)
{
    assert(size < Arena->Used);
    ZeroMemory(baseAddress,size);
    Arena->Used -= size;
}

internal void FreeBase(programState& data)
{
    Win32VirtualFree(data.memoryBase);
    data.size = 0;
    data.totalUsed = 0;
}

#endif