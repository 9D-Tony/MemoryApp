#include <iostream>
#include <assert.h>

#define internal static // for funtions
#define local_persist static // for variables in a local scope
#define global_variable static // for global variables

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32_t bool32;

typedef size_t memory_index;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float real32;

#define RAYGUI_IMPLEMENTATION
#include "include/raylib.h"
#include "include/raygui.h"

//defines so no errors from raylib when including windows.h
#if defined(WIN32) || defined(_WIN32) && !defined(__GNUC__)  

#define WIN32_LEAN_AND_MEAN
#define NOGDI             // All GDI defines and routines
#define NOUSER            // All USER defines and routines
#define MMNOSOUND         // PlaySound(A) ect

#include <windows.h>
#include "win32.cpp"

#elif defined(__GNUC__) || defined(__clang__)
// else define C standard mmap, free and fopen for compiling on clang and GCC
#endif

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)

#define ArrayCount(a) (sizeof(a) / sizeof(*a))
#define MapRange(oldMin,oldMax,newMin,newMax,value) (newMin + (value-oldMin)*(newMax-newMin)/(oldMax-oldMin))

#include "Math.h"
#include "MemoryTestApp.h"


// TODO: REMEBER TO TAKE THESE OUT AND FIX WARNINGS

#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#pragma warning(disable : 4018)

