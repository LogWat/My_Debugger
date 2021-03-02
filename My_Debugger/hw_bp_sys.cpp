#include "my_dbgr.h"

using std::vector;
using std::map;
using std::cout;
using std::endl;

vector<DWORD> thread_list;
map<int, hw_break_t> hardware_breakpoints;

static int available;

void enumerate_threads()
{
	THREADENTRY32 entry;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);

	if (snapshot != NULL)
	{
		entry.dwSize = sizeof(entry); // 構造体のサイズで初期化しないと失敗
		BOOL success = Thread32First(snapshot, &entry);

		while (success)
		{
			if (entry.th32OwnerProcessID == pid)
				thread_list.push_back(entry.th32ThreadID);
			success = Thread32Next(snapshot, &entry);
		}

		CloseHandle(snapshot);
	}
}


CONTEXT get_thread_context(HANDLE h_thread, DWORD thread_id)
{
	ct.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	// スレッドのハンドル取得
	if (h_thread == NULL)
		h_thread = open_thread(thread_id);
	if (GetThreadContext(h_thread, &ct))
	{
		CloseHandle(h_thread);
		return ct;
	}
	else
	{
		cout << "[!] Error: " << GetLastError() << endl;
		detach();
		exit(0);
	}
}

// hardware_breakpoint
BOOL bp_set_hw(LPVOID address, SIZE_T length, DWORD condition)
{

	if (!(length == 1 || length == 2 || length == 4))
		return FALSE;
	else
		length -= 1;

	// Type(条件)が有効か
	if (!(condition == HW_ACCESS || condition == HW_EXECUTE || condition == HW_WRITE))
		return FALSE;

	// 空いているレジスタ確認
	if (hardware_breakpoints.find(0) == hardware_breakpoints.end())
		available = 0;
	else if (hardware_breakpoints.find(1) == hardware_breakpoints.end())
		available = 1;
	else if (hardware_breakpoints.find(2) == hardware_breakpoints.end())
		available = 2;
	else if (hardware_breakpoints.find(3) == hardware_breakpoints.end())
		available = 3;
	else
		return FALSE;

	// Set debug registers for all threads
	enumerate_threads();
	for (auto thread_id : thread_list)
	{
		ct = get_thread_context(NULL, thread_id);

		// Set the corresponding bit in the DR7 register to available the breakpoint
		ct.Dr7 |= 1 << (available * 2);

		// Set the address of the breakpoint to a vacant register 
		switch (available)
		{
		case 0:
			ct.Dr0 = (DWORD)address;
			break;
		case 1:
			ct.Dr1 = (DWORD)address;
			break;
		case 2:
			ct.Dr2 = (DWORD)address;
			break;
		case 3:
			ct.Dr3 = (DWORD)address;
			break;
		}

		// Set the type of the breakpoint
		ct.Dr7 |= condition << ((available * 4) + 16);

		// Set the length
		ct.Dr7 |= length << ((available * 4) + 18);

		// Set thread context with breakpoints
		h_thread = open_thread(thread_id);
		SetThreadContext(h_thread, &ct);
	}
	// Update the dict of the hardware_breakpoint for the registers used
	hardware_breakpoints[available] = {address, length, condition};

	return TRUE;
}

BOOL bp_del_hw(int slot)
{
	// Disable the breakpoint for all active thread
	enumerate_threads();
	for (auto thread_id : thread_list)
	{
		ct = get_thread_context(NULL, thread_id);

		// Reset the flag bit and remove the breakpoint
		ct.Dr7 &= ~(1 << (slot * 2));

		// Clear the address to ZERO
		switch (slot)
		{
		case 0:
			ct.Dr0 = 0x00000000;
			break;
		case 1:
			ct.Dr1 = 0x00000000;
			break;
		case 2:
			ct.Dr2 = 0x00000000;
			break;
		case 3:
			ct.Dr3 = 0x00000000;
			break;
		}

		// Clear the condition flag
		ct.Dr7 = ~(3 << ((slot * 4) + 16));
		// Clear the length flag
		ct.Dr7 = ~(3 << ((slot * 4) + 18));

		// Reconfigure the context where the breakpoint was removed
		h_thread = open_thread(thread_id);
		SetThreadContext(h_thread, &ct);
	}
	// Remove the breakpoint from hw_bp_dict
	hardware_breakpoints.erase(slot);

	return TRUE;
}