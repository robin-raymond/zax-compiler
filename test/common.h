
#pragma once

#include "../src/types.h"

namespace zaxTest
{
void actualCheck(
  bool value,
  const char* str,
  const char* file,
  const char* function,
  size_t line
) noexcept;

#define TEST(x) actualCheck(x, #x, __FILE__, __FUNCTION__, __LINE__)

void testOperatorLut() noexcept(false);

void output(std::string_view testName) noexcept;

inline auto now() noexcept {
  return std::chrono::system_clock::now();
}

template<typename T>
inline auto diff(T start, T end) noexcept {
  return end - start;
}

} // namespace zaxTest
