
#include "pch.h"
#include "Tokenizer.h"
#include "OperatorLut.h"

using namespace zax;
using namespace std::string_view_literals;

//-----------------------------------------------------------------------------
bool TokenizerTypes::ParserPos::operator==(const ParserPos& rhs) const noexcept
{
  return
    (line_ == rhs.line_) &&
    (column_ == rhs.column_) &&
    (overrideLine == rhs.overrideLine) &&
    (pos_ == rhs.pos_) &&
    (tabSize_ == rhs.tabSize_) &&
    (overrideSkip == rhs.overrideSkip);
}

//-----------------------------------------------------------------------------
bool TokenizerTypes::ParserPos::sameLocation(const ParserPos& rhs) const noexcept
{
  return
    (line_ == rhs.line_) &&
    (column_ == rhs.column_) &&
    (overrideLine == rhs.overrideLine);
}

//-----------------------------------------------------------------------------
Tokenizer::Tokenizer() noexcept
{
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
    ++parserPos.column_;
    return;
  }

  parserPos.utf8Count_ = 0;
  if (!iscntrl(c)) {
    ++parserPos.column_;
    return;
  }

  parserPos.utf8Count_ = 0;

  switch (c) {
    case '\r':  {
      parserPos.column_ = 1;
      return;
    }
    case '\f':
    case '\n':  {
      ++parserPos.line_;
      parserPos.overrideLine += parserPos.overrideSkip;
      parserPos.column_ = 1;
      return;
    }
    case '\v':  {
      ++parserPos.line_;
      parserPos.overrideLine += parserPos.overrideSkip;
      break;
    }
    case '\t':  {
      parserPos.column_ += parserPos.tabSize_ - ((parserPos.column_ - 1) % parserPos.tabSize_);
      return;
    }
    case '\b':  {
      parserPos.column_ = std::max(1, parserPos.column_ - 1);
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
    auto startLine = parserPos.line_;
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
      .addNewLine_ = (startLine != parserPos.line_)
    };
  }

  // handle `/*` `*/` comments
  if (parserPos.pos_.substr(0, slashSlash.length()) == slashStar) {
    auto startLine = parserPos.line_;
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
      .addNewLine_ = (startLine != parserPos.line_)
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

    count(parserPos, *pos);
    ++pos;
  }

  result.originalToken_ = makeStringView(start, pos);
  result.token_ = makeStringView(startAfterOpen, preFinal);
  parserPos.pos_ = makeStringView(pos, end);
  result.foundEnding_ = (preFinal != end);
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
    if (c < 0)
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

  parserPos.column_ += operAsStr.length();

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
