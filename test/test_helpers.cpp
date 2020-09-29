
#include <pch.h>

#include <filesystem>

#include "common.h"

#include "../src/helpers.h"

using StringView = zax::StringView;
using StringList = zax::StringList;
using SemanticVersion = zax::SemanticVersion;

using namespace std::string_view_literals;

namespace zaxTest
{

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct HelperBasics
{
  //-------------------------------------------------------------------------
  void reset() noexcept
  {
  }

  //-------------------------------------------------------------------------
  void test() noexcept(false)
  {
    const std::string_view example{ "ignored/testing/helpers/a/b/example.txt" };
    const std::string_view expecting{ "ignored/testing/helpers/a/b/testing_1.txt" };
    std::string fullPath;
    auto filePath{ zax::makeIncludeFile(example, "testing_1.txt", fullPath) };

    std::string parentPath;
    std::string fileName{ zax::fileAndPathFromFilePath(filePath, parentPath) };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ parentPath }, ec);

    TEST(fileName == "testing_1.txt");

    std::string normalizedFilePath{ zax::stringReplace(filePath, "\\", "/") };
    std::string normalizedFullPath{ zax::stringReplace(fullPath, "\\", "/") };

    TEST(normalizedFilePath == expecting);
    TEST(normalizedFullPath.substr(fullPath.length() - expecting.length()) == expecting);

    TEST(zax::writeBinaryFile(filePath, "hello"));
    auto binary{ zax::readBinaryFile(filePath) };
    auto view{ std::string_view(reinterpret_cast<const char*>(binary.first.get()), binary.second) };
    TEST(view == "hello");

