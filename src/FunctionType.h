
#pragma once

#include "types.h"
#include "helpers.h"

#include "ParserDirectiveTypes.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct FunctionTypeTypes
{
  enum class FunctionType
  {
    Normal,
    Lazy,
    Promise,
    Task,
    Channel
  };
  using FunctionTypeEnum  = FunctionType;

  struct FunctionTypeDeclare final : public zs::EnumDeclare<FunctionType, 5>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {FunctionType::Normal, "normal"},
        {FunctionType::Lazy, "lazy"},
        {FunctionType::Promise, "promise"},
        {FunctionType::Promise, "task"},
        {FunctionType::Channel, "channel"}
      } };
    }
  };

  using FunctionTypeTraits = zs::EnumTraits<FunctionType, FunctionTypeDeclare>;

  using Inline = ParserDirectiveTypes::Inline;
  using Return = ParserDirectiveTypes::Return;
};

//-----------------------------------------------------------------------------
struct FunctionType : public FunctionTypeTypes
{
  const Puid id_{ puid() };

  TokenConstPtr location_;

  FunctionTypeEnum type_{};
  std::optional<bool> concept_{};
  std::optional<bool> mutable_{};
  std::optional<bool> shallow_{};
  std::optional<bool> selectable_{};  // via [[requires]] clause
  bool synchronous_{};

  Inline inline_{};
  Return return_{};
};

} // namespace zax
