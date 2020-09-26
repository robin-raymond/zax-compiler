
#include "pch.h"
#include "Parser.h"

//#include "ParserException.h"
#include "CompileState.h"
//#include "OperatorLut.h"
//#include "Source.h"
//#include "Tokenizer.h"

using namespace zax;

using namespace std::string_view_literals;

//-----------------------------------------------------------------------------
std::optional<Parser::DirectiveResult> Parser::parseDirective(
  Tokenizer::iterator iter,
  std::function<bool(bool, Tokenizer::iterator, StringView)>&& noValueFunc,
  std::function<bool(bool, Tokenizer::iterator, StringView, StringView)>&& literalValueFunc,
  std::function<bool(bool, Tokenizer::iterator, StringView, StringView)>&& quoteValueFunc,
  std::function<bool(bool, Tokenizer::iterator, StringView, StringView)>&& numberValueFunc,
  std::function<bool(bool, Tokenizer::iterator, StringView, TokenizerPtr)>&& extractedValueFunc) noexcept
{
  if (iter.isEnd())
    return {};

  if (!isOperator(*iter, Operator::DirectiveOpen))
    return {};

  const OperatorLut& lut{ *iter.list().operatorLut_ };

  bool lastWasComma{};
  bool syntax{};
  bool forceSyntax{};
  bool understood{ true };

  DirectiveResult result;
  result.openIter_ = iter;
  ++iter;

  while (!iter.isEnd())
  {
    bool primary{};
    if (isSeparator(*iter))
      break;
    if (isOperator(*iter, Operator::DirectiveClose))
      break;
    if (isOperator(*iter, Operator::Comma)) {
      if (lastWasComma)
        break;
      if (!result.literalIter_)
        break;
      ++iter;
      lastWasComma = true;
      continue;
    }
    lastWasComma = false;

    auto literal{ parseDirectiveLiteral(iter) };
    if (!literal)
      break;

    if (!result.literalIter_) {
      primary = true;
      result.literalIter_ = literal->literalIter_;
    }
    iter = literal->afterIter_;

    if (!isOperator(*iter, Operator::Assign)) {
      if (!isCommaOrCloseDirective(*iter))
        break;

      bool check{ noValueFunc && noValueFunc(primary, literal->literalIter_, literal->name_) };
      if (!check) {
        out(Warning::DirectiveNotUnderstood, *literal->literalIter_);
        understood = false;
        break;
      }
      continue;
    }

    ++iter;
    if (iter.isEnd())
      break;

    if (isSeparator(*iter))
      break;
    if (isCommaOrCloseDirective(*iter)) {
      forceSyntax = true;
      break;
    }

    if (auto assigned{ parseDirectiveLiteral(iter) }; assigned) {
      if (isCommaOrCloseDirective(*assigned->afterIter_)) {
        iter = assigned->afterIter_;
        auto check{ literalValueFunc && literalValueFunc(primary, literal->literalIter_, literal->name_, assigned->name_) };
        if ((!check) && (!isUnknownExtension(literal->name_))) {
          out(Warning::DirectiveNotUnderstood, *literal->literalIter_);
          understood = false;
          break;
        }
        continue;
      }
    }

    if (auto assigned{ parseQuote(iter) }; assigned) {
      if (isCommaOrCloseDirective(*assigned->afterIter_)) {
        iter = assigned->afterIter_;
        auto check{ quoteValueFunc && quoteValueFunc(primary, literal->literalIter_, literal->name_, assigned->quote_) };
        if ((!check) && (!isUnknownExtension(literal->name_))) {
          out(Warning::DirectiveNotUnderstood, *literal->literalIter_);
          understood = false;
          break;
        }
        continue;
      }
    }

    if (auto assigned{ parseSimpleNumber(iter) }; assigned) {
      if (isCommaOrCloseDirective(*assigned->afterIter_)) {
        iter = assigned->afterIter_;
        auto check{ numberValueFunc && numberValueFunc(primary, literal->literalIter_, literal->name_, assigned->number_) };
        if ((!check) && (!isUnknownExtension(literal->name_))) {
          out(Warning::DirectiveNotUnderstood, *literal->literalIter_);
          understood = false;
          break;
        }
        continue;
      }
    }
    if ((!extractedValueFunc) && (!isUnknownExtension(literal->name_))) {
      out(Warning::DirectiveNotUnderstood, *iter);
      understood = false;
      break;
    }

    auto startIter{ iter };

    while (!iter.isEnd()) {
      if (isSeparator(*iter))
        break;
      if (isCommaOrCloseDirective(*iter))
        break;
      ++iter;
    }

    // scope: check will extract function
    {
      auto extractedTokenizer{ extract(startIter, iter) };
      auto check{ extractedValueFunc && extractedValueFunc(primary, literal->literalIter_, literal->name_, extractedTokenizer) };
      if ((!check) && (!isUnknownExtension(literal->name_))) {
        out(Warning::DirectiveNotUnderstood, *literal->literalIter_);
        understood = false;
        break;
      }
    }
  }

  if (!understood) {
    result.success_ = false;
  }

  if (!result.literalIter_) {
    out(Error::Syntax, pickValid(validOrLastValid(*iter, iter), *result.openIter_));
    syntax = true;
  }

  if (understood) {
    if ((!isOperator(*iter, Operator::DirectiveClose)) || forceSyntax) {
      if (!syntax) {
        out(Error::Syntax, pickValid(validOrLastValid(*iter, iter), *result.literalIter_, *result.openIter_));
        syntax = true;
      }
    }
  }

  while (!iter.isEnd()) {
    if (isSeparator(*iter))
      break;
    if (isOperator(*iter, Operator::DirectiveClose)) {
      ++iter;
      break;
    }
    ++iter;
  }

  result.afterIter_ = iter;

  if (syntax)
    result.success_ = false;

  return result;
}