//this one is needed for raygui

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    // stop raylib printing things
    SetTraceLogLevel(LOG_NONE);
    
    programState pState = SetProgramState(screenWidth,screenHeight, 60,GetSystemPageSize());
    InitWindow(pState.screenWidth, pState.screenHeight, "Memory Test");
    
    pState.defaultFont = GetFontDefault();
    InitAudioDevice();
    
    SetTargetFPS(pState.FPSTarget);
    FilePathList droppedFiles = { 0 };
    
    //Retangles
    Rectangle baseMemoryRect = {screenWidth / 2 - 500,screenHeight / 2 - 100,1000,100};
    Rectangle allocateRect  = {screenWidth / 2 - 60 ,screenHeight / 2 - 180,180,60};
    Rectangle sliderBarRect = {screenWidth / 2 - 350 ,screenHeight / 2 - 250,800,50};
    Rectangle sliderRect = {screenWidth / 2 ,sliderBarRect.y - 32,20,80};
    
    GuiSetStyle(DEFAULT,TEXT_SIZE,17);
    GuiSetStyle( DEFAULT, BASE_COLOR_PRESSED, 0x003cb500); 
    Rectangle clearButtonPos = {0,0,0,0};
    
    uint32 blocksAssigned = 0;
    memoryBlock memoryBlocks[126] = {};
    Vector2 mousePos = { 0.0f, 0.0f };
    
    memory_arena programMemory = {};
    Font font = pState.defaultFont;
    fileData* tempFile = {};
    bool32 isMouseHeldDown = false;
    
    while (!WindowShouldClose()) 
    {
        (!IsWindowFocused()) ? SetTargetFPS(30) : SetTargetFPS(pState.FPSTarget);
        mousePos = GetMousePosition();
        
        if(IsFileDropped())
        {
            //NOTE: runs twice when you drop a file on win 10
            if (droppedFiles.count > 0) 
            {
                UnloadDroppedFiles(droppedFiles);
            }
            
            droppedFiles = LoadDroppedFiles();
            
            // droppedFiles, programMemory, MemoryBlocks
            if(droppedFiles.paths != NULL)
            {
                for(int i = 0; i < droppedFiles.count; i++)
                {
                    // load file into memory here create memory block rectangles
                    int32 beforeMemory = programMemory.Used;
                    fileData* filePtr = LoadFileIntoMemory(programMemory,droppedFiles.paths[i]);
                    
                    // if we managed to  load the data, if null then could not load data
                    if(filePtr != NULL)
                    {
                        //get string width
                        memoryBlocks[blocksAssigned].stringWidth = GetTextWidth(memoryBlocks[blocksAssigned].string,20);
                        
                        memoryBlocks[blocksAssigned] = SetMemoryBlock(memoryBlocks[blocksAssigned],baseMemoryRect,programMemory,filePtr);
                        
                        SetDroppedFiles(memoryBlocks, blocksAssigned);
                        
                        blocksAssigned++;
                    }
                }
            }
        }
        
        //DRAWING
        BeginDrawing();
        ClearBackground(BLACK);
        
        char textBuffer[24];
        
        //Maybe a layed / screen system for drawing and logic
        if(!pState.hasMemAllocated)
        {
            if (GuiButton(allocateRect, "#95#  Alloc Memory"))
            {
                pState.hasMemAllocated = true;
                AllocateBaseMemory(pState,&programMemory, pState.sliderValue);
            }
            
            pState.sliderValue = GuiSliderBar(sliderBarRect,"Left", "Right",pState.sliderValue,MIN_MEMORY,MAX_MEMORY);
            
            char* sliderText = 0;
            int32 sliderTextWidth = 0;
            
            if(pState.sliderValue < Megabytes(1))
            {
                sliderText = IntToChar(textBuffer,ToKilobytes(pState.sliderValue), "Kilobytes");
                sliderTextWidth = GetTextWidth(sliderText, titleSize);
                DrawText(sliderText,sliderBarRect.x + (sliderBarRect.width / 2)  - (sliderTextWidth / 2), sliderBarRect.y - 80, titleSize, WHITE);
            }else
            {
                sliderText = IntToChar(textBuffer,ToMegabytes(pState.sliderValue), "Megabytes");
                sliderTextWidth = GetTextWidth(sliderText, titleSize);
                DrawText(sliderText,sliderBarRect.x + (sliderBarRect.width / 2) - (sliderTextWidth / 2), sliderBarRect.y - 80, titleSize, WHITE);
            }
            
        }else // IF MEMORY ALLOCATED
        {
            DrawRectangleRec(baseMemoryRect,BLUE);
            
            //NOTE: cache text here
            real32 totalMemoryLeft = (real32)ToMegabytes((real32)programMemory.Size - (real32)programMemory.Used);
            int32 totalKiloytesLeft = ToKilobytes(programMemory.Size - programMemory.Used);
            
            char* totalMemoryText = FloatToChar(textBuffer, totalMemoryLeft, "MB", 2);
            
            if(totalMemoryLeft < 1)
            {
                // convert to kilobytes
                totalMemoryLeft = (real32)ToKilobytes((real32)programMemory.Size - (real32)programMemory.Used);
                totalMemoryText = IntToChar(textBuffer, (int32)totalMemoryLeft, "KB"); 
            }
            
            int32 textWidth = MeasureTextEx(font, totalMemoryText, titleSize, 1).x;
            int32 memLeftTxtWidth = MeasureTextEx(font,"Memory Left", titleSize, 1).x;
            
            DrawText(totalMemoryText,GetRectCenter(baseMemoryRect).x - (textWidth / 2),baseMemoryRect.y - baseMemoryRect.height * 1.5f,titleSize,WHITE);
            
            DrawText("Memory Left",GetRectCenter(baseMemoryRect).x - (memLeftTxtWidth / 2),baseMemoryRect.y - baseMemoryRect.height * 1.85f,titleSize,WHITE);
            
            if(programMemory.Used > 0)
            {
                Vector2 baseMemoryCenter = GetRectCenter(baseMemoryRect);
                
                clearButtonPos = SetRect((baseMemoryCenter.x - 80.0f),baseMemoryCenter.y - baseMemoryRect.height - 20,160,50); 
                
                
                if (GuiButton(clearButtonPos, "#113#  Clear Memory"))
                {
                    for(int i = 0 ; i < blocksAssigned; i++)
                    {
                        int32 fileSize = memoryBlocks[i].data->size;
                        int32 memTypeSize = sizeof(fileData);
                        
                        if(pState.globalTex.id > 0)
                        {
                            Texture defaultTexture = {};
                            UnloadTexture(pState.globalTex);
                            pState.globalTex = defaultTexture;
                        }
                        // clear block data
                        memset(memoryBlocks[i].data, 0, fileSize);
                        
                        if(pState.globalSound.frameCount > 0)
                        {
                            StopSound(pState.globalSound);
                            UnloadSound(pState.globalSound);
                            Sound emptySound = {};
                            pState.globalSound = emptySound;
                        }
                        
                        memoryBlocks[i] = {};
                    }
                    
                    programMemory.Used = 0;
                    blocksAssigned = 0;
                }
            }
            
            //mouse input for memoryBlocks
            for(int i = 0; i < blocksAssigned; i++)
            {
                MemoryblocksMouseIO(i, memoryBlocks,mousePos, &pState);
                Rectangle memoryRect = memoryBlocks[i].rect;
                
                // Draw each memory block
                DrawRectangleRec(memoryRect,memoryBlocks[i].color);
                
                int32 middleBlock = memoryRect.x + (memoryRect.width / 2);
                Vector2 memStringPos = memoryBlocks[i].stringPos;
                
                if(memoryBlocks[i].stringWidth < memoryRect.width)
                {
                    //If text can fit in memory block render it in the middle of the block
                    DrawTextEx(font,memoryBlocks[i].string,memStringPos,blockTxtSize,1,WHITE);
                }else
                {
                    
                    // check if the memory block string is colliding and move it up if it is.
                    Rectangle memoryTextLine = SetRect(middleBlock,memoryRect.y - 20, 2,20);
                    
                    DrawRectangleRec(memoryTextLine,WHITE);
                    DrawTextEx(font,memoryBlocks[i].string,memStringPos,blockTxtSize,1,WHITE);
                }
                
                if(pState.globalTex.id > 0)
                {
                    Rectangle testRect = {baseMemoryRect.x + (baseMemoryRect.width / 2) - 200,baseMemoryRect.y + baseMemoryRect.height, 400,   pState.screenHeight - (baseMemoryRect.y + baseMemoryRect.height) };
                    
                    real32 scale = ShinkToFitBounds(pState.globalTex,testRect);
                    
                    Vector2 texturePos = SetTextureAtCenter(baseMemoryRect,pState.globalTex,scale);
                    texturePos.y += (baseMemoryRect.height / 2) + (pState.globalTex.height * scale) / 2;
                    
                    DrawTextureEx(pState.globalTex,texturePos,0,scale,WHITE);
                }
            }
            
            
        }
        
        EndDrawing();
    }
    
    UnloadSound(pState.globalSound);
    CloseAudioDevice();
    CloseWindow();
    
    FreeBase(pState.memoryBase);
    return 0;
}
