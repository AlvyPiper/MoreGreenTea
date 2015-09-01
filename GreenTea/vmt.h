//hooking class made by oubliette.
//afaik it's vac detected.

#pragma once

#include <Windows.h>

class CVMTHook
{
public:
	CVMTHook()
	{
		memset(this, 0, sizeof(CVMTHook));
	}

	CVMTHook(DWORD** _ClassBase)
	{
		ClassBase = _ClassBase;
		OldVMT = *_ClassBase;
		VMTSize = GetVMTSize(*_ClassBase);
		NewVMT = new DWORD[VMTSize];
		memcpy(NewVMT, OldVMT, sizeof(DWORD) * VMTSize);
		*_ClassBase = NewVMT;
	}

	void UnHook()
	{
		if (ClassBase)
		{
			*ClassBase = OldVMT;
		}
	}

	void ReHook()
	{
		if (ClassBase)
		{
			*ClassBase = NewVMT;
		}
	}

	DWORD GetMethodAddress(unsigned int Index)
	{
		if (Index >= 0 && Index <= VMTSize && OldVMT != NULL)
		{
			return OldVMT[Index];
		}

		return NULL;
	}

	DWORD* GetOldVMT()
	{
		return OldVMT;
	}

	DWORD HookMethod(DWORD NewFunction, int Index)
	{
		if (NewVMT && OldVMT && Index <= (int) VMTSize)
		{
			NewVMT[Index] = NewFunction;
			return OldVMT[Index];
		}

		return NULL;
	}
private:
	DWORD GetVMTSize(DWORD* VMT)
	{
		DWORD Index = 0;

		for (Index = 0; VMT[Index]; Index++)
		if (IsBadCodePtr((FARPROC) VMT[Index]))
			break;

		return Index;
	}

	DWORD** ClassBase;
	DWORD* NewVMT;
	DWORD* OldVMT;
	DWORD VMTSize;
};