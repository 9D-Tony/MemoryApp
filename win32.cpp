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


#ifndef WIN32_PLATFORM
#define WIN32_PLATFORM

struct fileInfo 
{
    uint32 size;
    void* data;
};

// Code for windows functions
inline static uint64 GetPhysicalMemorySize()
{
    MEMORYSTATUSEX statex;
    GlobalMemoryStatusEx(&statex);
    return statex.ullTotalPhys;
}

static int32 GetSystemPageSize()
{
    SYSTEM_INFO sysInfo;
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

static void* Win32VirtualAlloc(uint32 size)
{
    return VirtualAlloc(0,size,  MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
}

static void Win32VirtualFree(void* memoryPtr)
{
    if(memoryPtr != NULL)
    {
        VirtualFree(memoryPtr, 0, MEM_RELEASE);
        return;
    }
    printf("could not free memory!");
}

static fileInfo LoadFile_UTF8(wchar_t* filename)
{
    // set console mode, doesn't handle UTF-8 fully, chinese characters are broken.
    
    _setmode(_fileno(stdout), _O_WTEXT);
    
    fileInfo fileResult = {};
    void* Result = 0;

    HANDLE FileHandle = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
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
                    wprintf(L"%ls sucessfully read!\n", filename);
                }
                else
                {
                    // free
                    VirtualFree(fileResult.data, 0, MEM_RELEASE);
                    Result = 0;
                    
                    wprintf(L"%ls file read unsuccessful!\n", filename);
                }
            }
        }
    }
    
    if (!fileResult.data)
    {
        DWORD Error = GetLastError();
        if (Error == 0x02)
        {
            wprintf(L"%ls file not found!\n", filename);
        }
        
        wprintf(L"%ls file read unsuccessful!\n", filename);
    }
    
    _setmode(_fileno(stdout), _O_TEXT);
    
    CloseHandle(FileHandle);
    
    return(fileResult);
    
}

static fileInfo LoadFile(char* filename)
{
    
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
    
    CloseHandle(FileHandle);
    
    return(fileResult);
}

static inline void  StringToWideString(char* input,wchar_t* output, int32 wCharSize)
{
    MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,input,-1, output, wCharSize);
}

static inline void MoveMem(void* dest, void* source, size_t length)
{
    //The first parameter, Destination, must be large enough to hold Length bytes of Source; otherwise, a buffer overrun may occur.
    MoveMemory(dest,source,length);
}

#endif