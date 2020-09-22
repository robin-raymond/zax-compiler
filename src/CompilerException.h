#pragma once

#include "types.h"
#include "Informationals.h"
#include "Warnings.h"
#include "Errors.h"

namespace zax
{

struct CompilerException final : public std::exception
{
  enum class ErrorType
  {
    Informational,
    Warning,
    Error,
    Fatal
  };

  struct ErrorTypeDeclare final : public zs::EnumDeclare<ErrorType, 7>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {ErrorType::Informational, "info"},
        {ErrorType::Warning, "warning"},
        {ErrorType::Error, "error"},
        {ErrorType::Fatal, "fatal"}
      } };
    }
  };

  using ErrorTypeTraits = zs::EnumTraits<ErrorType, ErrorTypeDeclare>;

  ErrorType type_{};
  String fileName_;
  int line_{};
  int column_{};
  String iana_;
  String message_;

  String what_;

  CompilerException(
    ErrorType type,
    const std::string& fileName,
    int line,
    int column,
    const String& iana,
    const String& message
  ) noexcept :
    fileName_{ fileName },
    line_{ line },
    column_{ column },
    iana_{ iana },
    message_{ message }
  {
    StringStream ss;

    bool added{};
    bool addedBracket{};

    ss << fileName;
    if (fileName.size() > 0) {
      added = true;
    }
 
   if (0 != line_) {
      ss << "(";
      ss << line_;
      addedBracket = true;
    }
    if (0 != column_) {
      if (!addedBracket)
        ss << "(";
      else
        ss << ",";
      ss << column_;
      addedBracket = true;
    }

    if (addedBracket)
      ss << ")";

    if (added || addedBracket)
      ss << ": ";

    ss << ErrorTypeTraits::toString(type_) << " " << iana_ << ": " << message_;

    what_ = ss.str();
  }

  const char* what() const noexcept final
  {
    return what_.c_str();
  }
};

int totalErrors() noexcept;
int totalWarnings() noexcept;
bool shouldAbort() noexcept;

TokenPtr makeInternalToken(CompileStatePtr state) noexcept;

void output(const CompilerException& exception) noexcept;
inline void throwException(const CompilerException& exception) noexcept(false) { throw exception; }

void output(
  InformationalTypes::Informational informational,
  const TokenConstPtr &token,
  const StringMap& params = {}) noexcept;
void output(
  WarningTypes::Warning warning,
  const TokenConstPtr& token,
  const StringMap& params = {}) noexcept;
void output(ErrorTypes::Error error,
  const TokenConstPtr& token,
  const StringMap& params = {}) noexcept;
void fatal(ErrorTypes::Error error,
  const TokenConstPtr& token,
  const StringMap& params = {}) noexcept;

void throwException(
  ErrorTypes::Error error,
  const TokenConstPtr& token,
  const StringMap& params = {}) noexcept(false);

} // namespace zax
