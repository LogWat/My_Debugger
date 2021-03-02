#include "my_dbgr.h"

using std::endl;
using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::hex;

DWORD pid = NULL;

int main()
{
    cout << "Enter the PID of the process to attach to :"; cin >> pid;
    // createprocess(L"C:\\Users\\user\\Desktop\\code\\C\\test.exe");

    // ProcessHandleの取得
    attach();

    // printf関数のアドレス取得
    PVOID printf_address = func_resolve(L"msvcrt.dll", "printf");

    cout << "[*] Address of printf: 0x" << printf_address << endl;

    bp_set_mem(printf_address, 16);
    
    run();

    return 0;
}