#include "my_dbgr.h"

using std::cout;
using std::hex;
using std::endl;
using std::map;

map<LPVOID, const void*> software_breakpoints;

// software_breakpoint
BOOL bp_set_sw(LPVOID address)
{
	cout << "[*] Setting breakpoint at: 0x" << hex << address << endl;

	auto itr = software_breakpoints.find(address);

	if (itr == software_breakpoints.end())
	{
		try
		{
			// オリジナルのバイトを保存
			const void* original_byte = read_process_memory(address, 1);

			// INT3(0xCC)のオペコードを書込み
			write_process_memory(address, "\xCC");

			//内部リストにブレークポイントを登録
			software_breakpoints[address] = (original_byte);
		}
		catch (const std::exception&)
		{
			return FALSE;
		}
	}
	return TRUE;
}

const void* read_process_memory(LPCVOID address, SIZE_T length)
{
	LPVOID read_buf = new char[length];

	unsigned long count = 0;

	if (!ReadProcessMemory(h_process, address, read_buf, length, &count))
	{
		delete[] read_buf;
		cout << "[!] Error: " << GetLastError() << endl;
		return FALSE;
	}
	else {
		const void* data = read_buf;
		delete[] read_buf;
		return data;
	}
}

bool write_process_memory(LPVOID address, LPCVOID data)
{
	unsigned long count = 0;
	int length = sizeof(data);

	if (!WriteProcessMemory(h_process, address, data, length, &count))
	{
		cout << "[!] Error: " << GetLastError() << endl;
		return FALSE;
	}
	else
		return TRUE;
}