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

#define pushStruct(Arena, type) (type *)pushSize_(Arena,sizeof(type))
#define pushArray(Arena, Count, type) (type *)pushSize_(Arena,(Count) * sizeof(type))
void* pushSize_(memory_arena *Arena, m_index size)
{
	assert(Arena->Used + size < Arena->Size);
	void *Result = Arena->Base + Arena->Used;
	Arena->Used += size;
	return (Result);
}

internal inline uint32 ToPageSize(uint32 input, uint32 pageSize)
{
    uint32 remainder = input % pageSize;
    return input - remainder;
}

internal void arena_align(memory_arena *arena, m_index boundary)
{
	m_index p =(arena->Used + (boundary - 1));
	arena->Used = p - p % boundary;
}

internal void ClearMemory(memory_arena *Arena, void *baseAddress, m_index size)
{
    assert(size < Arena->Used);
    ZeroMemory(baseAddress,size);
    Arena->Used -= size;
}

internal void FreeBase(void* memoryBase)
{
    Win32VirtualFree(memoryBase);
}

