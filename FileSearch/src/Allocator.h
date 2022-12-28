#pragma once


namespace Allocator
{
	struct Arena 
	{
		char* m_Buffer = nullptr;
		int m_Index = 0;
		int m_Size = 0;
	};

	void AllocateArena(Arena* pArena, size_t Size);
	void ClearArena(Arena* pArena);
	void ResetArena(Arena* pArena);
	char* PushSize(Arena* pArena, size_t Size);
	char* ReallocateArena(Arena* pArena, size_t Size);

};
