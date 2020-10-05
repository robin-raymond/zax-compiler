
#include "pch.h"
#include "Parser.h"

#include "CompileState.h"
#include "Context.h"

using namespace zax;

using namespace std::string_view_literals;

//-----------------------------------------------------------------------------
bool Parser::consumeAlias(Context& context) noexcept
{
  auto iter{ context->begin() };

  if (consumeKeywordAlias(context, iter))
    return true;

  return false;
}

//-----------------------------------------------------------------------------
bool Parser::consumeKeywordAlias(Context& context, Tokenizer::iterator iter) noexcept
{
  auto literal{ *iter };

  if (!isLiteral(literal))
    return false;
  ++iter;

  if (!isOperator(context, *iter, Operator::MetaDeclare))
    return false;
  ++iter;

  if (!isKeyword(context, *iter, Keyword::Alias))
    return false;
  ++iter;

  if (isKeyword(context, *iter, Keyword::Operator)) {
    ++iter;

    if (!isKeyword(context, *iter, Keyword::Keyword)) {
      out(Error::TokenExpected, pickValid(validOrLastValid(*iter, iter), literal), StringMap{ { "$token$", String{ TokenTypes::KeywordTraits::toString(Keyword::Keyword) } } });
      (void)consumeTo(isSeparatorFunc(), iter);
      return true;
    }
    ++iter;

    auto operToken(*iter);
    auto oper{ extractOperator(context, *iter) };
    if (!oper) {
      out(Error::Syntax, pickValid(validOrLastValid(*iter, iter), literal));
      (void)consumeTo(isSeparatorFunc(), iter);
      return true;
    }

    String newKeyword{ literal->token_ };
    if (auto found = context.aliasing_.operators_.find(newKeyword); found != context.aliasing_.operators_.end()) {
      out(Error::KeywordAliasAlreadyDefined, pickValid(validOrLastValid(*iter, iter), literal), StringMap{ {"$alias$", newKeyword} });
      (void)consumeTo(isSeparatorFunc(), iter);
      return true;
    }
    (void)consumeAfter(iter);

    context.aliasing_.operators_.emplace(newKeyword, operToken);
    return false;
  }

  if (!isKeyword(context, *iter, Keyword::Keyword))
    return false;
  ++iter;

  auto keywordLiteral{ *iter };
  if (!isLiteral(keywordLiteral)) {
    out(Error::Syntax, pickValid(validOrLastValid(keywordLiteral, iter), literal));
    (void)consumeTo(isSeparatorFunc(), iter);
    return true;
  }

  String newKeyword{ literal->token_ };
  if (auto found = context.aliasing_.keywords_.find(newKeyword); found != context.aliasing_.keywords_.end()) {
    out(Error::KeywordAliasAlreadyDefined, pickValid(validOrLastValid(*iter, iter), literal), StringMap{ {"$alias$", newKeyword} });
    (void)consumeTo(isSeparatorFunc(), iter);
    return true;
  }

  (void)consumeAfter(iter);
  context.aliasing_.operators_.emplace(newKeyword, keywordLiteral);
  return true;
}
