
#pragma once

#include "types.h"

#include "EntryCommon.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct UnionTypes
{
};

//-----------------------------------------------------------------------------
struct Union : public EntryCommon,
               public UnionTypes
{
};

} // namespace zax
