#include "my_dbgr.h"

using std::map;


BOOL debugger_active = FALSE;
HANDLE h_process = NULL;
HANDLE h_thread = NULL;
CONTEXT ct;
PVOID exception_address = NULL;
map<LPVOID, const void*> software_breakpoints;

inline HANDLE open_process()
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

void createprocess(const wchar_t* path_to_exe)
{
	int creation_flags = DEBUG_PROCESS; // GUI表示：CREATE_NEW_CONSOLE

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
	si.dwFlags = 0x1;
	si.wShowWindow = 0x0;

	if (!CreateProcess(path_to_exe,
		NULL,
		NULL,
		NULL,
		FALSE,
		creation_flags,
		NULL,
		NULL,
		&si,    // Pointer to STARTUPINFO structure.
		&pi))
	{
		cout << "[*] Error: " << GetLastError() << endl;
		return;
	}
	
	cout << "[*] We have successfully launched the process!" << endl;
	cout << "[*] PID: " << pi.dwProcessId << endl;

	pid = pi.dwProcessId;		//プロセスID取得
	h_process = open_process(); //プロセスのハンドル取得

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void attach()
{
	h_process = open_process();

	//Processへのアタッチ試行(失敗時は戻る)
	if (!DebugActiveProcess(pid))
	{
		cout << "[*] Unable to attach to the process." << endl;
		return;
	}
	else {
		debugger_active = TRUE;
		pid = int(pid);
	}
}



void run()
{
	while (debugger_active)
		get_debug_event();
}

void get_debug_event()
{
	DEBUG_EVENT de;
	int continue_status = DBG_CONTINUE;

	if (WaitForDebugEvent(&de, INFINITE))
	{
		h_thread = open_thread(de.dwThreadId);
		ct = get_thread_context(h_thread);

		cout << "Event Code: " << de.dwDebugEventCode << " Thread ID: " << de.dwThreadId << endl;

		// イベントコードが例外ならば更に調査
		if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
		{
			DWORD exception = de.u.Exception.ExceptionRecord.ExceptionCode;
			exception_address = de.u.Exception.ExceptionRecord.ExceptionAddress; // PVOID型(void *)

			switch (exception)
			{
			case EXCEPTION_ACCESS_VIOLATION:
				cout << "Access Violation Detected." << endl;
				break;

			case EXCEPTION_BREAKPOINT:
				continue_status = exception_handler_breakpoint();
				break;

			case EXCEPTION_GUARD_PAGE:
				cout << "Guard Page Access Detected." << endl;
				break;

			case EXCEPTION_SINGLE_STEP:
				cout << "Single Stepping." << endl;
				break;
			}
		}

		ContinueDebugEvent(
			de.dwProcessId,
			de.dwThreadId,
			continue_status
		);
	}
}

int exception_handler_breakpoint()
{
	cout << "[*] Inside the breakpoint handler." << endl;
	cout << "Exception Address : 0x" << hex << exception_address << endl;
	return DBG_CONTINUE;
}


bool detach()
{
	if (DebugActiveProcessStop(pid))
	{
		cout << "[*] Finished debugging. Exiting..." << endl;
		return TRUE;
	}
	else {
		cout << "There was an error" << endl;
		return FALSE;
	}
}



HANDLE open_thread(DWORD thread_id)
{
	h_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, thread_id);

	if (h_thread != 0)
	{
		return h_thread;
	}
	else {
		cout << "[*] Could not obtain a valid thread handle." << endl;
		return FALSE;
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
		detach();
}


const void* read_process_memory(LPCVOID address, int length)
{
	char* read_buf = new char[length];

	unsigned long count = 0;

	if (!ReadProcessMemory(h_process, address, (LPVOID)read_buf, length, &count))
	{
		delete[] read_buf;
		return FALSE;
	}
	else {
		const char* data = read_buf;
		delete[] read_buf;
		return data;
	}
}

bool write_process_memory(LPVOID address, LPCVOID data)
{
	unsigned long count = 0;
	int length = sizeof(data);

	if (!WriteProcessMemory(h_process, address, data, length, &count))
		return FALSE;
	else
		return TRUE;
}

bool bp_set_sw(LPVOID address)
{
	cout << "[*] Setting breakpoint at: 0x" << hex << address << endl;

	if (software_breakpoints.find(address) != software_breakpoints.end())
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

LPVOID func_resolve(LPCWSTR dll, LPCSTR function)
{
	HMODULE handle = LoadLibrary(dll);
	if (handle == NULL)
	{
		cout << "[!] Error: " << GetLastError() << endl;
		return FALSE;
	}
	else
	{
		LPVOID address = GetProcAddress(handle, function);

		return address;
	}
}