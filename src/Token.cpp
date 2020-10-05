
#include "pch.h"
#include "types.h"

#include "Token.h"

using namespace zax;


//-----------------------------------------------------------------------------
std::optional<TokenTypes::Operator> Token::lookupOperator() const noexcept
{
  if (alias_) {
    auto temp{ alias_->lookupOperator() };
    if (temp)
      return temp;
  }
  if (TokenTypes::Type::Operator != type_)
    return {};

  return operator_;
}

//-----------------------------------------------------------------------------
std::optional<TokenTypes::Keyword> Token::lookupKeyword() const noexcept
{
  if (alias_) {
    auto temp{ alias_->lookupKeyword() };
    if (temp)
      return temp;
  }
  return keyword_;
}
