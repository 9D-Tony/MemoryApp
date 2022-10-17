#include <iostream>
#include <assert.h>

#define SUPPORT_FILEFORMAT_FLAC

#include "include/raylib.h"
//defines so no errors from raylib when including windows.h
#if defined(WIN32) || defined(_WIN32) && !defined(__GNUC__)      
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
/*#define NONLS             // All NLS defines and routines*/
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define MMNOSOUND         // PlaySound(A) ect
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX         // All USER defines and routines

#include <windows.h>
#include "win32.cpp"

#endif
// else define C standard mmap and free for compiling on clang and GCC

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define ArrayCount(a) (sizeof(a) / sizeof(*a))
#define MapRange(oldMin,oldMax,newMin,newMax,value) (newMin + (value-oldMin)*(newMax-newMin)/(oldMax-oldMin))

#define ToKilobytes(Value) ((Value) / 1024)
#define ToMegabytes(Value) (ToKilobytes(Value) / 1024)
#define Min(X, Y) (((X) < (Y)) ? (X) : (Y))

#include "MemoryTestApp.h"

#define MAX_MEMORY Megabytes(10)
#define MIN_MEMORY Kilobytes(500)

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    // stop raylib printing things
    SetTraceLogLevel(LOG_NONE);
    
    programState programData = SetProgramState(screenWidth,screenHeight, 60,GetSystemPageSize());
    
    InitWindow(programData.screenWidth, programData.screenHeight, "Memory Test");
    
    InitAudioDevice();
    
    SetTargetFPS(programData.FPSTarget);
    FilePathList droppedFiles = { 0 };
    
    //Retangles
    Rectangle baseMemoryRect = {screenWidth / 2 - 500,screenHeight / 2 - 100,1000,100};
    Rectangle allocateRect  = {screenWidth / 2 - 60 ,screenHeight / 2 - 180,180,60};
    Rectangle sliderBarRect = {screenWidth / 2 - 350 ,screenHeight / 2 - 250,800,20};
    Rectangle sliderRect = {screenWidth / 2 ,sliderBarRect.y - 32,20,80};
    
    uint32 blocksAssigned = 0;
    memoryBlock memoryBlocks[126] = {};
    Vector2 mousePos = { 0.0f, 0.0f };
    
    memory_arena programMemory = {};
    fileData* tempFile = {};
    bool32 isMouseHeldDown = false;
    
    while (!WindowShouldClose()) 
    {
        (!IsWindowFocused()) ? SetTargetFPS(30) : SetTargetFPS(programData.FPSTarget);
        
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
                        
                        printf("programData size: %d\n", programMemory.Size);
                        printf("programData total used: %d\n", programMemory.Used);
                    }
                }
            }
        }
        
        //NOTE: don't like AllocateButton collision check
        //Allocate rect collision
        if(!programData.hasMemAllocated)
        {
            if (CheckCollisionPointRec(mousePos, allocateRect) && !programData.hasMemAllocated)
            {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    programData.hasMemAllocated = true;
                    AllocateBaseMemory(programData,&programMemory, programData.sliderValue);
                    
                    printf("programData size: %d\n", programMemory.Size);
                    printf("programData total used: %d\n", programMemory.Used);
                }
            }
            
            //Slider rect mouse input
            if(CheckCollisionPointRec(mousePos, sliderRect))
            {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    isMouseHeldDown = true;
                }
            }
            
            if(isMouseHeldDown)
            {
                if(IsMouseButtonUp(MOUSE_BUTTON_LEFT))
                {
                    isMouseHeldDown = false;
                }
                
                sliderRect.x = mousePos.x - (sliderRect.width / 2);
                
                if(sliderRect.x > (sliderBarRect.x + sliderBarRect.width)) sliderRect.x = (sliderBarRect.x + sliderBarRect.width) - 0.1f;
                
                if(sliderRect.x < sliderBarRect.x) sliderRect.x = sliderBarRect.x + 0.1f;
            }
        }
        
        //DRAWING
        BeginDrawing();
        ClearBackground(BLACK);
        
        char textBuffer[24];
        
        if(!programData.hasMemAllocated)
        {
            DrawRectangleRec(allocateRect ,RED);
            DrawText("Alloc memory",allocateRect.x  + (allocateRect.width / 20),allocateRect.y + (allocateRect.height / 4),25, WHITE);
            
            programData.sliderValue = ToPageSize(MapRange(sliderBarRect.x,sliderBarRect.x + sliderBarRect.width,MIN_MEMORY,MAX_MEMORY,sliderRect.x), programData.pageSize);
            
            char* sliderText = 0;
            int32 sliderTextWidth = 0;
            
            if(programData.sliderValue < Megabytes(1))
            {
                sliderText = IntToChar(textBuffer,ToKilobytes(programData.sliderValue), "Kilobytes");
                sliderTextWidth = GetTextWidth(sliderText, 30);
                
                DrawText(sliderText,sliderBarRect.x + (sliderBarRect.width / 2)  - (sliderTextWidth / 2), sliderBarRect.y - 80, 30, WHITE);
                
            }else
            {
                sliderText = IntToChar(textBuffer,ToMegabytes(programData.sliderValue), "Megabytes");
                
                sliderTextWidth = GetTextWidth(sliderText, 30);
                
                DrawText(sliderText,sliderBarRect.x + (sliderBarRect.width / 2) - (sliderTextWidth / 2), sliderBarRect.y - 80, 30, WHITE);
            }
            
            DrawRectangleRec(sliderBarRect,WHITE);
            
            (!isMouseHeldDown) ? DrawRectangleRec(sliderRect, BLUE) : DrawRectangleRec(sliderRect, DARKBLUE);
            
        }else // IF MEMORY ALLOCATED
        {
            DrawRectangleRec(baseMemoryRect,BLUE);
            
            //NOTE: cache text here
            real32 totalMemoryLeft = (real32)ToMegabytes((real32)programMemory.Size - (real32)programMemory.Used);
            int32 totalKiloytesLeft = ToKilobytes(programMemory.Size - programMemory.Used);
            
            char* totalMemoryText = FloatToChar(textBuffer, totalMemoryLeft, "Megabytes", 2);
            
            if(totalMemoryLeft < 1)
            {
                // convert to kilobytes
                totalMemoryLeft = (real32)ToKilobytes((real32)programMemory.Size - (real32)programMemory.Used);
                
                totalMemoryText = IntToChar(textBuffer, (int32)totalMemoryLeft, "Kilobytes"); 
            }
            
            int32 textWidth = MeasureTextEx(GetFontDefault(), totalMemoryText, 30, 1).x;
            
            DrawText(totalMemoryText,baseMemoryRect.width / 2 + (textWidth / 2),baseMemoryRect.y - baseMemoryRect.height * 1.5f,30,WHITE);
            
            //mouse input for memoryBlocks
            
            for(int i = 0; i < blocksAssigned; i++)
            {
                MemoryblocksMouseIO(i, memoryBlocks,mousePos, &programData);
                Rectangle memoryRect = memoryBlocks[i].rect;
                
                // Draw each memory block
                DrawRectangleRec(memoryRect,memoryBlocks[i].color);
                
                int32 middleBlock = memoryRect.x + (memoryRect.width / 2);
                Vector2 memStringPos = memoryBlocks[i].stringPos;
                
                if(memoryBlocks[i].stringWidth < memoryRect.width)
                {
                    //If text can fit in memory block render it in the middle of the block
                    DrawText(memoryBlocks[i].string,memStringPos.x,memStringPos.y,20,WHITE);
                }else
                {
                    // check if the memory block string is colliding and move it up if it is.
                    
                    Rectangle memoryTextLine = {middleBlock,memoryRect.y - 20, 2,20};
                    
                    DrawRectangleRec(memoryTextLine,WHITE);
                    DrawText(memoryBlocks[i].string,memStringPos.x,memStringPos.y,20,WHITE);
                }
                
                if(programData.globalTex.id > 0)
                {
                    Rectangle testRect = {baseMemoryRect.x + (baseMemoryRect.width / 2) - 200,baseMemoryRect.y + baseMemoryRect.height, 400,   programData.screenHeight - (baseMemoryRect.y + baseMemoryRect.height) };
                    
                    real32 scale = ShinkToFitBounds(programData.globalTex,testRect);
                    
                    Vector2 texturePos = SetTextureAtCenter(baseMemoryRect,programData.globalTex,scale);
                    
                    texturePos.y += (baseMemoryRect.height / 2) + (programData.globalTex.height * scale) / 2;
                    
                    DrawTextureEx(programData.globalTex,texturePos,0,scale,WHITE);
                }
            }
        }
        
        EndDrawing();
    }
    
    UnloadSound(programData.globalSound);
    CloseAudioDevice();
    CloseWindow();
    
    FreeBase(programData);
    return 0;
}
