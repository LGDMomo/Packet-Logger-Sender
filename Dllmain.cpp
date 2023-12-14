#pragma once
#include <windows.h>
#include <iostream>
#include"detours.h"
#include<string>
#include<winsock.h>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")
#define WSAAPI                  FAR PASCAL
#define IDC_READ_BUTTON 1001 // Adjust the value as needed
#define IDC_RESET_BUTTON 1002 // Adjust the value as needed


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
struct sockaddr_in clientService;

//add text
void AppendText(const char* text) {
    int len = GetWindowTextLength(hwndOutput);
    SendMessage(hwndOutput, EM_SETSEL, len, len);
    SendMessage(hwndOutput, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text));
}


//backup in case
SOCKET ConnectSocket = INVALID_SOCKET;

//Proto functions
typedef int (WINAPI* SendPtr)(SOCKET s, const char* buf, int len, int flags);
typedef int (WINAPI* WSASendPtr)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef int (WINAPI* WSARecvPtr)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef int (WINAPI* ConnectCustom)(SOCKET s, const SOCKADDR* sAddr, int nameLen);
typedef int (WINAPI* RecvCustom)(SOCKET s, const char* buf, int len, int flags);
typedef int (WINAPI* SendToCustom)(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int ToLen);

//Lib
HMODULE hLib = LoadLibrary("WS2_32.dll");


//get the internal function 
SendPtr pSend = (SendPtr)GetProcAddress(hLib, "send");
ConnectCustom pConnect = (ConnectCustom)GetProcAddress(hLib, "connect");
WSASendPtr pWsaSend = (WSASendPtr)GetProcAddress(hLib, "WSASend");
RecvCustom pRecv = (RecvCustom)GetProcAddress(hLib, "recv");
WSARecvPtr pWSARecv = (WSARecvPtr)GetProcAddress(hLib, "WSARecv");
SendToCustom pSendTo = (SendToCustom)GetProcAddress(hLib, "SendTo");


int SERVER_IP;
int PORT;

int TO_SERVER_IP;
int TO_PORT;
//For send() hook it to read the buffer and print it  
int WSAAPI MySend(SOCKET s, const char* buf, int len, int flags)
{
    AppendText("=======================================\n");
    AppendText("Send() Sent Data : \n");
    AppendText(buf);

    AppendText("\n");

    AppendText("Buffer Length: ");
    std::string myLen = std::to_string(len);
    const char* LenConstChar = myLen.c_str();
    AppendText(LenConstChar);

    AppendText("\n");

    AppendText("Flags : ");
    std::string myFlags = std::to_string(flags);
    const char* FlagsConstChar = myFlags.c_str();
    AppendText(FlagsConstChar);

    AppendText("\n");
    return pSend(s, buf, len, flags);
}

//For recv() hook it to read the buffer and print it  
int WSAAPI MyRecv(SOCKET s, const char* buf, int len, int flags)
{
    AppendText("=======================================\n");

    // Call the original recv() function
    int result = pRecv(s, buf, len, flags);

    if (result >= 0)
    {
        AppendText("Recv : Received Data : \n");
        AppendText(buf);
        AppendText("\n");

        AppendText("Buffer Length : \n");
        std::string myLen = std::to_string(len);
        const char* LenConstChar = myLen.c_str();
        AppendText(LenConstChar);

        AppendText("\n");
    }
    else
    {
        // Handle error if needed
        // You can log the error code or take appropriate action
        AppendText("Recv() failed with error code: ");
        std::string errorCode = std::to_string(WSAGetLastError());
        AppendText(errorCode.c_str());
        AppendText("\n");
    }

    AppendText("\n");
    return result;
}

