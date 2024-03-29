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

#include "Memory.h"
#include "Memory.cpp"

struct programState {
    char windowName[64];
    int32 FPSTarget;
    
    int32 screenWidth;
    int32 screenHeight;
    int32 pageSize;
    int32 size;
    int32 totalUsed;
    real32 sliderValue;
    Rectangle baseMemoryRect;
    
    memoryBlock* selectedBlock;
    uint32 selectedIndex;
    Color blockLastColor;
    Texture2D globalTex;
    Sound globalSound;
    Font defaultFont;
    bool32 hasMemAllocated;
    
    void* memoryBase;
};

char supportedTxtFiles[5][8] = { ".txt", ".blah"};
char supportedAudioFiles[5][8] = { ".wav",".mp3",".ogg"}; // have to rebuild raylib for flac support
char supportedImageFiles[6][8] = { ".gif", ".png", ".jpg", ".JPG", ".PNG",".GIF"};

// standard text sizes
#define titleSize 30
#define buttonTxtSize 25
#define blockTxtSize 20

static void AllocateBaseMemory(programState& data, memory_arena *arena, uint32 memSize)
{
    data.memoryBase = Win32VirtualAlloc(memSize);
    
    arena->Size = memSize;
	arena->Base = (uint8*)data.memoryBase;
	arena->Used = 0;
    
    ZeroMemory(data.memoryBase,memSize);
    data.size = memSize;
    data.totalUsed = 0;
}

static Rectangle SetRect(real32 x,real32 y,real32 width,real32 height)
{
    return Rectangle {x,y,width,height};
}

