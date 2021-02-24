#include <iostream>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include <map>

#pragma once

using std::endl;
using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::hex;

extern BOOL debugger_active;
extern DWORD pid;
extern HANDLE h_process;
extern HANDLE h_thread;
extern CONTEXT ct;
extern PVOID exception_address;
extern std::map<LPVOID, const void*> software_breakpoints;

void createprocess(const wchar_t*);
void attach();
void run();
void get_debug_event();
bool detach();
HANDLE open_thread(DWORD);
CONTEXT get_thread_context(HANDLE h_thread = NULL, DWORD thread_id = NULL);
int exception_handler_breakpoint();
const void* read_process_memory(LPCVOID, int);
bool write_process_memory(LPVOID , LPCVOID);
bool bp_set_sw(LPVOID);
LPVOID func_resolve(LPCWSTR, LPCSTR);
