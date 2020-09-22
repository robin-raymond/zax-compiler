
#include "pch.h"
#include "Tokenizer.h"
#include "OperatorLut.h"
#include "CompilerException.h"

using namespace zax;
using namespace std::string_view_literals;

//-----------------------------------------------------------------------------
bool TokenizerTypes::ParserPos::operator==(const ParserPos& rhs) const noexcept
{
  return
    (location_ == rhs.location_) &&
    (actualLocation_ == rhs.actualLocation_) &&
    (pos_ == rhs.pos_) &&
    (tabSize_ == rhs.tabSize_) &&
    (lineSkip_ == rhs.lineSkip_);
}

//-----------------------------------------------------------------------------
bool TokenizerTypes::ParserPos::sameLocation(const ParserPos& rhs) const noexcept
{
  return
    (location_ == rhs.location_) &&
    (actualLocation_ == rhs.actualLocation_);
}

//-----------------------------------------------------------------------------
Tokenizer::Tokenizer(
  const SourceTypes::FilePathPtr& filePath,
  std::pair< std::unique_ptr<std::byte[]>, size_t>&& rawContents,
  const CompileStatePtr& compileState,
  const OperatorLutConstPtr& operatorLut
) noexcept :
  filePath_(filePath),
  actualFilePath_(filePath),
  rawContents_(std::move(rawContents)),
  raw_(rawContents_.first.get()),
  compileState_(compileState),
  operatorLut_(operatorLut)
{
  assert(filePath_);
  assert(rawContents_.first);
  assert(compileState_);
  assert(operatorLut_);

  static_assert(sizeof(std::byte) == sizeof(char));
  parserPos_.pos_ = StringView{ reinterpret_cast<const char *>(raw_), rawContents_.second };

  errorCallback_ = [](ErrorTypes::Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept {
    output(error, token, mapping);
  };
  warningCallback_ = [](WarningTypes::Warning warning, const TokenConstPtr& token, const StringMap& mapping) noexcept {
    output(warning, token, mapping);
  };
}

//-----------------------------------------------------------------------------
Tokenizer::Tokenizer(
  const Tokenizer& original,
  TokenList&& tokenList) noexcept :
  parsedTokens_(std::move(tokenList)),
  filePath_(original.filePath_),
  actualFilePath_(original.actualFilePath_),
  compileState_(original.compileState_),
  operatorLut_(original.operatorLut_),
  errorCallback_(original.errorCallback_),
  warningCallback_(original.warningCallback_),
  skipComments_(original.skipComments_)
{
  constexpr static const char* const nulStr{ "" };
  raw_ = reinterpret_cast<const std::byte*>(nulStr);

  assert(filePath_);
  assert(rawContents_.first);
  assert(compileState_);
  assert(operatorLut_);

  static_assert(sizeof(std::byte) == sizeof(char));
  parserPos_.pos_ = StringView{ reinterpret_cast<const char*>(raw_), rawContents_.second };
}

//-----------------------------------------------------------------------------
Tokenizer::~Tokenizer() noexcept
{
}

//-----------------------------------------------------------------------------
void Tokenizer::count(ParserPos& parserPos, char c) noexcept
{
  if (c < 0) {
    if (parserPos.utf8Count_ > 0) {
      --parserPos.utf8Count_;
      return;
    }
    auto rawByte{ static_cast<unsigned char>(c) };
    if ((rawByte & 0xE0) == 0xC0)
      parserPos.utf8Count_ = 1;
    else if ((rawByte & 0xF0) == 0xE0)
      parserPos.utf8Count_ = 2;
    else if ((rawByte & 0xF8) == 0xF0)
      parserPos.utf8Count_ = 3;
    ++parserPos.location_.column_;
    parserPos.actualLocation_.column_ = parserPos.location_.column_;
    return;
  }

  parserPos.utf8Count_ = 0;
  if (!iscntrl(c)) {
    ++parserPos.location_.column_;
    parserPos.actualLocation_.column_ = parserPos.location_.column_;
    return;
  }

  parserPos.utf8Count_ = 0;

  switch (c) {
    case '\r':  {
      parserPos.location_.column_ = 1;
      parserPos.actualLocation_.column_ = parserPos.location_.column_;
      return;
    }
    case '\f':
    case '\n':  {
      parserPos.location_.line_ += parserPos.lineSkip_;
      parserPos.location_.column_ = 1;
      parserPos.actualLocation_.column_ = parserPos.location_.column_;
      ++parserPos.actualLocation_.line_;
      return;
    }
    case '\v':  {
      parserPos.location_.line_ += parserPos.lineSkip_;
      ++parserPos.actualLocation_.line_;
      break;
    }
    case '\t':  {
      parserPos.location_.column_ += parserPos.tabSize_ - ((parserPos.location_.column_ - 1) % parserPos.tabSize_);
      parserPos.actualLocation_.column_ = parserPos.location_.column_;
      return;
    }
    case '\b':  {
      parserPos.location_.column_ = std::max(1, parserPos.location_.column_ - 1);
      parserPos.actualLocation_.column_ = parserPos.location_.column_;
      return;
    }
    default:  return;
  }
}

//-----------------------------------------------------------------------------
bool Tokenizer::consumeUtf8Bom(ParserPos& parserPos) noexcept
{
  constexpr StringView utf8Bom{ "\xef\xbb\xbf" };
  if (parserPos.pos_.length() < utf8Bom.length())
    return false;

  if (utf8Bom != parserPos.pos_.substr(0, utf8Bom.length()))
    return false;

  parserPos.pos_ = parserPos.pos_.substr(utf8Bom.length());
  return true;
}

//-----------------------------------------------------------------------------
optional<TokenizerTypes::CommentToken> Tokenizer::consumeComment(ParserPos& parserPos) noexcept
{
  if (parserPos.pos_.size() < 1)
    return {};

  if ('/' != (*parserPos.pos_.data()))
    return {};

  constexpr StringView slashSlash{ "//" };
  constexpr StringView slashStarStar{ "/**" };
  constexpr StringView starStarSlash{ "**/" };
  constexpr StringView slashStar{ "/*" };
  constexpr StringView starSlash{ "*/" };

  // handle `//` comment
  if (parserPos.pos_.substr(0, slashSlash.length()) == slashSlash) {
    const char* start = parserPos.pos_.data();
    const char* startAfterSlashSlash = start + slashSlash.length();
    auto pos = start;
    auto end = start + parserPos.pos_.length();

    auto isEol{ [](char let) noexcept -> bool {
      switch (let)
      {
        case '\n':
        case '\r':  return true;
      }
      return false;
    } };

    while (pos < end) {
      if (isEol(*pos))
        break;
      count(parserPos, *pos);
      ++pos;
    }

    parserPos.pos_ = makeStringView(pos, end);
    return CommentToken{
      .originalToken_ = makeStringView(start, pos),
      .token_ = makeStringView(startAfterSlashSlash, pos),
      .foundEnding_ = true,
      .addNewLine_ = false
    };
  }

  // handle `/**` `**/` nested comments
  if (parserPos.pos_.substr(0, slashStarStar.length()) == slashStarStar) {
    auto startLine = parserPos.location_.line_;
    const char* start = parserPos.pos_.data();
    const char* startAfter = start + slashStarStar.length();
    auto pos = startAfter;
    auto end = start + parserPos.pos_.length();
    auto preFinal = end;

    advance(parserPos, slashStarStar);
    int nestCount{ 1 };

    while (pos < end) {
      StringView view{ pos, SafeInt<size_t>(end - pos) };
      auto ahead{ view.substr(0, slashStarStar.length()) };
      if (ahead == slashStarStar) {
        ++nestCount;
        advance(parserPos, slashStarStar);
        pos += slashStarStar.length();
        continue;
      }
      if (ahead == starStarSlash) {
        --nestCount;
        if (0 == nestCount) {
          preFinal = pos;
          advance(parserPos, starStarSlash);
          pos += starStarSlash.length();
          break;
        }
        advance(parserPos, starStarSlash);
        pos += starStarSlash.length();
        continue;
      }
      count(parserPos, *pos);
      ++pos;
    }

    parserPos.pos_ = makeStringView(pos, end);
    return CommentToken{
      .originalToken_ = makeStringView(start, pos),
      .token_ = makeStringView(startAfter, preFinal),
      .foundEnding_ = (0 == nestCount),
      .addNewLine_ = (startLine != parserPos.location_.line_)
    };
  }

  // handle `/*` `*/` comments
  if (parserPos.pos_.substr(0, slashSlash.length()) == slashStar) {
    auto startLine = parserPos.location_.line_;
    const char* start = parserPos.pos_.data();
    const char* startAfter = start + slashStar.length();
    auto pos = startAfter;
    auto end = start + parserPos.pos_.length();
    auto preFinal = end;
    bool foundEnding = false;

    advance(parserPos, slashStar);
    while (pos < end) {
      StringView view{ pos, SafeInt<size_t>(end - pos) };
      auto ahead{ view.substr(0, starSlash.length()) };
      if (ahead == starSlash) {
        preFinal = pos;
        advance(parserPos, slashStar);
        pos += starSlash.length();
        foundEnding = true;
        break;
      }
      count(parserPos, *pos);
      ++pos;
    }

    parserPos.pos_ = makeStringView(pos, end);
    return CommentToken{
      .originalToken_ = makeStringView(start, pos),
      .token_ = makeStringView(startAfter, preFinal),
      .foundEnding_ = foundEnding,
      .addNewLine_ = (startLine != parserPos.location_.line_)
    };
  }

  return {};
}

//-----------------------------------------------------------------------------
optional<TokenizerTypes::WhitespaceToken> Tokenizer::consumeWhitespace(ParserPos& parserPos) noexcept
{
  if (parserPos.pos_.size() < 1)
    return {};

  auto isSpace{ [&](char c) noexcept -> bool {
    if (c < 0)
      return false;
    if (isspace(c) || iscntrl(c))
      return true;
    return false;
  } };

  if (!isSpace(*parserPos.pos_.data()))
    return {};

  WhitespaceToken result;

  const char* start{ parserPos.pos_.data() };
  auto end = start + parserPos.pos_.length();
  auto pos{ start };
  const char* firstNewLine{};

  while (pos < end) {
    if (*pos < 0)
      break;
    if (!isSpace(*pos))
      break;

    if (!firstNewLine) {
      if (('\n' == *pos) ||
          ('\v' == *pos))
        firstNewLine = pos;
    }

    count(parserPos, *pos);
    ++pos;
  }

  result.originalToken_ = makeStringView(start, pos);
  if (firstNewLine) {
    result.token_ = makeStringView(firstNewLine, firstNewLine + 1);
    result.addNewLine_ = true;
  }
  else
    result.token_ = result.originalToken_;

  parserPos.pos_ = makeStringView(pos, end);
  return result;
}

//-----------------------------------------------------------------------------
optional<TokenizerTypes::QuoteToken> Tokenizer::consumeQuote(ParserPos& parserPos) noexcept
{
  if (parserPos.pos_.size() < 1)
    return {};

  char c{ *parserPos.pos_.data() };
  if ((c != '\'') && (c != '\"'))
    return {};

  count(parserPos, c);

  QuoteToken result;
  const char* start{ parserPos.pos_.data() };
  auto startAfterOpen{ start + 1 };
  auto end = start + parserPos.pos_.length();
  auto pos{ startAfterOpen };
  auto preFinal{ end };

  while (pos < end) {
    if (*pos == c) {
      preFinal = pos;
      count(parserPos, *pos);
      ++pos;
      break;
    }

    if (('\n' == *pos) ||
        ('\r' == *pos) ||
        ('\v' == *pos))
      break;

    count(parserPos, *pos);
    ++pos;
  }

  result.originalToken_ = makeStringView(start, pos);
  result.foundEnding_ = (preFinal != end);
  result.token_ = makeStringView(startAfterOpen, result.foundEnding_ ? preFinal : pos);
  parserPos.pos_ = makeStringView(pos, end);
  return result;
}

//-----------------------------------------------------------------------------
optional<TokenizerTypes::LiteralToken> Tokenizer::consumeLiteral(ParserPos& parserPos) noexcept
{
  if (parserPos.pos_.size() < 1)
    return {};

  auto isLiteral{ [&](char c) noexcept -> bool {
    if (c < 0)
      return true;
    if (isalnum(c))
      return true;
    if ('_' == c)
        return true;
    return false;
  } };

  auto isFirstLiteral{ [&](char c) noexcept -> bool {
    if (c < 0)
      return true;
    if (isdigit(c))
      return false;
    return isLiteral(c);
  } };

  char c{ *parserPos.pos_.data() };
  if (!isFirstLiteral(c))
    return {};

  count(parserPos, c);

  LiteralToken result;
  const char* start{ parserPos.pos_.data() };
  auto startAfterOpen{ start + 1};
  auto end = start + parserPos.pos_.length();
  auto pos{ startAfterOpen };

  while (pos < end) {
    if (!isLiteral(*pos))
      break;
    count(parserPos, *pos);
    ++pos;
  }

  result.token_ = makeStringView(start, pos);
  parserPos.pos_ = makeStringView(pos, end);
  return result;
}

//-----------------------------------------------------------------------------
optional<TokenizerTypes::NumericToken> Tokenizer::consumeNumeric(ParserPos& parserPos) noexcept
{
  if (parserPos.pos_.size() < 1)
    return {};

  auto isFirstNumeric{ [&](char c) noexcept -> bool {
    if (c < 0)
      return false;
    if (isdigit(c))
      return true;
    if ('.' == c) {
      if (parserPos.pos_.length() < 2)
        return false;
      char c2{ *(parserPos.pos_.data() + 1) };
      return isdigit(c2);
    }
    return false;
  } };

  char c{ *parserPos.pos_.data() };
  if (!isFirstNumeric(c))
    return {};

  NumericToken result;
  const char* start{ parserPos.pos_.data() };
  auto end = start + parserPos.pos_.length();
  auto pos{ start };

  bool foundDot{};
  bool foundE{};
  bool foundSign{};
  bool lastWasE{};
  bool lastWasLegal{};

  while (pos < end) {
    if (*pos < 0)
      break;
    if (!isdigit(*pos)) {
      if ('.' == *pos) {
        if ((foundDot) || (foundE)) {
          result.illegalSequence_ = true;
          break;
        }
        foundDot = true;
        lastWasLegal = true;
        count(parserPos, *pos);
        ++pos;
        continue;
      }
      if (('e' == *pos) || ('E' == *pos)) {
        if (foundE) {
          result.illegalSequence_ = true;
          break;
        }
        foundE = true;
        lastWasE = true;
        lastWasLegal = false;
        count(parserPos, *pos);
        ++pos;
        continue;
      }
      if (('+' == *pos) || ('-' == *pos)) {
        if (foundSign)
          break;
        if (!lastWasE)
          break;
        foundSign = true;
        lastWasLegal = false;
        lastWasE = false;
        count(parserPos, *pos);
        ++pos;
        continue;
      }
      break;
    }

    lastWasE = false;
    lastWasLegal = true;
    count(parserPos, *pos);
    ++pos;
  }
  if (!lastWasLegal)
    result.illegalSequence_ = true;

  result.token_ = makeStringView(start, pos);
  parserPos.pos_ = makeStringView(pos, end);
  return result;
}

//-----------------------------------------------------------------------------
optional<TokenizerTypes::OperatorToken> Tokenizer::consumeOperator(const OperatorLut& lut, ParserPos& parserPos) noexcept
{
  auto oper = lut.lookup(parserPos.pos_);
  if (!oper)
    return {};

  auto operAsStr = lut.lookup(*oper);

  parserPos.location_.column_ += operAsStr.length();
  parserPos.actualLocation_.column_ = parserPos.location_.column_;

  TokenizerTypes::OperatorToken result;
  result.operator_ = *oper;
  result.token_ = operAsStr;
  parserPos.pos_ = parserPos.pos_.substr(operAsStr.length());
  return result;
}

//-----------------------------------------------------------------------------
optional<TokenizerTypes::IllegalToken> Tokenizer::consumeKnownIllegalToken(ParserPos& parserPos) noexcept
{
  if (parserPos.pos_.size() < 1)
    return {};

  char c{ *parserPos.pos_.data() };

  IllegalToken result;
  const char* start{ parserPos.pos_.data() };
  auto end = start + parserPos.pos_.length();
  auto pos{ start };

  while (pos < end) {
    if (c != *pos)
      break;
    count(parserPos, *pos);
    ++pos;
  }

  result.token_ = makeStringView(start, pos);
  parserPos.pos_ = makeStringView(pos, end);
  return result;
}

//-----------------------------------------------------------------------------
void Tokenizer::prime() noexcept
{
  if (!parsedTokens_.empty())
    return;
  primeNext();
}

//-----------------------------------------------------------------------------
void Tokenizer::prime() const noexcept
{
  if (!parsedTokens_.empty())
    return;
  primeNext();
}

//-----------------------------------------------------------------------------
void Tokenizer::primeNext() noexcept
{
  bool firstPrime{ reinterpret_cast<const char *>(rawContents_.first.get()) == parserPos_.pos_.data() };
  if (firstPrime)
    consumeUtf8Bom(parserPos_);

  auto oldPos{ parserPos_ };

  auto makeToken{ [&]() noexcept -> TokenPtr {
    auto token{ std::make_shared<Token>() };
    token->origin_.filePath_ = filePath_;
    token->origin_.location_ = oldPos.location_;
    token->actualOrigin_.filePath_ = actualFilePath_;
    token->actualOrigin_.location_ = oldPos.actualLocation_;
    token->compileState_ = compileState_;
    return token;
  } };

  auto whitespace{ [&](
    bool skipWhitespace,
    bool& outDidConsumeWhitespace,
    bool& outContainedNewline) noexcept -> bool {
    auto value{ consumeWhitespace(parserPos_) };
    if (!value)
      return false;

    outDidConsumeWhitespace = true;
    if (value->addNewLine_) {
      outContainedNewline = true;
      if (!skipWhitespace) {
        auto tokenNewLine{ makeToken() };
        tokenNewLine->type_ = TokenTypes::Type::Separator;
        tokenNewLine->originalToken_ = value->originalToken_;
        tokenNewLine->token_ = value->token_;
        parsedTokens_.pushBack(tokenNewLine);
        return true;
      }
    }
    return false;
  } };

  auto comment{ [&](
    bool skipComments,
    bool allowedToInsertNewLine,
    bool& outDidConsumeComment,
    bool& outContainedNewline) noexcept -> bool {
    auto value{ consumeComment(parserPos_) };
    if (!value)
      return false;

    outDidConsumeComment = true;

    auto token{ makeToken() };
    token->type_ = TokenTypes::Type::Comment;
    token->originalToken_ = value->originalToken_;
    token->token_ = value->token_;
    if (!skipComments)
      parsedTokens_.pushBack(token);

    if (value->addNewLine_) {
      outContainedNewline = true;
      if ((!skipComments) &&
          (allowedToInsertNewLine)) {
        auto tokenNewLine{ makeToken() };
        tokenNewLine->type_ = TokenTypes::Type::Separator;
        tokenNewLine->originalToken_ = value->originalToken_;
        tokenNewLine->token_ = value->token_;
        parsedTokens_.pushBack(tokenNewLine);
      }
    }

    if (!value->foundEnding_)
      out(ErrorTypes::Error::MissingEndOfComment, token);

    return !skipComments;
  } };

  auto quote{ [&]() noexcept -> bool {
    auto value{ consumeQuote(parserPos_) };
    if (!value)
      return false;

    auto token{ makeToken() };
    token->type_ = TokenTypes::Type::Quote;
    token->originalToken_ = value->originalToken_;
    token->token_ = value->token_;
    parsedTokens_.pushBack(token);

    if (!value->foundEnding_)
      out(ErrorTypes::Error::MissingEndOfQuote, token);

    return true;
  } };

  auto literal{ [&]() noexcept -> bool {
    auto value{ consumeLiteral(parserPos_) };
    if (!value)
      return false;

    auto token{ makeToken() };
    token->type_ = TokenTypes::Type::Literal;
    token->originalToken_ = value->token_;
    token->token_ = value->token_;
    parsedTokens_.pushBack(token);
    return true;
  } };

  auto numeric{ [&]() noexcept -> bool {
    auto value{ consumeNumeric(parserPos_) };
    if (!value)
      return false;

    auto token{ makeToken() };
    token->type_ = TokenTypes::Type::Number;
    token->originalToken_ = value->token_;
    token->token_ = value->token_;
    if (value->illegalSequence_)
      out(ErrorTypes::Error::ConstantSyntax, token);

    parsedTokens_.pushBack(token);
    return true;
  } };

  auto oper{ [&](bool& outIsContinuation) noexcept -> bool {
    auto value{ consumeOperator(*operatorLut_, parserPos_) };
    if (!value)
      return false;

    if (value->operator_ == TokenTypes::Operator::Continuation) {
      outIsContinuation = true;
      return false;
    }

    auto token{ makeToken() };
    token->type_ = TokenTypes::Type::Operator;
    token->originalToken_ = value->token_;
    token->token_ = value->token_;
    token->operator_ = value->operator_;

    if (value->operator_ == TokenTypes::Operator::StatementSeparator) {
      token->type_ = TokenTypes::Type::Separator;
      token->forcedSeparator_ = true;
    }

    parsedTokens_.pushBack(token);
    return true;
  } };

  auto illegal{ [&]() noexcept -> bool {
    auto value{ consumeKnownIllegalToken(parserPos_) };
    if (!value)
      return true;

    auto token{ makeToken() };
    token->type_ = TokenTypes::Type::Literal;
    token->originalToken_ = value->token_;
    token->token_ = value->token_;

    out(ErrorTypes::Error::Syntax, token);
    return false;
  } };

  while (true) {
    bool didConsume{};
    bool containedNewline{};
    if (whitespace(false, didConsume, containedNewline))
      return;
    if (didConsume) {
      oldPos = parserPos_;
      continue;
    }
    if (comment(skipComments_, true, didConsume, containedNewline))
      return;
    if (didConsume) {
      oldPos = parserPos_;
      continue;
    }
    if (quote())
      return;
    if (literal())
      return;
    if (numeric())
      return;

    bool isContinuation{};
    if (oper(isContinuation))
      return;

    if (isContinuation) {
      while (true) {
        oldPos = parserPos_;
        didConsume = false;
        containedNewline = false;
        (void)comment(skipComments_, false, didConsume, containedNewline);
        if (containedNewline)
          break;

        oldPos = parserPos_;
        (void)whitespace(true, didConsume, containedNewline);
        if (containedNewline)
          break;
        if (!didConsume)
          break;
      }
      oldPos = parserPos_;

      if (!containedNewline) {
        auto token{ makeToken() };
        token->type_ = TokenTypes::Type::Literal;
        token->originalToken_ = parserPos_.pos_.size() > 0 ? parserPos_.pos_.substr(0, 1) : StringView{};
        token->token_ = token->originalToken_;

        out(WarningTypes::Warning::NewlineAfterContinuation, token);
      }
      oldPos = parserPos_;
      continue;
    }

    if (illegal())
      return;

    oldPos = parserPos_;
  }
}

//-----------------------------------------------------------------------------
void Tokenizer::primeNext() const noexcept
{
  // mutable lazy iterator is able to modify contents even though it's
  // technically supposed to be a const operation (and it is in the sense
  // that the list contents are not being modified)
  const_cast<Tokenizer*>(this)->primeNext();
}

//-----------------------------------------------------------------------------
TokenList::iterator Tokenizer::tokenListBegin() noexcept
{
  prime();
  return parsedTokens_.begin();
}

//-----------------------------------------------------------------------------
TokenList::const_iterator Tokenizer::tokenListBegin() const noexcept
{
  prime();
  return parsedTokens_.begin();
}

//-----------------------------------------------------------------------------
TokenList::const_iterator Tokenizer::tokenListCBegin() const noexcept
{
  prime();
  return parsedTokens_.cbegin();
}

//-----------------------------------------------------------------------------
TokenList::iterator Tokenizer::tokenListEnd() noexcept
{
  return parsedTokens_.end();
}

//-----------------------------------------------------------------------------
TokenList::const_iterator Tokenizer::tokenListEnd() const noexcept
{
  return parsedTokens_.end();
}

//-----------------------------------------------------------------------------
TokenList::const_iterator Tokenizer::tokenListCEnd() const noexcept
{
  return parsedTokens_.cend();
}

//-----------------------------------------------------------------------------
void Tokenizer::advance(TokenList::iterator& iter) noexcept
{
  if (iter == parsedTokens_.end())
    return;

  // attempt to use previously parsed token if available
  auto temp{ iter };
  ++temp;
  if (temp != parsedTokens_.end()) {
    iter = temp;
    return;
  }

  primeNext();
  ++iter;
}

//-----------------------------------------------------------------------------
void Tokenizer::advance(TokenList::const_iterator& iter) const noexcept
{
  if (iter == parsedTokens_.end())
    return;

  // attempt to use previously parsed token if available
  auto temp{ iter };
  ++temp;
  if (temp != parsedTokens_.end()) {
    iter = temp;
    return;
  }

  primeNext();
  ++iter;
}

//-----------------------------------------------------------------------------
TokenList Tokenizer::extractFromStartToPos(index_type count) noexcept
{
  if (count > 0)
    ensurePosExists(count - 1);
  return parsedTokens_.extractFromStartToPos(count);
}

//-----------------------------------------------------------------------------
TokenList Tokenizer::extractFromPosToEnd(index_type count) noexcept
{
  ensurePosExists(count);
  return parsedTokens_.extractFromPosToEnd(count);
}

//-----------------------------------------------------------------------------
void Tokenizer::erase(index_type count) noexcept
{
  ensurePosExists(count);
  parsedTokens_.erase(count);
}

//-----------------------------------------------------------------------------
void Tokenizer::pushFront(TokenPtr& token) noexcept
{
  parsedTokens_.pushFront(token);
}

//-----------------------------------------------------------------------------
void Tokenizer::pushBack(TokenPtr& token) noexcept
{
  parsedTokens_.pushBack(token);
}

//-----------------------------------------------------------------------------
void Tokenizer::pushFront(TokenPtr&& token) noexcept
{
  parsedTokens_.pushFront(std::move(token));
}

//-----------------------------------------------------------------------------
void Tokenizer::pushBack(TokenPtr&& token) noexcept
{
  parsedTokens_.pushBack(std::move(token));
}

//-----------------------------------------------------------------------------
TokenPtr Tokenizer::popFront() noexcept
{
  return parsedTokens_.popFront();
}

//-----------------------------------------------------------------------------
TokenPtr Tokenizer::popBack() noexcept
{
  return parsedTokens_.popBack();
}

//-----------------------------------------------------------------------------
TokenPtr Tokenizer::front() noexcept
{
  prime();
  return parsedTokens_.front();
}

//-----------------------------------------------------------------------------
TokenPtr Tokenizer::back() noexcept
{
  prime();
  return parsedTokens_.back();
}

//-----------------------------------------------------------------------------
TokenConstPtr Tokenizer::front() const noexcept
{
  prime();
  return parsedTokens_.front();
}

//-----------------------------------------------------------------------------
TokenConstPtr Tokenizer::back() const noexcept
{
  prime();
  return parsedTokens_.back();
}

//-----------------------------------------------------------------------------
void Tokenizer::extractThenPushFront(TokenList& rhs) noexcept
{
  parsedTokens_.extractThenPushFront(rhs);
}

//-----------------------------------------------------------------------------
void Tokenizer::extractThenPushBack(TokenList& rhs) noexcept
{
  parsedTokens_.extractThenPushBack(rhs);
}

//-----------------------------------------------------------------------------
void Tokenizer::extractThenPushFront(TokenList&& rhs) noexcept
{
  parsedTokens_.extractThenPushFront(std::move(rhs));
}

//-----------------------------------------------------------------------------
void Tokenizer::extractThenPushBack(TokenList&& rhs) noexcept
{
  parsedTokens_.extractThenPushBack(std::move(rhs));
}

//-----------------------------------------------------------------------------
void Tokenizer::copyPushFront(const TokenList& rhs) noexcept
{
  parsedTokens_.copyPushFront(rhs);
}

//-----------------------------------------------------------------------------
void Tokenizer::copyPushBack(const TokenList& rhs) noexcept
{
  parsedTokens_.copyPushBack(rhs);
}

//-----------------------------------------------------------------------------
TokenPtr Tokenizer::operator[](index_type count) noexcept
{
  ensurePosExists(count);
  return parsedTokens_[count];
}

//-----------------------------------------------------------------------------
const TokenConstPtr Tokenizer::operator[](index_type count) const noexcept
{
  ensurePosExists(count);
  return parsedTokens_[count];
}

//-----------------------------------------------------------------------------
Tokenizer::iterator Tokenizer::at(index_type count) noexcept
{
  ensurePosExists(count);
  return iterator{ *this, parsedTokens_.at(count) };
}

//-----------------------------------------------------------------------------
Tokenizer::const_iterator Tokenizer::at(index_type count) const noexcept
{
  ensurePosExists(count);
  return const_iterator{ *this, parsedTokens_.at(count) };
}

//-----------------------------------------------------------------------------
bool Tokenizer::empty() const noexcept
{
  prime();
  return parsedTokens_.empty();
}

//-----------------------------------------------------------------------------
void Tokenizer::clear() noexcept
{
  parsedTokens_.clear();

  if (parserPos_.pos_.size() < 1)
    return;

  const char* start{ parserPos_.pos_.data() };
  auto end = start + parserPos_.pos_.length();
  auto pos{ start };

  while (pos < end) {
    count(parserPos_, *pos);
    ++pos;
  }

  parserPos_.pos_ = makeStringView(pos, end);
}

//-----------------------------------------------------------------------------
Tokenizer::size_type Tokenizer::size() const noexcept
{
  size_type totalSize{ parsedTokens_.size() };
  totalSize += parserPos_.pos_.size();
  return totalSize;
}

//-----------------------------------------------------------------------------
TokenList Tokenizer::extract(iterator first, iterator last) noexcept
{
  assert(&(first.list()) == this);
  return zax::extract(first, last);
}

//-----------------------------------------------------------------------------
TokenList Tokenizer::extract(iterator first, index_type count) noexcept
{
  assert(&(first.list()) == this);
  return zax::extract(first, first + count);
}

//-----------------------------------------------------------------------------
TokenList Tokenizer::extract(index_type first, index_type count) noexcept
{
  return zax::extract(begin() + first, count);
}

//-----------------------------------------------------------------------------
TokenList Tokenizer::extractFromStartToPos(iterator pos) noexcept
{
  assert(&(pos.list()) == this);
  return zax::extractFromStartToPos(pos);
}

//-----------------------------------------------------------------------------
TokenList Tokenizer::extractFromPosToEnd(iterator pos) noexcept
{
  assert(&(pos.list()) == this);
  return zax::extractFromPosToEnd(pos);
}

//-----------------------------------------------------------------------------
void Tokenizer::erase(iterator pos) noexcept
{
  assert(&(pos.list()) == this);
  zax::erase(pos);
}

//-----------------------------------------------------------------------------
void Tokenizer::erase(iterator first, iterator last) noexcept
{
  assert(&(first.list()) == this);
  zax::erase(first, last);
}

//-----------------------------------------------------------------------------
void Tokenizer::erase(iterator first, index_type count) noexcept
{
  assert(&(first.list()) == this);
  zax::erase(first, count);
}

//-----------------------------------------------------------------------------
void Tokenizer::insertBefore(iterator pos, TokenPtr& token) noexcept
{
  assert(&(pos.list()) == this);
  zax::insertBefore(pos, token);
}

//-----------------------------------------------------------------------------
void Tokenizer::insertAfter(iterator pos, TokenPtr& token) noexcept
{
  assert(&(pos.list()) == this);
  zax::insertAfter(pos, token);
}

//-----------------------------------------------------------------------------
void Tokenizer::insert(iterator pos, TokenPtr& token) noexcept
{
  assert(&(pos.list()) == this);
  zax::insert(pos, token);
}

//-----------------------------------------------------------------------------
void Tokenizer::insertCopyBefore(iterator pos, const TokenList& rhs) noexcept
{
  assert(&(pos.list()) == this);
  zax::insertCopyBefore(pos, rhs);
}

//-----------------------------------------------------------------------------
void Tokenizer::insertCopyAfter(iterator pos, const TokenList& rhs) noexcept
{
  assert(&(pos.list()) == this);
  zax::insertCopyAfter(pos, rhs);
}

//-----------------------------------------------------------------------------
void Tokenizer::ensurePosExists(index_type pos) noexcept
{
  if (pos < 0)
    return;
  auto iter{ begin() };
  while (pos > 0) {
    if (iter == end())
      break;
    ++iter;
    --pos;
  }
}

//-----------------------------------------------------------------------------
void Tokenizer::ensurePosExists(index_type pos) const noexcept
{
  if (pos < 0)
    return;
  auto iter{ begin() };
  while (pos > 0) {
    if (iter == end())
      break;
    ++iter;
    --pos;
  }
}

//-----------------------------------------------------------------------------
void Tokenizer::out(ErrorTypes::Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  errorCallback_(error, token, mapping);
}

//-----------------------------------------------------------------------------
void Tokenizer::out(WarningTypes::Warning warning, const TokenConstPtr& token, const StringMap& mapping) noexcept
{
  warningCallback_(warning, token, mapping);
}


//-----------------------------------------------------------------------------
TokenList zax::extract(Tokenizer::iterator first, Tokenizer::iterator last) noexcept
{
  assert(first.valid());
  assert(last.valid());
  auto& list{ first.list() };
  assert(&list == &last.list());
  return list.parsedTokens_.extract(first.underlying(), last.underlying());
}

//-----------------------------------------------------------------------------
TokenList zax::extract(Tokenizer::iterator first, index_type count) noexcept
{
  assert(first.valid());
  auto& list{ first.list() };

  if (count > 1)
    (void)list.hasAhead(first, count - 1);
  return list.parsedTokens_.extract(first.underlying(), count);
}

//-----------------------------------------------------------------------------
TokenList zax::extractFromStartToPos(Tokenizer::iterator pos) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.extractFromStartToPos(pos.underlying());
}

