#pragma once

#include "types.h"

#include <zs/RandomAccessListIterator.h>

namespace zax
{

struct TokenListTypes
{
  using List = std::list<TokenPtr>;
  using list_iterator = List::iterator;
  using const_list_iterator = List::const_iterator;

  using iterator = zs::random_access_list_iterator<List>;
  using const_iterator = zs::const_random_access_list_iterator<List>;

  using index_type = zs::index_type;
};

inline bool hasAhead(TokenListTypes::iterator pos, index_type count) noexcept { return pos.hasAhead(count); }
inline bool hasBehind(TokenListTypes::iterator pos, index_type count) noexcept { return pos.hasBehind(count); }

TokenList extract(TokenListTypes::iterator first, TokenListTypes::iterator last) noexcept;
TokenList extract(TokenListTypes::iterator first, index_type count) noexcept;

TokenList extractFromStartToPos(TokenListTypes::iterator pos) noexcept;
TokenList extractFromPosToEnd(TokenListTypes::iterator pos) noexcept;

void erase(TokenListTypes::iterator pos) noexcept;
void erase(TokenListTypes::iterator first, TokenListTypes::iterator last) noexcept;
void erase(TokenListTypes::iterator first, index_type count) noexcept;

void insertBefore(TokenListTypes::iterator pos, TokenPtr& token) noexcept;
void insertAfter(TokenListTypes::iterator pos, TokenPtr& token) noexcept;
void insert(TokenListTypes::iterator pos, TokenPtr& token) noexcept;

void insertCopyBefore(TokenListTypes::iterator pos, const TokenList& rhs) noexcept;
void insertCopyAfter(TokenListTypes::iterator pos, const TokenList& rhs) noexcept;


struct TokenList final : public TokenListTypes
{
  List tokens_;

  TokenList extractFromStartToPos(index_type count) noexcept;
  TokenList extractFromPosToEnd(index_type count) noexcept;

  void erase(index_type pos) noexcept;

  void pushFront(TokenPtr& token) noexcept;
  void pushBack(TokenPtr& token) noexcept;

  void pushFront(TokenPtr&& token) noexcept;
  void pushBack(TokenPtr&& token) noexcept;

  TokenPtr popFront() noexcept;
  TokenPtr popBack() noexcept;

  void extractThenPushFront(TokenList& rhs) noexcept;
  void extractThenPushBack(TokenList& rhs) noexcept;

  void extractThenPushFront(TokenList&& rhs) noexcept;
  void extractThenPushBack(TokenList&& rhs) noexcept;

  void copyPushFront(const TokenList& rhs) noexcept;
  void copyPushBack(const TokenList& rhs) noexcept;

  iterator begin() noexcept;
  iterator end() noexcept;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  TokenPtr operator[](index_type pos) noexcept;
  const TokenPtr operator[](index_type pos) const noexcept;

  iterator at(index_type pos) noexcept;
  const_iterator at(index_type pos) const noexcept;

  bool empty() const noexcept { return tokens_.empty(); };
  auto size() const noexcept { return tokens_.size(); }

  bool hasAhead(TokenListTypes::iterator pos, index_type count) noexcept { assert(&(pos.list()) == &tokens_); return pos.hasAhead(count); }
  bool hasBehind(TokenListTypes::iterator pos, index_type count) noexcept { assert(&(pos.list()) == &tokens_); return pos.hasBehind(count); }

  TokenList extract(TokenListTypes::iterator first, TokenListTypes::iterator last) noexcept { assert(&(first.list()) == &tokens_); return zax::extract(first, last); }
  TokenList extract(TokenListTypes::iterator first, index_type count) noexcept { assert(&(first.list()) == &tokens_); return zax::extract(first, first + count); }

  TokenList extractFromStartToPos(TokenListTypes::iterator pos) noexcept { assert(&(pos.list()) == &tokens_); return zax::extractFromStartToPos(pos); }
  TokenList extractFromPosToEnd(TokenListTypes::iterator pos) noexcept { assert(&(pos.list()) == &tokens_); return zax::extractFromPosToEnd(pos); }

  void erase(TokenListTypes::iterator pos) noexcept { assert(&(pos.list()) == &tokens_);  zax::erase(pos); }
  void erase(TokenListTypes::iterator first, TokenListTypes::iterator last) noexcept { assert(&(first.list()) == &tokens_); zax::erase(first, last); }
  void erase(TokenListTypes::iterator first, index_type count) noexcept { assert(&(first.list()) == &tokens_); zax::erase(first, first + count); }

  void insertBefore(TokenListTypes::iterator pos, TokenPtr& token) noexcept { assert(&(pos.list()) == &tokens_); zax::insertBefore(pos, token); }
  void insertAfter(TokenListTypes::iterator pos, TokenPtr& token) noexcept { assert(&(pos.list()) == &tokens_); zax::insertAfter(pos, token); }
  void insert(TokenListTypes::iterator pos, TokenPtr& token) noexcept { assert(&(pos.list()) == &tokens_); zax::insertBefore(pos, token); }

  void insertCopyBefore(TokenListTypes::iterator pos, const TokenList& rhs) noexcept { assert(&(pos.list()) == &tokens_); zax::insertCopyBefore(pos, rhs); }
  void insertCopyAfter(TokenListTypes::iterator pos, const TokenList& rhs) noexcept { assert(&(pos.list()) == &tokens_); zax::insertCopyAfter(pos, rhs); }
};

} // namespace zax

namespace std
{

[[nodiscard]] inline decltype(auto) begin(zax::TokenList& e) { return e.begin(); }
[[nodiscard]] inline decltype(auto) end(zax::TokenList& e) { return e.end(); }

[[nodiscard]] inline decltype(auto) begin(const zax::TokenList& e) { return e.begin(); }
[[nodiscard]] inline decltype(auto) end(const zax::TokenList& e) { return e.end(); }

[[nodiscard]] inline decltype(auto) cbegin(const zax::TokenList& e) { return e.cbegin(); }
[[nodiscard]] inline decltype(auto) cend(const zax::TokenList& e) { return e.cend(); }

} // namespace std
