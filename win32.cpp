#ifndef WIN32_PLATFORM
#define WIN32_PLATFORM

// Code for windows functions
// (TODO): change to better name
struct fileInfo 
{
    uint32 size;
    void* data;
};

internal int32 GetSystemPageSize()
{
    SYSTEM_INFO sysInfo; // system infomation
    GetSystemInfo(&sysInfo);
    
    return sysInfo.dwPageSize;
}

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

internal void MoveMem(void* dest, void* source, size_t length)
{
    //The first parameter, Destination, must be large enough to hold Length bytes of Source; otherwise, a buffer overrun may occur.
    MoveMemory(dest,source,length);
}

#endif