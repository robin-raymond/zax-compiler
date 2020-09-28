#include "pch.h"
#include "Context.h"

using namespace zax;

//-----------------------------------------------------------------------------
ContextPtr Context::forkChild(const Type type) noexcept
{
  alive_();
  auto result{ std::make_shared<Context>() };
  result->thisWeak_ = result;
  result->type_ = type;
  result->parent_ = this;
  result->parser_ = parser_;
  result->module_ = module_;
  result->tokenizer_ = tokenizer_;
  result->singleLineState_ = singleLineState_;
  return result;
}

//-----------------------------------------------------------------------------
ContextPtr Context::forkChild(const Type type) const noexcept
{
  return const_cast<Context*>(this)->forkChild(type);
}

//-----------------------------------------------------------------------------
Context* Context::findParent(Type type) noexcept
{
  if (type_ == type)
    return this;

  for (auto parent{ parent_ }; parent; parent = parent->parent()) {
    if (parent->type_ == type)
      return parent;
  }
  return {};
}

//-----------------------------------------------------------------------------
const Context* Context::findParent(Type type) const noexcept
{
  return const_cast<Context*>(this)->findParent(type);
}

//-----------------------------------------------------------------------------
CompileStatePtr Context::state() noexcept
{
  if (singleLineState_)
    return singleLineState_;
  if (state_)
    return state_;

  for (auto parent{ parent_ }; parent; parent = parent->parent()) {
    if (parent->singleLineState_)
      return parent->singleLineState_;
    if (parent->state_)
      return parent->state_;
  }
  return {};
}

//-----------------------------------------------------------------------------
CompileStateConstPtr Context::state() const noexcept
{
  return const_cast<Context*>(this)->state();
}
