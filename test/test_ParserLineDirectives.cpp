
#include <pch.h>

#include <filesystem>
#include <variant>
#include <iterator>

#include "common.h"

#include "../src/Parser.h"
#include "../src/CompileState.h"
#include "../src/Context.h"

using Parser = zax::Parser;
using ParserPtr = zax::ParserPtr;
using ParserConstPtr = zax::ParserConstPtr;
using Config = zax::Config;
using Callbacks = zax::ParserTypes::Callbacks;
using Error = zax::ErrorTypes::Error;
using Warning = zax::WarningTypes::Warning;
using Panic = zax::PanicTypes::Panic;
using Informational = zax::InformationalTypes::Informational;
using StringMap = zax::StringMap;
using StringList = zax::StringList;
using String = zax::String;
using StringView = zax::StringView;
using Path = zax::Path;
using ErrorTypes = zax::ErrorTypes;
using WarningTypes = zax::WarningTypes;
using PanicTypes = zax::PanicTypes;
using Context = zax::Context;
using CompileState = zax::CompileState;
using Token = zax::Token;
using TokenPtr = zax::TokenPtr;
using TokenConstPtr = zax::TokenConstPtr;
using CompileStatePtr = zax::CompileStatePtr;
using CompileStateConstPtr = zax::CompileStateConstPtr;


using namespace std::string_literals;
using namespace std::string_view_literals;

