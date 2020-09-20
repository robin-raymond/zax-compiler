
#include <pch.h>

#include <filesystem>

#include "common.h"

#include "../src/helpers.h"


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
    const std::string_view example{ "ignored/testing/a/b/example.txt" };
    const std::string_view expecting{ "ignored/testing/a/b/testing_1.txt" };
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
      auto located2a{ zax::locateFile(example, "testing/2.txt", fullLocated2a) };
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
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test(); });

    reset();
  }
};

//---------------------------------------------------------------------------
void testHelpers() noexcept(false)
{
  HelperBasics{}.runAll();
}

} // namespace zaxTest
