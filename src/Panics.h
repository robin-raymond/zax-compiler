
#pragma once

#include "types.h"
#include "Faults.h"

namespace zax
{

struct PanicsTypes
{
  enum class Panic
  {
    OutOfMemory,
    IntrinsicTypeCastOverflow,
    StringConversionContainsIllegalSequence,
    ReferenceFromPointerToNothing,
    PointerToNothingAccessed,
    NotAllPointersDeallocatedDuringAllocatorCleanup,
    ImpossibleSwitchValue,
    ImpossibleIfValue,
    ImpossibleCodeFlow,
    LazyAlreadyComplete,
    ValuePolymorphicFunctionNotFound
  };

  struct PanicDeclare final : public zs::EnumDeclare<Panic, 11>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Panic::OutOfMemory, "out-of-memory"},
        {Panic::IntrinsicTypeCastOverflow, "intrinsic-type-cast-overflow"},
        {Panic::StringConversionContainsIllegalSequence, "string-conversion-contains-illegal-sequence"},
        {Panic::ReferenceFromPointerToNothing, "reference-from-pointer-to-nothing"},
        {Panic::PointerToNothingAccessed, "pointer-to-nothing-accessed"},
        {Panic::NotAllPointersDeallocatedDuringAllocatorCleanup, "not-all-pointers-deallocated-during-allocator-cleanup"},
        {Panic::ImpossibleSwitchValue, "impossible-switch-value"},
        {Panic::ImpossibleIfValue, "impossible-if-value"},
        {Panic::ImpossibleCodeFlow, "impossible-code-flow"},
        {Panic::LazyAlreadyComplete, "lazy-already-complete"},
        {Panic::ValuePolymorphicFunctionNotFound, "value-polymorphic-function-not-found"}

      } };
    }
  };

  using PanicTraits = zs::EnumTraits<Panic, PanicDeclare>;
};

using PanicFaults = Faults<PanicsTypes::Panic, PanicsTypes::PanicTraits>;
using Panics = PanicFaults;

} // namespace zax
