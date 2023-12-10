#pragma once
#include <windows.h>
#include <iostream>
#include"detours.h"
#define WSAAPI                  FAR PASCAL
#define IDC_READ_BUTTON 1001 // Adjust the value as needed

//Constant
HMODULE myhmod;
FILE* pFile = nullptr;
HWND hwndOutput = nullptr;
HWND hwndInput = nullptr;
HWND hwndInputLen = nullptr;

//struct for correct data handling
typedef struct _OVERLAPPED* LPWSAOVERLAPPED;
typedef struct _WSABUF {
    ULONG len;     /* the length of the buffer */
    _Field_size_bytes_(len) CHAR FAR* buf; /* the pointer to the buffer */
} WSABUF, FAR* LPWSABUF;
typedef void(CALLBACK* LPWSAOVERLAPPED_COMPLETION_ROUTINE)(IN DWORD dwError, IN DWORD cbTransferred, IN LPWSAOVERLAPPED lpOverlapped, IN DWORD dwFlags);

//add text
void AppendText(const char* text) {
    int len = GetWindowTextLength(hwndOutput);
    SendMessage(hwndOutput, EM_SETSEL, len, len);
    SendMessage(hwndOutput, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text));
}


//Proto functions
typedef int (WINAPI* SendPtr)(SOCKET s, const char* buf, int len, int flags);
typedef int (WINAPI* WSASendPtr)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

//Lib
HMODULE hLib = LoadLibrary("WS2_32.dll");

//get the internal function 
SendPtr pSend = (SendPtr)GetProcAddress(hLib, "send");
WSASendPtr pWsaSend = (WSASendPtr)GetProcAddress(hLib, "WSASend");
SOCKET BackupSocket;

//For send()
int WSAAPI MySend(SOCKET s, const char* buf, int len, int flags)
{
    //std::wcout << "===============================" << std::endl;
    //std::cout << "Buffer : \n" << buf << std::endl;
    //std::cout << "Buffer length : " << len << std::endl;
    //std::cout << "Flag : " << flags << std::endl;

    AppendText("=======================================\n");
    AppendText("Buffer : \n");
    AppendText(buf);

    AppendText("\n");

    AppendText("Buffer Length: ");
    AppendText((const char*)len);

    AppendText("\n");

    AppendText("Flags : ");
    AppendText((const char*)flags);

    AppendText("\n");

    BackupSocket = s;

    return pSend(s, buf, len, flags);
}

//For WSASEnd()                      
int WSAAPI MyWSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    //std::wcout << "===============================" << std::endl;
    //std::wcout << L"Number of bytes sent : " << *lpNumberOfBytesSent << std::endl;
    //std::wcout << L"Buffer : \n" << *lpBuffers->buf << std::endl;

    AppendText("=======================================\n");
    AppendText("Buffer : \n");
    AppendText(lpBuffers->buf);

    AppendText("\n");

    AppendText("Number of bytes sent : ");
    AppendText((const char*)lpNumberOfBytesSent);

    return pWsaSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {


    //creating the menu and text box and more
    case WM_CREATE:
        hwndInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", nullptr, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE,
            10, 340, 760, 70, hwnd, nullptr, nullptr, nullptr);

        hwndInputLen = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", nullptr, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE,
            10, 415, 760, 30, hwnd, nullptr, nullptr, nullptr);

        hwndOutput = CreateWindowEx(WS_EX_CLIENTEDGE, (LPCSTR)"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, 760, 320, hwnd, nullptr, nullptr, nullptr);

        CreateWindow("BUTTON", "Send Packet", WS_CHILD | WS_VISIBLE, 10, 460, 90, 45, hwnd, (HMENU)IDC_READ_BUTTON, nullptr, nullptr);
        break;
    //for button commands and stuff
    case WM_COMMAND:
        // Handle button click
        if (LOWORD(wParam) == IDC_READ_BUTTON) {
            // Read text from the input box
            char buffer[1024]; // Adjust the buffer size as needed
            GetWindowText(hwndInput, buffer, sizeof(buffer));

            char BufferLen[20];
            GetWindowText(hwndInput, BufferLen, sizeof(BufferLen));

            // Do something with the text (for example, print it to the output)
            AppendText("\n\nText from Input Box:\n");
            AppendText(buffer);

            //Sending the packet with the parameters read from the input boxes
            pSend(BackupSocket, (const char*)buffer, (int)BufferLen, 0);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


int Main()
{
    //AllocConsole();
    //freopen_s(&pFile, "CONOUT$", "w", stdout);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = (LPCSTR)"MyWindowClass";

    RegisterClass(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, (LPCSTR)"RainBot's Packet Logger", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 560, nullptr, nullptr, wc.hInstance, nullptr);



    const char* Choice = "send";

    DetourRestoreAfterWith();

    //for send
    if (Choice == "send")
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pSend, (PVOID)MySend);
        DetourTransactionCommit();
    }


    if (Choice == "WSASend")
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pWsaSend, (PVOID)MyWSASend);
        DetourTransactionCommit();
    }


    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //exiting
    while (true)
    {
        if (GetAsyncKeyState(VK_END))
        {
            break;
        }

        Sleep(100);
    }

    MessageBoxA(0, "UnInjecting", "Bye", 0);
    FreeLibraryAndExitThread((HMODULE)myhmod, 0);
    //FreeConsole();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        myhmod = hModule;
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, 0, 0, 0);
    }
    return TRUE;
}
