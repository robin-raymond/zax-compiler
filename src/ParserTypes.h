
#pragma once

#include "types.h"
#include "Config.h"
#include "Errors.h"
#include "Warnings.h"
#include "Informationals.h"
#include "Token.h"
#include "Tokenizer.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct ParserTypes
{
  constexpr static StringView unknownPrefix{ "x-" };

  using Operator = TokenTypes::Operator;

  using Error = ErrorTypes::Error;
  using Warning = WarningTypes::Warning;
  using Informational = InformationalTypes::Informational;
  using TokenType = TokenTypes::Type;

  using ModuleMap = std::map<String, ModulePtr>;
  using ModuleList = std::list<ModulePtr>;
  using SourceList = std::list<SourcePtr>;

  struct Callbacks
  {
    std::function<void(Error, const TokenConstPtr& token, const StringMap&)> fatal_;
    std::function<void(Error, const TokenConstPtr& token, const StringMap&)> error_;
    std::function<void(Warning, const TokenConstPtr& token, const StringMap&)> warning_;
    std::function<void(Informational, const TokenConstPtr& token, const StringMap&)> info_;

    std::function<bool()> shouldAbort_;
  };

  struct QuoteResult {
    Tokenizer::iterator quoteIter_;
    Tokenizer::iterator afterIter_;
    String quote_;
  };
  struct NumberResult {
    Tokenizer::iterator numberIter_;
    Tokenizer::iterator afterIter_;
    String number_;
  };
  struct Extraction {
    ContextPtr context_;
    TokenizerPtr tokenizer_;

    bool hasValue() const noexcept { return static_cast<bool>(tokenizer_); }
  };
};

} // namespace zax
