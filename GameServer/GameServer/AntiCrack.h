#pragma once

#include <windows.h>
#include <vector>
#include <string>

class CAntiCrack
{
public:
    CAntiCrack();
    virtual ~CAntiCrack();

    void MainProtection();

private:
    std::vector<std::string> dbgTools;

    void HideStrings(const char* str);
    void CheckDebugTools();
    bool CheckDbgPresentCloseHandle();
    bool Int2DCheck();
    bool IsDbgPresentPrefixCheck();
    bool CheckDebuggerAPI();
    bool CheckProcessList();
    bool DetectHardwareBreakpoints();
    bool DetectVirtualization();
    bool DetectInjectedDLLs();
    bool IsBeingDebugged();
    void StartProtectionThread();
    static DWORD WINAPI AntiDebugThread(LPVOID lpParam);

    std::string DecryptString(const char* str);
    void AsmCallSPK(const char* Char);
};

extern CAntiCrack gAntiCrack;
#pragma once
