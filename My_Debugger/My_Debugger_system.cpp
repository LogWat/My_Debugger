#include "my_dbgr.h"

using std::endl;
using std::cout;
using std::cin;
using std::vector;

BOOL debugger_active = FALSE;
HANDLE h_process = NULL;

vector<DWORD> thread_list; // �X���b�h���X�g


inline HANDLE open_process()
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

void createprocess(const wchar_t* path_to_exe)
{
	int creation_flags = DEBUG_PROCESS; // GUI�\���FCREATE_NEW_CONSOLE

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

	pid = pi.dwProcessId;		//�v���Z�XID�擾
	h_process = open_process(); //�v���Z�X�̃n���h���擾

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

// �v���Z�X�̊J�n************************************************
void attach()
{
	h_process = open_process();

	//Process�ւ̃A�^�b�`���s(���s���͖߂�)
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
	DWORD continue_status = DBG_CONTINUE;

	if (WaitForDebugEvent(&de, INFINITE))
	{
		cout << "Press a key to continue..." << endl;
		
		int flag = cin.get(); // �L�[���͑҂�

		debugger_active = FALSE;

		ContinueDebugEvent(
			de.dwProcessId,
			de.dwThreadId,
			continue_status
		);
	}
}


// �v���Z�X�̏I��**********************************************
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


// ThreadID�̎擾*******************************************************
HANDLE open_thread(DWORD thread_id)
{
	HANDLE h_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, thread_id);

	if (h_thread != 0)
	{
		return h_thread;
	}
	else {
		cout << "[*] Could not obtain a valid thread handle." << endl;
		return FALSE;
	}
}

void enumerate_threads()
{
	THREADENTRY32 entry;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);

	if (snapshot != NULL)
	{
		entry.dwSize = sizeof(entry); // �\���̂̃T�C�Y�ŏ��������Ȃ��Ǝ��s
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

CONTEXT get_thread_context(DWORD thread_id, HANDLE h_thread)
{
	CONTEXT ct;
	ct.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	// �X���b�h�̃n���h���擾
	if (h_thread == NULL)
		h_thread = open_thread(thread_id);
	if (GetThreadContext(h_thread, &ct))
	{
		CloseHandle(h_thread);
		return ct;
	}
}