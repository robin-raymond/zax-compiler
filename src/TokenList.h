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

inline bool hasAhead(TokenListTypes::const_iterator pos, index_type count) noexcept { return pos.hasAhead(count); }
inline bool hasBehind(TokenListTypes::const_iterator pos, index_type count) noexcept { return pos.hasBehind(count); }

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

  [[nodiscard]] TokenList extractFromStartToPos(index_type count) noexcept;
  [[nodiscard]] TokenList extractFromPosToEnd(index_type count) noexcept;

  void erase(index_type count) noexcept;

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

  [[nodiscard]] iterator begin() noexcept;
  [[nodiscard]] iterator end() noexcept;
  [[nodiscard]] const_iterator begin() const noexcept;
  [[nodiscard]] const_iterator end() const noexcept;
  [[nodiscard]] const_iterator cbegin() const noexcept;
  [[nodiscard]] const_iterator cend() const noexcept;

  [[nodiscard]] TokenPtr operator[](index_type count) noexcept;
  [[nodiscard]] const TokenConstPtr operator[](index_type count) const noexcept;

  [[nodiscard]] iterator at(index_type count) noexcept;
  [[nodiscard]] const_iterator at(index_type count) const noexcept;

  [[nodiscard]] bool empty() const noexcept { return tokens_.empty(); };
  [[nodiscard]] auto size() const noexcept { return tokens_.size(); }

  [[nodiscard]] void clear() noexcept { return tokens_.clear(); };

  [[nodiscard]] bool hasAhead(iterator pos, index_type count) const noexcept { assert(&(pos.list()) == &tokens_); return pos.hasAhead(count); }
  [[nodiscard]] bool hasBehind(iterator pos, index_type count) const noexcept { assert(&(pos.list()) == &tokens_); return pos.hasBehind(count); }

  [[nodiscard]] bool hasAhead(const_iterator pos, index_type count) const noexcept { assert(&(pos.list()) == &tokens_); return pos.hasAhead(count); }
  [[nodiscard]] bool hasBehind(const_iterator pos, index_type count) const noexcept { assert(&(pos.list()) == &tokens_); return pos.hasBehind(count); }

  [[nodiscard]] TokenList extract(iterator first, iterator last) noexcept { assert(&(first.list()) == &tokens_); return zax::extract(first, last); }
  [[nodiscard]] TokenList extract(iterator first, index_type count) noexcept { assert(&(first.list()) == &tokens_); return zax::extract(first, first + count); }
  [[nodiscard]] TokenList extract(index_type first, index_type count) noexcept { return extract(at(first), count); }

  [[nodiscard]] TokenList extractFromStartToPos(iterator pos) noexcept { assert(&(pos.list()) == &tokens_); return zax::extractFromStartToPos(pos); }
  [[nodiscard]] TokenList extractFromPosToEnd(iterator pos) noexcept { assert(&(pos.list()) == &tokens_); return zax::extractFromPosToEnd(pos); }

  void erase(iterator pos) noexcept { assert(&(pos.list()) == &tokens_); zax::erase(pos); }
  void erase(iterator first, iterator last) noexcept { assert(&(first.list()) == &tokens_); zax::erase(first, last); }
  void erase(iterator first, index_type count) noexcept { assert(&(first.list()) == &tokens_); zax::erase(first, first + count); }

  void insertBefore(iterator pos, TokenPtr& token) noexcept { assert(&(pos.list()) == &tokens_); zax::insertBefore(pos, token); }
  void insertAfter(iterator pos, TokenPtr& token) noexcept { assert(&(pos.list()) == &tokens_); zax::insertAfter(pos, token); }
  void insert(iterator pos, TokenPtr& token) noexcept { assert(&(pos.list()) == &tokens_); zax::insertBefore(pos, token); }

  void insertCopyBefore(iterator pos, const TokenList& rhs) noexcept { assert(&(pos.list()) == &tokens_); zax::insertCopyBefore(pos, rhs); }
  void insertCopyAfter(iterator pos, const TokenList& rhs) noexcept { assert(&(pos.list()) == &tokens_); zax::insertCopyAfter(pos, rhs); }
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
