
#pragma once

#include "types.h"
#include "helpers.h"

#include "ParserDirectiveTypes.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct TypeTypes
{
  using Resolve = ParserDirectiveTypes::Resolve;
};

//-----------------------------------------------------------------------------
struct Type : public TypeTypes
{
  const Puid id_{ puid() };

  TokenConstPtr location_;

  String name_;
  Context* context_{};
  CompileStatePtr state_;

  struct Flag
  {
    bool enabled_{};
    bool defaulted_{};
  };
  Flag mutable_{ .enabled_ = true };
  Flag immutable_{ .enabled_ = true };
  bool managed_{};

  Resolve resolve_;

  Context* context() noexcept;
  const Context* context() const noexcept;

  ContextPtr contextPtr() noexcept;
  const ContextConstPtr contextPtr() const noexcept;
};

} // namespace zax
