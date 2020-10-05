
#pragma once

#include "types.h"

#include "EntryCommon.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct TypeTypes
{
};

//-----------------------------------------------------------------------------
struct Type : public EntryCommon,
              public TypeTypes
{
  struct Partial
  {
    String name_;
    TypePtr type_;
  };
  std::optional<Partial> partial_;

  struct Flag
  {
    bool enabled_{};
    bool defaulted_{};
  };
  Flag mutable_{ .enabled_ = true };
  Flag immutable_{ .enabled_ = true };
  bool managed_{};

  TemplateArgumentsPtr templateArguments_;
};

} // namespace zax