//-----------------------------------------------------------------------------
TokenList zax::extractFromPosToEnd(Tokenizer::iterator pos) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.extractFromPosToEnd(pos.underlying());
}

//-----------------------------------------------------------------------------
void zax::erase(Tokenizer::iterator pos) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.erase(pos.underlying());
}

//-----------------------------------------------------------------------------
void zax::erase(Tokenizer::iterator first, Tokenizer::iterator last) noexcept
{
  assert(first.valid());
  assert(last.valid());
  auto& list{ first.list() };
  assert(&list == &last.list());
  return list.parsedTokens_.erase(first.underlying(), last.underlying());
}

//-----------------------------------------------------------------------------
void zax::erase(Tokenizer::iterator first, index_type count) noexcept
{
  assert(first.valid());
  auto& list{ first.list() };

  if (count > 1)
    (void)list.hasAhead(first, count - 1);
  return list.parsedTokens_.erase(first.underlying(), count);
}

//-----------------------------------------------------------------------------
void zax::insertBefore(Tokenizer::iterator pos, TokenPtr& token) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.insertBefore(pos.underlying(), token);
}

//-----------------------------------------------------------------------------
void zax::insertAfter(Tokenizer::iterator pos, TokenPtr& token) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.insertAfter(pos.underlying(), token);
}

//-----------------------------------------------------------------------------
void zax::insert(Tokenizer::iterator pos, TokenPtr& token) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.insert(pos.underlying(), token);
}

//-----------------------------------------------------------------------------
void zax::insertCopyBefore(Tokenizer::iterator pos, const TokenList& rhs) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.insertCopyBefore(pos.underlying(), rhs);
}

//-----------------------------------------------------------------------------
void zax::insertCopyAfter(Tokenizer::iterator pos, const TokenList& rhs) noexcept
{
  assert(pos.valid());
  auto& list{ pos.list() };
  return list.parsedTokens_.insertCopyAfter(pos.underlying(), rhs);
}
