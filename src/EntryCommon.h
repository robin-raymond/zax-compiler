
#pragma once

#include "types.h"
#include "helpers.h"

#include "ParserDirectiveTypes.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct EntryCommonTypes
{
  using Resolve = ParserDirectiveTypes::Resolve;
};

//-----------------------------------------------------------------------------
struct EntryCommon : public EntryCommonTypes
{
  const Puid id_{ puid() };

  TokenConstPtr location_;

  String name_;
  Context* context_{};
  CompileStatePtr state_;

  Resolve resolve_;

  Context* context() noexcept;
  const Context* context() const noexcept;

  ContextPtr contextPtr() noexcept;
  const ContextConstPtr contextPtr() const noexcept;
};

} // namespace zax
