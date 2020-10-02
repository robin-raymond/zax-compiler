
#pragma once

#include "types.h"

#include "TypeCommon.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct UnionTypes
{
};

//-----------------------------------------------------------------------------
struct Union : public TypeCommon,
               public UnionTypes
{
};

} // namespace zax
