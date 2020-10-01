
#pragma once

#include "types.h"
#include "helpers.h"
#include "Token.h"
#include "TokenList.h"
#include "Source.h"
#include "Errors.h"
#include "Warnings.h"

namespace zax {

struct TokenizerTypes
{
  using index_type = zs::index_type;
  using size_type = zs::size_type;

  struct ParserPos
  {
    StringView pos_;
    SourceTypes::Location location_;
    SourceTypes::Location actualLocation_;
    int utf8Count_{};

    int lineSkip_{ 1 };
    int tabStopWidth_{ 8 };

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

// The Tokenizer performs lazy iteration over a raw parser buffer and returns
// parsed token views into the owned content buffer. One important note, as the
// parser is a lazy parser, the "end" isn't the end of the parsed file but rather
// it's the end of the parsed tokens thus far.
struct Tokenizer : public TokenizerTypes
{

public:
  const Puid id_{ puid() };
  TokenList parsedTokens_;

  SourceTypes::FilePathPtr filePath_;
  SourceTypes::FilePathPtr actualFilePath_;
  std::pair< std::unique_ptr<std::byte[]>, size_t> rawContents_;
  const std::byte* raw_{};
  OperatorLutConstPtr operatorLut_;

  ParserPos parserPos_;
  bool skipComments_{};
  TokenPtr pendingComment_;

  std::function<CompileStateConstPtr()> getState_;
  std::function<void(ErrorTypes::Error, const TokenConstPtr&, const StringMap&)> errorCallback_;
  std::function<void(WarningTypes::Warning, const TokenConstPtr&, const StringMap&)> warningCallback_;

public:

  Tokenizer(
    const SourceTypes::FilePathPtr &filePath,
    std::pair< std::unique_ptr<std::byte[]>, size_t>&& rawContents,
    const OperatorLutConstPtr& operatorLut,
    decltype(getState_)&& getState
  ) noexcept;
  Tokenizer(
    const Tokenizer& original,
    TokenList&& tokenList) noexcept;
  ~Tokenizer() noexcept;

