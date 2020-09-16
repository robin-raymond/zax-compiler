#pragma once

#include "types.h"

namespace zax {

struct Version
{
  static constexpr const StringView name_{ "Zax Compiler" };
  static constexpr const StringView version_{ "0.0.1" };
  static constexpr const StringView copyright_{ "(c)2020 All rights reserved." };

  static constexpr auto name() noexcept { return name_; }
  static constexpr auto version() noexcept { return version_; }
  static constexpr auto copyright() noexcept { return copyright_; }
};

} // namespace zax
