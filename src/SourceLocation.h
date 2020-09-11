
#pragma once

#include "types.h"

namespace zax
{

struct SourceLocation
{
  SourceFilePathPtr filePath_;
  int line_{};
  int column_{};
};

} // namespace zax
