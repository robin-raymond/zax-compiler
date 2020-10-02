
#pragma once

#include "types.h"

#include "TypeCommon.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct TypeTypes
{
};

//-----------------------------------------------------------------------------
struct Type : public TypeCommon,
              public TypeTypes
{
  String extensionOf_;
  TypePtr extention_;

  struct Flag
  {
    bool enabled_{};
    bool defaulted_{};
  };
  Flag mutable_{ .enabled_ = true };
  Flag immutable_{ .enabled_ = true };
  bool managed_{};
};

} // namespace zax
