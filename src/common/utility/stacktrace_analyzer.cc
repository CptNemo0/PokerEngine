#include "stacktrace_analyzer.h"

#include <windows.h> // Order of inclusion actually matters on windows. A.D. 2026.

#include <cstdio>
#include <print>
#include <stacktrace>

#include <dbghelp.h>
#include <errhandlingapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <winbase.h>
#include <winnt.h>
#include <winuser.h>
#pragma comment(lib, "dbghelp.lib")

#include "aliasing.h"

namespace {

// Retrives error message and displays a pop up with it.
// This is taken straight out of Windows API reference.
void ErrorExit() {
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, NULL) == 0) {
    MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
    ExitProcess(dw);
  }

  MessageBox(NULL, static_cast<LPCTSTR>(lpMsgBuf), TEXT("Error"), MB_OK);

  LocalFree(lpMsgBuf);
  ExitProcess(dw);
}

} // namespace

namespace common::utility {

void StacktraceAnalyzer::Initialize() {
  // If the functions are not displayed correctly, there might be an issue with
  // a second parameter. It should be a path do .pdb file. When it's set to 0,
  // program will try to find the .pdb in the working directory and under some
  // eviromental variables. Refer to docs.
  if (!SymInitialize(GetCurrentProcess(), 0, true)) {
    ErrorExit();
  }
}

void StacktraceAnalyzer::PrintOut() {
  const auto trace = std::stacktrace::current();
  // Start from 1nd position since:
  // 0th function is always common::utility::StacktraceAnalyzer::PrintOut
  // "Last" (chronologically they were first) 6 calls are irrelevant as they are
  // main thread creation and program startup:
  // server!invoke_main+0x39
  // server!__scrt_common_main_seh+0x132
  // server!__scrt_common_main+0xE
  // server!mainCRTStartup+0xE
  // KERNEL32!BaseThreadInitThunk+0x17
  // ntdll!RtlUserThreadStart+0x2C
  // If you need to debug those I doubt that this little tool is helping
  // you - you're cooked.
  std::print("Stacktrace:\n");
  for (auto i{1uz}; i < trace.size() - 6; i++) {
    std::print("\t {} >>>{}\n", i - 1, trace[i].description());
  }
  std::print("\n");
}

} // namespace common::utility
