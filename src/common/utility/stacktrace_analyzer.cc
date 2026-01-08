#include "stacktrace_analyzer.h"

#include <charconv>
#include <mutex>
#include <print>
#include <stacktrace>
#include <string>
#include <string_view>
#include <unordered_map>

#include "aliasing.h"

namespace {

std::unordered_map<u64, std::string> location_method_mapping;
std::string executable_name = "";
std::mutex map_mutex;

// Parses entry from std::stacktrace::current()[n]::description()
// For executable serve.exe this entry is structured like that:
// server+0x1050.
// The identifier starts at executable_name.size() + 3 because from_chars, does
// not accept 0x prefix when parsing base 16.
// Returns part after "+" (location in the binary) converted to u64.
u64 ParseStacktraceDescription(std::string_view entry) {
  u64 result = 0;
  std::from_chars(entry.data() + executable_name.size() + 3,
                  entry.data() + entry.size(), result, 16);
  return result;
}

} // namespace

namespace common::utility {

void StacktraceAnalyzer::Initialize(const std::string& exe_name) {
  executable_name = exe_name;
}

void StacktraceAnalyzer::Map(const std::string& method_name) {
  // 2 :)
  // This is offset on the stacktrace
  const u64 id =
    ParseStacktraceDescription(std::stacktrace::current()[2].description());
  std::lock_guard lock{map_mutex};
  if (location_method_mapping.contains(id)) {
    return;
  }
  location_method_mapping[id] = method_name;
}

void StacktraceAnalyzer::PrintOut() {
  if (!STACKTRACE_LOG) {
    return;
  }
  const auto trace = std::stacktrace::current();

  std::lock_guard lock{map_mutex};
  // Start from 2nd position since:
  // 0th function is std::stacktrace::current();
  // 1st function is ::PrintOut()
  std::print("Stacktrace:\n");
  for (auto i{2uz}; i < trace.size(); i++) {
    const u64 id = ParseStacktraceDescription(trace[i].description());
    if (location_method_mapping.contains(id)) {
      std::print("\t{} >>> {}\n", i - 2, location_method_mapping[id]);
    } else {
      std::print("\t{} >>> Unrecognized method\n", i - 2);
    }
  }
  std::print("\n");
}

} // namespace common::utility
