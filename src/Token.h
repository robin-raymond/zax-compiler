
#pragma once

#include "types.h"
#include "helpers.h"
#include "SourceLocation.h"

namespace zax
{

struct TokenTypes
{
  enum class Type
  {
    Separator,
    Keyword,
    Symbolic,
    Punctuation,
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
        {Type::Keyword, "keyword"},
        {Type::Symbolic, "symbolic"},
        {Type::Punctuation, "punctuation"},
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
    AddAssign,
    MinusAssign,
    MultiplyAssign,
    DivideAssign,
    ModulusAssign,
    Equals,
    NotEquals,
    ThreeWayCompare,
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
    ValueInitializeOpen,
    ValueInitializeClose,
    DirectiveOpen,
    DirectiveClose,
    Self,
    Context,
    Constructor,
    Destructor
  };

  
  struct OperatorDeclare final : public zs::EnumDeclare<Operator, 99>
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
        { Operator::AddAssign, "+=" },
        { Operator::MinusAssign, "-=" },
        { Operator::MultiplyAssign, "*=" },
        { Operator::DivideAssign, "/=" },
        { Operator::ModulusAssign, "%=" },
        { Operator::Equals, "==" },
        { Operator::NotEquals, "!=" },
        { Operator::ThreeWayCompare, "<=>" },
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
        { Operator::ValueInitializeOpen, "{{" },
        { Operator::ValueInitializeClose, "}}" },
        { Operator::DirectiveOpen, "[[" },
        { Operator::DirectiveClose, "]]" },
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
    Build,
    Case,
    Channel,
    Continue,
    Collect,
    Constant,
    Deep,
    Default,
    Defer,
    Discard,
    Each,
    Else,
    Extension,
    Except,
    Export,
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
    Soa,
    Suspend,
    Switch,
    Task,
    True,
    Type,
    Union,
    Until,
    Using,
    Varies,
    Void,
    Weak,
    While,
    Yield
  };

  
  struct KeywordDeclare final : public zs::EnumDeclare<Keyword, 61>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Keyword::Aos, "aos"},
        {Keyword::Alias, "alias"},
        {Keyword::Atomic, "atomic"},
        {Keyword::Await, "await"},
        {Keyword::Break, "break"},
        {Keyword::Build, "build"},
        {Keyword::Case, "case"},
        {Keyword::Channel, "channel"},
        {Keyword::Continue, "continue"},
        {Keyword::Collect, "collect"},
        {Keyword::Constant, "constant"},
        {Keyword::Deep, "deep"},
        {Keyword::Default, "default"},
        {Keyword::Defer, "defer"},
        {Keyword::Discard, "discard"},
        {Keyword::Each, "each"},
        {Keyword::Else, "else"},
        {Keyword::Extension, "extension"},
        {Keyword::Except, "except"},
        {Keyword::Export, "export"},
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
        {Keyword::Soa, "soa"},
        {Keyword::Suspend, "suspend"},
        {Keyword::Switch, "switch"},
        {Keyword::Task, "task"},
        {Keyword::True, "true"},
        {Keyword::Type, "type"},
        {Keyword::Union, "union"},
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
  Puid id_{ puid() };
  Type type_ { Type::Separator };
  bool forcedSeparator_{};

  Operator operator_{};
  Keyword keyword_{};

  SourceLocation location_;
  CompileStatePtr compileState_;
};

} // namespace zax
