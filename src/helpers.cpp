
#include "pch.h"
#include "helpers.h"

using namespace zax;

Puid puid() noexcept {
  static std::atomic<Puid> singleton{ 1 };
  return singleton++;
}