  static void count(ParserPos& parserPos, char let) noexcept;
  static void advance(ParserPos& parserPos, StringView str) noexcept                { advance(parserPos, str.length()); }
  static void advance(ParserPos& parserPos, size_t length) noexcept                 { parserPos.location_.column_ += SafeInt<decltype(parserPos.location_.column_)>(length); }
  static bool consumeUtf8Bom(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<CommentToken> consumeComment(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<WhitespaceToken> consumeWhitespace(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<QuoteToken> consumeQuote(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<LiteralToken> consumeLiteral(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<NumericToken> consumeNumeric(ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<OperatorToken> consumeOperator(const OperatorLut& lut, ParserPos& parserPos) noexcept;
  [[nodiscard]] static optional<IllegalToken> consumeKnownIllegalToken(ParserPos& parserPos) noexcept;

public:
  template <typename TList>
  struct LazyIterator;

  template <typename TList>
  using lazy_iterator = LazyIterator<std::remove_cvref_t<TList>>;

  template <typename TList>
  using const_lazy_iterator = LazyIterator<std::add_const_t<std::remove_cvref_t<TList>>>;

  using iterator = lazy_iterator<Tokenizer>;
  using const_iterator = const_lazy_iterator<const Tokenizer>;

protected:

  template <typename TList>
  friend struct LazyIterator;

  void prime() noexcept;
  void prime() const noexcept;

  void primeNext() noexcept;
  void primeNext() const noexcept;

  TokenList::iterator tokenListBegin() noexcept;
  TokenList::const_iterator tokenListBegin() const noexcept;
  TokenList::const_iterator tokenListCBegin() const noexcept;

  TokenList::iterator tokenListEnd() noexcept;
  TokenList::const_iterator tokenListEnd() const noexcept;
  TokenList::const_iterator tokenListCEnd() const noexcept;

  void advance(TokenList::iterator& iter) noexcept;
  void advance(TokenList::const_iterator& iter) const noexcept;

public:

  template <typename TList>
  struct LazyIterator
  {
    friend struct LazyIterator;

    using List = TList;
    using MutableList = std::remove_const_t<List>;
    using ImmutableList = std::add_const_t<List>;
    using UseList = List;

    using MutableTokenList = std::remove_const_t<TokenList>;
    using ImmutableTokenList = std::add_const_t<TokenList>;
    using UseTokenList = TokenList;

    using list_iterator = typename MutableTokenList::iterator;
    using const_list_iterator = typename ImmutableTokenList::const_iterator;
    using use_list_iterator = std::conditional_t<std::is_const_v<List>, const_list_iterator, list_iterator>;

    using iterator = LazyIterator<MutableList>;
    using const_iterator = LazyIterator<ImmutableList>;
    using use_iterator = LazyIterator<List>;

    using index_type = typename List::index_type;
    using size_type = typename List::size_type;

    LazyIterator() noexcept = default;
    LazyIterator(const LazyIterator&) noexcept = default;
    LazyIterator(LazyIterator&&) noexcept = default;

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    LazyIterator(const LazyIterator& rhs) noexcept : list_{ rhs.list_ }, iterator_{ rhs.tokenListBegin() } {}

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    LazyIterator(const ImmutableList& rhs) noexcept : list_{ &rhs }, iterator_{ rhs.tokenListBegin() } {}
    template <typename T = MutableList, typename std::enable_if_t<!std::is_const_v<UseList>, T>* = nullptr>
    LazyIterator(MutableList& rhs) noexcept : list_{ &rhs }, iterator_{ rhs.tokenListBegin() } {}

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    LazyIterator(const ImmutableList& rhs, typename const_list_iterator iter) noexcept : list_{ &rhs }, iterator_{ iter } {}
    template <typename T = MutableList, typename std::enable_if_t<!std::is_const_v<UseList>, T>* = nullptr>
    LazyIterator(MutableList& rhs, typename list_iterator iter) noexcept : list_{ &rhs }, iterator_{ iter } {}

    LazyIterator& operator=(const LazyIterator&) noexcept = default;
    LazyIterator& operator=(LazyIterator&&) noexcept = default;

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    auto& operator=(const ImmutableList& value) noexcept { list_ = &value; iterator_ = value.tokenListBegin(); return *this; }

    template <typename T = MutableList, typename std::enable_if_t<!std::is_const_v<UseList>, T>* = nullptr>
    auto& operator=(MutableList& value) noexcept { list_ = &value; iterator_ = value.tokenListBegin(); return *this; }

    auto& operator++() noexcept
    {
      if (list_) {
        list_->advance(iterator_);
      }
      return *this;
    }

    auto& operator--() noexcept
    {
      if (list_) {
        if (iterator_ != list_->tokenListBegin())
          --iterator_;
      }
      return *this;
    }

    [[nodiscard]] bool empty() const noexcept
    {
      return list_ ? list_->empty() : true;
    }

    [[nodiscard]] use_list_iterator underlying() noexcept
    {
      return iterator_;
    }

    [[nodiscard]] auto& list() noexcept
    {
      assert(list_);
      return *list_;
    }

    [[nodiscard]] bool has_value() noexcept
    {
      return list_ ? list_->tokenListEnd() != iterator_ : false;
    }

    [[nodiscard]] bool valid() noexcept
    {
      return list_;
    }

    [[nodiscard]] explicit operator bool() noexcept
    {
      return has_value();
    }

    [[nodiscard]] bool isBegin() noexcept
    {
      return list_ ? list_->tokenListBegin() == iterator_ : true;
    }

    [[nodiscard]] bool isEnd() noexcept
    {
      return list_ ? list_->tokenListEnd() == iterator_ : true;
    }

    [[nodiscard]] auto size() noexcept
    {
      return list_ ? list_->size() : static_cast<typename std::remove_pointer_t<decltype(list_)>::size_type>(0);
    }

    [[nodiscard]] bool hasAhead(index_type size) noexcept
    {
      if (!list_)
        return false;

      if (size < 0)
        return hasBehind(size * static_cast<index_type>(-1));

      auto aheadIter = iterator_;
      if (list_->tokenListEnd() == aheadIter)
        return 0 == size;

      list_->advance(aheadIter);
      while (size > 0) {
        if (list_->tokenListEnd() == aheadIter)
          break;
        --size;
        list_->advance(aheadIter);
      }
      return 0 == size;
    }

    [[nodiscard]] bool hasBehind(index_type size) noexcept
    {
      if (!list_)
        return false;
      if (size < 0)
        return hasAhead(size * static_cast<index_type>(-1));

      auto behindIter = iterator_;
      while (size > 0) {
        if (list_->tokenListBegin() == behindIter)
          break;
        --size;
        --behindIter;
      }
      return 0 == size;
    }

    LazyIterator& operator+=(index_type distance) noexcept
    {
      if (distance < 0)
        return *this -= (distance * static_cast<index_type>(-1));

      if (list_) {
        while ((iterator_ != list_->tokenListEnd()) && (distance > 0)) {
          list_->advance(iterator_);
          --distance;
        }
      }
      return *this;
    }

    LazyIterator& operator-=(index_type distance) noexcept
    {
      if (distance < 0)
        return *this += (distance * static_cast<index_type>(-1));

      if (list_) {
        while ((iterator_ != list_->tokenListBegin()) && (distance > 0)) {
          --iterator_;
          --distance;
        }
      }
      return *this;
    }

    [[nodiscard]] TokenPtr operator*() noexcept {
      assert(list_);
      if (iterator_ == list_->tokenListEnd())
        return TokenPtr{};
      return *iterator_;
    }

    [[nodiscard]] TokenConstPtr operator*() const noexcept {
      assert(list_);
      if (iterator_ == list_->tokenListEnd())
        return TokenPtr{};
      return *iterator_;
    }

    [[nodiscard]] decltype(auto) operator->() noexcept {
      assert(list_);
      assert(list_->size() > 0);
      if (iterator_ == list_->tokenListEnd()) {
        auto last{ list_->end() };
        --last;
        return &(*last);
      }
      return &(*iterator_);
    }

    [[nodiscard]] decltype(auto) operator->() const noexcept {
      assert(list_);
      assert(list_->size() > 0);
      if (iterator_ == list_->tokenListEnd()) {
        auto last{ list_->cend() };
        --last;
        return &(*last);
      }
      return &(*iterator_);
    }

    [[nodiscard]] decltype(auto) operator[](index_type distance) noexcept {
      auto temp{ *this };
      temp += distance;
      return *temp;
    }

    [[nodiscard]] decltype(auto) operator[](index_type distance) const noexcept {
      auto temp{ *this };
      temp += distance;
      return *temp;
    }

    auto operator++(int) noexcept { auto temp{ *this }; ++(*this); return temp; }
    auto operator--(int) noexcept { auto temp{ *this }; --(*this); return temp; }

    [[nodiscard]] constexpr decltype(auto) operator+(index_type distance) const noexcept { auto temp{ *this }; temp += distance; return temp; }
    [[nodiscard]] constexpr decltype(auto) operator-(index_type distance) const noexcept { auto temp{ *this }; temp -= distance; return temp; }

    [[nodiscard]] friend constexpr decltype(auto) operator+(index_type distance, const LazyIterator& rhs) noexcept { auto temp{ rhs }; return temp + distance; }
    [[nodiscard]] friend constexpr decltype(auto) operator-(index_type distance, const LazyIterator& rhs) noexcept { auto temp{ rhs }; return temp + (static_cast<index_type>(-1) * distance); }

    [[nodiscard]] constexpr auto operator==(const lazy_iterator<TList>& rhs) const noexcept { return (list_ == rhs.list_) && (iterator_ == rhs.iterator_); }
    [[nodiscard]] constexpr auto operator!=(const lazy_iterator<TList>& rhs) const noexcept { return !((*this) == rhs); }

    [[nodiscard]] constexpr auto operator==(const const_lazy_iterator<TList>& rhs) const noexcept { return (list_ == rhs.list_) && (iterator_ == rhs.iterator_); }
    [[nodiscard]] constexpr auto operator!=(const const_lazy_iterator<TList>& rhs) const noexcept { return !((*this) == rhs); }

  protected:
    UseList* list_{};
    use_list_iterator iterator_;
  };

  [[nodiscard]] TokenList extractFromStartToPos(index_type count) noexcept;
  [[nodiscard]] TokenList extractFromPosToEnd(index_type count) noexcept;

  void erase(index_type pos) noexcept;

  void pushFront(TokenPtr& token) noexcept;
  void pushBack(TokenPtr& token) noexcept;

  void pushFront(TokenPtr&& token) noexcept;
  void pushBack(TokenPtr&& token) noexcept;

  TokenPtr popFront() noexcept;
  TokenPtr popBack() noexcept;

  TokenPtr front() noexcept;
  TokenPtr back() noexcept;

  TokenConstPtr front() const noexcept;
  TokenConstPtr back() const noexcept;

  void extractThenPushFront(TokenList& rhs) noexcept;
  void extractThenPushBack(TokenList& rhs) noexcept;

  void extractThenPushFront(TokenList&& rhs) noexcept;
  void extractThenPushBack(TokenList&& rhs) noexcept;

  void copyPushFront(const TokenList& rhs) noexcept;
  void copyPushBack(const TokenList& rhs) noexcept;

  [[nodiscard]] iterator begin() noexcept { return iterator{ *this, tokenListBegin() }; }
  [[nodiscard]] const_iterator begin() const noexcept { return const_iterator{ *this, tokenListBegin() }; }
  [[nodiscard]] const_iterator cbegin() const noexcept { return const_iterator{ *this, tokenListCBegin() }; }

  // "end" is only the end of the currently parsed lazy tokens and not
  // the end of the entire fully parsed file
  [[nodiscard]] iterator end() noexcept { return iterator{ *this, tokenListEnd() }; }
  [[nodiscard]] const_iterator end() const noexcept { return const_iterator{ *this, tokenListEnd() }; }
  [[nodiscard]] const_iterator cend() const noexcept { return const_iterator{ *this, tokenListCEnd() }; }

  [[nodiscard]] TokenPtr operator[](index_type pos) noexcept;
  [[nodiscard]] const TokenConstPtr operator[](index_type pos) const noexcept;

  [[nodiscard]] iterator at(index_type pos) noexcept;
  [[nodiscard]] const_iterator at(index_type pos) const noexcept;

  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] size_type size() const noexcept;  // size is a lazy projection and does not represent "true" size

  void clear() noexcept;

  [[nodiscard]] bool hasAhead(iterator pos, index_type count) noexcept { assert(&(pos.list()) == this); return pos.hasAhead(count); }
  [[nodiscard]] bool hasBehind(iterator pos, index_type count) noexcept { assert(&(pos.list()) == this); return pos.hasBehind(count); }

  [[nodiscard]] bool hasAhead(const_iterator pos, index_type count) const noexcept { assert(&(pos.list()) == this); return pos.hasAhead(count); }
  [[nodiscard]] bool hasBehind(const_iterator pos, index_type count) const noexcept { assert(&(pos.list()) == this); return pos.hasBehind(count); }

  [[nodiscard]] TokenList extract(iterator first, iterator last) noexcept;
  [[nodiscard]] TokenList extract(iterator first, index_type count) noexcept;
  [[nodiscard]] TokenList extract(index_type first, index_type count) noexcept;

  [[nodiscard]] TokenList extractFromStartToPos(iterator pos) noexcept;
  [[nodiscard]] TokenList extractFromPosToEnd(iterator pos) noexcept;

  void erase(iterator pos) noexcept;
  void erase(iterator first, iterator last) noexcept;
  void erase(iterator first, index_type count) noexcept;

  void insertBefore(iterator pos, TokenPtr& token) noexcept;
  void insertAfter(iterator pos, TokenPtr& token) noexcept;
  void insert(iterator pos, TokenPtr& token) noexcept;

  void insertCopyBefore(iterator pos, const TokenList& rhs) noexcept;
  void insertCopyAfter(iterator pos, const TokenList& rhs) noexcept;

private:
  void ensurePosExists(index_type pos) noexcept;
  void ensurePosExists(index_type pos) const noexcept;

  void out(ErrorTypes::Error error, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(WarningTypes::Warning warning, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
};


inline bool hasAhead(Tokenizer::iterator pos, index_type count) noexcept { return pos.hasAhead(count); }
inline bool hasBehind(Tokenizer::iterator pos, index_type count) noexcept { return pos.hasBehind(count); }

inline bool hasAhead(Tokenizer::const_iterator pos, index_type count) noexcept { return pos.hasAhead(count); }
inline bool hasBehind(Tokenizer::const_iterator pos, index_type count) noexcept { return pos.hasBehind(count); }

TokenList extract(Tokenizer::iterator first, Tokenizer::iterator last) noexcept;
TokenList extract(Tokenizer::iterator first, index_type count) noexcept;

TokenList extractFromStartToPos(Tokenizer::iterator pos) noexcept;
TokenList extractFromPosToEnd(Tokenizer::iterator pos) noexcept;

void erase(Tokenizer::iterator pos) noexcept;
void erase(Tokenizer::iterator first, Tokenizer::iterator last) noexcept;
void erase(Tokenizer::iterator first, index_type count) noexcept;

void insertBefore(Tokenizer::iterator pos, TokenPtr& token) noexcept;
void insertAfter(Tokenizer::iterator pos, TokenPtr& token) noexcept;
void insert(Tokenizer::iterator pos, TokenPtr& token) noexcept;

void insertCopyBefore(Tokenizer::iterator pos, const TokenList& rhs) noexcept;
void insertCopyAfter(Tokenizer::iterator pos, const TokenList& rhs) noexcept;

} // namespace zax

namespace std
{

// "end" is only the end of the currently parsed lazy tokens and not
// the end of the entire fully parsed file

[[nodiscard]] inline decltype(auto) begin(zax::Tokenizer& value) { return value.begin(); }
[[nodiscard]] inline decltype(auto) end(zax::Tokenizer& value) { return value.end(); }

[[nodiscard]] inline decltype(auto) begin(const zax::Tokenizer& value) { return value.begin(); }
[[nodiscard]] inline decltype(auto) end(const zax::Tokenizer& value) { return value.end(); }

[[nodiscard]] inline decltype(auto) cbegin(const zax::Tokenizer& value) { return value.cbegin(); }
[[nodiscard]] inline decltype(auto) cend(const zax::Tokenizer& value) { return value.cend(); }

} // namespace std
