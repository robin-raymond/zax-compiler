
#include "pch.h"
#include "TokenList.h"

using namespace zax;


//-----------------------------------------------------------------------------
TokenList zax::extract(TokenListTypes::iterator first, TokenListTypes::iterator last) noexcept
{
  assert(&first.list() == &last.list());
  TokenList result;
  result.tokens_.splice(result.tokens_.end(), first.list(), first.underlying(), last.underlying());
  return result;
}

//-----------------------------------------------------------------------------
TokenList zax::extract(TokenListTypes::iterator first, index_type count) noexcept
{
  return extract(first, first + count);
}

//-----------------------------------------------------------------------------
TokenList zax::extractFromStartToPos(TokenListTypes::iterator pos) noexcept
{
  TokenList result;
  result.tokens_.splice(result.tokens_.end(), pos.list(), pos.list().begin(), pos.underlying());
  return result;
}

//-----------------------------------------------------------------------------
TokenList zax::extractFromPosToEnd(TokenListTypes::iterator pos) noexcept
{
  TokenList result;
  result.tokens_.splice(result.tokens_.end(), pos.list(), pos.underlying(), pos.list().end());
  return result;
}

//-----------------------------------------------------------------------------
void zax::erase(TokenListTypes::iterator pos) noexcept
{
  if (pos.underlying() == pos.list().end())
    return;
  pos.list().erase(pos.underlying());
}

//-----------------------------------------------------------------------------
void zax::erase(TokenListTypes::iterator first, TokenListTypes::iterator last) noexcept
{
  assert(&first.list() == &last.list());
  first.list().erase(first.underlying(), last.underlying());
}

//-----------------------------------------------------------------------------
void zax::erase(TokenListTypes::iterator first, index_type count) noexcept
{
  erase(first, first + count);
}

//-----------------------------------------------------------------------------
void zax::insertBefore(TokenListTypes::iterator pos, TokenPtr& token) noexcept
{
  pos.list().insert(pos.underlying(), token);
}

//-----------------------------------------------------------------------------
void zax::insertAfter(TokenListTypes::iterator pos, TokenPtr& token) noexcept
{
  pos.list().insert((++pos).underlying(), token);
}

//-----------------------------------------------------------------------------
void zax::insert(TokenListTypes::iterator pos, TokenPtr& token) noexcept
{
  insertBefore(pos, token);
}

//-----------------------------------------------------------------------------
void zax::insertCopyBefore(TokenListTypes::iterator pos, const TokenList& rhs) noexcept
{
  TokenListTypes::List temp{ rhs.tokens_ };
  pos.list().splice(pos.underlying(), temp, temp.begin(), temp.end());
}

//-----------------------------------------------------------------------------
void zax::insertCopyAfter(TokenListTypes::iterator pos, const TokenList& rhs) noexcept
{
  TokenListTypes::List temp{ rhs.tokens_ };
  pos.list().splice((++pos).underlying(), temp, temp.begin(), temp.end());
}


//-----------------------------------------------------------------------------
TokenList TokenList::extractFromStartToPos(index_type count) noexcept
{
  return zax::extractFromStartToPos(zs::makeRandom(tokens_) + count);
}

//-----------------------------------------------------------------------------
TokenList TokenList::extractFromPosToEnd(index_type count) noexcept
{
  return zax::extractFromPosToEnd(zs::makeRandom(tokens_) + count);
}

//-----------------------------------------------------------------------------
void TokenList::erase(index_type count) noexcept
{
  zax::erase(zs::makeRandom(tokens_) + count);
}

//-----------------------------------------------------------------------------
void TokenList::pushFront(TokenPtr& token) noexcept
{
  tokens_.push_front(token);
}

//-----------------------------------------------------------------------------
void TokenList::pushFront(TokenPtr&& token) noexcept
{
  tokens_.push_front(std::move(token));
}

//-----------------------------------------------------------------------------
void TokenList::pushBack(TokenPtr& token) noexcept
{
  tokens_.push_back(token);
}

//-----------------------------------------------------------------------------
void TokenList::pushBack(TokenPtr&& token) noexcept
{
  tokens_.push_back(std::move(token));
}

//-----------------------------------------------------------------------------
TokenPtr TokenList::popFront() noexcept
{
  if (tokens_.empty())
    return {};
  auto result{ tokens_.front() };
  tokens_.pop_front();
  return result;
}

