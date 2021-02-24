#include "my_dbgr.h"

DWORD pid = NULL;

int main()
{
    cout << "Enter the PID of the process to attach to :"; cin >> pid;

    // ProcessHandleの取得
    attach();

    // printf関数のアドレス取得
    PVOID printf_address = func_resolve(L"msvcrt.dll", "printf");

    cout << "[*] Address of printf: 0x" << printf_address << endl;

    bp_set_sw(printf_address);
    
    run();

    return 0;
}