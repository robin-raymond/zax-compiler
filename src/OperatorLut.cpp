
#include "pch.h"
#include "OperatorLut.h"

using namespace zax;

//-----------------------------------------------------------------------------
OperatorLut::OperatorLut() noexcept
{
  const TokenTypes::OperatorTraits operators;

  auto prepareLut{ [&]() noexcept {
    const TokenTypes::OperatorTraits traits;
    for (auto& [oper, name] : traits) {
      auto lutIndex{ TokenTypes::OperatorTraits::toUnderlying(oper) };
      operatorsLut_[lutIndex] = name;
    }
  } };

  auto prepareSymbolsLut{ [&]() noexcept {

    // create a fast LUT for the operators
    for (auto& [value, name] : operators) {
      auto length = name.length();
      auto firstChar = static_cast<unsigned char>(name[0]);
      auto& entryMap{ operatorsSymbolLut_[SafeInt<size_t>(firstChar)] };
      auto inserted{ entryMap.emplace(length, SymbolVector{}) };
      auto& existingVector{ inserted.first->second };
      if (existingVector.size() < 1) {
        existingVector.reserve(5);
        existingVector.push_back(NameSymbolPair{ name, value });
      }
      else {
        auto existingFoundName{ std::find_if(begin(existingVector), end(existingVector), [&name](const NameSymbolPair& nameSymbolPair) noexcept -> bool {
          return name == nameSymbolPair.first;
        }) };
        if (existingFoundName == end(existingVector)) {
          existingVector.push_back(NameSymbolPair{ name, value });
        }
      }
    }
  } };

  auto prepareConflicts{ [&]() noexcept {
    auto prepareConflictSet{ [&](TokenTypes::Operator value) noexcept -> OperatorEnumSet& {
      return operatorConflictsWith_.emplace(value, OperatorEnumSet{}).first->second;
    } };
    auto mergeConflictSet{ [&](OperatorEnumSet& destSet, OperatorEnumSet& sourceSet) noexcept {
      destSet.insert(sourceSet.begin(), sourceSet.end());
    } };

    for (auto& [value, name] : operators) {
      if (auto found = operatorsNameMap_.find(name); found != end(operatorsNameMap_)) {
        operatorsConflicting_.insert(found->second);
        operatorsConflicting_.insert(value);
        auto& setSource{ prepareConflictSet(found->second) };

        setSource.insert(found->second);
        setSource.insert(value);
        for (auto symbol : setSource) {
          // do not merge with self
          if (symbol == found->second)
            continue;

          auto& setDest{ prepareConflictSet(symbol) };
          mergeConflictSet(setDest, setSource);
        }
      }
      else {
        operatorsNameMap_[name] = value;
      }
    }

  } };

  prepareLut();
  prepareSymbolsLut();
  prepareConflicts();
}

//-----------------------------------------------------------------------------
OperatorLut::~OperatorLut() noexcept
{
}

//-----------------------------------------------------------------------------
bool OperatorLut::hasConflicts(TokenTypes::Operator value) const noexcept
{
  return operatorsConflicting_.find(value) != end(operatorsConflicting_);
}

//-----------------------------------------------------------------------------
const OperatorLutTypes:: OperatorEnumSet& OperatorLut::lookupConflicts(TokenTypes::Operator value) const noexcept
{
  static const OperatorEnumSet nothing;
  if (auto found = operatorConflictsWith_.find(value); found != end(operatorConflictsWith_))
    return found->second;
  return nothing;
}

//-----------------------------------------------------------------------------
optional<TokenTypes::Operator> OperatorLut::lookup(const StringView contents) const noexcept
{
  if (contents.length() < 1)
    return {};

  auto pos{ contents.data() };

  auto& entryMap{ operatorsSymbolLut_[SafeInt<size_t>(static_cast<unsigned char>(*pos))] };
  if (entryMap.size() < 1)
    return {};

  auto longestSymbol{ [&]() noexcept -> size_t {
    auto lastVectorFound{ entryMap.rbegin() };
    return lastVectorFound->first;
  } };

  auto longest{ std::min(longestSymbol(), contents.length()) };
  
  for (; longest > 0; --longest) {
    StringView str{ pos, longest };

    if (auto foundLength{ entryMap.find(longest) }; foundLength != end(entryMap)) {
      auto& symbolVector{ foundLength->second };
      auto found = std::find_if(begin(symbolVector), end(symbolVector), [&](const NameSymbolPair& nameSymbolPair) noexcept -> bool {
        return nameSymbolPair.first == str;
        });
      if (found == end(symbolVector))
        continue;
      return found->second;
    }
  }

  return {};
}

//-----------------------------------------------------------------------------
StringView OperatorLut::lookup(TokenTypes::Operator value) const noexcept
{
  return operatorsLut_[TokenTypes::OperatorTraits::toUnderlying(value)];
}