//For WSASEnd() hook it to read the buffer and print it                    
int WSAAPI MyWSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    AppendText("=======================================\n");
    AppendText("WSASend() Sent Data : \n");

    for (DWORD i = 0; i < dwBufferCount; ++i)
    {
        // Assuming lpBuffers[i].buf is a pointer to the buffer and lpBuffers[i].len is the length
        const char* bufferContent = reinterpret_cast<const char*>(lpBuffers[i].buf);
        DWORD bufferLength = lpBuffers[i].len;

        std::string myLen = std::to_string(bufferLength);
        const char* LenConstChar = myLen.c_str();

        // Append buffer content to the text
        AppendText(bufferContent);
        AppendText("\n");


        AppendText("Buffer length : \n");
        AppendText(LenConstChar);
    }


    AppendText("\n");

    return pWsaSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}


// For WSARecv() hook it to read the buffer and print it
int WSAAPI MyWSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    AppendText("=======================================\n");

    // Call the original WSARecv() function
    int result = pWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, dwFlags, lpOverlapped, lpCompletionRoutine);

    if (result == 0)
    {
        AppendText("WSARecv : Received Data : \n");
        for (DWORD i = 0; i < dwBufferCount; ++i)
        {
            AppendText(lpBuffers[i].buf);
        }
        AppendText("\n");

        AppendText("Buffer Length : \n");
        std::string myLen = std::to_string(*lpNumberOfBytesRecvd);
        const char* LenConstChar = myLen.c_str();
        AppendText(LenConstChar);

        AppendText("\n");
    }

    AppendText("\n");
    return result;
}

//Hook the connect to get the IP and the PORT of the server for (send , recv)
int WSAAPI MyConnect(SOCKET s, const SOCKADDR* sAddr, int nameLen)
{
    // Assuming sAddr is of type SOCKADDR_IN
    const sockaddr_in* clientService = reinterpret_cast<const sockaddr_in*>(sAddr);
    unsigned long ipAddress = clientService->sin_addr.s_addr;

    std::string MyIpAddress = std::to_string(ipAddress);
    const char* IpConstChar = MyIpAddress.c_str();

    SERVER_IP = static_cast<int>(ipAddress);
    PORT = static_cast<int>(ntohs(clientService->sin_port));

    AppendText("\n");
    AppendText("IP address being used is: ");
    AppendText(IpConstChar);
    AppendText("\n");

    AppendText("Port being used is: ");
    AppendText(std::to_string(ntohs(clientService->sin_port)).c_str());
    AppendText("\n");

    //unhook the function cuz we dont need it anymore
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)pConnect, (PVOID)MyConnect);
    DetourTransactionCommit();

    return 0;
}

// For sendto() hook it to read the buffer and print it
int WSAAPI MySendTo(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen)
{
    AppendText("=======================================\n");


    // Assuming sAddr is of type SOCKADDR_IN
    //Extracting the ip and port of the reciever 
    const sockaddr_in* clientService = reinterpret_cast<const sockaddr_in*>(to);
    unsigned long ipAddress = clientService->sin_addr.s_addr;

    std::string MyIpAddress = std::to_string(ipAddress);
    const char* IpConstChar = MyIpAddress.c_str();

    TO_SERVER_IP = static_cast<int>(ipAddress);
    TO_PORT = static_cast<int>(ntohs(clientService->sin_port));


    // Call the original sendto() function
    int result = pSendTo(s, buf, len, flags, to, tolen);

    if (result >= 0)
    {
        AppendText("SentTo() Sent Data : \n");
        AppendText(buf);
        AppendText("\n");

        AppendText("Buffer Length : \n");
        std::string myLen = std::to_string(len);
        const char* LenConstChar = myLen.c_str();
        AppendText(LenConstChar);

        AppendText("\n");
    }
    else
    {
        AppendText("sendto() failed with error code: ");
        std::string errorCode = std::to_string(WSAGetLastError());
        AppendText(errorCode.c_str());
        AppendText("\n");
    }

    AppendText("\n");
    return result;
}



