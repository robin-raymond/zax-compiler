
#pragma once

#include "types.h"
#include "helpers.h"

namespace zax
{

struct ContextTypes
{
  enum class Type {
    Parser,
    Source,
    Expression
  };

  struct TypeDeclare final : public zs::EnumDeclare<Type, 3>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Type::Parser, "parser"},
        {Type::Source, "source"},
        {Type::Expression, "expression"}
      } };
    }
  };

  using TypeTraits = zs::EnumTraits<Type, TypeDeclare>;
};

struct Context : public ContextTypes
{
  const Puid id_{ puid() };
  Puid owner_{};
  Type type_{};

  Alive alive_;

  ContextWeakPtr thisWeak_;
  Context* parent_{};

  Parser* parser_{};
  Module* module_{};

  TokenizerPtr tokenizer_{};
  CompileStatePtr singleLineState_;
  CompileStatePtr state_;

  Context() = default;
  Context(const Context&) noexcept = delete;
  Context(Context&&) noexcept = delete;

  Context& operator=(const Context&) noexcept = delete;
  Context& operator=(Context&&) noexcept = delete;

  ContextPtr forkChild(const Type type) noexcept;
  ContextPtr forkChild(const Type type) const noexcept;
  ContextPtr forkChild(Puid owner, const Type type) const noexcept {
    auto result{ forkChild(type) };
    result->owner_ = owner;
    return result;
  }

  CompileStatePtr state() noexcept;
  CompileStateConstPtr state() const noexcept;

  Tokenizer& operator*() noexcept { return *tokenizer_; }
  const Tokenizer& operator*() const noexcept { return *tokenizer_; }

  Tokenizer* operator->() noexcept { return tokenizer_.get(); }
  const Tokenizer* operator->() const noexcept { return tokenizer_.get(); }

  Context* findParent(Type type) noexcept;
  const Context* findParent(Type type) const noexcept;

  Context* parent() noexcept { alive_();  return parent_; }
  const Context* parent() const noexcept { alive_();  return parent_; }

  Parser& parser() noexcept { alive_(); assert(parser_); return *parser_; }
  Module& module() noexcept { alive_(); assert(module_); return *module_; }

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
