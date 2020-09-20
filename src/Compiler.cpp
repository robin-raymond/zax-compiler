#include "pch.h"
#include "Compiler.h"

using namespace zax;

//-----------------------------------------------------------------------------
Compiler::Compiler(const Config& config) noexcept :
  config_(config)
{
}

//-----------------------------------------------------------------------------
void Compiler::compile() noexcept(false)
{
  prime();
}

//-----------------------------------------------------------------------------
void Compiler::prime() noexcept
{
  if (sources_.size() > 0)
    return;

  for (auto& pending : pendingSources_) {
    if (pending.generated_) {
      // TODO: execute pending compile time functions now
    }

  }
}

