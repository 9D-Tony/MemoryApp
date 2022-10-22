#include "Memory.h"
#include "Memory.cpp"


// TODO: REMEBER TO TAKE THESE OUT AND FIX WARNINGS
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#pragma warning(disable : 4018)

// Memory blocks are the assigned blocks that go within the baseMemoryRect.
struct programState {
    char windowName[64];
    int32 FPSTarget;
    
    int32 screenWidth;
    int32 screenHeight;
    int32 pageSize;
    int32 size;
    int32 totalUsed;
    real32 sliderValue;
    
    memoryBlock* selectedBlock;
    Color blockLastColor;
    Texture2D globalTex;
    Sound globalSound;
    Font defaultFont;
    bool32 hasMemAllocated;
    
    void* memoryBase;
};

char supportedTxtFiles[5][8] = { ".txt", ".blah"};
char supportedAudioFiles[5][8] = { ".wav",".mp3",".ogg"}; // have to rebuild for flac support
char supportedImageFiles[6][8] = { ".gif", ".png", ".jpg", ".JPG", ".PNG",".GIF"};

// standard text sizes
#define titleSize 30
#define buttonTxtSize 25
#define blockTxtSize 20

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

internal Rectangle SetRect(real32 x,real32 y,real32 width,real32 height)
{
    return Rectangle {x,y,width,height};
}

internal programState SetProgramState(int32 screenWidth, int32 screenHeight, int32 FPSTarget, int32 pageSize)
{
    programState programStateResult  = {};
    programStateResult.screenWidth = screenWidth;
    programStateResult.screenHeight = screenHeight;
    programStateResult.FPSTarget = FPSTarget;
    programStateResult.hasMemAllocated = false;
    programStateResult.sliderValue = 0;
    programStateResult.pageSize = pageSize;
    programStateResult.sliderValue = (real32)(MAX_MEMORY / 2);
    return (programStateResult);
}

inline const char* EnumToChar(fileData* fileStruct)
{
    switch(fileStruct->type)
    {
        case F_AUDIO:
        return "Audio";
        break;
        
        case F_IMAGE:
        return "Image";
        break;
        
        case F_TEXT:
        return  "Text";
        break;
        
        case F_OTHER:
        return "Other";
        break;
        
        default: 
        return "Other";
        break;
    }
}

internal void StopAudio(programState* programData)
{
    if(programData->globalSound.frameCount > 0)
    {
        StopSound(programData->globalSound);
    }
}

internal real32 GetTextWidth(char* string, real32 fontSize)
{
    return MeasureTextEx(GetFontDefault(),string,fontSize,1).x;
}

internal inline Rectangle SetMemoryBlockPos(Rectangle baseMemoryRect, memory_arena& programMemory, uint32 beforeUsedMemory)
{
    Rectangle Result = {};
    //NOTE: to get starting point we need to get memory_arena before allocation happened
    //calculate position based on memory
    
    real32 oldRange = (real32)programMemory.Size;
    
    real32 newRange = (real32)baseMemoryRect.width;
    Result.height = baseMemoryRect.height / 1.1f;
    
    Result.y = baseMemoryRect.y + (baseMemoryRect.height / 16);
    Result.x = beforeUsedMemory * newRange / oldRange + baseMemoryRect.x;
    
    Result.width = ((real32)programMemory.Used * newRange / oldRange + baseMemoryRect.x) - Result.x; 
    
    return Result;
}

internal void DrawButton(Rectangle buttonRect, const char* text,int32 textSize, Color color)
{
    //Draw a rectangle with text in the middle.
    DrawRectangleRec(buttonRect,color);
    Vector2 textDim =  MeasureTextEx(GetFontDefault() ,text, textSize, 1); 
    
    Vector2 textPos = GetRectCenter(buttonRect); 
    DrawText(text, (int32)(textPos.x - (textDim.x / 2)), (int32)(textPos.y - (textDim.y / 2)), textSize, WHITE);
}

