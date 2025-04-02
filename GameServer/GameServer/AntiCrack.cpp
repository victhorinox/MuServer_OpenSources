#include "stdafx.h"
#include "AntiCrack.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <intrin.h>

CAntiCrack gAntiCrack;

// Definición del typedef compatible con VS2010
typedef LONG(WINAPI* TNtQueryInformationProcess)(
    HANDLE,
    UINT,
    PVOID,
    ULONG,
    PULONG
    );

CAntiCrack::CAntiCrack()
{
    this->dbgTools.push_back("OllyDbg");
    this->dbgTools.push_back("HxD");
    this->dbgTools.push_back("ida64");
    this->dbgTools.push_back("idaq");
    this->dbgTools.push_back("x32dbg");
    this->dbgTools.push_back("x64dbg");
    this->dbgTools.push_back("Cheat Engine");
    this->dbgTools.push_back("Process Hacker");
    this->dbgTools.push_back("Wireshark");
}

CAntiCrack::~CAntiCrack() {}

void CAntiCrack::HideStrings(const char* str) {
    while (*str) volatile char c = *str++;
}

void CAntiCrack::AsmCallSPK(const char* Char) {
    HideStrings(Char);
    HANDLE Apps = FindWindowA(NULL, Char);
    if (Apps) {
        MessageBoxA(NULL, "Debugging tool detected!", "Warning", MB_OK | MB_ICONERROR);
        ExitProcess(0);
    }
}

bool CAntiCrack::CheckDbgPresentCloseHandle() {
    HANDLE Handle = (HANDLE)0x8000;
    __try {
        CloseHandle(Handle);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
    return false;
}

void CAntiCrack::CheckDebugTools() {
    for (size_t i = 0; i < this->dbgTools.size(); i++) {
        AsmCallSPK(this->dbgTools[i].c_str());
    }
}

bool CAntiCrack::DetectHardwareBreakpoints() {
    CONTEXT ctx;
    memset(&ctx, 0, sizeof(CONTEXT));
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    if (GetThreadContext(GetCurrentThread(), &ctx)) {
        if (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3) {
            return true;  // Breakpoint de hardware detectado
        }
    }
    return false;
}

bool CAntiCrack::IsBeingDebugged() {
    HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
    if (hNtDll) {
        TNtQueryInformationProcess pNtQueryInformationProcess =
            (TNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");

        if (pNtQueryInformationProcess) {
            DWORD dwFlags = 0;
            pNtQueryInformationProcess(GetCurrentProcess(), 0x1E, &dwFlags, sizeof(dwFlags), NULL);
            if (dwFlags & 0x00000001) {
                return true;
            }
        }
    }
    return false;
}

DWORD WINAPI CAntiCrack::AntiDebugThread(LPVOID lpParam) {
    while (true) {
        if (IsDebuggerPresent() || gAntiCrack.DetectHardwareBreakpoints() || gAntiCrack.IsBeingDebugged()) {
            MessageBoxA(NULL, "Debugging detected!", "Error", MB_OK | MB_ICONERROR);
            ExitProcess(0);
        }
        Sleep(5000);
    }
    return 0;
}

void CAntiCrack::StartProtectionThread() {
    CreateThread(NULL, 0, AntiDebugThread, NULL, 0, NULL);
}

void CAntiCrack::MainProtection() {
    CheckDebugTools();
    StartProtectionThread();
}
