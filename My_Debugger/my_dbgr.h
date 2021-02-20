#include <iostream>
#include <Windows.h>

#pragma once

using std::endl;
using std::cout;
using std::cin;

extern BOOL debugger_active;
extern DWORD pid;
extern HANDLE h_process;

void createprocess(const wchar_t*);
HANDLE open_process();
void attach();
void run();
void get_debug_event();
bool detach();
