
#pragma once

#include "types.h"
#include "helpers.h"

namespace zax {

struct TokenizerTypes
{
};

struct Tokenizer : public TokenizerTypes
{
  Puid id_{ puid() };

  Tokenizer() noexcept;
  ~Tokenizer() noexcept;

private:
};

} // namespace zax
