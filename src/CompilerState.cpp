#include "pch.h"
#include "CompileState.h"

using namespace zax;

//-----------------------------------------------------------------------------
bool CompileState::isWarningAnError(WarningTypes::Warning warning) noexcept
{
  return warnings_.at(warning).forceError_;
}

//-----------------------------------------------------------------------------
CompileStatePtr CompileState::fork(const CompileStateConstPtr& original) noexcept
{
  if (!original)
    return {};
  auto result{ std::make_shared<CompileState>() };
  result->errors_ = original->errors_.fork();
  result->warnings_ = original->warnings_.fork();
  result->panics_ = original->panics_.fork();
  result->variableDefault_ = original->variableDefault_;
  result->typeDefault_ = original->typeDefault_;
  result->functionDefault_ = original->functionDefault_;
  return result;
}
