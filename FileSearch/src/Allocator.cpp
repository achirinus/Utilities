#include "Allocator.h"


static void CopyMem(char* Src, int SrcSize, char* Dest)
{
	while (SrcSize--) *Dest++ = *Src++;
}

namespace Allocator
{
	void AllocateArena(Arena* pArena, size_t Size)
	{
		pArena->m_Buffer = new char[Size];
		pArena->m_Index = 0;
		pArena->m_Size = Size;
	}

	void ClearArena(Arena* pArena)
	{
		delete[] pArena->m_Buffer;
		pArena->m_Index = 0;
	}

	void ResetArena(Arena* pArena)
	{
		pArena->m_Index = 0;
	}


	char* PushSize(Arena* pArena, size_t Size)
	{
		if (Size > (pArena->m_Size - pArena->m_Index))
		{
			int NewSize = pArena->m_Size * 2;
			char* NewArena = new char[NewSize];
			CopyMem(pArena->m_Buffer, pArena->m_Size, NewArena);

			if(pArena->m_Buffer) delete[] pArena->m_Buffer;

			pArena->m_Buffer = NewArena;
			pArena->m_Size = NewSize;
		}

		char* Res = pArena->m_Buffer + pArena->m_Index;
		pArena->m_Index += Size;
		return Res;
	}

	char* ReallocateArena(Arena* pArena, size_t Size)
	{
		if (Size > pArena->m_Size)
		{
			int NewSize = Size;
			char* NewArena = new char[NewSize];

			if (pArena->m_Buffer) delete[] pArena->m_Buffer;

			pArena->m_Buffer = NewArena;
			pArena->m_Size = NewSize;
		}

		char* Res = pArena->m_Buffer;
		return Res;
	}
}
