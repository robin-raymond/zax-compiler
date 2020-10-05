
#include "pch.h"
#include "EntryCommon.h"
#include "Context.h"

using namespace zax;


//-----------------------------------------------------------------------------
Context* EntryCommon::context() noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_;
}

//-----------------------------------------------------------------------------
const Context* EntryCommon::context() const noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_;
}

//-----------------------------------------------------------------------------
ContextPtr EntryCommon::contextPtr() noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_->thisWeak_.lock();
}

//-----------------------------------------------------------------------------
const ContextConstPtr EntryCommon::contextPtr() const noexcept
{
  if (!context_)
    return {};
  context_->alive_();
  return context_->thisWeak_.lock();
}
