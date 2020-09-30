
#include "pch.h"
#include "Parser.h"

#include "CompileState.h"
#include "Context.h"

using namespace zax;

using namespace std::string_view_literals;

//-----------------------------------------------------------------------------
std::optional<Parser::DirectiveResult> Parser::parseDirective(
  Context& context,
  Tokenizer::iterator iter,
  ParseDirectiveFunctions& functions) noexcept
{
  if (iter.isEnd())
    return {};

  if (!isOperator(*iter, Operator::DirectiveOpen))
    return {};

  const OperatorLut& lut{ *iter.list().operatorLut_ };

  assert(functions.legalName_);

  bool lastWasComma{};
  bool syntax{};
  bool forceSyntax{};
  bool understood{ true };

  DirectiveResult result;
  result.openIter_ = iter;
  ++iter;

  auto skipToCommaOrClose{ [](Tokenizer::iterator iter) noexcept -> Tokenizer::iterator {
    while (!iter.isEnd()) {
      if (isSeparator(*iter))
        break;
      if (isCommaOrCloseDirective(*iter))
        break;
      ++iter;
    }
    return iter;
  } };

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

    if (!functions.legalName_(primary, literal->name_)) {
      if (!Parser::isUnknownExtension(literal->name_)) {
        out(Warning::UnknownDirectiveArgument, *literal->literalIter_);
        understood = false;
        break;
      }
      iter = skipToCommaOrClose(iter);
      continue;
    }

    if (!isOperator(*iter, Operator::Assign)) {
      if (!isCommaOrCloseDirective(*iter))
        break;

      bool check{ functions.noValueFunc_ && functions.noValueFunc_(primary, literal->literalIter_, literal->name_) };
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

    bool foundLiteral{};
    if (auto assigned{ parseDirectiveLiteral(iter) }; assigned) {
      if (isCommaOrCloseDirective(*assigned->afterIter_)) {
        foundLiteral = true;

        auto check{ functions.literalValueFunc_ && functions.literalValueFunc_(primary, literal->literalIter_, literal->name_, assigned->name_) };
        bool literalSuccess{ !((!check) && (!isUnknownExtension(literal->name_))) };

        // if this literal was processed then treat it as success, otherwise treat this as an evaluation
        if (literalSuccess) {
          iter = assigned->afterIter_;
          continue;
        }
      }
    }

    if (!foundLiteral) {
      if (auto assigned{ parseQuote(iter) }; assigned) {
        if (isCommaOrCloseDirective(*assigned->afterIter_)) {
          iter = assigned->afterIter_;
          auto check{ functions.quoteValueFunc_ && functions.quoteValueFunc_(primary, literal->literalIter_, literal->name_, assigned->quote_) };
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
          auto check{ functions.numberValueFunc_ && functions.numberValueFunc_(primary, literal->literalIter_, literal->name_, assigned->number_) };
          if ((!check) && (!isUnknownExtension(literal->name_))) {
            out(Warning::DirectiveNotUnderstood, *literal->literalIter_);
            understood = false;
            break;
          }
          continue;
        }
      }
    }

    if ((!functions.extractedValueFunc_) && (!isUnknownExtension(literal->name_))) {
      out(Warning::DirectiveNotUnderstood, *iter);
      understood = false;
      break;
    }

    auto startIter{ iter };

    iter = skipToCommaOrClose(iter);

    // scope: check will extract function
    {
      auto extraction{ extract(context, startIter, iter) };
      auto check{ functions.extractedValueFunc_ && functions.extractedValueFunc_(primary, literal->literalIter_, literal->name_, extraction) };
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
std::optional<ParserDirectiveTypes::DirectiveLiteral> Parser::parseDirectiveLiteral(Tokenizer::iterator iter) noexcept
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
    ParseDirectiveFunctions nothing;
    nothing.legalName_ = [](bool, StringView) noexcept -> bool { return true; };
    auto directive{ parseDirective(context, iter, nothing) };
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
  if ("panic"sv == primaryLiteral->name_)
    return consumePanicDirective(context, iter);
  if ("warning"sv == primaryLiteral->name_)
    return consumeWarningDirective(context, iter);
  if ("error"sv == primaryLiteral->name_)
    return consumeErrorDirective(context, iter);
  if ("functions"sv == primaryLiteral->name_)
    return consumeFunctionsDirective(context, iter);
  if ("types"sv == primaryLiteral->name_)
    return consumeTypesDirective(context, iter);
  if ("variables"sv == primaryLiteral->name_)
    return consumeVariablesDirective(context, iter);
  if ("deprecate"sv == primaryLiteral->name_)
    return consumeDeprecateDirective(context, iter);
  if ("export"sv == primaryLiteral->name_)
    return consumeExportDirective(context, iter);

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

  ParseDirectiveFunctions functions;

  functions.legalName_ = [isSource](bool primary, StringView name) noexcept -> bool {
    if (primary)
      return true;
    if ("required"sv == name)
      return true;
    if ("generated"sv == name)
      return true;
    if ((!isSource) && ("rename"sv == name))
      return true;
    return false;
  };

  functions.literalValueFunc_ = [&asset, &foundRequired, &foundGenerated](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
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
  };

  functions.quoteValueFunc_ = [isSource , &asset, &foundSourceOrAsset, &foundRename](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (value.empty())
      return false;
    if (primary) {
      assert(!foundSourceOrAsset);
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
  };

  functions.extractedValueFunc_ = [isSource , &asset, &foundSourceOrAsset, &foundRename](bool primary, Tokenizer::iterator foundAt, StringView name, Extraction& value) noexcept -> bool {
    if (primary) {
      assert(!foundSourceOrAsset);
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
  };

  auto directive{ parseDirective(context, iter, functions) };

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
  std::optional<int> applyTabStop{};

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    return primary;
  };

  functions.numberValueFunc_ = [&applyTabStop](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    assert(primary);
    assert("tab-stop"sv == name);
    assert(!foundAt.isEnd());

    auto numValue{ toInt(value) };
    if (!numValue)
      return false;
    if (*numValue < 1)
      return false;
    applyTabStop = *numValue;
    return true;
  };

  functions.extractedValueFunc_ = [](bool primary, Tokenizer::iterator foundAt, StringView name, Extraction& value) noexcept -> bool {
    assert(primary);
    assert("tab-stop"sv == name);

#define RESOLVE_TAB_STOP_NOW 1
#define RESOLVE_TAB_STOP_NOW 2
    return true;
  };

  auto directive{ parseDirective(context, iter, functions) };

  assert(directive);
  if (directive->success_ && applyTabStop) {
    context.state_ = CompileState::fork(context.state());
    context.state_->tabStopWidth_ = *applyTabStop;
  }

  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeFileAssignDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  String applyFileName;

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    return primary;
  };

  functions.quoteValueFunc_ = [&applyFileName](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    assert(primary);

    if (value.empty())
      return false;

    assert("file"sv == name);
    assert(!foundAt.isEnd());

    applyFileName = value;
    return true;
  };

  functions.extractedValueFunc_ = [](bool primary, Tokenizer::iterator foundAt, StringView name, Extraction& value) noexcept -> bool {
    assert(primary);

    assert("file"sv == name);

#define RESOLVE_FILE_NAME_NOW 1
#define RESOLVE_FILE_NAME_NOW 2
    return true;
  };

  auto directive{ parseDirective(context, iter, functions) };

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
  bool foundIncrement{};

  int deltaFrom{};
  std::optional<int> applyLine{};
  std::optional<int> applySkip{};

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    if (primary)
      return true;
    if ("increment"sv == name)
      return true;
    return false;
  };

  functions.numberValueFunc_ = [&foundIncrement, &applyLine, &applySkip, &deltaFrom](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    assert(!foundAt.isEnd());

    auto numValue{ toInt(value) };
    if (!numValue)
      return false;

    if (primary) {
      assert("line"sv == name);

      applyLine = *numValue;
      deltaFrom = (*foundAt)->actualOrigin_.location_.line_;
      return true;
    }

    assert("increment"sv == name);
    if (foundIncrement)
      return false;

    foundIncrement = true;
    applySkip = *numValue;
    return true;
  };

  functions.extractedValueFunc_ = [&foundIncrement](bool primary, Tokenizer::iterator foundAt, StringView name, Extraction& value) noexcept -> bool {
    if (primary) {
      assert("line"sv == name);

#define RESOLVE_LINE_NOW 1
#define RESOLVE_LINE_NOW 2
      //applyLine =
      return true;
    }

    assert("increment"sv == name);
    if (foundIncrement)
      return false;

#define RESOLVE_LINE_INCREMENT_NOW 1
#define RESOLVE_LINE_INCREMENT_NOW 2
    foundIncrement = true;
    //applySkip =
    return true;
  };

  auto directive{ parseDirective(context, iter, functions) };

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

namespace {

//-----------------------------------------------------------------------------
template <typename TEnumType, typename TEnumTraits, bool VAllowMessage, bool VAllowOption>
std::optional<ParserDirectiveTypes::DirectiveResult> consumeFaultDirective(
  Parser& parser,
  Context& context,
  Tokenizer::iterator iter,
  String& outMessage,
  std::optional<ParserDirectiveTypes::FaultOptions>& outOption,
  std::optional<TEnumType>& outWhich,
  String &outFoundUnknown,
  StringMap& outMapping) noexcept
{
  assert(!iter.isEnd());

  String lastName;
  std::optional<TEnumType> useEnum;

  ParserDirectiveTypes::ParseDirectiveFunctions functions;

  functions.legalName_ = [&useEnum](bool primary, StringView name) noexcept -> bool {
    useEnum.reset();
    if (primary)
      return true;
    if constexpr (VAllowMessage) {
      if ("name"sv == name)
        return true;
      if ("value"sv == name)
        return true;
    }
    useEnum = TEnumTraits::toEnum(name);
    if (useEnum.has_value())
      return true;
    // treat the unknown extension as handled
    if (Parser::isUnknownExtension(name))
      return true;
    return false;
  };

  functions.noValueFunc_ = [&outWhich, &outFoundUnknown, &useEnum](bool primary, Tokenizer::iterator foundAt, StringView name) noexcept -> bool {
    if (primary)
      return false;
    if ((outWhich) || (!outFoundUnknown.empty()))
      return false;
    outWhich = useEnum;
    if (!outWhich) {
      if (Parser::isUnknownExtension(name)) {
        outFoundUnknown = name;
        return true;
      }
      return false;
    }
    return true;
  };

  functions.literalValueFunc_ = [&outOption, &outWhich, &outFoundUnknown](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (!primary)
      return false;

    if constexpr (VAllowOption) {
      auto option{ ParserDirectiveTypes::FaultOptionsTraits::toEnum(value) };
      if (option) {
        outOption = option;
        return true;
      }
    }
    auto which{ TEnumTraits::toEnum(value) };
    if (which) {
      outWhich = which;
      return true;
    }
    if (Parser::isUnknownExtension(value)) {
      outFoundUnknown = value;
      return true;
    }
    return false;
  };

  functions.quoteValueFunc_ = [&outMessage, &outOption, &lastName, &outMapping](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if constexpr (VAllowMessage) {
      if (primary) {
        outMessage = value;
        return true;
      }

      if (outOption)
        return false;

      if ("name"sv == name) {
        if (!lastName.empty())
          return false;
        lastName = value;
        return true;
      }
      if ("value"sv == name) {
        if (lastName.empty())
          return false;
        outMapping[lastName] = value;
        lastName.clear();
        return true;
      }
    }
    return false;
  };

  functions.extractedValueFunc_ = [&outMessage, &lastName, &outMapping](bool primary, Tokenizer::iterator foundAt, StringView name, ParserTypes::Extraction& value) noexcept -> bool {
    if constexpr (VAllowMessage) {
      if (primary) {
#define RESOLVE_MESSAGE_NOW 1
#define RESOLVE_MESSAGE_NOW 2
        //message =
        return true;
      }

      if ("name"sv == name) {
        if (!lastName.empty())
          return false;
#define RESOLVE_NAME_NOW 1
#define RESOLVE_NAME_NOW 2
        return true;
      }

      if ("value"sv == name) {
        if (lastName.empty())
          return false;
#define RESOLVE_NAME_VALUE_NOW 1
#define RESOLVE_NAME_VALUE_NOW 2
        return true;
      }
    }
    return false;
  };

  auto result{ parser.parseDirective(context, iter, functions) };
  if (result) {
    if (result->success_) {
      if (!lastName.empty()) {
        result->success_ = false;
        parser.out(WarningTypes::Warning::DirectiveNotUnderstood, *result->literalIter_);
      }
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
bool applyToParsedTokens(
  Context& context,
  Tokenizer& tokenizer,
  CompileStatePtr state,
  bool stopAtSeparator) noexcept {

  for (auto& token : tokenizer.parsedTokens_) {
    token->compileState_ = state;
    for (auto comment = token->comment_; comment; comment = comment->comment_) {
      token->compileState_ = state;
    }
    if (stopAtSeparator) {
      if (Parser::isSeparator(token)) {
        context.singleLineState_.reset();
        return false;
      }
    }
  }
  return true;
}


//-----------------------------------------------------------------------------
void applyToSources(
  Context& context,
  CompileStatePtr state,
  bool stopAtSeparator = false) noexcept
{
  bool topmost{ true };
  auto& sources{ context.parser().sources_ };
  for (auto& source : sources) {
    if (!applyToParsedTokens(context, *source->tokenizer_, state, topmost && stopAtSeparator))
      return;
    topmost = false;
  }
}

//-----------------------------------------------------------------------------
template <typename TFaultEnum, typename TFaultType, bool VAllowError> 
void applyFaultDirective(
  Context& context,
  Tokenizer::iterator literalIter,
  std::optional<ParserDirectiveTypes::FaultOptions> option,
  std::optional<TFaultEnum> which,
  std::function<TFaultType&(CompileState&)>&& getFaultFunc
) noexcept
{
  assert(getFaultFunc);
  auto& parser{ context.parser() };
  switch (option.value()) {
    case ParserDirectiveTypes::FaultOptions::Yes: {
      context.singleLineState_ = CompileState::fork(context.state());
      if (which)
        getFaultFunc(*context.singleLineState_).enable(which.value());
      else
        getFaultFunc(*context.singleLineState_).enableAll();
      applyToSources(context, context.singleLineState_, true);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::No: {
      context.singleLineState_ = CompileState::fork(context.state());
      if (which)
        getFaultFunc(*context.singleLineState_).disable(which.value());
      else
        getFaultFunc(*context.singleLineState_).disableAll();
      applyToSources(context, context.singleLineState_, true);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Always: {
      context.state_ = CompileState::fork(context.state());
      if (which)
        getFaultFunc(*context.state_).enable(which.value());
      else
        getFaultFunc(*context.state_).enableAll();
      applyToSources(context, context.state_);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Never: {
      context.state_ = CompileState::fork(context.state());
      if (which)
        getFaultFunc(*context.state_).disable(which.value());
      else
        getFaultFunc(*context.state_).disableAll();
      applyToSources(context, context.state_);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Error: {
      if constexpr (!VAllowError) {
        parser.out(WarningTypes::Warning::DirectiveNotUnderstood, *(literalIter));
        break;
      }
      else {
        context.state_ = CompileState::fork(context.state());
        if (which)
          getFaultFunc(*context.state_).enableForceError(which.value());
        else
          getFaultFunc(*context.state_).enableForceErrorAll();
        applyToSources(context, context.state_);
      }
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Default: {
      context.state_ = CompileState::fork(context.state());
      if (which)
        getFaultFunc(*context.state_).applyDefault(which.value());
      else
        getFaultFunc(*context.state_).defaultAll();
      applyToSources(context, context.state_);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Lock: {
      context.state_ = CompileState::fork(context.state());
      if (which)
        getFaultFunc(*context.state_).lock(which.value(), parser.id_);
      else
        getFaultFunc(*context.state_).lockAll(parser.id_);
      applyToSources(context, context.state_);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Unlock: {
      context.state_ = CompileState::fork(context.state());
      if (which)
        getFaultFunc(*context.state_).unlock(which.value(), parser.id_);
      else
        getFaultFunc(*context.state_).unlockAll(parser.id_);
      applyToSources(context, context.state_);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Push: {
      if (which) {
        parser.out(WarningTypes::Warning::DirectiveNotUnderstood, *(literalIter));
        break;
      }
      context.state_ = CompileState::fork(context.state());
      getFaultFunc(*context.state_).push();
      applyToSources(context, context.state_);
      break;
    }
    case ParserDirectiveTypes::FaultOptions::Pop: {
      if (which) {
        parser.out(WarningTypes::Warning::DirectiveNotUnderstood, *(literalIter));
        break;
      }
      auto oldState{ context.state_ };
      context.state_ = CompileState::fork(context.state());
      if (!getFaultFunc(*context.state_).pop()) {
        context.state_ = oldState;
        parser.out(ErrorTypes::Error::UnmatchedPush, *(literalIter));
        break;
      }
      applyToSources(context, context.state_);
      break;
    }
  }
}

} // namespace

//-----------------------------------------------------------------------------
bool Parser::consumePanicDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  String message;
  std::optional<ParserDirectiveTypes::FaultOptions> option;
  std::optional<PanicTypes::Panic> which;
  String foundUnknown;
  StringMap mapping;
  auto directive{ consumeFaultDirective<PanicTypes::Panic, PanicTypes::PanicTraits, false, true>(*this, context, iter, message, option, which, foundUnknown, mapping) };
  if (!directive)
    return false;

  iter = directive->afterIter_;

  do {
    if (!directive->success_)
      break;

    if (!option) {
      out(Warning::DirectiveNotUnderstood, *(directive->literalIter_));
      break;
    }

    if (!foundUnknown.empty())
      break;

    applyFaultDirective<PanicTypes::Panic, Panics, false>(
      context,
      directive->literalIter_,
      option,
      which,
      [](CompileState& state) noexcept -> Panics& { return state.panics_; }
    );
  } while (false);

  (void)consumeTo(iter);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeWarningDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  String message;
  std::optional<ParserDirectiveTypes::FaultOptions> option;
  std::optional<WarningTypes::Warning> which;
  String foundUnknown;
  StringMap mapping;
  auto directive{ consumeFaultDirective<WarningTypes::Warning, WarningTypes::WarningTraits, true, true>(*this, context, iter, message, option, which, foundUnknown, mapping) };
  if (!directive)
    return false;

  iter = directive->afterIter_;

  do {
    if (!directive->success_)
      break;

    if (!message.empty()) {
      mapping["$message$"] = message;
      assert(!option);
    }

    if (!option) {
      if (which) {
        out(*which, *(directive->literalIter_), mapping);
        break;
      }

      if (!foundUnknown.empty()) {
        if (message.empty())
          mapping["$message$"] = foundUnknown;
      }
      out(WarningTypes::Warning::WarningDirective, *(directive->literalIter_), mapping);
      break;
    }

    if (!foundUnknown.empty())
      break;

    applyFaultDirective<WarningTypes::Warning, Warnings, true>(
      context,
      directive->literalIter_,
      option,
      which,
      [](CompileState& state) noexcept -> Warnings& { return state.warnings_; }
    );
  } while (false);

  (void)consumeTo(iter);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeErrorDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  String message;
  std::optional<ParserDirectiveTypes::FaultOptions> option;
  std::optional<ErrorTypes::Error> which;
  String foundUnknown;
  StringMap mapping;
  auto directive{ consumeFaultDirective<ErrorTypes::Error, ErrorTypes::ErrorTraits, true, false>(*this, context, iter, message, option, which, foundUnknown, mapping) };
  if (!directive)
    return false;

  assert(!option);

  iter = directive->afterIter_;

  do {
    if (!directive->success_)
      break;

    if (!message.empty()) {
      mapping["$message$"] = message;
    }

    if (which) {
      out(*which, *(directive->literalIter_), mapping);
      break;
    }

    if (!foundUnknown.empty()) {
      if (message.empty())
        mapping["$message$"] = foundUnknown;
    }

    out(ErrorTypes::Error::ErrorDirective, *(directive->literalIter_), mapping);
  } while (false);

  (void)consumeTo(iter);
  return true;
}

//-----------------------------------------------------------------------------
void Parser::handleAsset(Context& context, SourceAssetDirective& asset) noexcept
{
  if ((asset.unresolvedFile_.hasValue()) ||
      (asset.unresolvedRename_.hasValue())) {
#define TODO_RESOLVED_ASSET 1
#define TODO_RESOLVED_ASSET 2
    return;
  }

  SourceAsset newAsset;
  newAsset.token_ = asset.token_;
  newAsset.compileState_ = context.state();
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
bool Parser::consumeDeprecateDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  std::optional<YesNoAlwaysNever> option{};
  std::optional<CompileState::Deprecate::Context> csContext{};
  std::optional<bool> error{};
  std::optional<SemanticVersion> min;
  std::optional<SemanticVersion> max;

  auto optionNotSetOrNever{ [&option]() noexcept -> bool {
    if (!option)
      return true;
    switch (*option) {
      case YesNoAlwaysNever::Yes:     break;
      case YesNoAlwaysNever::No:      return true;
      case YesNoAlwaysNever::Always:  break;
      case YesNoAlwaysNever::Never:   return true;
    }
    return false;
  } };

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    if (primary)
      return true;
    if ("context"sv == name)
      return true;
    if ("error"sv == name)
      return true;
    if ("min"sv == name)
      return true;
    if ("max"sv == name)
      return true;
    return false;
  };

  functions.noValueFunc_ = [&optionNotSetOrNever, &option, &error](bool primary, Tokenizer::iterator foundAt, StringView name) noexcept -> bool {
    if (primary) {
      option = YesNoAlwaysNever::Yes;
      return true;
    }

    if (optionNotSetOrNever())
      return false;

    if (error)
      return false;

    if ("error"sv != name)
      return false;

    error = true;
    return true;
  };

  functions.literalValueFunc_ = [&optionNotSetOrNever, &option, &csContext](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (primary) {
      option = YesNoAlwaysNeverTraits::toEnum(value);
      return static_cast<bool>(option);
    }

    if (optionNotSetOrNever())
      return false;

    if ("context"sv == name) {
      if (csContext.has_value())
        return false;
      csContext = CompileState::Deprecate::ContextTraits::toEnum(value);
      if (!csContext)
        return false;
      return true;
    }
    return false;
  };

  functions.quoteValueFunc_ = [&optionNotSetOrNever, &min, &max](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (primary)
      return false;

    if (optionNotSetOrNever())
      return false;

#define VERSIONING_VALIDATION_CHECK 1
#define VERSIONING_VALIDATION_CHECK 2

    if ("min"sv == name) {
      if (min.has_value())
        return false;
      min = SemanticVersion::convert(value);
      if (!min)
        return false;
      return true;
    }
    if ("max"sv == name) {
      if (max.has_value())
        return false;
      max = SemanticVersion::convert(value);
      if (!max)
        return false;
      return true;
    }
    return false;
  };

  auto directive{ parseDirective(context, iter, functions) };
  assert(directive);
  if (directive->success_) {
    bool singleLineState{ false };
    auto tempState{ CompileState::fork(context.state()) };
    switch (*option) {
      case YesNoAlwaysNever::Yes:     singleLineState = true; break;
      case YesNoAlwaysNever::No:      singleLineState = true; break;
      case YesNoAlwaysNever::Always:  break;
      case YesNoAlwaysNever::Never:   break;
    }
    if (!optionNotSetOrNever()) {
      tempState->deprecate_.emplace();
      tempState->deprecate_->origin_ = (*directive->literalIter_)->origin_;
      if (csContext)
        tempState->deprecate_->context_ = *csContext;
      if (error.has_value())
        tempState->deprecate_->forceError_ = *error;
      tempState->deprecate_->min_ = min;
      tempState->deprecate_->max_ = max;
    }
    else
      tempState->deprecate_.reset();
    if (!singleLineState)
      context.state_ = tempState;
    else
      context.singleLineState_ = tempState;
    applyToSources(context, tempState, singleLineState);
  }
  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeExportDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  std::optional<YesNoAlwaysNever> option{};

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    return primary;
  };

  functions.noValueFunc_ = [&option](bool primary, Tokenizer::iterator foundAt, StringView name) noexcept -> bool {
    if (!primary)
      return false;
    option = YesNoAlwaysNever::Yes;
    return true;
  };

  functions.literalValueFunc_ = [&option](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    if (!primary)
      return false;
    option = YesNoAlwaysNeverTraits::toEnum(value);
    return static_cast<bool>(option);
  };

  auto directive{ parseDirective(context, iter, functions) };
  assert(directive);
  if (directive->success_) {
    bool singleLineState{ false };
    bool yes{};
    auto tempState{ CompileState::fork(context.state()) };
    switch (*option) {
      case YesNoAlwaysNever::Yes:     yes = singleLineState = true; break;
      case YesNoAlwaysNever::No:      yes = false;  singleLineState = true; break;
      case YesNoAlwaysNever::Always:  yes = true; break;
      case YesNoAlwaysNever::Never:   yes = false; break;
    }
    tempState->export_.export_ = yes;
    if (!singleLineState)
      context.state_ = tempState;
    else
      context.singleLineState_ = tempState;
    applyToSources(context, tempState, singleLineState);
  }
  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeVariablesDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  enum class VariablesDefault {
    Mutable,
    Immutable,
    Final,
    Varies
  };

  struct VariablesDefaultDeclare final : public zs::EnumDeclare<VariablesDefault, 4>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {VariablesDefault::Mutable, "mutable"},
        {VariablesDefault::Immutable, "immutable"},
        {VariablesDefault::Final, "final"},
        {VariablesDefault::Varies, "varies"}
      } };
    }
  };

  using VariablesDefaultTraits = zs::EnumTraits<VariablesDefault, VariablesDefaultDeclare>;

  std::optional<VariablesDefault> _default{};

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    return primary;
  };

  functions.literalValueFunc_ = [&_default](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    _default = VariablesDefaultTraits::toEnum(value);
    return static_cast<bool>(_default);
  };

  auto directive{ parseDirective(context, iter, functions) };
  assert(directive);
  if (directive->success_) {
    context.state_ = CompileState::fork(context.state());
    switch (*_default) {
      case VariablesDefault::Mutable:     context.state_->variableDefault_.mutable_ = true; break;
      case VariablesDefault::Immutable:   context.state_->variableDefault_.mutable_ = false; break;
      case VariablesDefault::Varies:      context.state_->variableDefault_.varies_ = true; break;
      case VariablesDefault::Final:       context.state_->variableDefault_.varies_ = false; break;
    }
    applyToSources(context, context.state_);
  }
  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeTypesDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  enum class TypesDefault {
    Mutable,
    Immutable,
    Constant,
    Inconstant
  };

  struct TypesDefaultDeclare final : public zs::EnumDeclare<TypesDefault, 4>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {TypesDefault::Mutable, "mutable"},
        {TypesDefault::Immutable, "immutable"},
        {TypesDefault::Constant, "constant"},
        {TypesDefault::Inconstant, "inconstant"}
      } };
    }
  };

  using TypesDefaultTraits = zs::EnumTraits<TypesDefault, TypesDefaultDeclare>;

  std::optional<TypesDefault> _default{};

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    return primary;
  };

  functions.literalValueFunc_ = [&_default](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    _default = TypesDefaultTraits::toEnum(value);
    return static_cast<bool>(_default);
  };

  auto directive{ parseDirective(context, iter, functions) };
  assert(directive);
  if (directive->success_) {
    context.state_ = CompileState::fork(context.state());
    switch (*_default) {
      case TypesDefault::Mutable:     context.state_->typeDefault_.mutable_ = true; break;
      case TypesDefault::Immutable:   context.state_->typeDefault_.mutable_ = false; break;
      case TypesDefault::Constant:    context.state_->typeDefault_.constant_ = true; break;
      case TypesDefault::Inconstant:  context.state_->typeDefault_.constant_ = false; break;
    }
    applyToSources(context, context.state_);
  }
  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
bool Parser::consumeFunctionsDirective(Context& context, Tokenizer::iterator iter) noexcept
{
  enum class FunctionsDefault {
    Constant,
    Inconstant
  };

  struct FunctionsDefaultDeclare final : public zs::EnumDeclare<FunctionsDefault, 2>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {FunctionsDefault::Constant, "constant"},
        {FunctionsDefault::Inconstant, "inconstant"}
      } };
    }
  };

  using FunctionsDefaultTraits = zs::EnumTraits<FunctionsDefault, FunctionsDefaultDeclare>;

  std::optional<FunctionsDefault> _default{};

  ParseDirectiveFunctions functions;

  functions.legalName_ = [](bool primary, StringView name) noexcept -> bool {
    return primary;
  };

  functions.literalValueFunc_ = [& _default](bool primary, Tokenizer::iterator foundAt, StringView name, StringView value) noexcept -> bool {
    _default = FunctionsDefaultTraits::toEnum(value);
    return static_cast<bool>(_default);
  };

  auto directive{ parseDirective(context, iter, functions) };
  assert(directive);
  if (directive->success_) {
    context.state_ = CompileState::fork(context.state());
    context.state_->functionDefault_.constant_ = (*_default) == FunctionsDefault::Constant;
    applyToSources(context, context.state_);
  }
  (void)consumeTo(directive->afterIter_);
  return true;
}

//-----------------------------------------------------------------------------
void Parser::handleSource(Context& context, SourceAssetDirective& source) noexcept
{
  if (source.unresolvedFile_.hasValue()) {
#define TODO_RESOLVED_SOURCE 1
#define TODO_RESOLVED_SOURCE 2
    return;
  }
  
  SourceAsset newSource;
  newSource.token_ = source.token_;
  newSource.compileState_ = context.state();
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
