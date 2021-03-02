#include "my_dbgr.h"

using std::endl;
using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::hex;
using std::map;


BOOL debugger_active = FALSE;
BOOL first_breakpoint = TRUE;

HANDLE h_process = NULL;
HANDLE h_thread = NULL;

CONTEXT ct;

PVOID exception_address = NULL;

inline HANDLE open_process()
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

void createprocess(const wchar_t* path_to_exe)
{
	int creation_flags = CREATE_NEW_CONSOLE; // GUI表示：CREATE_NEW_CONSOLE

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
		cout << "[!] Error: " << GetLastError() << endl;
		exit(0);
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
				continue_status = exception_handler_single_step();
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

int exception_handler_single_step()
{
	static int slot;
	int continue_status;
	// Comment of PyDbg
	// Check if this single step event was triggered by a hardware breakpoint and determine the breakpoint reached
	// ...However, Windows doesn't seem to communicate the flag properly
	if (ct.Dr6 & 0x1 && hardware_breakpoints.find(0) != hardware_breakpoints.end())
		slot = 0;
	else if (ct.Dr6 & 0x2 && hardware_breakpoints.find(1) != hardware_breakpoints.end())
		slot = 1;
	else if (ct.Dr6 & 0x4 && hardware_breakpoints.find(2) != hardware_breakpoints.end())
		slot = 2;
	else if (ct.Dr6 & 0x8 && hardware_breakpoints.find(3) != hardware_breakpoints.end())
		slot = 3;
	else
		continue_status = DBG_EXCEPTION_NOT_HANDLED;

	if ((slot >= 0 && slot <= 3) && bp_del_hw(slot))
		continue_status = DBG_CONTINUE;

	cout << "[*] Hardware breakpoint removed." << endl;

	return continue_status;
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
		// CloseHandle(handle);
		return address;
	}
}