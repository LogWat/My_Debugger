#include <iostream>
#include <Windows.h>
#include <vector>
#include <tlhelp32.h>

#pragma once

extern BOOL debugger_active;
extern DWORD pid;
extern HANDLE h_process;
extern std::vector<DWORD> thread_list;

void createprocess(const wchar_t*);
void attach();
void run();
void get_debug_event();
bool detach();
