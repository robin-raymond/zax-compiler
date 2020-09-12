
#pragma once

#include "types.h"
#include "helpers.h"
#include "CompileState.h"

namespace zax
{

struct Module
{
  Puid id_{ puid() };
  CompileState compileState_;
};

} // namespace zax
