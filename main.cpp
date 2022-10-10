
#include <iostream>
#include <assert.h>
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

#include "MemoryTestApp.h"

#define MAX_MEMORY Megabytes(10)

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SYSTEM_INFO sysInfo; // system infomation
    GetSystemInfo(&sysInfo);
    
    SetTraceLogLevel(LOG_NONE);
    
    programState programData = {};
    
    programData.screenWidth = screenWidth;
    programData.screenHeight = screenHeight;
    programData.FPSTarget = 60; // 30 FPS is fine for now
    programData.hasMemAllocated = false;
    programData.sliderValue = 0;
    programData. pageSize = sysInfo.dwPageSize;
    
    InitWindow(screenWidth, screenHeight, "Memory Test");
    
    SetTargetFPS(programData.FPSTarget);
    FilePathList droppedFiles = { 0 };
    
    //Retangles
    Rectangle baseMemoryRect = {screenWidth / 2 - 500,screenHeight / 2,1000,100};
    Rectangle allocateRect  = {screenWidth / 2 - 60 ,screenHeight / 2 - 180,180,60};
    
    Rectangle sliderBarRect = {screenWidth / 2 - 350 ,screenHeight / 2 - 250,800,20};
    Rectangle sliderRect = {screenWidth / 2 - 320 ,sliderBarRect.y - 32,20,80};
    
    uint32 blocksAssigned = 0;
    memoryBlock memoryBlocks[120] = {};
    Vector2 mousePos = { 0.0f, 0.0f };
    
    memory_arena programMemory = {};
    fileData* tempFile = {};
    const char* textData = "Hello world!";
    bool32 isMouseHeldDown = false;
    
    printf("Base memory rect X:%f, Y%f, width:%f, height:%f\n", baseMemoryRect.x, baseMemoryRect.y,baseMemoryRect.width,baseMemoryRect.height);
    // NOTE: allow user to dynamically allocate memory
    
    while (!WindowShouldClose()) 
    {
        (!IsWindowFocused) ? SetTargetFPS(15) : SetTargetFPS(programData.FPSTarget);
        
        mousePos = GetMousePosition();
        
        if(IsFileDropped())
        {
            //(NOTE): runs twice when you drop a file on win 10
            if (droppedFiles.count > 0) 
            {
                UnloadDroppedFiles(droppedFiles);
            }
            
            droppedFiles = LoadDroppedFiles();
            
            if(droppedFiles.paths != NULL)
            {
                for(int i = 0; i < droppedFiles.count; i++)
                {
                    // load file into memory here create memory block rectangles
                    uint32 beforeMemory = programMemory.Used;
                    fileData* filePtr = LoadDataIntoMemory(programMemory,droppedFiles.paths[i]);
                    // if we managed to  load the data
                    if(filePtr != NULL)
                    {
                        memoryBlocks[blocksAssigned].data = filePtr; 
                        memoryBlocks[blocksAssigned].rect = SetMemoryBlockPos(baseMemoryRect,programMemory, beforeMemory);
                        srand(programMemory.Used);
                        
                        memoryBlocks[blocksAssigned].color = GetRandomColor();
                        blocksAssigned++;
                        
                        printf("programData size: %d\n", programMemory.Size);
                        printf("programData total used: %d\n", programMemory.Used);
                    }
                }
            }
        }
        
        //(NOTE): don't like AllocateButton collision check change later
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
        
        if(!programData.hasMemAllocated)
        {
            DrawRectangleRec(allocateRect ,RED);
            DrawText("Alloc memory",allocateRect.x  + (allocateRect.width / 20),allocateRect.y + (allocateRect.height / 4),25, WHITE);
            
            //(TODO): add slider value here and assign value to memory alloc
            // map from BarRect width to memory size values
            
            programData.sliderValue = ToPageSize(MapRange(sliderBarRect.x,sliderBarRect.x + sliderBarRect.width,0,MAX_MEMORY,sliderRect.x), programData.pageSize);
            
            char textBuffer[20];
            
            if(programData.sliderValue < Megabytes(1))
            {
                DrawText(IntToChar(textBuffer,ToKilobytes(programData.sliderValue), "Kilobytes"),sliderBarRect.x + (sliderBarRect.width / 2), sliderBarRect.y - 80, 30, WHITE);
                
            }else
            {
                DrawText(IntToChar(textBuffer, ToMegabytes(programData.sliderValue), "Megabytes"),sliderBarRect.x + (sliderBarRect.width / 2), sliderBarRect.y - 80, 30, WHITE);
            }
            
            DrawRectangleRec(sliderBarRect,WHITE);
            DrawRectangleRec(sliderRect, BLUE);
            
        }else
        {
            
            DrawRectangleRec(baseMemoryRect,BLUE);
            
            char textBuffer[20];
            //(NOTE): cache text here.
            char* usedMemoryText = IntToChar(textBuffer, ToMegabytes(programMemory.Size), "Megabytes");
            int32 textWidth = MeasureTextEx(GetFontDefault(), usedMemoryText, 20, 1).x;
            
            DrawText(usedMemoryText,baseMemoryRect.width / 2 + textWidth,baseMemoryRect.y - baseMemoryRect.height / 2,20,WHITE);
            
            for(int i = 0; i < blocksAssigned; i++)
            {
                DrawRectangleRec(memoryBlocks[i].rect,memoryBlocks[i].color);
            }
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    
    FreeBase(programData);
    return 0;
}
