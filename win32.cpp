#ifndef WIN32_PLATFORM
#define WIN32_PLATFORM

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
typedef double real64;

// (TODO): change to better name
struct fileInfo 
{
    uint32 size;
    void* data;
};

inline uint32
SafeTrucate64(uint64 value)
{
	
    assert(value <= 0xFFFFFFFF);
	uint32 Result = (uint32)value;
	return Result;
}

internal void* Win32VirtualAlloc(int32 size)
{
    return VirtualAlloc(0,size,  MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
}

internal void Win32VirtualFree(void* memoryPtr)
{
    if(memoryPtr != NULL)
    {
        VirtualFree(memoryPtr, 0, MEM_RELEASE);
    }
}

internal fileInfo Win32LoadFile(char* filename)
{
    //get file size
    fileInfo fileResult = {};
    void* Result = 0;
    
    HANDLE FileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTrucate64(FileSize.QuadPart);
            fileResult.size = FileSize32;
            fileResult.data = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            if (fileResult.data)
            {
                DWORD BytesRead;
                if (ReadFile(FileHandle, fileResult.data, FileSize32, &BytesRead, 0))
                {
                    // Read into memory.
                    printf("%s sucessfully read!\n", filename);
                }
                else
                {
                    // free
                    VirtualFree(fileResult.data, 0, MEM_RELEASE);
                    Result = 0;
                    printf("%s file read unsuccessful!\n", filename);
                }
            }
        }
    }
    
    if (!fileResult.data)
    {
        DWORD Error = GetLastError();
        if (Error == 0x02)
        {
            printf("%s file not found!\n", filename);
        }
        
        printf("%s file read unsuccessful!\n", filename);
    }
    return(fileResult);
}

#endif