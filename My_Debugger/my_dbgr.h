#include <iostream>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include <map>

#pragma once

#define HW_WRITE 1
#define HW_ACCESS 2
#define HW_EXECUTE 3

typedef struct {
	LPVOID address;
	SIZE_T length;
	DWORD condition;
} hw_break_t;

extern BOOL debugger_active;
extern DWORD pid;
extern HANDLE h_process;
extern HANDLE h_thread;
extern CONTEXT ct;
extern PVOID exception_address;
extern std::map<LPVOID, const void*> software_breakpoints;
extern std::map<int, hw_break_t> hardware_breakpoints;
extern std::vector<DWORD> thread_list;

void createprocess(const wchar_t*);
void attach();
void run();
void get_debug_event();
void enumerate_threads();

bool detach();

HANDLE open_thread(DWORD);
CONTEXT get_thread_context(HANDLE h_thread = NULL, DWORD thread_id = NULL);

int exception_handler_breakpoint();
int exception_handler_single_step();

const void* read_process_memory(LPCVOID, SIZE_T);
bool write_process_memory(LPVOID , LPCVOID);

BOOL bp_set_sw(LPVOID);
BOOL bp_set_hw(LPVOID, SIZE_T, DWORD);
BOOL bp_del_hw(int);

LPVOID func_resolve(LPCWSTR, LPCSTR);
