
#pragma once

#include "types.h"
#include "helpers.h"
#include "Token.h"

namespace zax {

struct OperatorLutTypes
{
  using NameSymbolPair = std::pair<std::string_view, TokenTypes::Operator>;
  using SymbolVector = std::vector<NameSymbolPair>;
  using SymbolLengthMap = std::map<size_t, SymbolVector>;
  using SymbolLutArray = std::array<SymbolLengthMap, (sizeof(unsigned char) << 8)>;
  using OperatorEnumSet = std::set<TokenTypes::Operator>;
  using OperatorNameMap = std::map<std::string_view, TokenTypes::Operator>;
  using OperatorEnumConflictSetMap = std::map<TokenTypes::Operator, OperatorEnumSet>;
  inline static constexpr size_t TotalOperators{ TokenTypes::OperatorTraits::Total() };
  using OperatorLutArray = std::array<std::string_view, TotalOperators + 1>;
};

struct OperatorLut : public OperatorLutTypes
{
  Puid id_{ puid() };
  SymbolLutArray operatorsSymbolLut_;
  OperatorEnumSet operatorsConflicting_;
  OperatorEnumConflictSetMap operatorConflictsWith_;
  OperatorNameMap operatorsNameMap_;
  OperatorLutArray operatorsLut_;

  OperatorLut() noexcept;
  ~OperatorLut() noexcept;

  bool hasConflicts(TokenTypes::Operator value) const noexcept;
  const OperatorEnumSet& lookupConflicts(TokenTypes::Operator value) const noexcept;

  optional<TokenTypes::Operator> lookup(const std::string_view contents) noexcept;
  std::string_view lookup(TokenTypes::Operator value) noexcept;

private:
};

} // namespace zax
