
#include "pch.h"
#include "TypeCommon.h"
#include "Context.h"

using namespace zax;


//-----------------------------------------------------------------------------
Context* TypeCommon::context() noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_;
}

//-----------------------------------------------------------------------------
const Context* TypeCommon::context() const noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_;
}

//-----------------------------------------------------------------------------
ContextPtr TypeCommon::contextPtr() noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_->thisWeak_.lock();
}

//-----------------------------------------------------------------------------
const ContextConstPtr TypeCommon::contextPtr() const noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_->thisWeak_.lock();
}
