
#pragma once

#include "types.h"
#include "helpers.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct TemplatesArgumentTypes
{
  struct Argument
  {
    TokenConstPtr location_;

    String name_;

    struct Default {
      AliasPtr typeAlias_;
      VariablePtr variable_;
    };
    std::optional<Default> default_{};
  };

  ZAX_DECLARE_STRUCT_PTR(Argument);
  using ArgumentVector = std::vector<ArgumentPtr>;
};

//-----------------------------------------------------------------------------
struct TemplateArguments : public TemplatesArgumentTypes
{
  Puid id_{ puid() };

  TokenConstPtr location_;

  ArgumentVector arguments_;
};

} // namespace zax
