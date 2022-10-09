
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

#include "MemoryTestApp.h"

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)
#define ArrayCount(a) (sizeof(a) / sizeof(*a))

#define MAX_MEMORY Megabytes(10)

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SYSTEM_INFO sSysInfo; // system infomation
    GetSystemInfo(&sSysInfo);
    
    SetTraceLogLevel(LOG_NONE);
    programState programData = {};
    programData.screenWidth = screenWidth;
    programData.screenHeight = screenHeight;
    programData.FPSTarget = 30; // 30 FPS is fine for now
    programData.hasMemAllocated = false;
    
    InitWindow(screenWidth, screenHeight, "Memory Test");
    
    SetTargetFPS(programData.FPSTarget);
    
    FilePathList droppedFiles = { 0 };
    
    Rectangle baseMemoryRect = {screenWidth / 2 - 500,screenHeight / 2,1000,100};
    Rectangle buttonRec  = {screenWidth / 2 - 60 ,screenHeight / 2 - 200,180,60};
    uint32 blocksAssigned = 0;
    memoryBlock memoryBlocks[120] = {};
    Vector2 mousePoint = { 0.0f, 0.0f };
    
    memory_arena programMemory = {};
    fileData* tempFile = {};
    const char* textData = "Hello world!";
    
    printf("Base memory rect X:%f, Y%f, width:%f, height:%f\n", baseMemoryRect.x, baseMemoryRect.y,baseMemoryRect.width,baseMemoryRect.height);
    // NOTE: allow user to dynamically allocate memory
    
    while (!WindowShouldClose()) 
    {
        (!IsWindowFocused) ? SetTargetFPS(15) : SetTargetFPS(programData.FPSTarget);
        
        mousePoint = GetMousePosition();
        
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
                        
                        printf("rand seed: %d \n",programMemory.Used);
                        memoryBlocks[blocksAssigned].color = GetRandomColor();
                        blocksAssigned++;
                        
                        printf("programData size: %d\n", programMemory.Size);
                        printf("programData total used: %d\n", programMemory.Used);
                    }
                    
                }
            }
        }
        
        //don't like this change later
        if (CheckCollisionPointRec(mousePoint, buttonRec) && !programData.hasMemAllocated)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                programData.hasMemAllocated = true;
                AllocateBaseMemory(programData,&programMemory, MAX_MEMORY);
                
                printf("programData size: %d\n", programMemory.Size);
                printf("programData total used: %d\n", programMemory.Used);
            }
        }
        
        //DRAWING
        BeginDrawing();
        
        ClearBackground(BLACK);
        
        if(!programData.hasMemAllocated)
        {
            DrawRectangleRec(buttonRec ,RED);
            DrawText("Alloc memory",buttonRec.x  + (buttonRec.width / 20),buttonRec.y + (buttonRec.height / 4),25,WHITE);
        }else
        {
            DrawRectangleRec(baseMemoryRect,BLUE);
            
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