//-----------------------------------------------------------------------------
std::optional<ParserTypes::DirectiveLiteral> Parser::parseDirectiveLiteral(Tokenizer::iterator iter) noexcept
{
  if (iter.isEnd())
    return {};

  if (!isLiteral(*iter))
    return {};

  const OperatorLut& lut{ *iter.list().operatorLut_ };

  Parser::DirectiveLiteral result;
  result.literalIter_ = iter;
  result.name_ = (*iter)->token_;

  auto lastValid{ iter };
  bool lastWasDash{};
  while (true) {
    ++iter;
    if (isOperatorOrAlternative(lut, *iter, Operator::MinusPreUnary)) {
      if (lastWasDash)
        break;
      lastWasDash = true;
      continue;
    }
    if (!isLiteral(*iter))
      break;
    if (lastWasDash)
      result.name_ += "-";
    lastValid = iter;
    result.name_ += (*iter)->token_;
    lastWasDash = false;
  }
  result.afterIter_ = ++lastValid;
  return result;
}

//-----------------------------------------------------------------------------
bool Parser::consumeLineParserDirective(Context& context) noexcept
{
  auto iter{ context->begin() };

  if (!isOperator(*iter, Operator::DirectiveOpen))
    return false;

  auto primaryLiteral{ parseDirectiveLiteral(iter + 1) };
  if (!primaryLiteral) {
    auto directive{ parseDirective(iter, {}, {}, {}, {}, {}) };
    assert(directive);
    iter = directive->afterIter_;
    (void)consumeTo(directive->afterIter_);
    return true;
  }

  if ("asset"sv == primaryLiteral->name_)
    return consumeAssetOrSourceDirective(context, iter, false);

  if ("source"sv == primaryLiteral->name_)
    return consumeAssetOrSourceDirective(context, iter, true);

  if ("tab-stop"sv == primaryLiteral->name_)
    return consumeTabStopDirective(context, iter);

  if ("file"sv == primaryLiteral->name_) {
    if (!isOperatorOrAlternative(*(primaryLiteral->afterIter_), Operator::Assign))
      return false;
    return consumeFileAssignDirective(context, iter);
  }
  if ("line"sv == primaryLiteral->name_) {
    if (!isOperatorOrAlternative(*(primaryLiteral->afterIter_), Operator::Assign))
      return false;
    return consumeLineAssignDirective(context, iter);
  }

  return false;
}

