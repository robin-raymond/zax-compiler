
#pragma once

#include "types.h"
#include "Faults.h"

namespace zax
{

struct InformationalTypes
{
  enum class Informational
  {
    ActualOrigin,
    ToDo
  };

  struct InformationalDeclare final : public zs::EnumDeclare<Informational, 2>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Informational::ActualOrigin, "origin"},
        {Informational::ToDo, "to-do"}
      } };
    }
  };

  struct InformationalHumanReadableDeclare final : public zs::EnumDeclare<Informational, 2>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Informational::ActualOrigin, "origin"},
        {Informational::ToDo, "to-do"}
      } };
    }
  };

  using InformationalTraits = zs::EnumTraits<Informational, InformationalDeclare>;
  using InformationalHumanReadableTraits = zs::EnumTraits<Informational, InformationalHumanReadableDeclare>;
};

using InformationalFaults = Faults<InformationalTypes::Informational, InformationalTypes::InformationalTraits>;
using Informational = InformationalFaults;

} // namespace zax
