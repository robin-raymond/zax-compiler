
#include "pch.h"
#include "Type.h"
#include "Context.h"

using namespace zax;


//-----------------------------------------------------------------------------
Context* Type::context() noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_;
}

//-----------------------------------------------------------------------------
const Context* Type::context() const noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_;
}

//-----------------------------------------------------------------------------
ContextPtr Type::contextPtr() noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_->thisWeak_.lock();
}

//-----------------------------------------------------------------------------
const ContextConstPtr Type::contextPtr() const noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_->thisWeak_.lock();
}
