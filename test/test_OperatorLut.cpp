
#include <pch.h>

#include "common.h"

#include "../src/OperatorLut.h"


#if 0
using namespace std::string_view_literals;
#endif //0

namespace zaxTest
{

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct OperatorLutBasics
{
  //-------------------------------------------------------------------------
  void reset() noexcept
  {
  }

  //-------------------------------------------------------------------------
  void test() noexcept(false)
  {
    std::atomic_int value{};

    TEST(0 == 0);

    zax::OperatorLut lut;

    auto expectNoResult{ [&](std::string_view str) noexcept(false) {
      auto result{ lut.lookup(str) };
      TEST(!result.has_value());
    } };

    auto expectSingleResult{ [&](std::string_view str, zax::TokenTypes::Operator expecting) noexcept(false) {
      auto result{ lut.lookup(str) };
      TEST(result.has_value());
      TEST(*result == expecting);
      TEST(!lut.hasConflicts(*result));
      auto& conflicts{ lut.lookupConflicts(*result) };
      TEST(conflicts.empty());
      auto prefix{ lut.lookup(expecting) };
      TEST(prefix.length() > 0);
      TEST(prefix == str.substr(0, prefix.length()));
    } };

    auto expectMultipleResults{ [&](std::string_view str, const std::set<zax::TokenTypes::Operator>& expecting) noexcept(false) {
      auto result{ lut.lookup(str) };
      TEST(result.has_value());
      TEST(end(expecting) != expecting.find(*result));
      TEST(lut.hasConflicts(*result));

      for (auto oper : expecting) {
        auto all{ expecting };
        auto& conflicts{ lut.lookupConflicts(oper) };
        TEST(conflicts.size() == expecting.size());
        size_t count{};

        auto prefix{ lut.lookup(oper) };
        TEST(prefix.length() > 0);
        TEST(prefix == str.substr(0, prefix.length()));

        while (all.size() > 0) {
          auto found = conflicts.find(*begin(all));
          TEST(end(conflicts) != found);
          all.erase(begin(all));

          ++count;
          TEST(count <= expecting.size());
          if (count > expecting.size())
            break;
        }
      }
    } };

    auto expectEatTokens{ [&](std::string_view str, std::string_view remaining, const std::vector<std::string_view>& expecting) noexcept(false) {
      auto all{ expecting };
      auto parseStr{ str };
      while (parseStr.length() > 0) {
        auto result{ lut.lookup(parseStr) };
        if (!result)
          break;

        TEST(all.size() > 0);

        auto prefix{ lut.lookup(*result) };
        TEST(prefix.length() > 0);

        TEST(prefix == all[0]);
        all.erase(begin(all));

        parseStr = parseStr.substr(prefix.length());
      }
      TEST(remaining == parseStr);
      TEST(all.size() == 0);
    } };

    expectSingleResult("+= hello", zax::TokenTypes::Operator::AddAssign);
    expectSingleResult("+=+ hello", zax::TokenTypes::Operator::AddAssign);
    expectSingleResult("+++!hello", zax::TokenTypes::Operator::Constructor);
    expectSingleResult("---+hello", zax::TokenTypes::Operator::Destructor);
    expectMultipleResults("+/hello", std::set<zax::TokenTypes::Operator>{
      zax::TokenTypes::Operator::PlusPreUnary,
      zax::TokenTypes::Operator::PlusBinary
    });
    expectMultipleResults("-/hello", std::set<zax::TokenTypes::Operator>{
      zax::TokenTypes::Operator::MinusPreUnary,
      zax::TokenTypes::Operator::MinusBinary
    });
    expectMultipleResults("++/hello", std::set<zax::TokenTypes::Operator>{
      zax::TokenTypes::Operator::PlusPlusPreUnary,
      zax::TokenTypes::Operator::PlusPlusPostUnary
    });
    expectMultipleResults("--/hello", std::set<zax::TokenTypes::Operator>{
      zax::TokenTypes::Operator::MinusMinusPreUnary,
      zax::TokenTypes::Operator::MinusMinusPostUnary
    });
    expectNoResult("`hello`");

    expectEatTokens("+++++.+", "", std::vector<std::string_view>{
      "+++",
      "++",
      ".",
      "+"
    });

    expectEatTokens("[[]]]hello", "hello", std::vector<std::string_view>{
      "[[",
      "]]",
      "]"
    });

    expectEatTokens("//*", "", std::vector<std::string_view>{
      "/",
      "/",
      "*"
    });

    expectEatTokens("{{{]~|=fine", "fine", std::vector<std::string_view>{
      "{{",
      "{",
      "]",
      "~|="
    });

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { test(); });
  }
};

//---------------------------------------------------------------------------
void testOperatorLut() noexcept(false)
{
  OperatorLutBasics{}.runAll();
}

} // namespace zaxTest
