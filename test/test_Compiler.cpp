
#include <pch.h>

#include <filesystem>
#include <variant>

#include "common.h"

#include "../src/Compiler.h"

using Compiler = zax::Compiler;
using Config = zax::Config;
using Callbacks = zax::CompilerTypes::Callbacks;
using Error = zax::ErrorTypes::Error;
using Warning = zax::WarningTypes::Warning;
using Informational = zax::InformationalTypes::Informational;
using TokenConstPtr = zax::TokenConstPtr;
using StringMap = zax::StringMap;
using StringList = zax::StringList;
using String = zax::String;

namespace zaxTest
{

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct CompilerBasics
{
  struct ExpectedFailures
  {
    std::variant<Error, Warning, Informational> type_;
    int line_{};
    int column_{};
    bool isFatal_{};

    ExpectedFailures(bool fatal, zax::ErrorTypes::Error error, int line, int column) noexcept(false) :
      type_{ error },
      line_{ line },
      column_{ column },
      isFatal_{ fatal }
    {}
    ExpectedFailures(zax::WarningTypes::Warning warning, int line, int column) noexcept(false) :
      type_{ warning },
      line_{ line },
      column_{ column }
    {}

  };

  std::list<ExpectedFailures> failures_;
  std::optional<Callbacks> callbacks_;

  //-------------------------------------------------------------------------
  void reset() noexcept
  {
    TEST(failures_.empty());
    failures_.clear();
    callbacks_.reset();
  }

  //-------------------------------------------------------------------------
  Callbacks* callbacks() noexcept(false)
  {
    callbacks_.emplace();
    callbacks(*callbacks_);
    return &(*callbacks_);
  }

  //-------------------------------------------------------------------------
  void callbacks(Callbacks& output) noexcept(false)
  {
    output.shouldAbort_ = []() noexcept -> bool {
      return false;
    };
    output.fatal_ = [&](Error error, const TokenConstPtr& token, const StringMap&) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Error>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == error);
      TEST(!!token);
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);

      failures_.pop_front();
    };
    output.error_ = [&](Error error, const TokenConstPtr& token, const StringMap&) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Error>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == error);
      TEST(!!token);
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);

      failures_.pop_front();
    };
    output.warning_ = [&](Warning warning, const TokenConstPtr& token, const StringMap&) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Warning>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == warning);
      TEST(!!token);
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);

      failures_.pop_front();
    };
    output.info_ = [&](Informational info, const TokenConstPtr& token, const StringMap&) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Informational>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == info);
      TEST(!!token);
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);

      failures_.pop_front();
    };
  }

  //-------------------------------------------------------------------------
  void fatal(zax::ErrorTypes::Error error, int line, int column) noexcept(false)
  {
    failures_.emplace_back(true, error, line, column);
  }

  //-------------------------------------------------------------------------
  void expect(zax::ErrorTypes::Error error, int line, int column) noexcept(false)
  {
    failures_.emplace_back(false, error, line, column);
  }

  //-------------------------------------------------------------------------
  void expect(zax::WarningTypes::Warning warning, int line, int column) noexcept(false)
  {
    failures_.emplace_back(warning, line, column);
  }

  //-------------------------------------------------------------------------
  void test() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/compiler/a.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "\\;\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto compiler{ std::make_shared<Compiler>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::NewlineAfterContinuation, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, 2, 2);

    compiler->compile();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test(); });

    reset();
  }
};

//---------------------------------------------------------------------------
void testCompiler() noexcept(false)
{
  CompilerBasics{}.runAll();
}

} // namespace zaxTest