//Events and stuff for the windows
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {


        //creating the menu and text box and more
    case WM_CREATE:

        CreateWindow("STATIC", "Input Packet", WS_CHILD | WS_VISIBLE, 10, 355, 100, 20, hwnd, nullptr, nullptr, nullptr);
        CreateWindow("STATIC", "Output Packet", WS_CHILD | WS_VISIBLE, 10, 10, 100, 20, hwnd, nullptr, nullptr, nullptr);
        CreateWindow("STATIC", "Input Buffer Length", WS_CHILD | WS_VISIBLE, 10, 495, 150, 20, hwnd, nullptr, nullptr, nullptr);

        hwndInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", nullptr, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL,
            10, 375, 760, 115, hwnd, nullptr, nullptr, nullptr);

        hwndInputLen = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", nullptr, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 515, 765, 30, hwnd, nullptr, nullptr, nullptr);

        hwndOutput = CreateWindowEx(WS_EX_CLIENTEDGE, (LPCSTR)"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 30, 760, 320, hwnd, nullptr, nullptr, nullptr);

        CreateWindow("BUTTON", "Send Packet", WS_CHILD | WS_VISIBLE, 10, 550, 90, 45, hwnd, (HMENU)IDC_READ_BUTTON, nullptr, nullptr);
        CreateWindow("BUTTON", "Reset Output", WS_CHILD | WS_VISIBLE, 120, 550, 90, 45, hwnd, (HMENU)IDC_RESET_BUTTON, nullptr, nullptr);
        break;
        //for button commands and stuff
    case WM_COMMAND:
        // Handle button click
        if (LOWORD(wParam) == IDC_READ_BUTTON) {
            // Read text from the input box
            char buffer[2048]; // Adjust the buffer size as needed
            GetWindowText(hwndInput, buffer, sizeof(buffer));

            char BufferLen[20];
            GetWindowText(hwndInputLen, BufferLen, sizeof(BufferLen));

            ///////////////////////////////////////////////////////////////////////SEND
            // Create a SOCKET for connecting to server
            ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            clientService.sin_family = AF_INET;
            clientService.sin_addr.s_addr = SERVER_IP;
            clientService.sin_port = htons(PORT);

            connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));

            //send()
            int SentBytes = pSend(ConnectSocket, (const char*)buffer, std::stoi(BufferLen), 0);
            if (SentBytes != SOCKET_ERROR) {
                AppendText("Packet sent successfully !");
                AppendText("\n");
            }

            shutdown(ConnectSocket, 1);
            closesocket(ConnectSocket);

            //////////////////////////////////////////////////////////////////////////////////////////

            // sendto()
            // 
            //sockaddr_in RecvAddr;
            //ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            //
            //RecvAddr.sin_family = AF_INET;
            //RecvAddr.sin_port = htons(PORT);
            //RecvAddr.sin_addr.s_addr = SERVER_IP;
            //iResult = sendto(ConnectSocket,(const char*)buffer, std::stoi(BufferLen), 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
            //if (iResult >= 0){
            //AppendText("Packet sent successfully !");
            //AppendText("\n");
            //}
        }
        if (LOWORD(wParam) == IDC_RESET_BUTTON) {
            //Reset the output box text
            SendMessage(hwndOutput, WM_SETTEXT, 0, (LPARAM)"");
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
    AllocConsole();
    freopen_s(&pFile, "CONOUT$", "w", stdout);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = (LPCSTR)"MyWindowClass";

    RegisterClass(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, (LPCSTR)"RainBot's Packet Logger", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 800, 635, nullptr, nullptr, wc.hInstance, nullptr);

    
    std::cout << "Before init \n";
    //Init hook
    if (DetourRestoreAfterWith())
    {
        AppendText("Before Init\n");
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());


    DetourAttach(&(PVOID&)pSend, (PVOID)MySend);
    DetourAttach(&(PVOID&)pConnect, (PVOID)MyConnect);
    DetourAttach(&(PVOID&)pRecv, (PVOID)MyRecv);
    DetourAttach(&(PVOID&)pWsaSend, (PVOID)MyWSASend);
    DetourAttach(&(PVOID&)pWSARecv, (PVOID)MyWSARecv);
    DetourAttach(&(PVOID&)pSendTo, (PVOID)MySendTo);



    DetourTransactionCommit();

    AppendText("After all Init\n");
    std::cout << "After all Init \n";
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
    FreeConsole();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        myhmod = hModule;
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, 0, 0, 0);
    }
    return TRUE;
}
