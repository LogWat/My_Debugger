#include "my_dbgr.h"

using std::endl;
using std::cout;
using std::cin;
using std::ios;
using std::hex;

DWORD pid = NULL;

int main()
{
    // createprocess(L"C:\\WINDOWS\\system32\\calc.exe");
    cout << "Enter the PID of the process to attach to :"; cin >> pid;

    attach(); // OpenProcess

    enumerate_threads(); // Process内で実行中のスレッドを取得-->列挙-->thread_listへの要素追加

    for (int i = 0; i < sizeof(thread_list); i++)
    {
        CONTEXT thread_context = get_thread_context(thread_list[i]);

        cout << "[*] Dumping registers for thread ID: 0x" << hex << thread_list[i] << endl;;
        cout << "[**] EIP: 0x" << hex << thread_context.Eip << endl;
        cout << "[**] ESP: 0x" << hex << thread_context.Esp << endl;
        cout << "[**] EBP: 0x" << hex << thread_context.Ebp << endl;
        cout << "[**] EAX: 0x" << hex << thread_context.Eax << endl;
        cout << "[**] EBX: 0x" << hex << thread_context.Ebx << endl;
        cout << "[**] ECX: 0x" << hex << thread_context.Ecx << endl;
        cout << "[**] EDX: 0x" << hex << thread_context.Edx << endl;
        cout << "[*] END DUMP" << endl;
    }

    detach();

    return 0;
}