static programState SetProgramState(int32 screenWidth, int32 screenHeight, int32 FPSTarget, int32 pageSize)
{
    programState pState  = {};
    pState.screenWidth = screenWidth;
    pState.screenHeight = screenHeight;
    pState.FPSTarget = FPSTarget;
    pState.hasMemAllocated = false;
    pState.sliderValue = 0;
    pState.pageSize = pageSize;
    pState.sliderValue = (real32)(MAX_MEMORY / 2);
    
    pState.baseMemoryRect = {(real32)screenWidth / 2 - 500,(real32)screenHeight / 2 - 100,1000,100};
    
    return pState;
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

static void StopAudio(programState* programData)
{
    if(programData->globalSound.frameCount > 0)
    {
        StopSound(programData->globalSound);
    }
}

static real32 GetTextWidth(char* string, real32 fontSize)
{
    return MeasureTextEx(GetFontDefault(),string,fontSize,1).x;
}

static void DrawButton(Rectangle buttonRect, const char* text,int32 textSize, Color color)
{
    //Draw a rectangle with text in the middle.
    DrawRectangleRec(buttonRect,color);
    Vector2 textDim =  MeasureTextEx(GetFontDefault() ,text, (real32)textSize, 1);
    
    Vector2 textPos = GetRectCenter(buttonRect);
    DrawText(text, (int32)(textPos.x - (textDim.x / 2)), (int32)(textPos.y - (textDim.y / 2)), textSize, WHITE);
}

static inline Rectangle SetMemoryBlockPos(Rectangle baseMemoryRect, memory_arena& programMemory, uint32 beforeUsedMemory)
{
    Rectangle Result = {};
    
    real32 oldRange = (real32)programMemory.Size;
    
    real32 newRange = (real32)baseMemoryRect.width;
    Result.height = baseMemoryRect.height / 1.1f;
    
    Result.y = baseMemoryRect.y + (baseMemoryRect.height / 16);
    Result.x = beforeUsedMemory * newRange / oldRange + baseMemoryRect.x;
    
    Result.width = ((real32)programMemory.Used * newRange / oldRange + baseMemoryRect.x) - Result.x;
    
    return Result;
}

static memoryBlock SetMemoryBlock(memoryBlock block,Rectangle baseMemoryRect,memory_arena programMemory, fileData* filePtr)
{
    memoryBlock resultBlock = {};
    resultBlock.data = filePtr;
    
    int32 beforeMemory = (int32)programMemory.Used - filePtr->size;
    resultBlock.rect = SetMemoryBlockPos(baseMemoryRect,programMemory, beforeMemory);
    
    srand((uint32)programMemory.Used);
    resultBlock.color = GetRandomColor();
    
    strcpy_s(resultBlock.string,EnumToChar(resultBlock.data));
    
    resultBlock.stringWidth = GetTextWidth(resultBlock.string,blockTxtSize);
    
    return resultBlock;
}

static Sound LoadSoundFromMemory(fileData* data, programState* programData)
{
    Sound sound = {};
    Wave wave = {};
    
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
    
    sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    
    return sound;
}

static Texture2D LoadImageFrmMemory(fileData* data, programState* programData)
{
    //NOTE: For the moment cannot load images that have extensions in all caps.
    Texture2D texture = {};
    
    if(programData->globalTex.id > 0)
    {
        UnloadTexture(programData->globalTex);
    }
    
    Image inputImage = LoadImageFromMemory(data->extension, data->baseData, data->size - sizeof(fileData));
    
    texture = LoadTextureFromImage(inputImage);
    
    if(texture.id == NULL)
    {
        printf("can't load image\n");
        return texture;
    }
    
    UnloadImage(inputImage);
    
    return (texture);
}

static inline bool32 CheckIfExtension(char* extensionArray,int32 arraySize, char* extension)
{
    if(strlen(extension) < 1)
    {
        return false;
    }
    
    for(int i=0; i < arraySize; i++)
    {
        if(strcmp(extensionArray + (i * 8), extension) == 0)
        {
            return true;
        }
    }
    
    return false;
}

static void ClearTexture(programState* programData)
{
    Texture texture = {};
    UnloadTexture(programData->globalTex);
    programData->globalTex = texture;
}

#if defined(_DEBUG)

uint32 screenshotNum = 0;
static void CheckDebugScreenshot()
{
    if (IsKeyPressed(KEY_F1))
    {
        char numBuffer[40];
        sprintf(numBuffer, "DebugScreenshots/Debug_%d.png",screenshotNum);
        TakeScreenshot(numBuffer);
        screenshotNum++;
    }
}

#else
static void CheckDebugScreenshot() {} //stub
#endif

static void SetBlockInfo(programState* pState, memoryBlock* memoryBlocks, int32 nextIndex)
{
    
    if(pState->selectedBlock != NULL)
    {
        pState->selectedBlock->color = pState->blockLastColor;
    }
    
    pState->selectedIndex = nextIndex;
    
    pState->selectedBlock = &memoryBlocks[pState->selectedIndex];
    pState->blockLastColor = memoryBlocks[pState->selectedIndex].color;
    memoryBlocks[pState->selectedIndex].color = GRAY;
    
    fileData* memoryData = memoryBlocks[pState->selectedIndex].data;
    
    // Actions for each filetype
    if(CheckIfExtension((char*)supportedImageFiles,ArrayCount(supportedImageFiles), memoryData->extension))
    {
        StopAudio(pState);
        pState->globalTex = LoadImageFrmMemory(memoryData,pState);
    }
    
    if(CheckIfExtension((char*)supportedAudioFiles,ArrayCount(supportedAudioFiles), memoryData->extension))
    {
        ClearTexture(pState);
        StopAudio(pState);
    }
}

static void MemoryBlocksMouseIO(uint32 index, memoryBlock* memoryBlocks, Vector2 mousePos, programState* pState)
{
    Rectangle memoryRect = memoryBlocks[index].rect;
    
    // colliding with a memory rect
    if ((CheckCollisionPointRec(mousePos, memoryRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && memoryRect.width > 5))
    {
        SetBlockInfo(pState, memoryBlocks,index);
    }
    
    if(pState->selectedBlock != NULL && pState->selectedBlock->data != NULL && pState->selectedBlock->data->type == F_AUDIO)
    {
        Vector2 baseMemoryRect = GetRectCenter(pState->baseMemoryRect);
        Rectangle playRect = {baseMemoryRect.x - 40,baseMemoryRect.y + 60,80,40};
        if(GuiButton(playRect, "#131# Play"))
        {
            pState->globalSound = LoadSoundFromMemory(pState->selectedBlock->data,pState);
            PlaySound(pState->globalSound);
        }
    }
}

static fileData* LoadFileIntoMemory_UTF8(memory_arena& programMemory, wchar_t* filename)
{
    fileData* fileResult = {};
    fileInfo fileLoadResult = LoadFile_UTF8(filename);
    
    if(fileLoadResult.data == NULL || fileLoadResult.size > (programMemory.Size - programMemory.Used))
    {
        printf("File could not fit inside avaliable memory\n");
        Win32VirtualFree(fileLoadResult.data);
        return NULL;
    }
    
    int32 filenameLength = (int32)wcslen(filename);
    
    int32 foundChar = (int32)wcscspn(filename, L".");
    
    wchar_t extensionString[64]; // most extensions will be < 4
    //works if the filename only has one "." in it,
    
    //TODO: search from the back of the string to get the extension, e.g If the user ever accidentally names a file .jpg.png
    //TODO: probably need to replace wchar_t with custome UTF-8 implimentation
    
    int32 extensionLength = filenameLength - foundChar;
    
    char convertedExtension[40];
    
    wcscpy_s(extensionString,filename + foundChar);
    
    wcstombs(convertedExtension,extensionString,sizeof(convertedExtension) + 1);
    
    //fails here as converted is way over
    if(strlen(convertedExtension) > 12)
    {
        //Don't add to the memory file extension too long
        printf("File extension tool long at %d chracters", extensionLength);
        Win32VirtualFree(fileLoadResult.data);
        return NULL;
    }
    
    // get memory for the file
    fileResult = pushStruct(&programMemory,fileData);
    fileResult->size = sizeof(fileData) + fileLoadResult.size;
    fileResult->baseData = pushArray(&programMemory,fileLoadResult.size,uint8);
    
    
    strcpy_s(fileResult->extension, convertedExtension);
    
    if(strcmp(convertedExtension, ".txt") == 0)
    {
        fileResult->type = F_TEXT;
        Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
        
    }else if(CheckIfExtension((char*)supportedAudioFiles,ArrayCount(supportedImageFiles), convertedExtension))
    {
        fileResult->type = F_AUDIO;
        Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
        
    }else if(CheckIfExtension((char*)supportedImageFiles,ArrayCount(supportedImageFiles), convertedExtension))
    {
        fileResult->type = F_IMAGE;
        Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
        
    }else
    {
        fileResult->type = F_OTHER;
        Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
    }
    
    
    Win32VirtualFree(fileLoadResult.data);
    
    return(fileResult);
}

static fileData* LoadFileIntoMemory(memory_arena& programMemory, char* filename)
{
    fileData* fileResult = {};
    fileInfo fileLoadResult = LoadFile(filename);
    
    if(fileLoadResult.data == NULL || fileLoadResult.size > (programMemory.Size - programMemory.Used))
    {
        printf("File could not fit inside avaliable memory\n");
        Win32VirtualFree(fileLoadResult.data);
        return NULL;
    }
    
    int32 filenameLength = (int32)strlen(filename);
    int32 foundChar = (int32)strcspn(filename, ".");
    
    char extensionString[12]; // most extensions will be < 4
    
    // get memory for the file
    fileResult = pushStruct(&programMemory,fileData);
    fileResult->size = sizeof(fileData) + fileLoadResult.size;
    fileResult->baseData = pushArray(&programMemory,fileLoadResult.size,uint8);
    
    //works if the filename only has one "." in it,
    //TODO: search from the back of the string to get the extension.
    if(foundChar != filenameLength)
    {
        strcpy_s(extensionString, filename + foundChar);
        strcpy_s(fileResult->extension, extensionString);
        
        //no support UTF-8 for the moment
        if(strcmp(extensionString, ".txt") == 0)
        {
            fileResult->type = F_TEXT;
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else if(CheckIfExtension((char*)supportedAudioFiles,ArrayCount(supportedImageFiles), extensionString))
        {
            fileResult->type = F_AUDIO;
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else if(CheckIfExtension((char*)supportedImageFiles,ArrayCount(supportedImageFiles), extensionString))
        {
            fileResult->type = F_IMAGE;
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
            
        }else
        {
            fileResult->type = F_OTHER;
            Copy(fileResult->baseData,fileLoadResult.size,(uint8*)fileLoadResult.data);
        }
    }
    
    Win32VirtualFree(fileLoadResult.data);
    return(fileResult);
}

static void SetStringPos(memoryBlock* memoryBlocks,uint32 blocksAssigned)
{
    Rectangle memoryRect = memoryBlocks[blocksAssigned].rect;
    int32 middleBlock = (int32)(memoryRect.x + (memoryRect.width / 2));
    
    // if string width can fit in the memory block
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

static char* IntToChar(char* buffer, int32 input,const char* extraString)
{
    uint32 totalDigits = numDigits(input);
    uint32 extraLength = (int32)strlen(extraString);
    sprintf_s(buffer,totalDigits + extraLength + 2, "%d %s", input, extraString);
    return buffer;
}

static char* FloatToChar(char* buffer, real32 input, const char* extraString, uint32 precision)
{
    // not completly accurate but works for the moment.
    // NOTE: sprintf rounds if over .06 of a number. 4.468 > 4.47 ect doing the x 10 trick will fix this.
    uint32 totalDigits = numDigits((uint32)input);
    uint32 extraLength = (uint32)strlen(extraString);
    
    sprintf_s(buffer,totalDigits + extraLength + 2 + (precision + totalDigits + 1), "%.2f %s", input, extraString);
    return buffer;
}
