#include <iostream>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>

#pragma once

using std::endl;
using std::cout;
using std::cin;

extern BOOL debugger_active;
extern DWORD pid;
extern HANDLE h_process;
extern HANDLE h_thread;
extern CONTEXT ct;

void createprocess(const wchar_t*);
void attach();
void run();
void get_debug_event();
bool detach();
HANDLE open_thread(DWORD);
CONTEXT get_thread_context(HANDLE h_thread = NULL, DWORD thread_id = NULL);