namespace zaxTest
{

struct ParserCommon
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
struct ParserSourceAssetDirectives : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/a.zax" };
    expect(Warning::NewlineAfterContinuation, example, 2, 2);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 2);

    testCommon(example,
      "\n"
      "\\;\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveInclude() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/b.zax" };
    const std::string_view example2{ "ignored/testing/parser/directive/source/c.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);

    const std::string_view content1{
      "\n"
      "[[\\\n"
      "source='c.zax']];\n"
    };

    const std::string_view content2{
      "\n"
      "\\;\n"
    };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(Warning::NewlineAfterContinuation, example2, 2, 2);
    expect(Warning::StatementSeparatorOperatorRedundant, example2, 2, 2);
    expect(Warning::StatementSeparatorOperatorRedundant, example1, 3, 24 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveInclude2() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/d.zax" };
    const std::string_view example2{ "ignored/testing/parser/directive/source/e.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);

    const std::string_view content1{
      "\n"
      "[[source='e.zax', generated = /* hello */ no , required = warn  ]] // ignore this\n"
    };

    const std::string_view content2{
      "\n"
      "\\;\n"
    };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(Warning::NewlineAfterContinuation, example2, 2, 2);
    expect(Warning::StatementSeparatorOperatorRedundant, example2, 2, 2);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveOdd() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/f.zax" };

    expect(Error::Syntax, example, 2, 4);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 22 - 8 + 1);

    testCommon(example,
      "\n"
      "[[ < foobar ]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceBadAfterComma() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/g.zax" };

    expect(Error::Syntax, example, 2, 45 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 49 - 8 + 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=warn, < ]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRepeated() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/h.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 56 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 23 - 8 + 1);

    testCommon(example,
      "\n"
      "[[source='bog''us.zax', required=warn, required=yes, generated=no, generated=yes, rename=\"foo.zax\"\\\n"
      ", unknown='' ]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetRepeated() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/i.zax" };
    const std::string_view example2{ "ignored/testing/parser/directive/source/asset.txt" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);

    const std::string_view content1{
      "\n"
      "[[asset='as''set.txt', required=warn, required=yes, generated=no, generated=yes, rename=\"ignored/output/asset.txt\"\\\n"
      ", unknown='' ]]; // ignore this\n"
    };
    const std::string_view content2{ "HELLO" };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    error(Warning::DirectiveNotUnderstood, example1, 2, 55 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example1, 3, 23 - 8 + 1);

    parser->parse();

    TEST(!std::filesystem::is_regular_file(Path{ "ignored/output/asset.txt" }, ec));

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetNotFound() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/j.zax" };
    const std::string_view example2{ "ignored/testing/parser/directive/source/asset.txt" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);

    const std::string_view content1{
      "\n"
      "[[asset='bogus_asset.txt', required=yes,rename=\"ignored/output/bogus_asset.txt\"\\   \n"
      "]]; // ignore this\n"
    };
    const std::string_view content2{ "HELLO" };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(Error::AssetNotFound, example1, 2, 3, StringMap{ { "$file$", "bogus_asset.txt" } });
    expect(Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetPattern() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/k.zax" };
    const std::string_view example2{ "ignored/testing/beebop/apple_fruit.txt" };
    const std::string_view example3{ "ignored/testing/beebop/banana_fruit.txt" };
    const std::string_view example4{ "ignored/testing/beecop/apple_fruit.txt" };
    const std::string_view example5{ "ignored/testing/beecop/banana_fruit.txt" };
    const std::string_view example6{ "ignored/testing/beecop/caraway_seed.txt" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example2 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example3 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example4 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example5 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example6 }.parent_path(), ec);

    const std::string_view content1{
      "\n"
      "[[asset='bee?op/*_fruit.txt', required=yes, rename=\"ignored/output/?op/*_food.txt\"\\\n"
      "]]; // ignore this\n"
    };
    const std::string_view content2{ "HELLO2" };
    const std::string_view content3{ "HELLO3" };
    const std::string_view content4{ "HELLO4" };
    const std::string_view content5{ "HELLO5" };
    const std::string_view content6{ "HELLO6" };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));
    TEST(zax::writeBinaryFile(example3, content3));
    TEST(zax::writeBinaryFile(example4, content4));
    TEST(zax::writeBinaryFile(example5, content5));
    TEST(zax::writeBinaryFile(example6, content6));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    auto checkExists{ [&](StringView fileName) noexcept(false) {
      std::error_code ec{};
      TEST(std::filesystem::is_regular_file(Path{ fileName }, ec));
      TEST(!ec);
    } };
    checkExists("ignored/output/bop/apple_food.txt");
    checkExists("ignored/output/bop/banana_food.txt");
    checkExists("ignored/output/cop/apple_food.txt");
    checkExists("ignored/output/cop/banana_food.txt");

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetPattern2() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/k.zax" };
    const std::string_view example2{ "ignored/testing/deebop/apple_fruit.txt" };
    const std::string_view example3{ "ignored/testing/deebop/banana_fruit.txt" };
    const std::string_view example4{ "ignored/testing/deecop/apple_fruit.txt" };
    const std::string_view example5{ "ignored/testing/deecop/banana_fruit.txt" };
    const std::string_view example6{ "ignored/testing/deecop/caraway_seed.txt" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example2 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example3 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example4 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example5 }.parent_path(), ec);
    std::filesystem::create_directories(std::filesystem::path{ example6 }.parent_path(), ec);

    const std::string_view content1{
      "\n"
      "[[asset='bee?op/*_fruit.txt', required=yes, rename=\"ignored/output/d?op/many_food.txt\"\\\n"
      "]]; // ignore this\n"
    };
    const std::string_view content2{ "HELLO2" };
    const std::string_view content3{ "HELLO3" };
    const std::string_view content4{ "HELLO4" };
    const std::string_view content5{ "HELLO5" };
    const std::string_view content6{ "HELLO6" };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));
    TEST(zax::writeBinaryFile(example3, content3));
    TEST(zax::writeBinaryFile(example4, content4));
    TEST(zax::writeBinaryFile(example5, content5));
    TEST(zax::writeBinaryFile(example6, content6));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(Error::WildCharacterMismatch, example1, 2, 3, StringMap{ { "$wild$", "ignored/output/dbop/many_food.txt" } });
    expect(Error::WildCharacterMismatch, example1, 2, 3, StringMap{ { "$wild$", "ignored/output/dbop/many_food.txt" } });
    expect(Error::WildCharacterMismatch, example1, 2, 3, StringMap{ { "$wild$", "ignored/output/dcop/many_food.txt" } });
    expect(Error::WildCharacterMismatch, example1, 2, 3, StringMap{ { "$wild$", "ignored/output/dcop/many_food.txt" } });
    expect(Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    auto checkNotExists{ [&](StringView fileName) noexcept(false) {
      std::error_code ec{};
      TEST(!std::filesystem::is_regular_file(Path{ fileName }, ec));
    } };
    checkNotExists("ignored/output/dbop/many_food.txt");
    checkNotExists("ignored/output/dcop/many_food.txt");

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalOutName() noexcept(false)
  {
    auto check{[&](const String& letter, const String& prefix) {
      const String example1{ "ignored/testing/parser/directive/source/"s + letter  +".zax" };
      const String example2{ "ignored/testing/parser/directive/source/"s + letter + "-asset.txt" };

      std::error_code ec;
      std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);

      const String content1{
        "\n"
        "[[asset='"s + letter + "-asset.txt', rename=\"" + prefix + "ignored/output/" + letter + "-asset.txt\"\\\n"
        "]]; // ignore this\n"
      };
      const std::string_view content2{ "HELLO" };

      TEST(zax::writeBinaryFile(example1, content1));
      TEST(zax::writeBinaryFile(example2, content2));

      Config config;
      config.inputFilePaths_.emplace_back(example1);
      auto parser{ std::make_shared<Parser>(config, callbacks()) };

      expect(Error::OutputFailure, example1, 2, 3, StringMap{ {"$file$", prefix + "ignored/output/" + letter + "-asset.txt" } });
      expect(Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

      parser->parse();

      ec = {};
      TEST(!std::filesystem::is_regular_file(Path{ "ignored/output/"s + letter + "-asset.txt" }, ec));
    } };

    check("l", "../");
    check("m", "/");

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalOutName2() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/n.zax"sv };
    const std::string_view example2{ "ignored/testing/parser/directive/source/n-asset.txt"sv };

    auto illegalChars{ "*\0"s };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);

    const String content1{
      "\n[[asset='n-asset.txt', rename=\"ignored/output/n"s + illegalChars + "asset.txt\"\\\n]]; // ignore this\n"
    };
    const std::string_view content2{ "HELLO" };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(Error::OutputFailure, example1, 2, 3, StringMap{ {"$file$"s, "ignored/output/n"s + illegalChars + "asset.txt"s} });
    expect(Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    ec = {};
    TEST(!std::filesystem::is_regular_file(Path{ String{ "ignored/output/n"} + illegalChars + "asset.txt" }, ec));

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalQuote() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/o1.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 43 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[asset='bogus.zax', required=yes, foo='hi']]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalQuote2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/o2.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 29 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[asset='bogus.zax', asset=? ?]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalQuote3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/o3.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 29 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[asset='bogus.zax', foo=? ?]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceMissingError() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/o4.zax" };

    expect(Error::SourceNotFound, example, 2, 3, StringMap{ { "$file$", "bogus.zax" } });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=yes\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceMissingIgnored() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/p.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=no\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveIncludeTwice() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/directive/source/q.zax" };
    const std::string_view example2{ "ignored/testing/parser/directive/source/r.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example1 }.parent_path(), ec);

    const std::string_view content1{
      "\n"
      "[[\\\n"
      "source='r.zax']]\n"
      "[[\\\n"
      "source='r.zax']];\n"
    };

    const std::string_view content2{
      "\n"
      "\\;\n"
    };

    TEST(zax::writeBinaryFile(example1, content1));
    TEST(zax::writeBinaryFile(example2, content2));

    Config config;
    config.inputFilePaths_.emplace_back(example1);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(Warning::NewlineAfterContinuation, example2, 2, 2);
    expect(Warning::StatementSeparatorOperatorRedundant, example2, 2, 2);
    expect(Warning::StatementSeparatorOperatorRedundant, example1, 5, 24 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceNotUnderstood() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/s.zax" };

    expect(Error::Syntax, example, 2, 16 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source!='bogus.zax'\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceNotUnderstood2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/t.zax" };

    expect(Error::Syntax, example, 2, 16 - 8 + 1);

    testCommon(example,
      "\n"
      "[[source=");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceNotUnderstood3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/t3.zax" };

    expect(Error::Syntax, example, 2, 3);

    testCommon(example,
      "\n"
      "[[source");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/u.zax" };

    expect(Error::Syntax, example, 2, 38 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required!=no\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/v.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 39 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=?\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh2a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/va.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 39 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=?\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh2b() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/vb.zax" };

    expect(Error::Syntax, example, 2, 16 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source- - ='bogus.zax',required=yes]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceLiteral() noexcept(false)
  {
#if 0
    const std::string_view example{ "ignored/testing/parser/directive/source/w.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source=yes]]\n"
      "; // ignore this\n");
#endif //0
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceEnumFail() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/x.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 38 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',required=foo]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceDoubleGenerated() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/x.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 53 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',generated=yes,generated=no]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceInvalidGenerated() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/x.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 39 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',generated=warn]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceInvalidLiteral() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/x.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 29 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',bogus=warn]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceEmptyValue() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/y.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='']]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceDoubleSource() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/source/z.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 30 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', source = 'hello.zax']]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test(); });
    runner([&]() { testDirectiveInclude(); });
    runner([&]() { testDirectiveInclude2(); });
    runner([&]() { testDirectiveOdd(); });
    runner([&]() { testDirectiveSourceBadAfterComma(); });
    runner([&]() { testDirectiveSourceRepeated(); });
    runner([&]() { testDirectiveAssetRepeated(); });
    runner([&]() { testDirectiveAssetNotFound(); });
    runner([&]() { testDirectiveAssetPattern(); });
    runner([&]() { testDirectiveAssetPattern2(); });
    runner([&]() { testDirectiveAssetIllegalOutName(); });
    runner([&]() { testDirectiveAssetIllegalOutName2(); });
    runner([&]() { testDirectiveAssetIllegalQuote(); });
    runner([&]() { testDirectiveAssetIllegalQuote2(); });
    runner([&]() { testDirectiveAssetIllegalQuote3(); });
    runner([&]() { testDirectiveSourceMissingError(); });
    runner([&]() { testDirectiveSourceMissingIgnored(); });
    runner([&]() { testDirectiveIncludeTwice(); });
    runner([&]() { testDirectiveSourceNotUnderstood(); });
    runner([&]() { testDirectiveSourceNotUnderstood2(); });
    runner([&]() { testDirectiveSourceNotUnderstood3(); });
    runner([&]() { testDirectiveSourceRequireHuh(); });
    runner([&]() { testDirectiveSourceRequireHuh2(); });
    runner([&]() { testDirectiveSourceRequireHuh2a(); });
    runner([&]() { testDirectiveSourceRequireHuh2b(); });
    runner([&]() { testDirectiveSourceLiteral(); });
    runner([&]() { testDirectiveSourceEnumFail(); });
    runner([&]() { testDirectiveSourceDoubleGenerated(); });
    runner([&]() { testDirectiveSourceInvalidGenerated(); });
    runner([&]() { testDirectiveSourceInvalidLiteral(); });
    runner([&]() { testDirectiveSourceEmptyValue(); });
    runner([&]() { testDirectiveSourceDoubleSource(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserTabStopDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/1.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[tab-stop]]\n"
      ";\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/2.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 22 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 5);

    testCommon(example,
      "\n"
      "[[tab-stop=4]];\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/3.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[tab-stop=-4]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/4.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[tab-stop=.3]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/5.zax" };

    expect(Error::Syntax, example, 2, 18 - 8 + 1);

    testCommon(example,
      "\n"
      "[[tab-stop=");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/6.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 21 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[tab-stop=4,bogus=7]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test6a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/6a.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 32 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 5);

    testCommon(example,
      "\n"
      "[[tab-stop=4,x-bogus=7]];\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/tabstop/7.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 21 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[tab-stop=4,bogus=? ?]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });
    runner([&]() { test6(); });
    runner([&]() { test6a(); });
    runner([&]() { test7(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserLineAssignDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/1.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 19 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 27 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,huh=10]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/1a.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 32 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 46 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,increment=10,increment=10]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1b() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/1b.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 19 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 26 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,huh=?]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1c() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/1c.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 42 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 47 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,increment=10,increment=? ?]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/2.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 99, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 100, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 101, 9);

    testCommon(example,
      "\n"
      "[[line=100]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2b() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/2b.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 98, 32 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 100, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 102, 9);

    testCommon(example,
      "\n"
      "[[line=100,increment=2]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2c() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/2c.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 99, 41 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 100, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 101, 9);

    testCommon(example,
      "\n"
      "[[line=100]] /* hello */ /* hi */;\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/3.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, -4, 9);

    testCommon(example,
      "\n"
      "[[line=-4]] \\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/4.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=.3]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/5.zax" };

    expect(Error::Syntax, example, 2, 14 - 8 + 1);

    testCommon(example,
      "\n"
      "[[line=");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/6.zax" };

    expect(Error::Syntax, example, 2, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=4100,\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/7.zax" };

    expect(Error::Syntax, example, 2, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=4100,,increment=1]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test8() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/8.zax" };

    expect(Error::Syntax, example, 2, 15 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test9() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/9.zax" };

    expect(Error::Syntax, example, 2, 15 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=,]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test10() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/10.zax" };

    expect(Error::Syntax, example, 2, 15 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test11() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/line-assign/11.zax" };

    expect(Error::Syntax, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test1a(); });
    runner([&]() { test1b(); });
    runner([&]() { test1c(); });
    runner([&]() { test2(); });
    runner([&]() { test2b(); });
    runner([&]() { test2c(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });
    runner([&]() { test6(); });
    runner([&]() { test7(); });
    runner([&]() { test8(); });
    runner([&]() { test9(); });
    runner([&]() { test10(); });
    runner([&]() { test11(); });

    reset();
  }
};


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserFileAssignDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/file-assign/1.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 35 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo.source',what='foo']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/file-assign/1a.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 35 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo.source',what=? ?]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/file-assign/2.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, "beebaddabo.source", 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo.source']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/file-assign/3.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, "beebaddabo.source", 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo''.source']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/file-assign/4.zax" };

    expect(Error::Syntax, example, 2, 14 - 8 + 1);

    testCommon(example,
      "\n"
      "[[file=");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/file-assign/5.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[file='']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test1a(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserErrorDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/error/1.zax" };

    expect(Error::Syntax, example, 2, 3 );
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error=syntax]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/error/2.zax" };

    expect(Error::ErrorDirective, example, 2, 3, StringMap{ {"$message$", "hello"} });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/error/3.zax" };

    expect(Error::ErrorDirective, example, 2, 3, StringMap{ {"$message$", "x-unknown"} });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error=x-unknown]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
#if 0
    const std::string_view example{ "ignored/testing/parser/directive/error/4.zax" };

    expect(Warning::UnknownDirective, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error=foo]]\n"
      "\t;\n");
#endif //0
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/error/5.zax" };

    expect(Error::Syntax, example, 2, 3, StringMap{ {"$message$", "hello"} });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error='hello',syntax]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/error/6.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 24 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error='hello',whatever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/error/7.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test8() noexcept(false)
  {
#if 0
    const std::string_view example{ "ignored/testing/parser/directive/error/8.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error=yes,foo]]\n"
      "\t;\n");
#endif //0
  }

  //-------------------------------------------------------------------------
  void test9() noexcept(false)
  {
#if 0
    const std::string_view example{ "ignored/testing/parser/directive/error/9.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error=yes,x-foo]]\n"
      "\t;\n");
#endif //0
  }

  //-------------------------------------------------------------------------
  void test10() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/error/10.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 31 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[error='hello',syntax,syntax]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });
    runner([&]() { test6(); });
    runner([&]() { test7(); });
    runner([&]() { test8(); });
    runner([&]() { test9(); });
    runner([&]() { test10(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserWarningDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/1.zax" };

    expect(Warning::Forever, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=forever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/2.zax" };

    expect(Warning::WarningDirective, example, 2, 3, StringMap{ {"$message$", "hello"} });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/3.zax" };

    expect(Warning::WarningDirective, example, 2, 3, StringMap{ {"$message$", "x-unknown"} });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=x-unknown]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
#if 0
    const std::string_view example{ "ignored/testing/parser/directive/warning/4.zax" };

    expect(Warning::UnknownDirective, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=foo]]\n"
      "\t;\n");
#endif //0
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/5.zax" };

    expect(Warning::Forever, example, 2, 3, StringMap{ {"$message$", "hello"} });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning='hello',forever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/6.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 26 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning='hello',whatever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/7.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test8() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/8.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 22 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=yes,foo]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test9() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/9.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=yes,x-foo]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test10() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/10.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 34 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning='hello',forever,forever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test11() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/11.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=push,forever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test12() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/12.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=pop,forever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test13() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/13.zax" };

    expect(Error::UnmatchedPush, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test14() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/14.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[warning=no,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15a.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[warning=no]]\\\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15b() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15b.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);

    testCommon(example,
      "\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=always]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15c() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15c.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);

    testCommon(example,
      "\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=yes]]\\\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15d() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15d.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);

    testCommon(example,
      "\n"
      "[[warning=never]]\n"
      "\t;\n"
      "[[warning=yes]]\\\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15e() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15e.zax" };

    error(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
    error(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);

    testCommon(example,
      "\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=error]]\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15f() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15f.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    error(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
    error(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
    expect(Warning::DirectiveNotUnderstood, example, 7, 3);
    error(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);

    testCommon(example,
      "\n"
      "[[warning=always]]\n"
      "\t;\n"
      "[[warning=error,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "\t;\n"
      "[[error=5]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15g() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/15g.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    error(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
    error(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
    expect(Warning::DirectiveNotUnderstood, example, 8, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);

    testCommon(example,
      "\n"
      "[[warning=always]]\n"
      "\t;\n"
      "[[warning=error,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=default,statement-separator-operator-redundant]]\n"
      "[[error=5]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test16() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/16.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=never,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "\t;\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test17() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/17.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 12, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=never,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "\t;\n"
      "[[warning=push]]\n"
      "[[warning=always,statement-separator-operator-redundant]]\\\n"
      "[[warning=pop]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test18() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/18.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 14, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=never,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "\t;\n"
      "[[warning=push]]\n"
      "\t;\n"
      "[[warning=always,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test19() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/19.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 15, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=push]]\n"
      "\t;\n"
      "[[warning=yes,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test20() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/20.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 16, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "[[warning=lock]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=push]]\n"
      "\t;\n"
      "[[warning=yes,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test21() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/21.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 13, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=yes,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "[[warning=lock]]\n"
      "[[warning=yes,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "[[warning=no,statement-separator-operator-redundant]]\\\n"
      "\t;\n"
      "\t;\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test22() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/22.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
    error(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 10, 9);

    testCommon(example,
      "\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=default]]\n"
      "\t;\n"
      "[[warning=error,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=default]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test23() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/23.zax" };

    testCommon(example,
      "\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "[[warning=lock,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=default]]\n"
      "\t;\n"
      "[[warning=error,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=default]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test24() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/24.zax" };

    expect(Warning::UnknownDirectiveArgument, example, 5, 21 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 14, 9);
    error(Warning::UnknownDirectiveArgument, example, 15, 21 - 8 + 1);

    testCommon(example,
      "\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "[[warning=lock,statement-separator-operator-redundant]]\n"
      "[[warning=always,unknown-directive-argument]]\n"
      "[[tab-stop=8,foo]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=default]]\n"
      "\t;\n"
      "[[warning=error,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=unlock,statement-separator-operator-redundant]]\n"
      "[[warning=default]]\n"
      "\t;\n"
      "[[tab-stop=8,foo]]\n");
  }

  //-------------------------------------------------------------------------
  void test25() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/25.zax" };

    expect(Warning::UnknownDirectiveArgument, example, 5, 21 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 14, 9);
    error(Warning::UnknownDirectiveArgument, example, 15, 21 - 8 + 1);

    testCommon(example,
      "\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "[[warning=lock,statement-separator-operator-redundant]]\n"
      "[[warning=always,unknown-directive-argument]]\n"
      "[[tab-stop=8,foo]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=default]]\n"
      "\t;\n"
      "[[warning=error,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=unlock]]\n"
      "[[warning=default]]\n"
      "\t;\n"
      "[[tab-stop=8,foo]]\n");
  }

  //-------------------------------------------------------------------------
  void test26() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/26.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 7, 9);
    error(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);
    expect(Error::UnmatchedPush, example, 12, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 13, 9);

    testCommon(example,
      "\n"
      "[[warning=push]]\n"
      "[[warning=never,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "\t;\n"
      "[[warning=default]]\n"
      "\t;\n"
      "[[warning=error,statement-separator-operator-redundant]]\n"
      "\t;\n"
      "[[warning=default]]\n"
      "[[warning=pop]]\n"
      "[[warning=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test27() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/27.zax" };

    expect(Warning::AssetNotFound, example, 2, 3, StringMap{ {"$file$", "foo"}, {"$message$","hello"} });
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning='hello',asset-not-found,name='$file$',value='foo']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test28() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/28.zax" };

    expect(Error::Syntax, example, 2, 3);

    testCommon(example,
      "\n"
      "[[warning");
  }

  //-------------------------------------------------------------------------
  void test29() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/29.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 56 -8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning='hello',asset-not-found,name='$file$',name='$foo$']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test30() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/30.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 42 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning='hello',asset-not-found,value='file',name='$file$']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test31() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/warning/31.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 23 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[warning=push,name='$file$',value='file']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });
    runner([&]() { test6(); });
    runner([&]() { test7(); });
    runner([&]() { test8(); });
    runner([&]() { test9(); });
    runner([&]() { test11(); });
    runner([&]() { test12(); });
    runner([&]() { test14(); });
    runner([&]() { test15(); });
    runner([&]() { test15a(); });
    runner([&]() { test15b(); });
    runner([&]() { test15c(); });
    runner([&]() { test15d(); });
    runner([&]() { test15e(); });
    runner([&]() { test15f(); });
    runner([&]() { test15g(); });
    runner([&]() { test16(); });
    runner([&]() { test17(); });
    runner([&]() { test18(); });
    runner([&]() { test19(); });
    runner([&]() { test20(); });
    runner([&]() { test21(); });
    runner([&]() { test22(); });
    runner([&]() { test23(); });
    runner([&]() { test24(); });
    runner([&]() { test25(); });
    runner([&]() { test26(); });
    runner([&]() { test27(); });
    runner([&]() { test28(); });
    runner([&]() { test29(); });
    runner([&]() { test30(); });
    runner([&]() { test31(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserPanicDirective : public ParserCommon
{

  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[panic=always,out-of-memory]]\n"
        "\t;\n");

      expect(0, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1a.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[panic=never,out-of-memory]]\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1c.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[panic=never]]\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[panic=always]]\n"
        "\t;\n");

      expect(0, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1e.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[panic=never]]\n"
        "\t;\n"
        "[[panic=yes]]\\\n"
        "\t;\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      expect(1, Panic::OutOfMemory);
      disabled(2, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1f.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[panic=never]]\n"
        "\t;\n"
        "[[panic=yes,out-of-memory]]\\\n"
        "\t;\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      expect(1, Panic::OutOfMemory);
      disabled(2, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1g.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[panic=never]]\n"
        "\t;\n"
        "[[panic=yes,impossible-code-flow]]\\\n"
        "\t;\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      disabled(1, Panic::OutOfMemory);
      disabled(2, Panic::OutOfMemory);
      reset();
    }

    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1h.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[panic=always]]\n"
        "\t;\n"
        "[[panic=no]]\\\n"
        "\t;\n"
        "\t;\n");

      expect(0, Panic::OutOfMemory);
      disabled(1, Panic::OutOfMemory);
      expect(2, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1i.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[panic=always]]\n"
        "\t;\n"
        "[[panic=no,out-of-memory]]\\\n"
        "\t;\n"
        "\t;\n");

      expect(0, Panic::OutOfMemory);
      disabled(1, Panic::OutOfMemory);
      expect(2, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/1j.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[panic=always]]\n"
        "\t;\n"
        "[[panic=no,impossible-code-flow]]\\\n"
        "\t;\n"
        "\t;\n");

      expect(0, Panic::OutOfMemory);
      expect(1, Panic::OutOfMemory);
      expect(2, Panic::OutOfMemory);
      reset();
    }
    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/panic/2.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[panic='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/panic/3.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 21 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[panic=push,name='$file$',value='file']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/panic/4.zax" };

    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[panic=always,x-bogus]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/panic/5.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[panic=error]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/6a.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[panic=never,out-of-memory]]\n"
        "\t;\n"
        "[[panic=default,out-of-memory]]\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      expect(1, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/6b.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[panic=never]]\n"
        "\t;\n"
        "[[panic=default]]\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      expect(1, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/6b.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[panic=never]]\n"
        "[[panic=lock]]\n"
        "\t;\n"
        "[[panic=default]]\n"
        "\t;\n");

      disabled(0, Panic::OutOfMemory);
      disabled(1, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/6c.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 10, 9);
      testCommon(example,
        "\n"
        "[[panic=never]]\n"
        "[[panic=lock]]\n"
        "\t;\n"
        "[[panic=default]]\n"
        "\t;\n"
        "[[panic=unlock]]\n"
        "\t;\n"
        "[[panic=default]]\n"
        "\t;\n"
      );

      disabled(0, Panic::OutOfMemory);
      disabled(1, Panic::OutOfMemory);
      disabled(2, Panic::OutOfMemory);
      expect(3, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/6d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 10, 9);
      testCommon(example,
        "\n"
        "[[panic=never,out-of-memory]]\n"
        "[[panic=lock,out-of-memory]]\n"
        "\t;\n"
        "[[panic=default,out-of-memory]]\n"
        "\t;\n"
        "[[panic=unlock,out-of-memory]]\n"
        "\t;\n"
        "[[panic=default,out-of-memory]]\n"
        "\t;\n"
      );

      disabled(0, Panic::OutOfMemory);
      disabled(1, Panic::OutOfMemory);
      disabled(2, Panic::OutOfMemory);
      expect(3, Panic::OutOfMemory);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/panic/6d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 7, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 11, 9);
      testCommon(example,
        "\n"
        "[[panic=push]]\n"
        "[[panic=never,out-of-memory]]\n"
        "[[panic=lock,out-of-memory]]\n"
        "\t;\n"
        "[[panic=default,out-of-memory]]\n"
        "\t;\n"
        "[[panic=pop]]\n"
        "\t;\n"
        "[[panic=default,out-of-memory]]\n"
        "\t;\n"
      );

      disabled(0, Panic::OutOfMemory);
      disabled(1, Panic::OutOfMemory);
      expect(2, Panic::OutOfMemory);
      expect(3, Panic::OutOfMemory);
      reset();
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/panic/7.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[panic=push,out-of-memory]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test8() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/panic/8.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[panic=pop,out-of-memory]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test9() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/panic/9.zax" };

    expect(Error::UnmatchedPush, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[panic=pop]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });
    runner([&]() { test6(); });
    runner([&]() { test7(); });
    runner([&]() { test8(); });
    runner([&]() { test9(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserFunctionsDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    {
      const std::string_view example{ "ignored/testing/parser/directive/functions/1a.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      testCommon(example,
        "\n"
        "\t;\n");

      TEST(faultTokenState(0)->functionDefaults_.inconstant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/functions/1b.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[functions=constant]]\n"
        "\t;\n");

      TEST(faultTokenState(0)->functionDefaults_.constant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/functions/1c.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[functions=inconstant]]\n"
        "\t;\n");

      TEST(faultTokenState(0)->functionDefaults_.inconstant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/functions/1d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 7, 9);
      testCommon(example,
        "\n"
        "\t;\n"
        "[[functions=push]]\n"
        "[[functions=constant]]\n"
        "\t;\n"
        "[[functions=pop]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->functionDefaults_.inconstant());
      TEST(faultTokenState(1)->functionDefaults_.constant());
      TEST(faultTokenState(2)->functionDefaults_.inconstant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/functions/1e.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Error::UnmatchedPush, example, 7, 3);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);
      testCommon(example,
        "\n"
        "\t;\n"
        "[[functions=push]]\n"
        "[[functions=constant]]\n"
        "\t;\n"
        "[[functions=pop]]\n"
        "[[functions=pop]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->functionDefaults_.inconstant());
      TEST(faultTokenState(1)->functionDefaults_.constant());
      TEST(faultTokenState(2)->functionDefaults_.inconstant());
      reset();
    }
    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/functions/2.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[functions='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/functions/3.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 29 -8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[functions=constant,hello='foo']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/functions/4.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 29 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[functions=constant,hello='constant']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/functions/5.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[functions=whatever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserTypesDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    {
      const std::string_view example{ "ignored/testing/parser/directive/types/1a.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      testCommon(example,
        "\n"
        "\t;\n");

      TEST(faultTokenState(0)->typeDefaults_._mutable());
      TEST(faultTokenState(0)->typeDefaults_.inconstant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/types/1b.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[types=mutable]]\n"
        "\t;\n"
        "[[types=constant]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->typeDefaults_._mutable());
      TEST(faultTokenState(0)->typeDefaults_.inconstant());

      TEST(faultTokenState(1)->typeDefaults_._mutable());
      TEST(faultTokenState(1)->typeDefaults_.constant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/types/1c.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[types=inconstant]]\n"
        "\t;\n"
        "[[types=immutable]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->typeDefaults_._mutable());
      TEST(faultTokenState(0)->typeDefaults_.inconstant());

      TEST(faultTokenState(1)->typeDefaults_.immutable());
      TEST(faultTokenState(1)->typeDefaults_.inconstant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/types/1d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);
      testCommon(example,
        "\n"
        "\t;\n"
        "[[types=push]]\n"
        "[[types=constant]]\n"
        "[[types=immutable]]\n"
        "\t;\n"
        "[[types=pop]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->typeDefaults_._mutable());
      TEST(faultTokenState(0)->typeDefaults_.inconstant());

      TEST(faultTokenState(1)->typeDefaults_.immutable());
      TEST(faultTokenState(1)->typeDefaults_.constant());

      TEST(faultTokenState(2)->typeDefaults_._mutable());
      TEST(faultTokenState(2)->typeDefaults_.inconstant());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/types/1e.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      expect(Error::UnmatchedPush, example, 8, 3);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);
      testCommon(example,
        "\n"
        "\t;\n"
        "[[types=push]]\n"
        "[[types=constant]]\n"
        "[[types=immutable]]\n"
        "\t;\n"
        "[[types=pop]]\n"
        "[[types=pop]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->typeDefaults_._mutable());
      TEST(faultTokenState(0)->typeDefaults_.inconstant());

      TEST(faultTokenState(1)->typeDefaults_.immutable());
      TEST(faultTokenState(1)->typeDefaults_.constant());

      TEST(faultTokenState(2)->typeDefaults_._mutable());
      TEST(faultTokenState(2)->typeDefaults_.inconstant());
      reset();
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/types/2.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[types='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/types/3.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 25 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[types=constant,mutable]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/types/4.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 25 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[types=constant,types=mutable]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/types/5.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 16 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[types=whatever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserVariablesDirective : public ParserCommon
{
  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    {
      const std::string_view example{ "ignored/testing/parser/directive/variables/1a.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      testCommon(example,
        "\n"
        "\t;\n");

      TEST(faultTokenState(0)->variableDefaults_._mutable());
      TEST(faultTokenState(0)->variableDefaults_.varies());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/variables/1b.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[variables=mutable]]\n"
        "\t;\n"
        "[[variables=final]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->variableDefaults_._mutable());
      TEST(faultTokenState(0)->variableDefaults_.varies());

      TEST(faultTokenState(1)->variableDefaults_._mutable());
      TEST(faultTokenState(1)->variableDefaults_._final());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/variables/1c.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[variables=varies]]\n"
        "\t;\n"
        "[[variables=immutable]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->variableDefaults_._mutable());
      TEST(faultTokenState(0)->variableDefaults_.varies());

      TEST(faultTokenState(1)->variableDefaults_.immutable());
      TEST(faultTokenState(1)->variableDefaults_.varies());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/variables/1d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);
      testCommon(example,
        "\n"
        "\t;\n"
        "[[variables=push]]\n"
        "[[variables=final]]\n"
        "[[variables=immutable]]\n"
        "\t;\n"
        "[[variables=pop]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->variableDefaults_._mutable());
      TEST(faultTokenState(0)->variableDefaults_.varies());

      TEST(faultTokenState(1)->variableDefaults_.immutable());
      TEST(faultTokenState(1)->variableDefaults_._final());

      TEST(faultTokenState(0)->variableDefaults_._mutable());
      TEST(faultTokenState(0)->variableDefaults_.varies());
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/variables/1e.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      expect(Error::UnmatchedPush, example, 8, 3);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 9, 9);
      testCommon(example,
        "\n"
        "\t;\n"
        "[[variables=push]]\n"
        "[[variables=final]]\n"
        "[[variables=immutable]]\n"
        "\t;\n"
        "[[variables=pop]]\n"
        "[[variables=pop]]\n"
        "\t;\n"
      );

      TEST(faultTokenState(0)->variableDefaults_._mutable());
      TEST(faultTokenState(0)->variableDefaults_.varies());

      TEST(faultTokenState(1)->variableDefaults_.immutable());
      TEST(faultTokenState(1)->variableDefaults_._final());

      TEST(faultTokenState(0)->variableDefaults_._mutable());
      TEST(faultTokenState(0)->variableDefaults_.varies());
      reset();
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/variables/2.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[variables='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/variables/3.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 27 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[variables=varies,mutable]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/variables/4.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 27 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[variables=varies,types=mutable]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/variables/5.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[variables=whatever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserDeprecateDirective : public ParserCommon
{
  using CsContext = CompileState::Deprecate::Context;

  void deprecate(
    size_t index,
    CsContext context,
    bool error,
    StringView min,
    StringView max) noexcept(false)
  {
    TEST(faultTokenState(index)->deprecate_.has_value());
    TEST(error == faultTokenState(index)->deprecate_->forceError_);
    TEST(faultTokenState(index)->deprecate_->context_ == context);
    if (min.empty())
      TEST(!faultTokenState(index)->deprecate_->min_.has_value());
    else {
      TEST(faultTokenState(index)->deprecate_->min_.has_value());
      TEST(std::strong_ordering::equal == faultTokenState(index)->deprecate_->min_->fullCompare(zax::SemanticVersion{ min }));
    }
    if (max.empty())
      TEST(!faultTokenState(index)->deprecate_->max_.has_value());
    else {
      TEST(faultTokenState(index)->deprecate_->min_.has_value());
      TEST(std::strong_ordering::equal == faultTokenState(index)->deprecate_->max_->fullCompare(zax::SemanticVersion{ max }));
    }
  }

  void noDeprecate(size_t index) noexcept(false)
  {
    TEST(!faultTokenState(index)->deprecate_.has_value());
  }

  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1a.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      testCommon(example,
        "\n"
        "\t;\n");

      noDeprecate(0);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1b.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[deprecate=never]]\n"
        "\t;\n"
        "[[deprecate=always]]\n"
        "\t;\n"
      );

      noDeprecate(0);
      deprecate(1, CsContext::Import, false, ""sv, ""sv);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1b-location.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      auto parser{ testCommon(example,
        "\n"
        "[[deprecate=never]]\n"
        "\t;\n"
        "[[deprecate=always]]\n"
        "\t;\n"
      ) };

      noDeprecate(0);
      deprecate(1, CsContext::Import, false, ""sv, ""sv);

      TEST(faultTokenState(1)->deprecate_->origin_.location_.line_ == 4);
      TEST(faultTokenState(1)->deprecate_->origin_.location_.column_ == 3);

      TEST(static_cast<bool>(faultTokenState(1)->deprecate_->origin_.filePath_));
      auto source{ faultTokenState(1)->deprecate_->origin_.filePath_->source_.lock() };
      TEST(static_cast<bool>(source));
      TEST(static_cast<bool>(source->realPath_));

      std::string normalizedFilePath{ zax::stringReplace(source->realPath_->filePath_, "\\", "/") };
      TEST(normalizedFilePath == example);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1c.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[deprecate=no]]\n"
        "\t;\n"
        "[[deprecate=yes]]\\\n"
        "\t;\n"
        "\t;\n"
      );

      noDeprecate(0);
      deprecate(1, CsContext::Import, false, ""sv, ""sv);
      noDeprecate(2);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[deprecate=no]]\n"
        "\t;\n"
        "[[deprecate]]\\\n"
        "\t;\n"
        "\t;\n"
      );

      noDeprecate(0);
      deprecate(1, CsContext::Import, false, ""sv, ""sv);
      noDeprecate(2);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1e.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[deprecate=always]]\n"
        "\t;\n"
        "[[deprecate=no]]\\\n"
        "\t;\n"
        "\t;\n"
      );
      deprecate(0, CsContext::Import, false, ""sv, ""sv);
      noDeprecate(1);
      deprecate(2, CsContext::Import, false, ""sv, ""sv);
      reset();
    }
    {
        const std::string_view example{ "ignored/testing/parser/directive/deprecate/1f.zax" };
        expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
        testCommon(example,
          "\n"
          "[[deprecate=always,min='1.2.3',max='4.5.6-alpha']]\n"
          "\t;\n"
        );

      deprecate(0, CsContext::Import, false, "1.2.3"sv, "4.5.6-alpha"sv);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1g.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[deprecate=always,context=local,min='1.2.3',error]]\n"
        "\t;\n"
      );

      deprecate(0, CsContext::Local, true, "1.2.3"sv, ""sv);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/deprecate/1g.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      testCommon(example,
        "\n"
        "[[deprecate=always,context=all,max='1.0.1',min='1.2.3',error]]\n"
        "\t;\n"
      );

      deprecate(0, CsContext::All, true, "1.2.3"sv, "1.0.1"sv);
      reset();
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/2.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/3.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 26 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=never,min='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/4.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=whatever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/5.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 26 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=never,error]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/6.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 23 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=no,error]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/7.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 35 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,context=whatever]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test8() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/8.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 33 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,error=whatever]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test9() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/9.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 47 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,context=all,context=all]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test10() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/10.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 39 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,min='1.1.1',min='1.1.1']]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test11() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/11.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 39 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,max='1.1.1',max='1.1.1']]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test12() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/12.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 27 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,min='1.1.bogus']]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test13() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/13.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 27 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,max='1.1.bogus']]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test14() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/14.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 27 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,error='1.1.bogus']]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test15() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/15.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 27 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,context]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test16() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/16.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 33 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,error,error]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test17() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/17.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 33 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=always,error,foo]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test18() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/deprecate/18.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 34 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[deprecate=never,context=all]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });
    runner([&]() { test6(); });
    runner([&]() { test7(); });
    runner([&]() { test8(); });
    runner([&]() { test9(); });
    runner([&]() { test10(); });
    runner([&]() { test11(); });
    runner([&]() { test12(); });
    runner([&]() { test13(); });
    runner([&]() { test14(); });
    runner([&]() { test15(); });
    runner([&]() { test16(); });
    runner([&]() { test17(); });
    runner([&]() { test18(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserExportDirective : public ParserCommon
{
  using CsContext = CompileState::Deprecate::Context;

  void visible(size_t index) noexcept(false)
  {
    TEST(faultTokenState(index)->export_.visible());
  }

  void hidden(size_t index) noexcept(false)
  {
    TEST(faultTokenState(index)->export_.hidden());
  }

  //-------------------------------------------------------------------------
  void test1() noexcept(false)
  {
    {
      const std::string_view example{ "ignored/testing/parser/directive/export/1a.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 2, 9);
      testCommon(example,
        "\n"
        "\t;\n");

      hidden(0);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/export/1b.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      testCommon(example,
        "\n"
        "[[export=never]]\n"
        "\t;\n"
        "[[export=always]]\n"
        "\t;\n"
      );

      hidden(0);
      visible(1);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/export/1c.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[export=no]]\n"
        "\t;\n"
        "[[export=yes]]\\\n"
        "\t;\n"
        "\t;\n"
      );

      hidden(0);
      visible(1);
      hidden(2);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/export/1d.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[export=no]]\n"
        "\t;\n"
        "[[export]]\\\n"
        "\t;\n"
        "\t;\n"
      );

      hidden(0);
      visible(1);
      hidden(2);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/export/1e.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 5, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      testCommon(example,
        "\n"
        "[[export=always]]\n"
        "\t;\n"
        "[[export=no]]\\\n"
        "\t;\n"
        "\t;\n"
      );
      visible(0);
      hidden(1);
      visible(2);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/export/1f.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 6, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 7, 9);
      testCommon(example,
        "\n"
        "[[export=push]]\n"
        "[[export=always]]\n"
        "\t;\n"
        "[[export=pop]]\n"
        "\t;\n"
        "\t;\n"
      );
      visible(0);
      hidden(1);
      hidden(2);
      reset();
    }
    {
      const std::string_view example{ "ignored/testing/parser/directive/export/1g.zax" };
      expect(Warning::StatementSeparatorOperatorRedundant, example, 4, 9);
      expect(Error::UnmatchedPush, example, 6, 3);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 7, 9);
      expect(Warning::StatementSeparatorOperatorRedundant, example, 8, 9);
      testCommon(example,
        "\n"
        "[[export=push]]\n"
        "[[export=always]]\n"
        "\t;\n"
        "[[export=pop]]\n"
        "[[export=pop]]\n"
        "\t;\n"
        "\t;\n"
      );
      visible(0);
      hidden(1);
      hidden(2);
      reset();
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/export/2.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[export='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/export/3.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 23 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[export=never,min='hello']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/export/4.zax" };

    error(Warning::DirectiveNotUnderstood, example, 2, 17 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[export=whatever]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/export/5.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 23 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[export=never,error]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/directive/export/6.zax" };

    error(Warning::UnknownDirectiveArgument, example, 2, 20 - 8 + 1);
    expect(Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[export=no,error]]\\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test1(); });
    runner([&]() { test2(); });
    runner([&]() { test3(); });
    runner([&]() { test4(); });
    runner([&]() { test5(); });
    runner([&]() { test6(); });

    reset();
  }
};

//---------------------------------------------------------------------------
void testParserLineDirectives() noexcept(false)
{
  ParserSourceAssetDirectives{}.runAll();
  ParserTabStopDirective{}.runAll();
  ParserLineAssignDirective{}.runAll();
  ParserFileAssignDirective{}.runAll();
  ParserErrorDirective{}.runAll();
  ParserWarningDirective{}.runAll();
  ParserPanicDirective{}.runAll();
  ParserFunctionsDirective{}.runAll();
  ParserTypesDirective{}.runAll();
  ParserVariablesDirective{}.runAll();
  ParserDeprecateDirective{}.runAll();
  ParserExportDirective{}.runAll();
}

} // namespace zaxTest