    {
      std::string fullPath1;
      std::string fullPath2;
      std::string fullPath3;
      std::string fullPath4;
      auto filePath1{ zax::makeIncludeFile(example, "../1.txt", fullPath1) };
      auto filePath2{ zax::makeIncludeFile(example, "../../2.txt", fullPath2) };
      auto filePath3{ zax::makeIncludeFile(example, "../../3.txt", fullPath3) };
      auto filePath4{ zax::makeIncludeFile(example, "../../../4.txt", fullPath4) };

      TEST(zax::writeBinaryFile(filePath1, "hello1"));
      TEST(zax::writeBinaryFile(filePath2, "hello2"));
      TEST(zax::writeBinaryFile(filePath3, "hello3"));
      TEST(zax::writeBinaryFile(filePath4, "hello4"));

      std::string fullLocated1;
      auto located1{ zax::locateFile(example, "1.txt", fullLocated1) };
      std::string fullLocated1a;
      auto located1a{ zax::locateFile(example, "a/1.txt", fullLocated1a) };
      TEST(fullPath1 == fullLocated1);
      TEST(fullPath1 == fullLocated1a);

      std::string fullLocated1b;
      auto located1b{ zax::locateFile(example, "b/1.txt", fullLocated1b) };
      TEST(located1b.empty());
      TEST(fullLocated1b.empty());

      std::string fullLocated2;
      auto located2{ zax::locateFile(example, "../2.txt", fullLocated2) };
      TEST(fullPath2 == fullLocated2);
      std::string fullLocated2a;
      auto located2a{ zax::locateFile(example, "helpers/2.txt", fullLocated2a) };
      TEST(fullPath2 == fullLocated2a);
      std::string fullLocated2b;
      auto located2b{ zax::locateFile(example, "2.txt", fullLocated2b) };
      TEST(fullPath2 == fullLocated2b);

      std::string fullLocated3;
      auto located3{ zax::locateFile(example, "3.txt", fullLocated3) };
      TEST(fullPath3 == fullLocated3);

      std::string fullLocated4;
      auto located4{ zax::locateFile(example, "4.txt", fullLocated4) };
      TEST(fullPath4 == fullLocated4);
      std::string fullLocated4a;
      auto located4a{ zax::locateFile(example, "../../4.txt", fullLocated4a) };
      TEST(fullPath4 == fullLocated4a);
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testWildCard() noexcept(false)
  {
    const std::string path1{ "ignored/testing/helpers/fruit/apple_fruit" };
    const std::string path2{ "ignored/testing/helpers/fruit/banana_fruit" };
    const std::string path3{ "ignored/testing/helpers/fruit/caraway_seed" };
    const std::string path4{ "ignored/testing/helpers/fruit/dandy_seed" };

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path{ path1 }, ec);
    std::filesystem::create_directories(std::filesystem::path{ path2 }, ec);
    std::filesystem::create_directories(std::filesystem::path{ path3 }, ec);
    std::filesystem::create_directories(std::filesystem::path{ path4 }, ec);

    TEST(zax::writeBinaryFile(path1 + "/hello.txt", "hello1"));
    TEST(zax::writeBinaryFile(path1 + "/helloa.txt", "hello2"));
    TEST(zax::writeBinaryFile(path1 + "/helloab.txt", "hello3"));
    TEST(zax::writeBinaryFile(path1 + "/hellobbc.txt", "hello3"));
    TEST(zax::writeBinaryFile(path1 + "/helloabc.txt", "hello4"));
    TEST(zax::writeBinaryFile(path2 + "/hello.txt", "hello1"));
    TEST(zax::writeBinaryFile(path2 + "/helloa.txt", "hello2"));
    TEST(zax::writeBinaryFile(path2 + "/helloab.txt", "hello3"));
    TEST(zax::writeBinaryFile(path2 + "/hellobbc.txt", "hello3"));
    TEST(zax::writeBinaryFile(path2 + "/helloabc.txt", "hello4"));
    TEST(zax::writeBinaryFile(path3 + "/hello.txt", "hello1"));
    TEST(zax::writeBinaryFile(path3 + "/helloa.txt", "hello2"));
    TEST(zax::writeBinaryFile(path3 + "/helloab.txt", "hello3"));
    TEST(zax::writeBinaryFile(path3 + "/hellobbc.txt", "hello3"));
    TEST(zax::writeBinaryFile(path3 + "/helloabc.txt", "hello4"));
    TEST(zax::writeBinaryFile(path4 + "/hello.txt", "hello1"));
    TEST(zax::writeBinaryFile(path4 + "/helloa.txt", "hello2"));
    TEST(zax::writeBinaryFile(path4 + "/helloab.txt", "hello3"));
    TEST(zax::writeBinaryFile(path4 + "/hellobbc.txt", "hello3"));
    TEST(zax::writeBinaryFile(path4 + "/helloabc.txt", "hello4"));

    std::list<zax::LocateWildCardFilesResult> found;
    zax::locateWildCardFiles(found, "ignored/testing/helpers/fruit/apple_fruit/test.txt", "testing/helpers/fr*/*_?ruit/hello*.txt");

    auto validate{ [&](const std::string& path, const StringList& matches) noexcept -> bool {
      for (auto& entry : found) {
        std::string normalizedEntryPath{ zax::stringReplace(entry.path_, "\\", "/") };
        if (normalizedEntryPath != path)
          continue;
        TEST(matches.size() == entry.foundMatches_.size());
        for (auto& match : matches) {
          auto found{ entry.foundMatches_.front() };
          TEST(found == match);
          entry.foundMatches_.pop_front();
        }
        return true;
      }
      return false;
    } };
    
    TEST(found.size() == 10);
    TEST(validate("ignored/testing/helpers/fruit/apple_fruit/hello.txt", StringList{ "uit", "apple", "f", "" }));
    TEST(validate("ignored/testing/helpers/fruit/apple_fruit/helloa.txt", StringList{ "uit", "apple", "f", "a" }));
    TEST(validate("ignored/testing/helpers/fruit/apple_fruit/helloab.txt", StringList{ "uit", "apple", "f", "ab" }));
    TEST(validate("ignored/testing/helpers/fruit/apple_fruit/helloabc.txt", StringList{ "uit", "apple", "f", "abc" }));
    TEST(validate("ignored/testing/helpers/fruit/apple_fruit/hellobbc.txt", StringList{ "uit", "apple", "f", "bbc" }));
    TEST(validate("ignored/testing/helpers/fruit/banana_fruit/hello.txt", StringList{ "uit", "banana", "f", "" }));
    TEST(validate("ignored/testing/helpers/fruit/banana_fruit/helloa.txt", StringList{ "uit", "banana", "f", "a" }));
    TEST(validate("ignored/testing/helpers/fruit/banana_fruit/helloab.txt", StringList{ "uit", "banana", "f", "ab" }));
    TEST(validate("ignored/testing/helpers/fruit/banana_fruit/helloabc.txt", StringList{ "uit", "banana", "f", "abc" }));
    TEST(validate("ignored/testing/helpers/fruit/banana_fruit/hellobbc.txt", StringList{ "uit", "banana", "f", "bbc" }));

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test(); });
    runner([&]() { testWildCard(); });

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct HelperSemanticVersion
{

  //-------------------------------------------------------------------------
  void reset() noexcept
  {
  }

  //-------------------------------------------------------------------------
  void expect(
    const SemanticVersion& value,
    int major,
    int minor,
    int patch,
    StringView preRelease,
    StringView build) noexcept(false)
  {
    TEST(value.major_ == major);
    TEST(value.minor_ == minor);
    TEST(value.patch_ == patch);
    TEST(value.preRelease_ == preRelease);
    TEST(value.build_ == build);
  }

  //-------------------------------------------------------------------------
  void expect(
    const std::optional<const SemanticVersion>& value,
    int major,
    int minor,
    int patch,
    StringView preRelease,
    StringView build) noexcept(false)
  {
    TEST(static_cast<bool>(value));
    expect(*value, major, minor, patch, preRelease, build);
  }

  //-------------------------------------------------------------------------
  void fail(const std::optional<const SemanticVersion>& value) noexcept
  {
    TEST(!static_cast<bool>(value));
  }

  //-------------------------------------------------------------------------
  void test() noexcept(false)
  {
    expect(SemanticVersion::convert("1.0.0"sv), 1, 0, 0, ""sv, ""sv);
    fail(SemanticVersion::convert("1.0"sv));
    fail(SemanticVersion::convert(".1.0.0"sv));
    fail(SemanticVersion::convert("1.0.0."sv));
    fail(SemanticVersion::convert("1.0.0.0"sv));
    fail(SemanticVersion::convert("1a.0.0"sv));
    fail(SemanticVersion::convert("111111111111111111111111111111111111111111111111.0.0"sv));
    fail(SemanticVersion::convert("1.-1.0"sv));
    fail(SemanticVersion::convert("1.1..0"sv));
    fail(SemanticVersion::convert("1.0.0.+alpha"sv));
    fail(SemanticVersion::convert("1.0.0+"sv));
    fail(SemanticVersion::convert("1.0.0-"sv));
    fail(SemanticVersion::convert("1.0.0+-build"sv));
    fail(SemanticVersion::convert("1.0.0-+build"sv));
    fail(SemanticVersion::convert("1.0.0++build"sv));
    fail(SemanticVersion::convert("1.0.0--build"sv));
    fail(SemanticVersion::convert("1.0.11111111111111111111111111111111111111111111111111+build"sv));
    fail(SemanticVersion::convert("1.0.11111111111111111111111111111111111111111111111111-build"sv));
    fail(SemanticVersion::convert("1.0-build"sv));
    fail(SemanticVersion::convert("1.0+build"sv));
    fail(SemanticVersion::convert("1.0.0a"sv));
    fail(SemanticVersion::convert("1.0.11111111111111111111111111111111111111111111111111"sv));

    expect(SemanticVersion::convert("1.2.3"sv), 1, 2, 3, ""sv, ""sv);
    expect(SemanticVersion::convert("1.2.3+alpha"sv), 1, 2, 3, ""sv, "alpha"sv);
    expect(SemanticVersion::convert("1.2.3-alpha"sv), 1, 2, 3, "alpha"sv, ""sv);
    expect(SemanticVersion::convert("1.2.3+alpha.beta"sv), 1, 2, 3, ""sv, "alpha.beta"sv);
    expect(SemanticVersion::convert("1.2.3+11111111111111111111111111111111111111111111111111"sv), 1, 2, 3, ""sv, "11111111111111111111111111111111111111111111111111"sv);
    expect(SemanticVersion::convert("1.2.3-11111111111111111111111111111111111111111111111111"sv), 1, 2, 3, "11111111111111111111111111111111111111111111111111"sv, ""sv);
    expect(SemanticVersion::convert("1.2.3+beta-alpha"sv), 1, 2, 3, "alpha"sv, "beta"sv);
    expect(SemanticVersion::convert("1.2.3-beta+alpha"sv), 1, 2, 3, "beta"sv, "alpha"sv);

    output(__FILE__ "::" __FUNCTION__);
  }


  //-------------------------------------------------------------------------
  void test2() noexcept(false)
  {
    TEST(SemanticVersion{ "1.0.0" } == SemanticVersion{ "1.0.0" });
    TEST(SemanticVersion{ "1.0.0" } >= SemanticVersion{ "1.0.0" });
    TEST(SemanticVersion{ "1.0.1" } >= SemanticVersion{ "1.0.0" });
    TEST(SemanticVersion{ "1.0.0" } <= SemanticVersion{ "1.0.0" });
    TEST(SemanticVersion{ "0.9.9999" } <= SemanticVersion{ "1.0.0" });
    TEST(SemanticVersion{ "1.0.0" } > SemanticVersion{ "0.1.1" });
    TEST(SemanticVersion{ "1.0.0" } > SemanticVersion{ "0.0.1" });
    TEST(SemanticVersion{ "1.0.0" } > SemanticVersion{ "0.1.0" });
    TEST(SemanticVersion{ "1.0.0" } < SemanticVersion{ "1.1.1" });
    TEST(SemanticVersion{ "1.0.0" } < SemanticVersion{ "1.0.1" });
    TEST(SemanticVersion{ "1.0.0" } < SemanticVersion{ "1.1.0" });
    TEST(SemanticVersion{ "0.0.0" } < SemanticVersion{ "0.1.1" });
    TEST(SemanticVersion{ "0.0.0" } < SemanticVersion{ "0.0.1" });
    TEST(SemanticVersion{ "0.0.0" } < SemanticVersion{ "1.0.0" });
    TEST(std::strong_ordering::equal == SemanticVersion{ "1.0.0" }.fullCompare(SemanticVersion{ "1.0.0" }));
    TEST(std::strong_ordering::less == SemanticVersion{ "1.0.0-alpha" }.fullCompare(SemanticVersion{ "1.0.0-blpha" }));
    TEST(std::strong_ordering::greater == SemanticVersion{ "1.0.0-blpha" }.fullCompare(SemanticVersion{ "1.0.0-alpha" }));
    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test(); });
    runner([&]() { test2(); });

    reset();
  }
};

//---------------------------------------------------------------------------
void testHelpers() noexcept(false)
{
  HelperBasics{}.runAll();
  HelperSemanticVersion{}.runAll();
}

} // namespace zaxTest
