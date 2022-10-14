#ifndef MEMORY_TEST_APP
#define MEMORY_TEST_APP

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
    bool32 selected;
    char string[20];
    real32 stringWidth;
    Vector2 stringPos;
    Rectangle stringLine;
    Rectangle rect; // rectangle that visually represents it
    fileData* data; // the data
};

struct programState {
    int32 FPSTarget;
    char windowName[64];
    
    int32 screenWidth;
    int32 screenHeight;
    int32 pageSize;
    int32 size;
    int32 totalUsed;
    int32 sliderValue;
    
    memoryBlock* selectedBlock;
    Color blockLastColor;
    Texture2D globalTex;
    Sound globalSound;
    
    bool32 hasMemAllocated;
    void* memoryBase;
};

struct memory_arena
{
	memory_index Size;
	uint8* Base;
	memory_index Used;
};

char supportedTxtFiles[5][8] = { ".txt", ".blah"};
char supportedAudioFiles[5][8] = { ".wav",".mp3",".ogg", ".flac"};
char supportedImageFiles[5][8] = { ".gif", ".png", ".jpg"};

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

internal Vector2 SetPos(real32 x, real32 y)
{
    Vector2 resultPos = {x,y};
    return resultPos;
}

internal programState SetProgramState(int32 screenWidth, int32 screenHeight, int32 FPSTarget, int32 pageSize)
{
    programState programStateResult  = {};
    programStateResult.screenWidth = screenWidth;
    programStateResult.screenHeight = screenHeight;
    programStateResult.FPSTarget = FPSTarget;
    programStateResult.hasMemAllocated = false;
    programStateResult.sliderValue = 0;
    programStateResult. pageSize = pageSize;
    return (programStateResult);
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

inline const char* EnumToChar(fileData* fileStruct)
{
    switch(fileStruct->type)
    {
        case AUDIO:
        return "Audio";
        break;
        
        case IMAGE:
        return "Image";
        break;
        
        case TEXT:
        return  "Text";
        break;
        
        case OTHER:
        return "Other";
        break;
        
        default: 
        return "Other";
        break;
    }
}

internal real32 GetTextWidth(char* string, uint32 fontSize)
{
    return MeasureTextEx(GetFontDefault(),string,fontSize,1).x;
}

internal inline uint32 ToPageSize(uint32 input, uint32 pageSize)
{
    uint32 remainder = input % pageSize;
    return input - remainder;
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
    //NOTE: to get starting point we need to get memory_arena before allocation happened
    //calculate position based on memory
    
    real32 oldRange = (real32)programMemory.Size;
    real32 newRange = (real32)(baseMemoryRect.width - baseMemoryRect.x);
    Result.height = baseMemoryRect.height / 1.1;
    
    Result.y = baseMemoryRect.y + (baseMemoryRect.height / 16);
    Result.x = beforeUsedMemory * newRange / oldRange + baseMemoryRect.x;
    
    Result.width = ((real32)programMemory.Used * newRange / oldRange + baseMemoryRect.x) - Result.x; 
    
    return Result;
}

internal Sound LoadSoundFrmMemory(Sound audio, char* filetype, uint8* data, int32 size)
{
    // Load audio and return it to play it
    //TODO: impliment audio loading and returning Sound
    Sound sound = {};
    
    //audio = LoadImageFromMemory(filetype, data, size);
    return sound;
}

internal Texture2D LoadImageFrmMemory(Image inputImage, char* filetype, uint8* data, int32 size)
{
    Texture2D texture = {};
    inputImage = LoadImageFromMemory(filetype, data, size);
    texture = LoadTextureFromImage(inputImage);
    
    // if texture can't load for some reason
    if(texture.id == NULL)
    {
        return texture;
    }
    
    UnloadImage(inputImage);
    return (texture);
}

internal inline bool32 CheckIfExtension(char* extensionArray,uint32 arraySize,char* extension)
{
    for(int i=0; i < arraySize; i++)
    {
        if(strcmp(extensionArray + (i * 8), extension) == 0)
        {
            return true;
        }
    }
    
    return false;
}

internal memoryBlock SetMemoryBlock(memoryBlock block,Rectangle baseMemoryRect,memory_arena programMemory, fileData* filePtr)
{
    memoryBlock resultBlock = {};
    resultBlock.data = filePtr; 
    
    int32 beforeMemory = programMemory.Used - filePtr->size;
    resultBlock.rect = SetMemoryBlockPos(baseMemoryRect,programMemory, beforeMemory);
    
    srand(programMemory.Used);
    resultBlock.color = GetRandomColor();
    
    strcpy_s(resultBlock.string,EnumToChar(resultBlock.data));
    
    //NOTE: maybe standardise string sizes
    resultBlock.stringWidth = GetTextWidth(resultBlock.string,20);
    
    return resultBlock;
    
}

internal fileData* LoadFileIntoMemory(memory_arena& programMemory, char* filename)
{
    // NOTE: can strlen fail here?
    //fileData 
    fileData* fileResult = {};
    fileInfo fileLoadResult = Win32LoadFile(filename);
    
    //Error could not get file
    if(fileLoadResult.data == NULL || fileLoadResult.size > (programMemory.Size - programMemory.Used))
    {
        return NULL;
    }
    
    int32 filenameLength = strlen(filename);
    int32 foundChar = strcspn(filename, ".");
    
    char extensionString[12]; // most extensions will be < 4
    
    // get memory for the file
    fileResult = pushStruct(&programMemory,fileData);
    fileResult->size = sizeof(fileData) + fileLoadResult.size;
    fileResult->baseData = pushArray(&programMemory,fileLoadResult.size,uint8);
    
    //works if the filename only has one "." in it,  
    //TODO: search from the back of the string to get the extension.
    if(foundChar != filenameLength)
    {
        int32 extensionLength = filenameLength - foundChar;
        strcpy_s(extensionString, filename + foundChar);
        printf("extension is: %s \n",extensionString);
        
        //TODO: BLECH, what is this?!
        if(strcmp(extensionString, ".txt") == 0)
        {
            fileResult->type = TEXT;
            printf("file is a text file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else if(CheckIfExtension((char*)supportedAudioFiles, ArrayCount(supportedAudioFiles), extensionString))
        {
            fileResult->type = AUDIO;
            printf("file is a text file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else if(CheckIfExtension((char*)supportedImageFiles, ArrayCount(supportedAudioFiles), extensionString))
        {
            fileResult->type = IMAGE;
            printf("file is a text file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else
        {
            fileResult->type = OTHER;
            printf("file is a text file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
        }
    }
    
    Win32VirtualFree(fileLoadResult.data);
    return(fileResult);
}

internal inline uint32 numDigits(const uint32 n) {
    if (n < 10) return 1;
    return 1 + numDigits(n / 10);
}

internal char* IntToChar(char* buffer, int32 input,const char* extraString)
{
    // sprintf is slow find better way
    uint32 totalDigits = numDigits(input);
    uint32 extraLength = strlen(extraString);
    
    sprintf_s(buffer,totalDigits + extraLength + 2, "%d %s", input, extraString);
    return buffer;
}

internal void ClearMemory(memory_arena *Arena, void *baseAddress, memory_index size)
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