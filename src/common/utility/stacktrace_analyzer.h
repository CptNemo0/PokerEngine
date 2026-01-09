#ifndef COMMON_STACKTRACE_ANALYZER_H_
#define COMMON_STACKTRACE_ANALYZER_H_

namespace common::utility {

// This class implements a call stack analyzer.
// Since I am creating this project using Zed Editor I have no access to a
// debugger. I could port this project to VS and use it's powerful debugger,
// however that wouldn't be as ambitious as creating my own call stack analyzer.
class StacktraceAnalyzer {
  public:
    // Initialize the SA with name of the executable without the extension part.
    // Example: for server.exe pass in server.
    static void Initialize();

    // Prints out the current call stack.
    // If the function has not been traced with the `TRACE_CURRENT_FUNCTION`
    // macro it will show up as "Unrecognized functions".
    static void PrintOut();
};

} // namespace common::utility

#endif // !COMMON_STACKTRACE_ANALYZER_H_
