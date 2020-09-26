#include "pch.h"
#include "Parser.h"
#include "CompilerException.h"
#include "CompileState.h"
#include "OperatorLut.h"
#include "Source.h"
#include "Tokenizer.h"

using namespace zax;

using namespace std::string_view_literals;


//-----------------------------------------------------------------------------
Parser::Parser(
  const Config& config,
  Callbacks* callbacks) noexcept :
  config_{ config },
  operatorLut_{ std::make_shared<OperatorLut>() },
  activeState_{ std::make_shared<CompileState>() }
{
  activeState_->tabStopWidth_ = config_.tabStopWidth_;
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
void Parser::parse() noexcept
{
  while (!shouldAbort()) {
    prime();
    processAssets();

    if (sources_.empty())
      break;

    auto& tokenizer{ getTokenizer() };
    if (tokenizer.empty()) {
      processedSources_.push_back(sources_.front());
      sources_.pop_front();
      continue;
    }

    Context context;
    context.parser_ = this;
    context.tokenizer_ = &tokenizer;
    process(context);
  }
}

//-----------------------------------------------------------------------------
void Parser::process(Context& context) noexcept
{
  while ((!shouldAbort()) && (!context->empty())) {
    if ((pendingSources_.size() > 0) || (pendingAssets_.size()> 0))
      break;

    if (consumeSeparators(context, false)) {
      context.singleLineState_ = {};
      continue;
    }
    if (consumeLineParserDirective(context))
      continue;
  }
}

//-----------------------------------------------------------------------------
void Parser::processAssets() noexcept
{
  if (pendingAssets_.size() < 1)
    return;

  for (auto& pending : pendingAssets_) {
    if (pending.generated_) {
      // TODO: execute pending compile time functions now
    }

    std::error_code ec;
    if (pending.renameFilePath_.empty()) {
      Path useOutputName{ pending.filePath_ };
      pending.renameFilePath_ = useOutputName.filename().string();
    }

    if (Path{ pending.renameFilePath_ }.has_root_path()) {
      out(Error::OutputFailure, pending.token_, StringMap{ {"$file$", pending.renameFilePath_ } });
      continue;
    }

    if (String::npos != pending.renameFilePath_.find("..")) {
      out(Error::OutputFailure, pending.token_, StringMap{ {"$file$", pending.renameFilePath_ } });
      continue;
    }

    String fullPath;
    auto useOutputPath{ makeIncludeFile(config_.outputPath_, pending.renameFilePath_, fullPath) };

    Path parentOutputPath{ Path{ useOutputPath }.parent_path() };
    if (!parentOutputPath.empty()) {
      std::filesystem::create_directories(parentOutputPath, ec);
    }

    Path sourcePath { pending.filePath_ };
    if (!std::filesystem::is_regular_file(sourcePath)) {
      out(Error::AssetNotFound, pending.token_, StringMap{ {"$file$", pending.filePath_ } });
      continue;
    }
    Path temp{ pending.renameFilePath_ };
    std::filesystem::copy_file(
      Path{ pending.filePath_ },
      Path{ pending.renameFilePath_ },
      std::filesystem::copy_options::update_existing,
      ec
    );
    if (ec) {
      out(Error::OutputFailure, pending.token_, StringMap{ {"$file$", pending.renameFilePath_ } });
      continue;
    }
  }
  pendingAssets_.clear();
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Parser::skipUntil(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept
{
  return skipUntil(std::move(until), tokenizer.begin());
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Parser::skipUntil(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept
{
  while (!iter.isEnd()) {
    if (until(*iter))
      break;
    ++iter;
  }

  return iter;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Parser::skipUntilAfter(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept
{
  return skipUntilAfter(std::move(until), tokenizer.begin());
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Parser::skipUntilAfter(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept
{
  return skipUntil(std::move(until), iter + 1);
}

//-----------------------------------------------------------------------------
bool Parser::consumeSeparator(Context& context, bool forcedOkay) noexcept
{
  auto token{ context->front() };
  if (!isSeparator(token))
    return false;

  if ((!forcedOkay) && (token->forcedSeparator_)) {
    out(Warning::StatementSeparatorOperatorRedundant, token);
  }
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeSeparators(Context& context, bool forcedOkay) noexcept
{
  bool consumed{};
  while (true) {
    auto token{ context->front() };
    if (!isSeparator(token))
      break;

    context->popFront();

    consumed = true;
    if ((!forcedOkay) && (token->forcedSeparator_)) {
      out(Warning::StatementSeparatorOperatorRedundant, token);
    }
  }
  return consumed;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Parser::consumeTo(Tokenizer::iterator  iter) noexcept
{
  if (iter.isEnd()) {
    iter.list().clear();
    return iter;
  }
  iter.list().erase(iter.list().begin(), iter);
  return iter;
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Parser::consumeAfter(Tokenizer::iterator  iter) noexcept
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
TokenConstPtr Parser::validOrLastValid(const TokenConstPtr& token, Tokenizer& tokenizer) const noexcept
{
  if (token)
    return token;

  if (tokenizer.empty())
    return {};
  return *(tokenizer.end() - 1);
}

//-----------------------------------------------------------------------------
CompileStatePtr Parser::pickState(Context& context, TokenConstPtr token) noexcept
{
  if (context.singleLineState_)
    return context.singleLineState_;
  if (token->compileState_)
    return token->compileState_;
  return activeState_;
}

//-----------------------------------------------------------------------------
TokenizerPtr Parser::extract(Tokenizer::iterator first, Tokenizer::iterator last) noexcept
{
  return std::make_shared<Tokenizer>(first.list(), first.list().extract(first, last));
}

//-----------------------------------------------------------------------------
std::optional<ParserTypes::QuoteResult> Parser::parseQuote(Tokenizer::iterator iter) noexcept
{
  if (iter.isEnd())
    return {};
  if (!isQuote(*iter))
    return {};

  QuoteResult result;
  result.quoteIter_ = iter;
  result.quote_ = (*iter)->token_;
  ++iter;

  while (!iter.isEnd()) {
    if (!isQuote(*iter))
      break;
    result.quote_ += (*iter)->token_;
    ++iter;
  }

  result.afterIter_ = iter;
  return result;
}

//-----------------------------------------------------------------------------
std::optional<ParserTypes::NumberResult> Parser::parseSimpleNumber(Tokenizer::iterator iter) noexcept
{
  if (iter.isEnd())
    return {};

  const OperatorLut& lut{ *iter.list().operatorLut_ };

  String prefix;
  if (isOperatorOrAlternative(lut, *iter, Operator::PlusPreUnary))
    prefix = "+"sv;
  if (isOperatorOrAlternative(lut, *iter, Operator::MinusPreUnary))
    prefix = "-"sv;

  if (!prefix.empty())
    ++iter;

  if (!isNumber(*iter))
    return {};

  NumberResult result;
  result.numberIter_ = iter;
  result.number_ = prefix + String{ (*iter)->token_ };

  ++iter;
  result.afterIter_ = iter;
  return result;
}

//-----------------------------------------------------------------------------
bool Parser::isOperator(const TokenConstPtr& token, Operator oper) noexcept
{
  if (!token)
    return false;
  if (TokenType::Operator != token->type_)
    return false;
  return oper == token->operator_;
}

//-----------------------------------------------------------------------------
bool Parser::isOperatorOrAlternative(const TokenConstPtr& token, Operator oper) noexcept
{
  assert(operatorLut_);
  return isOperatorOrAlternative(*operatorLut_, token, oper);
}

//-----------------------------------------------------------------------------
bool Parser::isOperatorOrAlternative(const OperatorLut& lut, const TokenConstPtr& token, Operator oper) noexcept
{
  if (!token)
    return false;
  if (TokenType::Operator != token->type_)
    return false;
  if (oper == token->operator_)
    return true;
  auto& conflicts{ lut.lookupConflicts(oper) };
  return conflicts.end() != conflicts.find(token->operator_);
}

//-----------------------------------------------------------------------------
bool Parser::isLiteral(const TokenConstPtr& token) noexcept
{
  if (!token)
    return false;
  return TokenType::Literal == token->type_;
}

//-----------------------------------------------------------------------------
bool Parser::isLiteral(const TokenConstPtr& token, StringView value) noexcept
{
  if (!isLiteral(token))
    return false;
  return value == token->token_;
}

//-----------------------------------------------------------------------------
bool Parser::isNumber(const TokenConstPtr& token) noexcept
{
  if (!token)
    return false;
  return TokenType::Number == token->type_;
}

//-----------------------------------------------------------------------------
bool Parser::isSeparator(const TokenConstPtr& token) noexcept
{
  if (!token)
    return false;
  return TokenType::Separator == token->type_;
}

//-----------------------------------------------------------------------------
bool Parser::isQuote(const TokenConstPtr& token) noexcept
{
  if (!token)
    return false;
  return TokenType::Quote == token->type_;
}

//-----------------------------------------------------------------------------
bool Parser::isCommaOrCloseDirective(const TokenConstPtr& token) noexcept
{
  if (isOperator(token, Operator::Comma))
    return true;
  if (isOperator(token, Operator::DirectiveClose))
    return true;
  return false;
}

//-----------------------------------------------------------------------------
bool Parser::isUnknownExtension(const StringView name) noexcept
{
  constexpr static StringView prefix{ "x-" };
  if (name.length() < prefix.length())
    return false;
  return prefix == name.substr(0, prefix.length());
}

//-----------------------------------------------------------------------------
std::function<bool(const TokenConstPtr& token)> Parser::isOperatorFunc(Operator oper) noexcept
{
  return [oper](const TokenConstPtr& token) noexcept -> bool {
    return isOperator(token, oper);
  };
}

//-----------------------------------------------------------------------------
std::function<bool(const TokenConstPtr& token)> Parser::isOperatorOrAlternativeFunc(Operator oper) noexcept
{
  return isOperatorOrAlternativeFunc(*operatorLut_, oper);
}

//-----------------------------------------------------------------------------
std::function<bool(const TokenConstPtr& token)> Parser::isOperatorOrAlternativeFunc(const OperatorLut& lut, Operator oper) noexcept
{
  return [oper, &lut](const TokenConstPtr& token) noexcept -> bool {
    if (!token)
      return false;
    if (TokenType::Operator != token->type_)
      return false;
    return token->operator_ == oper;
  };
}

//-----------------------------------------------------------------------------
void Parser::prime() noexcept
{
  if (shouldAbort())
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

  SourceList pushFrontSourceList;
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
          out(Warning::SourceNotFound, pending.token_, StringMap{ {"$file$", pending.filePath_} });
          break;
        }
      }
      continue;
    }

    if (!alreadyIncludedSources_.insert(pending.fullFilePath_).second)
      continue;

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
    pushFrontSourceList.push_back(source);
  }
  if (pushFrontSourceList.size() > 0) {
    sources_.insert(sources_.begin(), pushFrontSourceList.begin(), pushFrontSourceList.end());
  }
  pendingSources_.clear();
}


//-----------------------------------------------------------------------------
Tokenizer& Parser::getTokenizer() noexcept
{
  assert(!sources_.empty());
  return *(sources_.front()->tokenizer_);
}

//-----------------------------------------------------------------------------
void Parser::fatal(Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.fatal_(error, token, mapping);
}

//-----------------------------------------------------------------------------
void Parser::out(Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.error_(error, token, mapping);
}

//-----------------------------------------------------------------------------
void Parser::out(Warning warning, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.warning_(warning, token, mapping);
}

//-----------------------------------------------------------------------------
void Parser::out(Informational info, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  callbacks_.info_(info, token, mapping);
}

//-----------------------------------------------------------------------------
bool Parser::shouldAbort() noexcept
{
  return callbacks_.shouldAbort_();
}
