
#pragma once

#include "types.h"

#include "Token.h"
#include "TypeCommon.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct VariableTypes
{
  using Operator = TokenTypes::Operator;
};

//-----------------------------------------------------------------------------
struct Variable : public VariableTypes
{
  String name_;

  bool discard_{};
  bool once_{};
  bool private_{};
  bool varies_{ true };
  bool override_{};
  bool own_{};
  bool mutator_{};

  bool operator_{};
  Operator whichOperator_{};

  AliasPtr type_;
};

} // namespace zax
