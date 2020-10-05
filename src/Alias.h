
#pragma once

#include "types.h"
#include "helpers.h"

#include "EntryCommon.h"

#include "Type.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct AliasTypes
{
  enum class PointerType
  {
    Reference,
    Raw,
    Unique,
    Own,
    Strong,
    Handle,
    Discard,
    Collect
  };

  struct PointerTypeDeclare final : public zs::EnumDeclare<PointerType, 8>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {PointerType::Reference, "reference"},
        {PointerType::Raw, "raw"},
        {PointerType::Unique, "unique"},
        {PointerType::Own, "own"},
        {PointerType::Strong, "string"},
        {PointerType::Handle, "handle"},
        {PointerType::Discard, "discard"},
        {PointerType::Collect, "collect"}
      } };
    }
  };

  using PointerTypeTraits = zs::EnumTraits<PointerType, PointerTypeDeclare>;

};

//-----------------------------------------------------------------------------
struct Alias : public EntryCommon,
               public AliasTypes
{

  std::optional<bool> optional_{};
  std::optional<bool> mutable_{};
  std::optional<bool> constant_{};

  std::optional<bool> lease_{};   // least or last
  std::optional<bool> shallow_{}; // shallow or deep
  std::optional<bool> copy_{};    // copy or move

  struct Pointer {
    PointerType type_{};
    bool weak_{};
  };

  std::vector<Pointer> pointers_;

  struct Array {
    size_t size_{};
  };

  std::vector<Array> arrays_;
  bool aos_{ true };

  TypePtr type_;
  FunctionTypePtr functionType_;
  UnionPtr union_;

  TemplateArgumentsPtr templateArguments_;
};

} // namespace zax