internal memoryBlock SetMemoryBlock(memoryBlock block,Rectangle baseMemoryRect,memory_arena programMemory, fileData* filePtr)
{
    memoryBlock resultBlock = {};
    resultBlock.data = filePtr; 
    
    int32 beforeMemory = programMemory.Used - filePtr->size;
    resultBlock.rect = SetMemoryBlockPos(baseMemoryRect,programMemory, beforeMemory);
    
    srand((uint32)programMemory.Used);
    resultBlock.color = GetRandomColor();
    
    strcpy_s(resultBlock.string,EnumToChar(resultBlock.data));
    
    //NOTE: maybe standardise string sizes
    resultBlock.stringWidth = GetTextWidth(resultBlock.string,20);
    
    return resultBlock;
}

internal Sound LoadSoundFromMemory(fileData* data, programState* programData)
{
    Sound sound = {};
    Wave wave = {};
    
    //NOTE: maybe consider a ring buffer for audio and images
    //unload the other sound from memory first
    if(programData->globalSound.frameCount > 0)
    {
        UnloadSound(programData->globalSound);
    }
    
    wave = LoadWaveFromMemory(data->extension, data->baseData, data->size - sizeof(fileData));
    
    if(wave.frameCount == 0)
    {
        printf("could not load file with extension: %s\n", data->extension);
        return sound;
    }
    
    //loading sound causes it's own allocation in raylib.dll
    sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    
    return sound;
}

internal Texture2D LoadImageFrmMemory(fileData* data, programState* programData)
{
    //NOTE: For the moment cannot load images that have extensions in all caps.
    Texture2D texture = {};
    
    if(programData->globalTex.id > 0)
    {
        UnloadTexture(programData->globalTex);
    }
    
    Image inputImage = LoadImageFromMemory(data->extension, data->baseData, data->size - sizeof(fileData));
    texture = LoadTextureFromImage(inputImage);
    
    // if texture can't load for some reason
    if(texture.id == NULL)
    {
        printf("can't load image\n");
        return texture;
    }
    
    UnloadImage(inputImage);
    
    return (texture);
}

internal inline bool32 CheckIfExtension(char* extensionArray,int32 arraySize, char* extension)
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

internal void ClearTexture(programState* programData)
{
    Texture texture = {};
    UnloadTexture(programData->globalTex);
    programData->globalTex = texture;
}

internal void MemoryblocksMouseIO(uint32 index, memoryBlock* memoryBlocks, Vector2 mousePos, programState* programData)
{
    Rectangle memoryRect = memoryBlocks[index].rect;
    
    // colliding with a memory rect
    if (CheckCollisionPointRec(mousePos, memoryRect) && memoryRect.width > 5)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            // reset last block to original color
            if(programData->selectedBlock != NULL)
            {
                programData->selectedBlock->color = programData->blockLastColor; 
            }
            
            programData->selectedBlock = &memoryBlocks[index]; 
            programData->blockLastColor = memoryBlocks[index].color;
            memoryBlocks[index].color = GRAY;
            
            // get file entension and load image/audio
            fileData* memoryData = memoryBlocks[index].data;
            
            // Actions for each filetype
            if(CheckIfExtension((char*)supportedImageFiles,ArrayCount(supportedImageFiles), memoryData->extension))
            {
                // stop audio if clicking on image
                StopAudio(programData);
                programData->globalTex = LoadImageFrmMemory(memoryData,programData); 
            }
            
            if(CheckIfExtension((char*)supportedAudioFiles,ArrayCount(supportedAudioFiles), memoryData->extension))
            {
                ClearTexture(programData);
                
                StopAudio(programData);
                
                programData->globalSound = LoadSoundFromMemory(memoryData,programData);
                
                PlaySound(programData->globalSound);
            }
        }
    }
}

