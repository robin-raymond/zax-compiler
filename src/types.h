
#pragma once

#include "pch.h"

#define ZAX_DECLARE_STRUCT_PTR(xStruct)                 \
  struct xStruct;                                       \
  using xStruct##UniPtr = std::unique_ptr<xStruct>;     \
  using xStruct##Ptr = std::shared_ptr<xStruct>;        \
  using xStruct##WeakPtr = std::weak_ptr<xStruct>;      \
  using xStruct##Optional = std::optional<xStruct>;

namespace zax {

using String = std::string;
using StringView = std::string_view;

using Puid = unsigned int;

template <typename T>
using optional = std::optional<T>;

ZAX_DECLARE_STRUCT_PTR(CodeBlock);
ZAX_DECLARE_STRUCT_PTR(Compiler);
ZAX_DECLARE_STRUCT_PTR(CompileState);
ZAX_DECLARE_STRUCT_PTR(Config);
ZAX_DECLARE_STRUCT_PTR(Context);
ZAX_DECLARE_STRUCT_PTR(Module);
ZAX_DECLARE_STRUCT_PTR(OperatorLutTypes);
ZAX_DECLARE_STRUCT_PTR(OperatorLut);
ZAX_DECLARE_STRUCT_PTR(Source);
ZAX_DECLARE_STRUCT_PTR(SourceFilePath);
ZAX_DECLARE_STRUCT_PTR(SourceLocation);
ZAX_DECLARE_STRUCT_PTR(TokenTypes);
ZAX_DECLARE_STRUCT_PTR(Token);
ZAX_DECLARE_STRUCT_PTR(TypeAlias);
ZAX_DECLARE_STRUCT_PTR(TypeDefine);
ZAX_DECLARE_STRUCT_PTR(TokenizerTypes);
ZAX_DECLARE_STRUCT_PTR(Tokenizer);

} // namespace zax
