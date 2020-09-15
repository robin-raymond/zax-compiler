
#include <pch.h>

#include "common.h"

#include "../src/TokenList.h"
#include "../src/Token.h"

using TokenListTypes = zax::TokenListTypes;
using TokenList = zax::TokenList;
using Token = zax::Token;
using TokenPtr = zax::TokenPtr;

namespace zaxTest
{

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct TokenListBasics
{
  TokenList list_;
  TokenList oldList_;

  //-------------------------------------------------------------------------
  void reset() noexcept
  {
    list_ = {};
  }

  //-------------------------------------------------------------------------
  TokenPtr makeToken(std::string_view str) noexcept
  {
    auto result{ std::make_shared<Token>() };
    result->token_ = str;
    return result;
  }

  //-------------------------------------------------------------------------
  void testBasic() noexcept(false)
  {
    auto checkEmpty{ [&](auto& tokens) noexcept(false) {
      TEST(tokens.empty());
      TEST(tokens.size() == 0);
      {
        auto temp1{ zax::extract(tokens.begin(), tokens.end()) };
        TEST(temp1.empty());
        auto temp2{ zax::extract(tokens.begin(), 500) };
        TEST(temp2.empty());
        auto temp3{ zax::extractFromStartToPos(tokens.begin()) };
        TEST(temp3.empty());
        auto temp4{ zax::extractFromStartToPos(tokens.end()) };
        TEST(temp4.empty());
        auto temp5{ zax::extractFromPosToEnd(tokens.begin()) };
        TEST(temp5.empty());
        auto temp6{ zax::extractFromPosToEnd(tokens.end()) };
        TEST(temp6.empty());
        zax::erase(tokens.begin());
        zax::erase(tokens.end());
      }
      {
        auto temp1{ tokens.extract(tokens.begin(), tokens.end()) };
        TEST(temp1.empty());
        auto temp2{ tokens.extract(tokens.begin(), 500) };
        TEST(temp2.empty());
        auto temp3{ tokens.extractFromStartToPos(tokens.begin()) };
        TEST(temp3.empty());
        auto temp4{ tokens.extractFromStartToPos(tokens.end()) };
        TEST(temp4.empty());
        auto temp5{ tokens.extractFromPosToEnd(tokens.begin()) };
        TEST(temp5.empty());
        auto temp6{ tokens.extractFromPosToEnd(tokens.end()) };
        TEST(temp6.empty());
        tokens.erase(tokens.begin());
        tokens.erase(tokens.end());
      }
      TEST(!tokens.popFront());
      TEST(!tokens.popBack());

      tokens.extractThenPushFront(tokens);
      tokens.extractThenPushBack(tokens);

      tokens.copyPushFront(tokens);
      tokens.copyPushBack(tokens);

      TEST(!tokens[0]);
      TEST(tokens.at(0) == tokens.end());

      TEST(tokens.begin() == tokens.end());
      TEST(tokens.cbegin() == tokens.cend());

      TEST(tokens.empty());
      TEST(tokens.size() == 0);
    } };

    {
      reset();
      checkEmpty(list_);
    }

    {
      reset();
      list_.pushBack(makeToken("value"));
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { testBasic(); });
  }
};

//---------------------------------------------------------------------------
void testTokenList() noexcept(false)
{
  TokenListBasics{}.runAll();
}

} // namespace zaxTest
