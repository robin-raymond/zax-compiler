
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
  void testCommon(
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
    const std::string_view example{ "ignored/testing/parser/sources/a.zax" };
    expect(zax::WarningTypes::Warning::NewlineAfterContinuation, example, 2, 2);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 2);

    testCommon(example,
      "\n"
      "\\;\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveInclude() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/sources/b.zax" };
    const std::string_view example2{ "ignored/testing/parser/sources/c.zax" };

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
    const std::string_view example1{ "ignored/testing/parser/sources/d.zax" };
    const std::string_view example2{ "ignored/testing/parser/sources/e.zax" };

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
    const std::string_view example{ "ignored/testing/parser/sources/f.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 4);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 22 - 8 + 1);

    testCommon(example,
      "\n"
      "[[ < foobar ]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceBadAfterComma() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/g.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 45 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 49 - 8 + 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=warn, < ]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRepeated() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/h.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 47 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 23 - 8 + 1);

    testCommon(example,
      "\n"
      "[[source='bog''us.zax', required=warn, required=yes, generated=no, generated=yes, rename=\"foo.zax\"\\\n"
      ", unknown='' ]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetRepeated() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/sources/i.zax" };
    const std::string_view example2{ "ignored/testing/parser/sources/asset.txt" };

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
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 23 - 8 + 1);

    parser->parse();

    TEST(!std::filesystem::is_regular_file(Path{ "ignored/output/asset.txt" }, ec));

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetNotFound() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/sources/j.zax" };
    const std::string_view example2{ "ignored/testing/parser/sources/asset.txt" };

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

    expect(zax::ErrorTypes::Error::AssetNotFound, example1, 2, 3, StringMap{ { "$file$", "bogus_asset.txt" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetPattern() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/sources/k.zax" };
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
  void testDirectiveAssetPattern2() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/sources/k.zax" };
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

    expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 3, StringMap{ { "$file$", "ignored/output/dbop/many_food.txt" } });
    expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 3, StringMap{ { "$file$", "ignored/output/dbop/many_food.txt" } });
    expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 3, StringMap{ { "$file$", "ignored/output/dcop/many_food.txt" } });
    expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 3, StringMap{ { "$file$", "ignored/output/dcop/many_food.txt" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

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
      const String example1{ "ignored/testing/parser/sources/"s + letter  +".zax" };
      const String example2{ "ignored/testing/parser/sources/"s + letter + "-asset.txt" };

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

      expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 3, StringMap{ {"$file$", prefix + "ignored/output/" + letter + "-asset.txt" } });
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
    const std::string_view example1{ "ignored/testing/parser/sources/n.zax"sv };
    const std::string_view example2{ "ignored/testing/parser/sources/n-asset.txt"sv };

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

    expect(zax::ErrorTypes::Error::OutputFailure, example1, 2, 3, StringMap{ {"$file$"s, "ignored/output/n"s + illegalChars + "asset.txt"s} });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example1, 3, 3);

    parser->parse();

    ec = {};
    TEST(!std::filesystem::is_regular_file(Path{ String{ "ignored/output/n"} + illegalChars + "asset.txt" }, ec));

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalQuote() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/o1.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 43 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[asset='bogus.zax', required=yes, foo='hi']]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalQuote2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/o2.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 29 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[asset='bogus.zax', asset=? ?]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveAssetIllegalQuote3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/o3.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 29 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[asset='bogus.zax', foo=? ?]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceMissingError() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/o4.zax" };

    expect(zax::ErrorTypes::Error::SourceNotFound, example, 2, 3, StringMap{ { "$file$", "bogus.zax" } });
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=yes\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceMissingIgnored() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/p.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=no\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveIncludeTwice() noexcept(false)
  {
    const std::string_view example1{ "ignored/testing/parser/sources/q.zax" };
    const std::string_view example2{ "ignored/testing/parser/sources/r.zax" };

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
    const std::string_view example{ "ignored/testing/parser/sources/s.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 16 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source!='bogus.zax'\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceNotUnderstood2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/t.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 16 - 8 + 1);

    testCommon(example,
      "\n"
      "[[source=");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceNotUnderstood3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/t3.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 3);

    testCommon(example,
      "\n"
      "[[source");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/u.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 38 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required!=no\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/v.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 30 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 3);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=?\\\n"
      "]]; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh2a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/va.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 30 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax', required=?\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceRequireHuh2b() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/vb.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 16 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source- - ='bogus.zax',required=yes]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceLiteral() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/w.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source=yes]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceEnumFail() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/x.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 29 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',required=foo]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceDoubleGenerated() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/x.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 43 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',generated=yes,generated=no]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceInvalidGenerated() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/x.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 29 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',generated=warn]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceInvalidLiteral() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/x.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 29 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='bogus.zax',bogus=warn]]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceEmptyValue() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/y.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[source='']]\n"
      "; // ignore this\n");
  }

  //-------------------------------------------------------------------------
  void testDirectiveSourceDoubleSource() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/sources/z.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 30 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

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
    const std::string_view example{ "ignored/testing/parser/tabstop/a.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 1);

    testCommon(example,
      "\n"
      "[[tab-stop]]\n"
      ";\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/tabstop/b.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 22 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 5);

    testCommon(example,
      "\n"
      "[[tab-stop=4]];\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/tabstop/c.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[tab-stop=-4]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/tabstop/d.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[tab-stop=.3]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/tabstop/e.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 18 - 8 + 1);

    testCommon(example,
      "\n"
      "[[tab-stop=");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/tabstop/f.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 21 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[tab-stop=4,bogus=7]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/tabstop/g.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 21 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

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
    const std::string_view example{ "ignored/testing/parser/lineassign/1.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 19 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 27 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,huh=10]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/1a.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 32 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 46 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,increment=10,increment=10]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1b() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/1b.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 19 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 26 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,huh=?]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1c() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/1c.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 32 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 2, 47 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 4, 9);

    testCommon(example,
      "\n"
      "[[line=100,increment=10,increment=? ?]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/2.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 99, 20 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 100, 9);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 101, 9);

    testCommon(example,
      "\n"
      "[[line=100]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2b() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/2b.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 98, 32 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 100, 9);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 102, 9);

    testCommon(example,
      "\n"
      "[[line=100,increment=2]];\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2c() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/2c.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 99, 41 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 100, 9);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 101, 9);

    testCommon(example,
      "\n"
      "[[line=100]] /* hello */ /* hi */;\n"
      "\t;\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/3.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, -4, 9);

    testCommon(example,
      "\n"
      "[[line=-4]] \\\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/4.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=.3]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/5.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 14 - 8 + 1);

    testCommon(example,
      "\n"
      "[[line=");
  }

  //-------------------------------------------------------------------------
  void test6() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/6.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 20 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=4100,\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test7() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/7.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 20 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=4100,,increment=1]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test8() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/8.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 15 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test9() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/9.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 15 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=,]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test10() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/10.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 15 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[line=]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test11() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/lineassign/11.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

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
    const std::string_view example{ "ignored/testing/parser/fileassign/1.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 35 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo.source',what='foo']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test1a() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/fileassign/1a.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 35 - 8 + 1);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo.source',what=? ?]]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/fileassign/2.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, "beebaddabo.source", 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo.source']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test3() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/fileassign/3.zax" };

    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, "beebaddabo.source", 3, 9);

    testCommon(example,
      "\n"
      "[[file='beebaddabo''.source']]\n"
      "\t;\n");
  }

  //-------------------------------------------------------------------------
  void test4() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/fileassign/4.zax" };

    expect(zax::ErrorTypes::Error::Syntax, example, 2, 14 - 8 + 1);

    testCommon(example,
      "\n"
      "[[file=");
  }

  //-------------------------------------------------------------------------
  void test5() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/parser/fileassign/5.zax" };

    expect(zax::WarningTypes::Warning::DirectiveNotUnderstood, example, 2, 3);
    expect(zax::WarningTypes::Warning::StatementSeparatorOperatorRedundant, example, 3, 9);

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
void testParser() noexcept(false)
{
  ParserSourceAssetDirectives{}.runAll();
  ParserTabStopDirective{}.runAll();
  ParserLineAssignDirective{}.runAll();
  ParserFileAssignDirective{}.runAll();
}

} // namespace zaxTest
