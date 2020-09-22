
#include "pch.h"
#include "Compiler.h"

//#include "CompilerException.h"
//#include "CompileState.h"
//#include "OperatorLut.h"
//#include "Source.h"
//#include "Tokenizer.h"

using namespace zax;

using namespace std::string_view_literals;

//-----------------------------------------------------------------------------
bool Compiler::consumeLineCompilerDirective(Tokenizer& tokenizer) noexcept
{
  if (isOperator(tokenizer.front(), Operator::DirectiveOpen))
    return false;

  auto iter{ tokenizer.begin() + 1 };

  auto literal{ *iter };
  if (!isLiteral(literal)) {
    out(Warning::DirectiveNotUnderstood, literal);
    (void)consumeAfter(skipUntil(tokenizer, isOperatorFunc(Operator::DirectiveClose)));
    return true;
  }
  ++iter;

  if ("asset"sv == literal->token_) {
    (void)consumeAfter(iter);
    return consumeAssetOrSourceDirective(tokenizer, false);
  }

  if ("source"sv == literal->token_) {
    (void)consumeAfter(iter);
    return consumeAssetOrSourceDirective(tokenizer, true);
  }

  return true;
}

//-----------------------------------------------------------------------------
bool Compiler::consumeAssetOrSourceDirective(Tokenizer& tokenizer, bool isSource) noexcept
{
  auto iter{ tokenizer.begin() };

  SourceAssetDirective asset;
  asset.token_ = validOrLastValid(iter, *iter);

  iter = extractDirectiveEqualsQuoteOrResolveLater(iter, asset.file_, asset.unresolvedFile_);

  bool foundRequired{};
  bool foundGenerated{};
  bool foundRename{};
  while (isOperator(*iter, Operator::Comma)) {
    ++iter;

    auto literal{ *iter };
    if (!isLiteral(literal)) {
      out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
      iter = consumeAfter(skipUntil(isCommaOrCloseDirective, iter));
      continue;
    }

    if ("required"sv == literal->token_) {
      String str;
      iter = extractDirectiveEqualsLiteral(iter + 1, str);

      auto value{ SourceAssetRequiredTraits::toEnum(str) };
      if (foundRequired || (!value)) {
        iter = consumeAfter(skipUntil(isCommaOrCloseDirective, iter));
        out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
        continue;
      }
      asset.required_ = *value;
      foundRequired = true;
      continue;
    }
    if ("generated"sv == literal->token_) {
      String str;
      iter = extractDirectiveEqualsLiteral(iter + 1, str);

      auto value{ YesNoTraits::toEnum(str) };
      if (foundGenerated || (!value)) {
        iter = consumeAfter(skipUntil(isCommaOrCloseDirective, iter));
        out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
        continue;
      }
      asset.generated_ = *value == YesNo::Yes ? true : false;
      foundGenerated = true;
      continue;
    }
    if ("rename"sv == literal->token_) {
      if (foundRename || isSource) {
        iter = consumeAfter(skipUntil(isCommaOrCloseDirective, iter));
        out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
        continue;
      }
      iter = extractDirectiveEqualsQuoteOrResolveLater(iter, asset.rename_, asset.unresolvedRename_);
      continue;
    }

    iter = consumeAfter(skipUntil(isCommaOrCloseDirective, iter));
    out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
  }

  if (!isOperator(*iter, Operator::DirectiveClose)) {
    out(Error::Syntax, validOrLastValid(iter, *iter));
    consumeTo(iter);
    return true;
  }

  if (isSource)
    handleSource(asset);
  else
    handleAsset(asset);

  return true;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::extractDirectiveEqualsQuoteOrResolveLater(
  Tokenizer::iterator iter,
  String& output,
  TokenizerPtr& outUnresolved,
  bool warnIfNotFound
) noexcept
{
  output = {};

  auto equalsToken{ *iter };
  if (!isOperator(equalsToken, Operator::Assign)) {
    if (warnIfNotFound) {
      out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, equalsToken));
      iter = consumeAfter(skipUntil(iter.list(), isCommaOrCloseDirective));
    }
    return iter;
  }

  ++iter;

  if (!*iter) {
    out(Error::Syntax, validOrLastValid(iter, equalsToken));
    iter = consumeAfter(skipUntil(iter.list(), isCommaOrCloseDirective));
    return iter;
  }

  if (!isQuote(*iter)) {
    auto afterIter{ skipUntil(isCommaOrCloseDirective, iter) };
    outUnresolved = extract(iter, afterIter);
    iter = afterIter;
  }
  else {
    while (isQuote(*iter)) {
      output += (*iter)->token_;
      ++iter;
    }
  }

  return iter;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::extractDirectiveEqualsLiteral(
  Tokenizer::iterator iter,
  String& output,
  bool warnIfNotFound) noexcept
{
  output = {};
  
  auto equalsToken{ *iter };
  if (!isOperator(equalsToken, Operator::Assign)) {
    if (warnIfNotFound) {
      out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, equalsToken));
      iter = consumeAfter(skipUntil(iter.list(), isCommaOrCloseDirective));
    }
    return iter;
  }
  ++iter;

  if (!isLiteral(*iter)) {
    out(Error::Syntax, validOrLastValid(iter, equalsToken));
    return consumeAfter(skipUntil(iter.list(), isCommaOrCloseDirective));
  }

  output = (*iter)->token_;
  return ++iter;
}

//-----------------------------------------------------------------------------
void Compiler::handleAsset(SourceAssetDirective& asset) noexcept
{
}

//-----------------------------------------------------------------------------
void Compiler::handleSource(SourceAssetDirective& source) noexcept
{
}
