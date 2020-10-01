#include "pch.h"
#include "CompileState.h"

using namespace zax;

//-----------------------------------------------------------------------------
bool CompileState::isWarningAnError(WarningTypes::Warning warning) const noexcept
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
  result->variableDefaults_ = original->variableDefaults_;
  result->typeDefaults_ = original->typeDefaults_;
  result->functionDefaults_ = original->functionDefaults_;

  result->variableDefaultsStack_ = original->variableDefaultsStack_;
  result->typeDefaultsStack_ = original->typeDefaultsStack_;
  result->functionDefaultsStack_ = original->functionDefaultsStack_;
  result->exportStack_ = original->exportStack_;
  return result;
}
