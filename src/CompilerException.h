#pragma once

#include "types.h"

namespace zax
{

void output(const CompilerException& exception) noexcept;

struct CompilerException final : public std::exception
{
  enum class ErrorType
  {
    Info,
    Warning,
    Error,
    Fatal
  };

  struct ErrorTypeDeclare final : public zs::EnumDeclare<ErrorType, 7>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {ErrorType::Info, "info"},
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
    String iana,
    const std::string& message
  ) noexcept :
    fileName_{ fileName },
    line_{ line },
    column_{ column },
    iana_{ iana },
    message_{ message }
  {
    std::stringstream ss;

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

  static void throwError(
    ErrorType type,
    String fileName,
    int line,
    int column,
    String iana,
    String message
  ) noexcept(false)
  {
    throw CompilerException{ type, fileName, line, column, iana, message };
  }

  static void showError(
    ErrorType type,
    String fileName,
    int line,
    int column,
    String iana,
    String message
  ) noexcept(false)
  {
    output(CompilerException{ type, fileName, line, column, iana, message });
  }

  const char* what() const noexcept final
  {
    return what_.c_str();
  }
};

} // namespace zax
