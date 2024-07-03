#include "CLCommon.h"

namespace CL
{
	unsigned long Crc32(const unsigned char* Buff, size_t Length)
	{
		unsigned long crc_table[256];
		unsigned long crc;

		for (int i = 0; i < 256; i++)
		{
			crc = i;
			for (int j = 0; j < 8; j++)
				crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

			crc_table[i] = crc;
		}

		crc = 0xFFFFFFFFUL;

		while (Length--)
		{
			crc = crc_table[(crc ^ *Buff++) & 0xFF] ^ (crc >> 8);
		}

		return crc ^ 0xFFFFFFFFUL;
	}

	// #TODO make refptr lock-free implementation
	CL::CriticalSection& GetRefPtrCriticalSection()
	{
		static CL::CriticalSection CS;
		return CS;
	}
}