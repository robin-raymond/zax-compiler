
#pragma once

#include "types.h"
#include "helpers.h"
#include "Source.h"

namespace zax
{

struct TokenTypes
{
  enum class Type
  {
    Separator,
    Literal,
    Operator,
    Number,
    Quote,
    Comment
  };

  struct TypeDeclare final : public zs::EnumDeclare<Type, 7>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Type::Separator, "separator"},
        {Type::Literal, "literal"},
        {Type::Operator, "operator"},
        {Type::Number, "number"},
        {Type::Quote, "quote"},
        {Type::Comment, "comment"}
      } };
    }
  };

  using TypeTraits = zs::EnumTraits<Type, TypeDeclare>;

  enum class Operator
  {
    PlusPreUnary,
    MinusPreUnary,
    PlusBinary,
    MinusBinary,
    PlusPlusPreUnary,
    MinusMinusPreUnary,
    PlusPlusPostUnary,
    MinusMinusPostUnary,
    Multiply,
    Divide,
    Modulus,
    Assign,
    XorBitwise,
    AndBitwise,
    OrBitwise,
    LeftShift,
    RightShift,
    LeftRotate,
    RightRotate,
    OnesCompliment,
    ParityBitwise,
    ClearBitwise,
    Not,
    And,
    Or,
    Xor,
    PlusAssign,
    MinusAssign,
    MultiplyAssign,
    DivideAssign,
    ModulusAssign,
    Equals,
    NotEquals,
    ThreeWayCompare,
    Swap,
    LessThan,
    GreaterThan,
    LessThanEquals,
    GreaterThanEquals,
    OnesComplimentBitwiseAssign,
    XorBitwiseAssign,
    OrBitwiseAssign,
    ParityBitwiseAssign,
    ClearBitwiseAssign,
    LeftShiftAssign,
    RightShiftAssign,
    LeftRotateAssign,
    RightRotateAssign,
    Dereference,
    As,
    OpenParenthisis,
    CloseParenthisis,
    OpenSquare,
    CloseSquare,
    CountOf,
    Overhead,
    OverheadOf,
    AllocatorOf,

    PointerType,
    ReferenceCapture,
    ReferenceDeclare,
    Allocate,
    ParallelAllocate,
    SequentialAllocate,
    NameResolution,
    Comma,
    StatementSeparator,
    SubStatementSeparator,
    TypeDeclare,
    MetaDeclare,
    MetaDereference,
    Optional,
    Ternary,
    UninitializedData,
    FunctionComposition,
    FunctionInvocationChaining,
    Combine,
    Split,
    Continuation,
    Cast,
    OuterCast,
    CopyCast,
    LifetimeCast,
    OuterOf,
    LifetimeOf,
    SizeOf,
    AlignOf,
    OffsetOf,

    Templated,
    VariadicValues,
    VariadicTypes,
    ScopeOpen,
    ScopeClose,
    InitializeOpen,
    InitializeClose,
    ArrayValueInitializeOpen,
    ArrayValueInitializeClose,
    DirectiveOpen,
    DirectiveClose,
    Discard,
    Self,
    Context,
    Constructor,
    Destructor
  };

  
  struct OperatorDeclare final : public zs::EnumDeclare<Operator, 104>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        { Operator::PlusPreUnary, "+" },
        { Operator::MinusPreUnary, "-" },
        { Operator::PlusBinary, "+"} ,
        { Operator::MinusBinary, "-" },
        { Operator::PlusPlusPreUnary, "++" },
        { Operator::MinusMinusPreUnary, "--" },
        { Operator::PlusPlusPostUnary, "++" },
        { Operator::MinusMinusPostUnary, "--" },
        { Operator::Multiply, "*" },
        { Operator::Divide, "/" },
        { Operator::Modulus, "%" },
        { Operator::Assign, "=" },
        { Operator::XorBitwise, "^" },
        { Operator::AndBitwise, "&" },
        { Operator::OrBitwise, "|" },
        { Operator::LeftShift, "<<" },
        { Operator::RightShift, ">>" },
        { Operator::LeftRotate, "<<<" },
        { Operator::RightRotate, ">>>" },
        { Operator::OnesCompliment, "~" },
        { Operator::ParityBitwise, "~|" },
        { Operator::ClearBitwise, "~&" },
        { Operator::Not, "!" },
        { Operator::And, "&&" },
        { Operator::Or, "||" },
        { Operator::Xor, "^^" },
        { Operator::PlusAssign, "+=" },
        { Operator::MinusAssign, "-=" },
        { Operator::MultiplyAssign, "*=" },
        { Operator::DivideAssign, "/=" },
        { Operator::ModulusAssign, "%=" },
        { Operator::Equals, "==" },
        { Operator::NotEquals, "!=" },
        { Operator::ThreeWayCompare, "<=>" },
        { Operator::Swap, "<<>>" },
        { Operator::LessThan, "<" },
        { Operator::GreaterThan, ">" },
        { Operator::LessThanEquals, "<=" },
        { Operator::GreaterThanEquals, ">=" },
        { Operator::OnesComplimentBitwiseAssign, "~=" },
        { Operator::XorBitwiseAssign, "^=" },
        { Operator::OrBitwiseAssign, "|=" },
        { Operator::ParityBitwiseAssign, "~|=" },
        { Operator::ClearBitwiseAssign, "~&=" },
        { Operator::LeftShiftAssign, "<<=" },
        { Operator::RightShiftAssign, ">>=" },
        { Operator::LeftRotateAssign, "<<<=" },
        { Operator::RightRotateAssign, ">>>=" },
        { Operator::Dereference, "." },
        { Operator::As, "as" },
        { Operator::OpenParenthisis, "(" },
        { Operator::CloseParenthisis, ")" },
        { Operator::OpenSquare, "[" },
        { Operator::CloseSquare, "]" },
        { Operator::CountOf, "countof" },
        { Operator::Overhead, "overhead" },
        { Operator::OverheadOf, "overheadof" },
        { Operator::AllocatorOf, "allocatorof" },
        { Operator::PointerType, "*" },
        { Operator::ReferenceCapture, "&" },
        { Operator::ReferenceDeclare, "&" },
        { Operator::Allocate, "@" },
        { Operator::ParallelAllocate, "@@" },
        { Operator::SequentialAllocate, "@!" },
        { Operator::NameResolution, "." },
        { Operator::Comma, "," },
        { Operator::StatementSeparator, ";" },
        { Operator::SubStatementSeparator, ";;" },
        { Operator::TypeDeclare, ":" },
        { Operator::MetaDeclare, "::" },
        { Operator::MetaDereference, "::." },
        { Operator::Optional, "?" },
        { Operator::Ternary, "??" },
        { Operator::UninitializedData, "???" },
        { Operator::FunctionComposition, ">>" },
        { Operator::FunctionInvocationChaining, "|>" },
        { Operator::Combine, "->" },
        { Operator::Split, "<-" },
        { Operator::Continuation, "\\" },
        { Operator::Cast, "cast" },
        { Operator::OuterCast, "outercast" },
        { Operator::CopyCast, "copycast" },
        { Operator::LifetimeCast, "lifetimecast" },
        { Operator::OuterOf, "outerof" },
        { Operator::LifetimeOf, "lifetimeof" },
        { Operator::SizeOf, "sizeof" },
        { Operator::AlignOf, "aligneof" },
        { Operator::OffsetOf, "offsetof" },
        { Operator::Templated, "$" },
        { Operator::VariadicValues, "..." },
        { Operator::VariadicTypes, "$..." },
        { Operator::ScopeOpen, "{" },
        { Operator::ScopeClose, "}" },
        { Operator::InitializeOpen, "{" },
        { Operator::InitializeClose, "}" },
        { Operator::ArrayValueInitializeOpen, "[{" },
        { Operator::ArrayValueInitializeClose, "}]" },
        { Operator::DirectiveOpen, "[[" },
        { Operator::DirectiveClose, "]]" },
        { Operator::Discard, "#" },
        { Operator::Self, "_" },
        { Operator::Context, "___" },
        { Operator::Constructor, "+++" },
        { Operator::Destructor, "---" }
      } };
    }
  };

  using OperatorTraits = zs::EnumTraits<Operator, OperatorDeclare>;


  enum class Keyword
  {
    Aos,
    Alias,
    Atomic,
    Await,
    Break,
    Case,
    Channel,
    Continue,
    Collect,
    Constant,
    Copy,
    Deep,
    Default,
    Defer,
    Discard,
    Each,
    Else,
    Extension,
    Except,
    False,
    For,
    Forever,
    Handle,
    Hidden,
    Hint,
    If,
    In,
    Is,
    Immutable,
    Import,
    Inconstant,
    Keyword,
    Lazy,
    Lease,
    Managed,
    Mutable,
    Mutator,
    Once,
    Operator,
    Override,
    Own,
    Private,
    Promise,
    Redo,
    Return,
    Requires,
    Scope,
    Shallow,
    Soa,
    Suspend,
    Switch,
    Task,
    True,
    Type,
    Union,
    Unique,
    Until,
    Using,
    Varies,
    Void,
    Weak,
    While,
    Yield
  };

  
  struct KeywordDeclare final : public zs::EnumDeclare<Keyword, 65>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Keyword::Aos, "aos"},
        {Keyword::Alias, "alias"},
        {Keyword::Atomic, "atomic"},
        {Keyword::Await, "await"},
        {Keyword::Break, "break"},
        {Keyword::Case, "case"},
        {Keyword::Channel, "channel"},
        {Keyword::Continue, "continue"},
        {Keyword::Collect, "collect"},
        {Keyword::Constant, "constant"},
        {Keyword::Copy, "copy"},
        {Keyword::Deep, "deep"},
        {Keyword::Default, "default"},
        {Keyword::Defer, "defer"},
        {Keyword::Discard, "discard"},
        {Keyword::Each, "each"},
        {Keyword::Else, "else"},
        {Keyword::Extension, "extension"},
        {Keyword::Except, "except"},
        {Keyword::False, "false"},
        {Keyword::For, "for"},
        {Keyword::Forever, "forever"},
        {Keyword::Handle, "handle"},
        {Keyword::Hidden, "hidden"},
        {Keyword::Hint, "hint"},
        {Keyword::If, "if"},
        {Keyword::In, "in"},
        {Keyword::Is, "is"},
        {Keyword::Immutable, "immutable"},
        {Keyword::Import, "import"},
        {Keyword::Inconstant, "inconstant"},
        {Keyword::Keyword, "keyword"},
        {Keyword::Lazy, "lazy"},
        {Keyword::Lease, "lease"},
        {Keyword::Managed, "managed"},
        {Keyword::Mutable, "mutable"},
        {Keyword::Mutator, "mutator"},
        {Keyword::Once, "once"},
        {Keyword::Operator, "operator"},
        {Keyword::Override, "override"},
        {Keyword::Own, "own"},
        {Keyword::Private, "private"},
        {Keyword::Promise, "promise"},
        {Keyword::Redo, "redo"},
        {Keyword::Return, "return"},
        {Keyword::Requires, "requires"},
        {Keyword::Scope, "scope"},
        {Keyword::Shallow, "shallow"},
        {Keyword::Soa, "soa"},
        {Keyword::Suspend, "suspend"},
        {Keyword::Switch, "switch"},
        {Keyword::Task, "task"},
        {Keyword::True, "true"},
        {Keyword::Type, "type"},
        {Keyword::Union, "union"},
        {Keyword::Unique, "unique"},
        {Keyword::Until, "until"},
        {Keyword::Using, "using"},
        {Keyword::Varies, "varies"},
        {Keyword::Void, "void"},
        {Keyword::Weak, "weak"},
        {Keyword::While, "while"},
        {Keyword::Yield, "yield"}
      } };
    }
  };

  using KeywordTraits = zs::EnumTraits<Keyword, KeywordDeclare>;
};

struct Token : public TokenTypes
{
  const Puid id_{ puid() };
  Type type_ { Type::Separator };
  bool forcedSeparator_{};

  Operator operator_{};
  std::optional<Keyword> keyword_{};

  StringView originalToken_;
  StringView token_;

  SourceTypes::Origin origin_;
  SourceTypes::Origin actualOrigin_;

  CompileStateConstPtr compileState_;
  TokenPtr comment_;

  mutable bool aliasSearched_{};
  mutable TokenConstPtr alias_;

  std::optional<Operator> lookupOperator() const noexcept;
  static std::optional<Operator> lookupOperator(const TokenConstPtr& token) noexcept { if (!token) return {}; return token->lookupOperator(); }

  std::optional<Keyword> lookupKeyword() const noexcept;
  static std::optional<Keyword> lookupKeyword(const TokenConstPtr& token) noexcept { if (!token) return {}; return token->lookupKeyword(); }
};

} // namespace zax
