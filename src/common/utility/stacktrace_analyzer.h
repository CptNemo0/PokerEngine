#ifndef COMMON_STACKTRACE_ANALYZER_H_
#define COMMON_STACKTRACE_ANALYZER_H_

#include <source_location> // IWYU pragma: keep
#include <string>

#ifndef STACKTRACE_LOG
// Change to 0 to totally compile out insertion and printing logic.
// I know that this is an ancient C - like approach, but it's simple and
// effective,
#define STACKTRACE_LOG 1
#endif // !STACKTRACE_LOG

// Macro responsible for creating a record inside internal mapping. Call inside
// a function to recognize it globally.
#define TRACE_CURRENT_FUNCTION()                                               \
  if (STACKTRACE_LOG) {                                                        \
    std::string method_name = std::source_location::current().function_name(); \
    common::utility::StacktraceAnalyzer::Map(method_name);                     \
  }

namespace common::utility {

// This class implements a call stack analyzer.
// Each method that a developer wants to recognize on runtime should call
// `TRACE_CURRENT_FUNCTION` macro in it - ideally in the beginning.
// This is a limited implementation since one cannot recognize the library
// functions which can take up most of the stack. However, smart usage will
// greatly help with recognizing execution paths.
//
// Since I am creating this project using Zed Editor I have no access to a
// debugger. I could port this project to VS and use it's powerful debugger,
// however that wouldn't be as ambitious as creating my own call stack analyzer.
// Oh, and it's thread-safe :).
// Behold!
class StacktraceAnalyzer {
  public:
    // Initialize the SA with name of the executable without the extension part.
    // Example: for server.exe pass in server.
    static void Initialize(const std::string& exe_name);

    // Prints out the current call stack.
    // If the function has not been traced with the `TRACE_CURRENT_FUNCTION`
    // macro it will show up as "Unrecognized functions".
    static void PrintOut();

    // DO NOT USE MAP() RAW!!! It will mess up the analyzer BIG TIME.
    // Please use provided macro.
    static void Map(const std::string& method_name);
};

} // namespace common::utility

#endif // !COMMON_STACKTRACE_ANALYZER_H_
