#include "my_dbgr.h"

using std::endl;
using std::cout;
using std::cin;

DWORD pid = NULL;

int main()
{
    // createprocess(L"C:\\WINDOWS\\system32\\calc.exe");
    cout << "Enter the PID of the process to attach to :"; cin >> pid;

    attach();
    run();
    detach();

    return 0;
}