//-----------------------------------------------------------------------------
TokenPtr TokenList::popBack() noexcept
{
  if (tokens_.empty())
    return {};
  auto result{ tokens_.back() };
  tokens_.pop_back();
  return result;
}

//-----------------------------------------------------------------------------
TokenPtr TokenList::front() noexcept
{
  if (tokens_.empty())
    return {};
  return tokens_.front();
}

//-----------------------------------------------------------------------------
TokenPtr TokenList::back() noexcept
{
  if (tokens_.empty())
    return {};
  return tokens_.back();
}

//-----------------------------------------------------------------------------
TokenConstPtr TokenList::front() const noexcept
{
  if (tokens_.empty())
    return {};
  return tokens_.front();
}

//-----------------------------------------------------------------------------
TokenConstPtr TokenList::back() const noexcept
{
  if (tokens_.empty())
    return {};
  return tokens_.back();
}

//-----------------------------------------------------------------------------
void TokenList::extractThenPushFront(TokenList& rhs) noexcept
{
  tokens_.splice(tokens_.begin(), rhs.tokens_, rhs.tokens_.begin(), rhs.tokens_.end());
}

//-----------------------------------------------------------------------------
void TokenList::extractThenPushBack(TokenList& rhs) noexcept
{
  tokens_.splice(tokens_.end(), rhs.tokens_, rhs.tokens_.begin(), rhs.tokens_.end());
}

//-----------------------------------------------------------------------------
void TokenList::extractThenPushFront(TokenList&& rhs) noexcept
{
  tokens_.splice(tokens_.begin(), std::move(rhs.tokens_), rhs.tokens_.begin(), rhs.tokens_.end());
}

//-----------------------------------------------------------------------------
void TokenList::extractThenPushBack(TokenList&& rhs) noexcept
{
  tokens_.splice(tokens_.end(), std::move(rhs.tokens_), rhs.tokens_.begin(), rhs.tokens_.end());
}

//-----------------------------------------------------------------------------
void TokenList::copyPushFront(const TokenList& rhs) noexcept
{
  List temp{ rhs.tokens_ };
  tokens_.splice(tokens_.begin(), temp, temp.begin(), temp.end());
}

//-----------------------------------------------------------------------------
void TokenList::copyPushBack(const TokenList& rhs) noexcept
{
  List temp{ rhs.tokens_ };
  tokens_.splice(tokens_.end(), temp, temp.begin(), temp.end());
}

//-----------------------------------------------------------------------------
TokenListTypes::iterator TokenList::begin() noexcept
{
  return zs::makeRandom(tokens_);
}

//-----------------------------------------------------------------------------
TokenListTypes::iterator TokenList::end() noexcept
{
  return zs::makeRandom(tokens_, tokens_.end());
}

//-----------------------------------------------------------------------------
TokenListTypes::const_iterator TokenList::begin() const noexcept
{
  return zs::makeRandom(tokens_);
}

//-----------------------------------------------------------------------------
TokenListTypes::const_iterator TokenList::end() const noexcept
{
  return zs::makeRandom(tokens_, tokens_.end());
}

//-----------------------------------------------------------------------------
TokenListTypes::const_iterator TokenList::cbegin() const noexcept
{
  return zs::makeRandom(tokens_);
}

//-----------------------------------------------------------------------------
TokenListTypes::const_iterator TokenList::cend() const noexcept
{
  return zs::makeRandom(tokens_, tokens_.end());
}

//-----------------------------------------------------------------------------
TokenPtr TokenList::operator[](index_type count) noexcept
{
  if (tokens_.empty())
    return {};
  auto iter{ zs::makeRandom(tokens_) + count };
  if (iter.isEnd())
    return {};
  return *iter;
}

//-----------------------------------------------------------------------------
const TokenConstPtr TokenList::operator[](index_type count) const noexcept
{
  if (tokens_.empty())
    return {};

  auto iter{ zs::makeRandom(tokens_) + count };
  if (iter.isEnd())
    return {};
  return *iter;
}

//-----------------------------------------------------------------------------
TokenListTypes::iterator TokenList::at(index_type pos) noexcept
{
  return zs::makeRandom(tokens_) + pos;
}

//-----------------------------------------------------------------------------
TokenListTypes::const_iterator TokenList::at(index_type pos) const noexcept
{
  return zs::makeRandom(tokens_) + pos;
}


