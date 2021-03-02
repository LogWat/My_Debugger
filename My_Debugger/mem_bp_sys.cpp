#include "my_dbgr.h"

using std::vector;
using std::map;
using std::cout;
using std::endl;

DWORD page_size;
vector<LPVOID> guarded_pages;
map<LPVOID, mem_bp> memory_breakpoints;

SYSTEM_INFO s_i;

void init()
{
	GetSystemInfo(&s_i);
	page_size = s_i.dwPageSize;
}

BOOL bp_set_mem(LPVOID address, SIZE_T size)
{
	init();

	MEMORY_BASIC_INFORMATION mbi;

	// if the value returned by VirtualQueryEx() is less than the size of MEMORY_BASIC_INFORMATION, return FALSE
	if (VirtualQueryEx(h_process, address, &mbi, sizeof(mbi)) < sizeof(mbi))
		return FALSE;

	DWORD current_page = DWORD(mbi.BaseAddress);

	// Set permissions on all pages to be targeted
	while (current_page <= (DWORD)address + size)
	{
		// Add the appropriate page to the list
		// -> This allow us to distinguish our protected pages from the pages set by the OS or the target process
		guarded_pages.push_back((LPVOID)current_page);

		unsigned long old_protection = 0;
		if (!VirtualProtectEx(h_process, (LPVOID)current_page, size, mbi.Protect | PAGE_GUARD, &old_protection))
		{
			cout << "[!] Error: " << GetLastError() << endl;
			return FALSE;
		}
		// Expand the range by the system's default size
		current_page += page_size;
	}
	// Add the target memory breakpoints to the global dict
	memory_breakpoints[address] = { size, mbi };
	
	return TRUE;
}