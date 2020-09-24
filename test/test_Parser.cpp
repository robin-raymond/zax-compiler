
#include <pch.h>

#include <filesystem>
#include <variant>

#include "common.h"

#include "../src/Parser.h"

using Parser = zax::Parser;
using Config = zax::Config;
using Callbacks = zax::ParserTypes::Callbacks;
using Error = zax::ErrorTypes::Error;
using Warning = zax::WarningTypes::Warning;
using Informational = zax::InformationalTypes::Informational;
using TokenConstPtr = zax::TokenConstPtr;
using StringMap = zax::StringMap;
using StringList = zax::StringList;
using String = zax::String;
using StringView = zax::StringView;
using Path = zax::Path;

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace zaxTest
{

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ParserBasics
{
  struct ExpectedFailures
  {
    std::variant<Error, Warning, Informational> type_;
    bool isFatal_{};
    String fileName_;
    int line_{};
    int column_{};
    StringMap mapping_;

    ExpectedFailures(bool fatal, zax::ErrorTypes::Error error, StringView fileName, int line, int column, const StringMap& mapping) noexcept(false) :
      type_{ error },
      isFatal_{ fatal },
      fileName_{ fileName },
      line_{ line },
      column_{ column },
      mapping_{ mapping }
    {}
    ExpectedFailures(zax::WarningTypes::Warning warning, StringView fileName, int line, int column, const StringMap& mapping) noexcept(false) :
      type_{ warning },
      fileName_{ fileName },
      line_{ line },
      column_{ column },
      mapping_{ mapping }
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

      failures_.pop_front();
    };
  }

  //-------------------------------------------------------------------------
  void fatal(zax::ErrorTypes::Error error, StringView fileName, int line, int column, const StringMap& mapping = {}) noexcept(false)
  {
    failures_.emplace_back(true, error, fileName, line, column, mapping);
  }

  //-------------------------------------------------------------------------
  void expect(zax::ErrorTypes::Error error, StringView fileName, int line, int column, const StringMap& mapping = {}) noexcept(false)
  {
    failures_.emplace_back(false, error, fileName, line, column, mapping);
  }

  //-------------------------------------------------------------------------
  void expect(zax::WarningTypes::Warning warning, StringView fileName, int line, int column, const StringMap& mapping = {}) noexcept(false)
  {
    failures_.emplace_back(warning, fileName, line, column, mapping);
  }

  //-------------------------------------------------------------------------
  void test() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/a.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "\\;\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::NewlineAfterContinuation, example, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 2);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveInclude() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/b.zax" };
    const std::string_view example2{ "ignored/testing/parser/c.zax" };

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

    expect(zax::WarningTypes::Warning::NewlineAfterContinuation, example2, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example2, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 24 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveInclude2() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/d.zax" };
    const std::string_view example2{ "ignored/testing/parser/e.zax" };

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

    expect(zax::WarningTypes::Warning::NewlineAfterContinuation, example2, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example2, 2, 2);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveOdd() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/f.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[ < foobar ]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 4);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 22 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceBadAfterComma() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/g.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source='bogus.zax', required=warn, < ]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 45 - 8 + 1);
    expect(zax::WarningTypes::Warning::SourceNotFound, example, 2, 17 - 8 + 1, StringMap{ {"$file$", "bogus.zax"} });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 49 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRepeated() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/h.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source='bog''us.zax', required=warn, required=yes, generated=no, generated=yes, rename=\"foo.zax\"\\\n"
      ", unknown='' ]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 47 - 8 + 1);
    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 75 - 8 + 1);
    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 90 - 8 + 1);
    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 3, 10 - 8 + 1);
    expect(zax::WarningTypes::Warning::SourceNotFound, example, 2, 17 - 8 + 1, StringMap{ { "$file$", "bogus.zax" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 23 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetRepeated() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/i.zax" };
    const std::string_view example2{ "ignored/testing/parser/asset.txt" };

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

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example1, 2, 46 - 8 + 1);
    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example1, 2, 74 - 8 + 1);
    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example1, 3, 10 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 23 - 8 + 1);

    parser->parse();

    ec = {};
    TEST(std::filesystem::is_regular_file(Path{ "ignored/output/asset.txt" }, ec));
    TEST(!ec);

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetNotFound() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/j.zax" };
    const std::string_view example2{ "ignored/testing/parser/asset.txt" };

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

    expect(zax::ErrorTypes::Error::AssetNotFound, example1, 2, 16 - 8 + 1, StringMap{ { "$file$", "bogus_asset.txt" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetPattern() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/k.zax" };
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

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

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
  void testDirectiveAssetIllegalOutName() noexcept(false)
  {
    auto check{[&](const String& letter, const String& prefix) {
      const String example1{ "ignored/testing/parser/"s + letter  +".zax" };
      const String example2{ "ignored/testing/parser/"s + letter + "-asset.txt" };

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

      expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 16 - 8 + 1, StringMap{ {"$file$", prefix + "ignored/output/" + letter + "-asset.txt" } });
      expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

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
    const std::string_view example1{ "ignored/testing/parser/n.zax"sv };
    const std::string_view example2{ "ignored/testing/parser/n-asset.txt"sv };

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

    expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 16 - 8 + 1, StringMap{ {"$file$"s, "ignored/output/n"s + illegalChars + "asset.txt"s} });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    ec = {};
    TEST(!std::filesystem::is_regular_file(Path{ String{ "ignored/output/n"} + illegalChars + "asset.txt" }, ec));

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceMissingError() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/o.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source='bogus.zax', required=yes\\\n"
      "]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::ErrorTypes::Error::SourceNotFound, example, 2, 17 - 8 + 1, StringMap{ { "$file$", "bogus.zax" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceMissingIgnored() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/p.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source='bogus.zax', required=no\\\n"
      "]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveIncludeTwice() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/q.zax" };
    const std::string_view example2{ "ignored/testing/parser/r.zax" };

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

    expect(zax::WarningTypes::Warning::NewlineAfterContinuation, example2, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example2, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 5, 24 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceNotUnderstood() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/s.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source!='bogus.zax'\\\n"
      "]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 16 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceNotUnderstood2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/t.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source="
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 16 - 8 + 1);
    expect(zax::ErrorTypes::Error::Syntax, example, 2, 16 - 8 + 1);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/u.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source='bogus.zax', required!=no\\\n"
      "]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 38 - 8 + 1);
    expect(zax::ErrorTypes::Error::SourceNotFound, example, 2, 17 - 8 + 1, StringMap{ {"$file$", "bogus.zax" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/v.zax" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ example }.parent_path(), ec);

    const std::string_view content{
      "\n"
      "[[source='bogus.zax', required=?\\\n"
      "]]; // ignore this\n"
    };

    TEST(zax::writeBinaryFile(example, content));

    Config config;
    config.inputFilePaths_.emplace_back(example);
    auto parser{ std::make_shared<Parser>(config, callbacks()) };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 39 - 8 + 1);
    expect(zax::ErrorTypes::Error::SourceNotFound, example, 2, 17 - 8 + 1, StringMap{ {"$file$", "bogus.zax" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
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
    runner([&]() { testDirectiveAssetIllegalOutName(); });
    runner([&]() { testDirectiveAssetIllegalOutName2(); });
    runner([&]() { testDirectiveSourceMissingError(); });
    runner([&]() { testDirectiveSourceMissingIgnored(); });
    runner([&]() { testDirectiveIncludeTwice(); });
    runner([&]() { testDirectiveSourceNotUnderstood(); });
    runner([&]() { testDirectiveSourceNotUnderstood2(); });
    runner([&]() { testDirectiveSourceRequireHuh(); });
    runner([&]() { testDirectiveSourceRequireHuh2(); });

    reset();
  }
};

//---------------------------------------------------------------------------
void testParser() noexcept(false)
{
  ParserBasics{}.runAll();
}

} // namespace zaxTest
