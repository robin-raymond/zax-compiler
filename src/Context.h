
#pragma once

#include "types.h"
#include "helpers.h"

namespace zax
{

struct Context
{
  Puid id_{ puid() };
  Puid owner_{};
  ModuleWeakPtr module_;
  ContextWeakPtr parent_;

  struct Aliasing
  {
    std::map<String, String> keywords_;
    std::map<String, String> operators_;
  } aliasing_;

  struct Types {
    std::map<String, TypeDefinePtr> types_;
  } types_;
};

} // namespace zax
