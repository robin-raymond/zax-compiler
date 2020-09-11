
#pragma once

#include "types.h"

namespace zax
{

struct Source
{
  ModuleWeakPtr module_;

  SourceFilePathPtr realPath_;

  struct PathOverride {
    SourceFilePathPtr activePath_;
    int line_{};
    int increment_{ 1 };
  };

  optional<PathOverride> pathOverride_;
};

} // namespace zax