internal fileData* LoadFileIntoMemory(memory_arena& programMemory, char* filename)
{
    // NOTE: can strlen fail here?
    //fileData 
    fileData* fileResult = {};
    fileInfo fileLoadResult = Win32LoadFile(filename);
    
    //Error could not fit file inside memory
    if(fileLoadResult.data == NULL || fileLoadResult.size > (programMemory.Size - programMemory.Used))
    {
        printf("File could not fit inside avaliable memory\n");
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
        strcpy_s(fileResult->extension, extensionString);
        
        //NOTE:  doesn't support UTF-8
        //TODO: BLECH, what is this?!
        if(strcmp(extensionString, ".txt") == 0)
        {
            fileResult->type = F_TEXT;
            printf("file is a text file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else if(CheckIfExtension((char*)supportedAudioFiles,ArrayCount(supportedImageFiles), extensionString))
        {
            fileResult->type = F_AUDIO;
            printf("file is an audio file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else if(CheckIfExtension((char*)supportedImageFiles,ArrayCount(supportedImageFiles), extensionString))
        {
            fileResult->type = F_IMAGE;
            printf("file is an image file with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else
        {
            fileResult->type = F_OTHER;
            printf("file is other filetype  with size: %d\n", fileLoadResult.size);
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
        }
    }
    
    Win32VirtualFree(fileLoadResult.data);
    return(fileResult);
}

internal void SetDroppedFiles(memoryBlock* memoryBlocks,uint32 blocksAssigned)
{
    // if we managed to  load the data, if null then could not load data
    
    //get string width
    Rectangle memoryRect = memoryBlocks[blocksAssigned].rect;
    
    int32 middleBlock = (int32)(memoryRect.x + (memoryRect.width / 2));
    
    // if it can fit in the memory block
    if(memoryBlocks[blocksAssigned].stringWidth < memoryRect.width)
    {
        memoryBlocks[blocksAssigned].stringPos = SetPos(middleBlock - (memoryBlocks[blocksAssigned].stringWidth / 2), memoryRect.y + (memoryRect.height / 2));
    }
    else
    {
        real32 textOffset = 40.0;
        real32 memoryTextY = memoryRect.y - textOffset; 
        //render a rect here pointing to the middle of the block
        
        memoryBlocks[blocksAssigned].stringPos = SetPos(middleBlock - (memoryBlocks[blocksAssigned].stringWidth / 2) , memoryTextY);
        
        if(blocksAssigned > 0)
        {
            memoryBlock curStringBlock = memoryBlocks[blocksAssigned];
            memoryBlock lastStringBlock = memoryBlocks[blocksAssigned - 1];
            real32 YDistance = curStringBlock.stringPos.y -  lastStringBlock.stringPos.y;
            printf("Distance Y: %f \n", YDistance);
            
            //TODO: collides with text rendered in middle of memory block 
            if(lastStringBlock.stringPos.x  > curStringBlock.stringPos.x ||
               lastStringBlock.stringPos.x + lastStringBlock.stringWidth > curStringBlock.stringPos.x &&
               lastStringBlock.stringPos.x + lastStringBlock.stringWidth < 
               curStringBlock.stringPos.x + curStringBlock.stringWidth)
            {
                memoryBlocks[blocksAssigned].stringPos = SetPos(middleBlock - (curStringBlock.stringWidth / 2) ,  (lastStringBlock.rect.y - textOffset - 24));
            }
        }
    }
}

internal char* IntToChar(char* buffer, int32 input,const char* extraString)
{
    //NOTE: sprintf is slow find better way
    uint32 totalDigits = numDigits(input);
    uint32 extraLength = strlen(extraString);
    
    sprintf_s(buffer,totalDigits + extraLength + 2, "%d %s", input, extraString);
    return buffer;
}

internal char* FloatToChar(char* buffer, real32 input, const char* extraString, uint32 precision)
{
    // f - floor(f) to get decimal part and then use precision to get fractional parts.
    // not accurate but works for the moment.
    // NOTE: sprintf rounds if over .06 of a number. 4.468 > 4.47 ect doing the x 10 trick will fix this.
    uint32 totalDigits = numDigits(input);
    uint32 extraLength = strlen(extraString);
    
    sprintf_s(buffer,totalDigits + extraLength + 2 + (precision + totalDigits + 1), "%.2f %s", input, extraString);
    return buffer;
}
