
#pragma once

#include "types.h"
#include "helpers.h"
#include "Token.h"

namespace zax {

struct TokenizerTypes
{

  struct ParserPos
  {
    StringView pos_;
    int line_{ 1 };
    int column_{ 1 };
    int tabSize_{ 8 };
    int utf8Count_{};

    int overrideLine{ 1 };
    int overrideSkip{ 1 };

    bool operator==(const ParserPos& rhs) const noexcept;
    bool sameLocation(const ParserPos& rhs) const noexcept;
  };

  struct CommentToken
  {
    StringView originalToken_;
    StringView token_;
    bool foundEnding_{};
    bool addNewLine_{};
  };

  struct WhitespaceToken
  {
    StringView originalToken_;
    StringView token_;
    bool addNewLine_{};
  };

  struct QuoteToken
  {
    StringView originalToken_;
    StringView token_;
    bool foundEnding_{};
  };

  struct LiteralToken
  {
    StringView token_;
  };

  struct NumericToken
  {
    StringView token_;
    bool illegalSequence_{};
  };

  struct OperatorToken
  {
    StringView token_;
    TokenTypes::Operator operator_{};
  };

  struct IllegalToken
  {
    StringView token_;
  };

};

struct Tokenizer : public TokenizerTypes
{
  Puid id_{ puid() };

  Tokenizer() noexcept;
  ~Tokenizer() noexcept;

  static void count(ParserPos& parserPos, char let) noexcept;
  static void advance(ParserPos& parserPos, StringView str) noexcept                { advance(parserPos, str.length()); }
  static void advance(ParserPos& parserPos, size_t length) noexcept                 { parserPos.column_ += SafeInt<decltype(parserPos.column_)>(length); }
  static bool consumeUtf8Bom(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<CommentToken> consumeComment(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<WhitespaceToken> consumeWhitespace(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<QuoteToken> consumeQuote(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<LiteralToken> consumeLiteral(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<NumericToken> consumeNumeric(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<OperatorToken> consumeOperator(const OperatorLut& lut, ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<IllegalToken> consumeKnownIllegalToken(ParserPos& parserPos) noexcept;

private:
};

} // namespace zax
