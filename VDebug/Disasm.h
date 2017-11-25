#ifndef DISASM_VDEBUG_H_H_
#define DISASM_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include "capstone/capstone.h"
#include "mstring.h"

using namespace std;

struct DisasmInfo
{
    ustring m_wstrAddr;
    DWORD64 m_dwAddr;
    ustring m_wstrOpt;
    ustring m_wstrContent;
    ustring m_wstrByteCode;
    int m_iByteCount;
    BYTE m_vByteData[16];

    DisasmInfo()
    {
        m_dwAddr = 0;
        m_iByteCount = 0;
        ZeroMemory(m_vByteData, sizeof(m_vByteData));
    }
};

typedef BOOL (WINAPI *pfnDisasmProc)(const cs_insn *pAsmInfo, LPVOID pParam);

class CDisasmParser
{
public:
    CDisasmParser(HANDLE hProcess);
    virtual ~CDisasmParser();

    bool DisasmUntilReturn(DWORD64 dwAddr, vector<DisasmInfo> &vInfo) const;
    bool DisasmWithSize(DWORD64 dwAddr, DWORD dwMaxSize, vector<DisasmInfo> &vInfo) const;

protected:
    bool DisasmInternal(DWORD64 dwAddr, pfnDisasmProc pfn, LPVOID pParam) const;
    static BOOL WINAPI DisasmCallback(const cs_insn *pAsmInfo, LPVOID pParam);

protected:
    HANDLE m_hProcess;
};

#endif