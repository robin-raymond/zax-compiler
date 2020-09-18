#include "pch.h"
#include "CompileState.h"

using namespace zax;

//-----------------------------------------------------------------------------
bool CompileState::isWarningAnError(WarningTypes::Warning warning) noexcept
{
  return warnings_.at(warning).forceError_;
}