//-----------------------------------------------------------------------------
bool Parser::consumeAssetOrSourceDirective(Context& context, Tokenizer::iterator iter, bool isSource) noexcept
{
  SourceAssetDirective asset;
  bool foundSourceOrAsset{};
  bool foundRename{};
  bool foundRequired{};
  bool foundGenerated{};

  auto literalValueFunc{ [&asset, &foundRequired, &foundGenerated](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (primary)
      return false;
    if ("required"sv == name) {
      if (foundRequired)
        return false;
      auto evalue { SourceAssetRequiredTraits::toEnum(value) };
      if (!evalue)
        return false;
      asset.required_ = *evalue;
      foundRequired = true;
      return true;
    }
    if ("generated"sv == name) {
      if (foundGenerated)
        return false;
      auto evalue{ YesNoTraits::toEnum(value) };
      if (!evalue)
        return false;
      asset.generated_ = *evalue == YesNo::Yes ? true : false;
      foundGenerated = true;
      return true;
    }
    return false;
  } };

  auto quoteValueFunc{ [isSource , &asset, &foundSourceOrAsset, &foundRename](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (value.empty())
      return false;
    if (primary) {
      assert(!foundSourceOrAsset);
      StringView useName{ isSource ? "source"sv : "asset"sv };
      if (useName != name)
        return false;
      asset.token_ = (*foundAt);
      asset.file_ = value;
      foundSourceOrAsset = true;
      return true;
    }
    if (isSource)
      return false;
    if ("rename"sv != name)
      return false;
    asset.rename_ = value;
    foundRename = true;
    return true;
  } };

  auto extractedValueFunc{ [isSource , &asset, &foundSourceOrAsset, &foundRename](bool primary, Tokenizer::iterator foundAt, StringView name, TokenizerPtr value) noexcept -> bool {
    if (primary) {
      assert(!foundSourceOrAsset);
      StringView useName{ isSource ? "source"sv : "asset"sv };
      if (useName != name)
        return false;
      asset.unresolvedFile_ = value;
      foundSourceOrAsset = true;
      return true;
    }
    if (isSource)
      return false;
    if ("rename"sv != name)
      return false;
    asset.unresolvedRename_ = value;
    foundRename = true;
    return true;
  } };

  auto directive{ parseDirective(
    iter,
    {},
    literalValueFunc,
    quoteValueFunc,
    {},
    extractedValueFunc
  ) };

  assert(directive);

  zs::AutoScope consumeScope{ [&]() noexcept {
    (void)consumeTo(directive->afterIter_);
  } };

  if (!directive->success_)
    return true;

  if (!asset.token_)
    asset.token_ = *(directive->literalIter_);

  if (isSource)
    handleSource(context, asset);
  else
    handleAsset(context, asset);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeTabStopDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  bool foundPrimary{};

  std::optional<int> applyTabStop{};

  auto numberValueFunc{ [&foundPrimary, &applyTabStop](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (!primary)
      return false;

    assert(!foundPrimary);

    assert("tab-stop"sv == name);
    assert(!foundAt.isEnd());

    foundPrimary = true;
    auto numValue{ toInt(value) };
    if (!numValue)
      return false;
    if (*numValue < 1)
      return false;
    applyTabStop = *numValue;
    return true;
  } };

  auto extractedValueFunc{ [&foundPrimary](bool primary, Tokenizer::iterator foundAt, StringView name, TokenizerPtr value) noexcept -> bool {
    if (!primary)
      return false;

    assert(!foundPrimary);

    assert("tab-stop"sv == name);

    foundPrimary = true;

#define RESOLVE_TAB_STOP_NOW 1
#define RESOLVE_TAB_STOP_NOW 2
    return true;
  } };

  auto directive{ parseDirective(
    iter,
    {},
    {},
    {},
    numberValueFunc,
    extractedValueFunc
  ) };

  assert(directive);
  if (directive->success_ && applyTabStop)
    (*directive->openIter_)->compileState_->tabStopWidth_ = *applyTabStop;

  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeFileAssignDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  bool foundPrimary{};

  String applyFileName;

  auto quoteValueFunc{ [&foundPrimary, &applyFileName](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (!primary)
      return false;

    assert(!foundPrimary);

    if (value.empty())
      return false;

    assert("file"sv == name);
    assert(!foundAt.isEnd());

    applyFileName = value;
    foundPrimary = true;
    return true;
  } };

  auto extractedValueFunc{ [&foundPrimary](bool primary, Tokenizer::iterator foundAt, StringView name, TokenizerPtr value) noexcept -> bool {
    if (!primary)
      return false;

    assert(!foundPrimary);

    assert("file"sv == name);

    foundPrimary = true;

#define RESOLVE_FILE_NAME_NOW 1
#define RESOLVE_FILE_NAME_NOW 2
    return true;
  } };

  auto directive{ parseDirective(
    iter,
    {},
    {},
    quoteValueFunc,
    {},
    extractedValueFunc
  ) };

  assert(directive);
  if (directive->success_ && (!applyFileName.empty())) {
    auto& tokenizer{ directive->openIter_.list() };
    auto newPath{ std::make_shared<SourceTypes::FilePath>(*tokenizer.actualFilePath_) };
    newPath->filePath_ = applyFileName;
    newPath->fullFilePath_ = applyFileName;
    tokenizer.filePath_ = newPath;
  }
  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeLineAssignDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  bool foundPrimary{};
  bool foundIncrement{};

  int deltaFrom{};
  std::optional<int> applyLine{};
  std::optional<int> applySkip{};

  auto numberValueFunc{ [&foundPrimary, &foundIncrement, &applyLine, &applySkip, &deltaFrom](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    assert(!foundAt.isEnd());

    auto numValue{ toInt(value) };
    if (!numValue)
      return false;

    if (primary) {
      assert(!foundPrimary);

      assert("line"sv == name);

      foundPrimary = true;
      applyLine = *numValue;
      deltaFrom = (*foundAt)->actualOrigin_.location_.line_;
      return true;
    }

    if ("increment"sv != name)
      return false;
    if (foundIncrement)
      return false;

    foundIncrement = true;
    applySkip = *numValue;
    return true;
  } };

  auto extractedValueFunc{ [&foundPrimary, &foundIncrement](bool primary, Tokenizer::iterator foundAt, StringView name, TokenizerPtr value) noexcept -> bool {
    if (primary) {
      assert(!foundPrimary);

      assert("line"sv == name);

      foundPrimary = true;
#define RESOLVE_LINE_NOW 1
#define RESOLVE_LINE_NOW 2
      //applyLine =
      return true;
    }

    if ("increment"sv != name)
      return false;
    if (foundIncrement)
      return false;

#define RESOLVE_LINE_INCREMENT_NOW 1
#define RESOLVE_LINE_INCREMENT_NOW 2
    foundIncrement = true;
    //applySkip =
    return true;
  } };

  auto directive{ parseDirective(
    iter,
    {},
    {},
    {},
    numberValueFunc,
    extractedValueFunc
  ) };

  assert(directive);
  if (directive->success_) {
    if (applyLine) {
      if (!applySkip)
        applySkip = 1;

      *applyLine -= *applySkip;

      auto& tokenizer{ directive->openIter_.list() };

      auto calculateNewLine{ [deltaFrom, applyLine, applySkip](int line) noexcept -> int {
        auto delta{ line - deltaFrom };
        auto addCount{ delta * (*applySkip) };
        return (*applyLine) + addCount;
      } };

      auto calculateTokenNewLine{ [&calculateNewLine](TokenPtr& token) noexcept {
        token->origin_.location_.line_ = calculateNewLine(token->actualOrigin_.location_.line_);
      } };

      for (auto& parsedToken : tokenizer.parsedTokens_) {
        calculateTokenNewLine(parsedToken);
        for (auto comment{ parsedToken->comment_ }; comment; comment = comment->comment_) {
          calculateTokenNewLine(comment);
        }
      }

      tokenizer.parserPos_.location_.line_ = calculateNewLine(tokenizer.parserPos_.actualLocation_.line_);
      tokenizer.parserPos_.lineSkip_ = *applySkip;
    }
  }
  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
void Parser::handleAsset(Context& context, SourceAssetDirective& asset) noexcept
{
  if ((asset.unresolvedFile_) ||
      (asset.unresolvedRename_)) {
#define TODO_RESOLVED_ASSET 1
#define TODO_RESOLVED_ASSET 2
    return;
  }

  SourceAsset newAsset;
  newAsset.token_ = asset.token_;
  newAsset.compileState_ = pickState(context, asset.token_);
  newAsset.filePath_ = asset.file_;
  newAsset.fullFilePath_ = asset.file_;
  newAsset.required_ = asset.required_;
  newAsset.renameFilePath_ = asset.rename_;
  newAsset.generated_ = asset.generated_;

  std::list<LocateWildCardFilesResult> results;
  locateWildCardFiles(results, asset.token_->actualOrigin_.filePath_->filePath_, asset.file_);

  if (results.size()) {
    for (auto& located : results) {
      newAsset.filePath_ = located.path_;
      newAsset.fullFilePath_ = located.fullPath_;
      newAsset.renameFilePath_ = asset.rename_;

      String& replacingStr{ newAsset.renameFilePath_ };
      bool failure{};
      for (auto& match : located.foundMatches_) {
        auto pos{ replacingStr.find_first_of("?*"sv) };
        if (String::npos == pos) {
          out(Error::OutputFailure, asset.token_, StringMap{ { "$file$", replacingStr } });
          failure = true;
          break;
        }
        replacingStr = replacingStr.substr(0, pos) + match + replacingStr.substr(pos + 1);
      }
      if (!failure)
        pendingAssets_.push_front(newAsset);
    }
  }
  else {
    pendingAssets_.push_back(newAsset);
  }
}

//-----------------------------------------------------------------------------
void Parser::handleSource(Context& context, SourceAssetDirective& source) noexcept
{
  if (source.unresolvedFile_) {
#define TODO_RESOLVED_SOURCE 1
#define TODO_RESOLVED_SOURCE 2
    return;
  }
  
  SourceAsset newSource;
  newSource.token_ = source.token_;
  newSource.compileState_ = pickState(context, source.token_);
  newSource.filePath_ = source.file_;
  newSource.fullFilePath_ = source.file_;
  newSource.required_ = source.required_;
  newSource.generated_ = source.generated_;

  std::list<LocateWildCardFilesResult> results;
  locateWildCardFiles(results, source.token_->actualOrigin_.filePath_->filePath_, source.file_);

  if (results.size()) {
    for (auto& located : results) {
      newSource.filePath_ = located.path_;
      newSource.fullFilePath_ = located.fullPath_;
      pendingSources_.push_back(newSource);
    }
  }
  else {
    pendingSources_.push_front(newSource);
  }
}
