
#pragma once

#include "pch.h"

#define ZAX_DECLARE_STRUCT_PTR(xStruct)                       \
  struct xStruct;                                             \
  using xStruct##UniPtr = std::unique_ptr<xStruct>;           \
  using xStruct##Ptr = std::shared_ptr<xStruct>;              \
  using xStruct##ConstPtr = std::shared_ptr<const xStruct>;   \
  using xStruct##WeakPtr = std::weak_ptr<xStruct>;            \
  using xStruct##Optional = std::optional<xStruct>;

namespace zax {

using StringStream = std::stringstream;
using String = std::string;
using StringView = std::string_view;
using StringList = std::list<String>;
using StringViewList = std::list<StringView>;
using StringMap = std::map<String, String>;
using StringViewMap = std::map<StringView, StringView>;

using Puid = unsigned int;

template <typename T>
using optional = std::optional<T>;

using index_type = zs::index_type;

ZAX_DECLARE_STRUCT_PTR(CodeBlock);
ZAX_DECLARE_STRUCT_PTR(Compiler);
ZAX_DECLARE_STRUCT_PTR(CompilerTypes);
ZAX_DECLARE_STRUCT_PTR(CompileState);
ZAX_DECLARE_STRUCT_PTR(CompilerException);
ZAX_DECLARE_STRUCT_PTR(Config);
ZAX_DECLARE_STRUCT_PTR(Context);
ZAX_DECLARE_STRUCT_PTR(ErrorTypes);
ZAX_DECLARE_STRUCT_PTR(Module);
ZAX_DECLARE_STRUCT_PTR(OperatorLutTypes);
ZAX_DECLARE_STRUCT_PTR(OperatorLut);
ZAX_DECLARE_STRUCT_PTR(SourceTypes);
ZAX_DECLARE_STRUCT_PTR(Source);
ZAX_DECLARE_STRUCT_PTR(TokenTypes);
ZAX_DECLARE_STRUCT_PTR(Token);
ZAX_DECLARE_STRUCT_PTR(TokenizerTypes);
ZAX_DECLARE_STRUCT_PTR(Tokenizer);
ZAX_DECLARE_STRUCT_PTR(TokenListTypes);
ZAX_DECLARE_STRUCT_PTR(TokenList);
ZAX_DECLARE_STRUCT_PTR(TypeAlias);
ZAX_DECLARE_STRUCT_PTR(TypeDefine);
ZAX_DECLARE_STRUCT_PTR(TokenizerTypes);
ZAX_DECLARE_STRUCT_PTR(Tokenizer);
ZAX_DECLARE_STRUCT_PTR(WarningTypes);

} // namespace zax
