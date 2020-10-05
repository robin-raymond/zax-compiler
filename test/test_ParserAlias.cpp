
#include <pch.h>

#include <filesystem>
#include <variant>
#include <iterator>

#include "common.h"

#include "../src/Parser.h"
#include "../src/CompileState.h"
#include "../src/Context.h"

using Error = zax::ErrorTypes::Error;
using Warning = zax::WarningTypes::Warning;
using Panic = zax::PanicTypes::Panic;
using Informational = zax::InformationalTypes::Informational;
using StringMap = zax::StringMap;
using String = zax::String;
using StringView = zax::StringView;
using Callbacks = zax::ParserTypes::Callbacks;
using Token = zax::Token;
using TokenPtr = zax::TokenPtr;
using TokenConstPtr = zax::TokenConstPtr;
using Path = zax::Path;
using CompileState = zax::CompileState;
using CompileStatePtr = zax::CompileStatePtr;
using CompileStateConstPtr = zax::CompileStateConstPtr;
using Parser = zax::Parser;
using ParserPtr = zax::ParserPtr;
using ParserConstPtr = zax::ParserConstPtr;
using Config = zax::Config;
#if 0
using StringList = zax::StringList;
using ErrorTypes = zax::ErrorTypes;
using WarningTypes = zax::WarningTypes;
using PanicTypes = zax::PanicTypes;
using Context = zax::Context;
#endif //0

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace zaxTest
{

struct ParserAliasCommon
{
  struct ExpectedFailures
  {
    std::variant<Error, Warning, Informational> type_;
    bool isFatal_{};
    String fileName_;
    int line_{};
    int column_{};
    StringMap mapping_;
    bool forcedError_{};

    ExpectedFailures(bool fatal, Error error, StringView fileName, int line, int column, const StringMap& mapping) noexcept(false) :
      type_{ error },
      isFatal_{ fatal },
      fileName_{ fileName },
      line_{ line },
      column_{ column },
      mapping_{ mapping }
    {}
    ExpectedFailures(Warning warning, StringView fileName, int line, int column, const StringMap& mapping, bool forcedError = false) noexcept(false) :
      type_{ warning },
      fileName_{ fileName },
      line_{ line },
      column_{ column },
      mapping_{ mapping },
      forcedError_(forcedError)
    {}

  };

  std::list<ExpectedFailures> failures_;
  std::optional<Callbacks> callbacks_;

  std::list<TokenConstPtr> faultTokens_;

  //-------------------------------------------------------------------------
  void reset() noexcept
  {
    TEST(failures_.empty());
    failures_.clear();
    callbacks_.reset();
    faultTokens_.clear();
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
    output.fatal_ = [&](Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Error>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == error);
      TEST(!!token);
      TEST(Path{ token->origin_.filePath_->filePath_ } == Path{ front.fileName_ });
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);
      TEST(mapping.size() == front.mapping_.size());
      TEST(mapping == front.mapping_);

      faultTokens_.push_back(token);
      failures_.pop_front();
    };
    output.error_ = [&](Error error, const TokenConstPtr& token, const StringMap& mapping) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Error>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == error);
      TEST(!!token);
      TEST(Path{ token->origin_.filePath_->filePath_ } == Path{ front.fileName_ });
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);
      TEST(mapping.size() == front.mapping_.size());
      TEST(mapping == front.mapping_);

      faultTokens_.push_back(token);
      failures_.pop_front();
    };
    output.warning_ = [&](Warning warning, const TokenConstPtr& token, const StringMap& mapping) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Warning>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == warning);
      TEST(!!token);
      TEST(Path{ token->origin_.filePath_->filePath_ } == Path{ front.fileName_ });
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);
      TEST(mapping.size() == front.mapping_.size());
      TEST(mapping == front.mapping_);
      TEST(token->compileState_->isWarningAnError(warning) == front.forcedError_);

      faultTokens_.push_back(token);
      failures_.pop_front();
    };
    output.info_ = [&](Informational info, const TokenConstPtr& token, const StringMap& mapping) noexcept(false) {
      TEST(failures_.size() > 0);
      auto& front{ failures_.front() };
      auto ptr{ std::get_if<Informational>(&(front.type_)) };
      TEST(ptr);
      TEST(*ptr == info);
      TEST(!!token);
      TEST(Path{ token->origin_.filePath_->filePath_ } == Path{ front.fileName_ });
      TEST(token->origin_.location_.line_ == front.line_);
      TEST(token->origin_.location_.column_ == front.column_);
      TEST(!front.isFatal_);
      TEST(mapping.size() == front.mapping_.size());
      TEST(mapping == front.mapping_);

      faultTokens_.push_back(token);
      failures_.pop_front();
    };
  }

  //-------------------------------------------------------------------------
  void fatal(Error error, StringView fileName, int line, int column, const StringMap& mapping = {}) noexcept(false)
  {
    failures_.emplace_back(true, error, fileName, line, column, mapping);
  }

  //-------------------------------------------------------------------------
  void expect(Error error, StringView fileName, int line, int column, const StringMap& mapping = {}) noexcept(false)
  {
    failures_.emplace_back(false, error, fileName, line, column, mapping);
  }

  //-------------------------------------------------------------------------
  void expect(Warning warning, StringView fileName, int line, int column, const StringMap& mapping = {}) noexcept(false)
  {
    failures_.emplace_back(warning, fileName, line, column, mapping);
  }

  //-------------------------------------------------------------------------
  void error(Warning warning, StringView fileName, int line, int column, const StringMap& mapping = {}) noexcept(false)
  {
    failures_.emplace_back(warning, fileName, line, column, mapping, true);
  }

  //-------------------------------------------------------------------------
  void expect(const CompileState& state, const Panic which) noexcept(false)
  {
    TEST(state.panics_.at(which).enabled_);
  }

  //-------------------------------------------------------------------------
  void disabled(const CompileState& state, const Panic which) noexcept(false)
  {
    TEST(!state.panics_.at(which).enabled_);
  }

  //-------------------------------------------------------------------------
  void expect(const TokenConstPtr& token, const Panic which) noexcept(false)
  {
    TEST(static_cast<bool>(token));
    TEST(static_cast<bool>(token->compileState_));
    expect(*(token->compileState_), which);
  }

  //-------------------------------------------------------------------------
  void disabled(const TokenConstPtr& token, const Panic which) noexcept(false)
  {
    TEST(static_cast<bool>(token));
    TEST(static_cast<bool>(token->compileState_));
    disabled(*(token->compileState_), which);
  }

  //-------------------------------------------------------------------------
  void expect(size_t index, const Panic which) noexcept(false)
  {
    TEST(index < faultTokens_.size());
    auto iter{ faultTokens_.begin() };
    std::advance(iter, index);
    expect(*iter, which);
  }

  //-------------------------------------------------------------------------
  void disabled(size_t index, const Panic which) noexcept(false)
  {
    TEST(index < faultTokens_.size());
    auto iter{ faultTokens_.begin() };
    std::advance(iter, index);
    disabled(*iter, which);
  }

  //-------------------------------------------------------------------------
  TokenConstPtr faultToken(size_t index) noexcept(false)
  {
    TEST(index < faultTokens_.size());
    auto iter{ faultTokens_.begin() };
    std::advance(iter, index);
    return *iter;
  }

  //-------------------------------------------------------------------------
  CompileStateConstPtr faultTokenState(size_t index) noexcept(false)
  {
    auto token{ faultToken(index) };
    TEST(static_cast<bool>(token));
    TEST(static_cast<bool>(token->compileState_));
    return token->compileState_;
  }

  //-------------------------------------------------------------------------
  ParserPtr testCommon(
    const std::string_view filePath,
    const std::string_view content) noexcept(false)
  {
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ filePath }.parent_path(), ec);

    TEST(zax::writeBinaryFile(filePath, content));

    Config config;
    config.inputFilePaths_.emplace_back(filePath);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
    return parser;
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserAlias : public ParserAliasCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/alias/keyword/1.zax" };
    expect(Warning::NewlineAfterContinuation, example, 2, 2);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 2);

    testCommon(example,
      "\n"
      "\\;\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/alias/keyword/2.zax" };
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 39 -8 + 1);

    testCommon(example,
      "\n"
      "const :: alias keyword constant;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });

    reset();
  }
};

//---------------------------------------------------------------------------
void testParserAlias() noexcept(false)
{
  ParserAlias{}.runAll();
}

} // namespace zaxTest
