#include "pch.h"
#include "Compiler.h"
#include "CompilerException.h"
#include "CompileState.h"
#include "OperatorLut.h"
#include "Source.h"
#include "Tokenizer.h"

using namespace zax;

using namespace std::string_view_literals;


//-----------------------------------------------------------------------------
Compiler::Compiler(
  const Config& config,
  Callbacks* callbacks) noexcept :
  config_{ config },
  operatorLut_{ std::make_shared<OperatorLut>() },
  activeState_{ std::make_shared<CompileState>() }
{
  if (!callbacks) {
    callbacks_.fatal_ = [](Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept {
      zax::fatal(error, token, mapping);
    };
    callbacks_.error_ = [](Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept {
      zax::output(error, token, mapping);
    };
    callbacks_.warning_ = [](Warning warning, const TokenConstPtr& token, const StringMap& mapping) noexcept {
      zax::output(warning, token, mapping);
    };
    callbacks_.info_ = [](Informational info, const TokenConstPtr& token, const StringMap& mapping) noexcept {
      zax::output(info, token, mapping);
    };
    callbacks_.shouldAbort_ = []() noexcept -> bool { return zax::shouldAbort(); };
  }
  else
    callbacks_ = std::move(*callbacks);
}

//-----------------------------------------------------------------------------
void Compiler::compile() noexcept
{
  while (!shouldAbort()) {
    prime();

    if (sources_.empty())
      break;

    auto& tokenizer{ getTokenizer() };
    if (tokenizer.empty()) {
      processedSources_.push_back(sources_.front());
      sources_.pop_front();
      continue;
    }

    process(tokenizer);
  }
}

//-----------------------------------------------------------------------------
void Compiler::process(Tokenizer& tokenizer) noexcept
{
  while ((!shouldAbort()) && (!tokenizer.empty())) {
    if (consumeSeparators(tokenizer, false))
      continue;
    if (consumeLineCompilerDirective(tokenizer))
      continue;
  }
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::skipUntil(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept
{
  return skipUntil(std::move(until), tokenizer.begin());
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::skipUntil(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept
{
  while (!iter.isEnd()) {
    if (until(*iter))
      break;
    ++iter;
  }

  return iter;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::skipUntilAfter(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept
{
  return skipUntilAfter(std::move(until), tokenizer.begin());
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::skipUntilAfter(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept
{
  return skipUntil(std::move(until), iter + 1);
}

//-----------------------------------------------------------------------------
bool Compiler::consumeSeparator(Tokenizer& tokenizer, bool forcedOkay) noexcept
{
  auto token{ tokenizer.front() };
  if (!isSeparator(token))
    return false;

  if ((!forcedOkay) && (token->forcedSeparator_)) {
    out(Warning::StatementSeparatorOperatorRedundant, token);
  }
  return true;
}

//-----------------------------------------------------------------------------
bool Compiler::consumeSeparators(Tokenizer& tokenizer, bool forcedOkay) noexcept
{
  bool consumed{};
  while (true) {
    auto token{ tokenizer.front() };
    if (!isSeparator(token))
      break;

    tokenizer.popFront();

    consumed = true;
    if ((!forcedOkay) && (token->forcedSeparator_)) {
      out(Warning::StatementSeparatorOperatorRedundant, token);
    }
  }
  return consumed;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::consumeTo(Tokenizer::iterator  iter) noexcept
{
  if (iter.isEnd()) {
    iter.list().clear();
    return iter;
  }
  iter.list().erase(iter.list().begin(), iter);
  return iter;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Compiler::consumeAfter(Tokenizer::iterator  iter) noexcept
{
  if (iter.isEnd()) {
    iter.list().clear();
    return iter;
  }
  ++iter;
  iter.list().erase(iter.list().begin(), iter);
  return iter;
}

//-----------------------------------------------------------------------------
TokenPtr Compiler::validOrLastValid(Tokenizer& tokenizer, const TokenPtr& token) noexcept
{
  if (token)
    return token;

  if (tokenizer.empty())
    return {};
  return *(tokenizer.end() - 1);
}

//-----------------------------------------------------------------------------
TokenizerPtr Compiler::extract(Tokenizer::iterator first, Tokenizer::iterator last) noexcept
{
  return std::make_shared<Tokenizer>(first.list(), first.list().extract(first, last));
}

//-----------------------------------------------------------------------------
bool Compiler::isOperator(const TokenConstPtr& token, Operator oper) noexcept
{
  if (!token)
    return false;
  if (TokenType::Operator != token->type_)
    return false;
  return oper == token->operator_;
}

//-----------------------------------------------------------------------------
bool Compiler::isLiteral(const TokenConstPtr& token) noexcept
{
  if (!token)
    return false;
  return TokenType::Literal == token->type_;
}

//-----------------------------------------------------------------------------
bool Compiler::isSeparator(const TokenConstPtr& token) noexcept
{
  if (!token)
    return false;
  return TokenType::Separator == token->type_;
}


//-----------------------------------------------------------------------------
bool Compiler::isQuote(const TokenConstPtr& token) noexcept
{
  if (!token)
    return false;
  return TokenType::Quote == token->type_;
}

//-----------------------------------------------------------------------------
bool Compiler::isCommaOrCloseDirective(const TokenConstPtr& token) noexcept
{
  if (isOperator(token, Operator::Comma))
    return true;
  if (isOperator(token, Operator::DirectiveClose))
    return true;
  return false;
}

//-----------------------------------------------------------------------------
std::function<bool(const TokenConstPtr& token)> Compiler::isOperatorFunc(Operator oper) noexcept
{
  return [oper](const TokenConstPtr& token) noexcept -> bool {
    if (!token)
      return false;
    if (TokenType::Operator != token->type_)
      return false;
    return token->operator_ == oper;
  };
}


//-----------------------------------------------------------------------------
void Compiler::prime() noexcept
{
  if (shouldAbort())
    return;

  if (sources_.size() > 0)
    return;

  if (pendingSources_.size() < 1) {
    for (auto& file : config_.inputFilePaths_) {
      auto token{ makeInternalToken(activeState_) };
      SourceAsset source{};
      source.token_ = token;
      source.compileState_ = activeState_;
      source.filePath_ = makeIncludeFile("ignored.bin", file, source.fullFilePath_);
      if (source.filePath_.empty()) {
        // try to load anyway
        source.filePath_ = file;
        source.fullFilePath_ = file;
      }
      source.required_ = decltype(source.required_)::Yes;
      source.commandLine_ = true;
      pendingSources_.push_back(source);
    }
    config_.inputFilePaths_.clear();
  }

  for (auto& pending : pendingSources_) {
    if (pending.generated_) {
      // TODO: execute pending compile time functions now
    }

    auto fileContents{ readBinaryFile(pending.filePath_) };
    if ((!fileContents.first) ||
        (fileContents.second < 1)) {
      switch (pending.required_) {
        case SourceAssetRequired::Yes: {
          if (pending.commandLine_)
            fatal(Error::SourceNotFound, pending.token_, StringMap{ {"$file$", pending.filePath_} });
          else
            out(Error::SourceNotFound, pending.token_, StringMap{ {"$file$", pending.filePath_} });
          break;
        }
        case SourceAssetRequired::No: {
          break;
        }
        case SourceAssetRequired::Warn: {
          out(Error::SourceNotFound, pending.token_, StringMap{ {"$file$", pending.filePath_} });
          break;
        }
      }
      continue;
    }

    auto source{ std::make_shared<Source>() };
    source->module_ = rootModule_;
    source->realPath_ = std::make_shared<SourceTypes::FilePath>();
    source->effectivePath_ = source->realPath_;
    source->realPath_->filePath_ = pending.filePath_;
    source->realPath_->fullFilePath_ = pending.fullFilePath_;
    source->realPath_->source_ = source;
    source->tokenizer_ = std::make_shared<Tokenizer>(
      source->realPath_,
      std::move(fileContents),
      pending.compileState_,
      operatorLut_);
    source->tokenizer_->skipComments_ = true;
    source->tokenizer_->errorCallback_ = callbacks_.error_;
    source->tokenizer_->warningCallback_ = callbacks_.warning_;
    sources_.push_back(source);
  }
  pendingSources_.clear();
}

//-----------------------------------------------------------------------------
Tokenizer& Compiler::getTokenizer() noexcept
{
  assert(!sources_.empty());
  return *(sources_.front()->tokenizer_);
}

//-----------------------------------------------------------------------------
void Compiler::fatal(Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.fatal_(error, token, mapping);
}

//-----------------------------------------------------------------------------
void Compiler::out(Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.error_(error, token, mapping);
}

//-----------------------------------------------------------------------------
void Compiler::out(Warning warning, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.warning_(warning, token, mapping);
}

//-----------------------------------------------------------------------------
void Compiler::out(Informational info, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.info_(info, token, mapping);
}

//-----------------------------------------------------------------------------
bool Compiler::shouldAbort() noexcept
{
  return callbacks_.shouldAbort_();
}
