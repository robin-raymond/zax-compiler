
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
  TokenList alphabetList_;

  std::vector<std::string_view> compareVector_{ "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
  std::vector<std::string_view> new_{ "new" };

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
    auto checkEmpty{ [&](auto&& tokens) noexcept(false) {
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

    auto makeVector{ [&](size_t firstLet, size_t lastLet) noexcept(false) -> auto {
      assert(firstLet <= lastLet);
      auto temp{ compareVector_ };
      if (0 != firstLet) {
        temp.erase(begin(temp), begin(temp) + firstLet);
        lastLet -= firstLet;
      }
      if (lastLet <= temp.size()) {
        temp.erase(begin(temp) + lastLet, end(temp));
      }
      return temp;
    } };

    auto checkList{ [&] (auto&& list, auto&& vector) noexcept(false) {
      if (vector.size() < 1) {
        checkEmpty(list);
        return;
      }
      TEST(!list.empty());
      TEST(zax::hasAhead(std::begin(list), vector.size() - 1 ));
      TEST(list.hasAhead(std::begin(list), vector.size() - 1));
      TEST(zax::hasBehind(std::end(list), vector.size() - 1));
      TEST(list.hasBehind(std::end(list), vector.size() - 1));
      TEST(!zax::hasAhead(std::begin(list), vector.size()));
      TEST(!list.hasAhead(std::begin(list), vector.size()));
      TEST(zax::hasBehind(std::end(list), vector.size()));
      TEST(!zax::hasBehind(std::end(list), vector.size()) + 1);
      TEST(list.hasBehind(std::end(list), vector.size()));
      TEST(!list.hasBehind(std::end(list), vector.size()) + 1);
      TEST(list.size() == vector.size());
      size_t count = 0;
      for (auto value : list) {
        TEST(value->token_ == vector[count]);
        ++count;
      }
      TEST(count == vector.size());
    } };

    {
      reset();
      checkEmpty(list_);
    }

    {
      for (auto value : compareVector_) {
        alphabetList_.pushBack(makeToken(value));
      }
    }

    //-------------------------------------------------------------------------
    auto checkExtract{ [&]() noexcept(false) {
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::begin(list_), std::end(list_)) };
        checkList(extracted, makeVector(0, 26));
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::begin(list_), --std::end(list_)) };
        checkList(extracted, makeVector(0, 25));
        checkList(list_, makeVector(25, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(++std::begin(list_), std::end(list_)) };
        checkList(extracted, makeVector(1, 26));
        checkList(list_, makeVector(0, 1));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(++std::begin(list_), --std::end(list_)) };
        checkList(extracted, makeVector(1, 25));
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::begin(list_), std::begin(list_)) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::end(list_), std::end(list_)) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

    } };

    //-------------------------------------------------------------------------
    auto checkExtractCount{ [&]() noexcept(false) {
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::begin(list_), 26) };
        checkList(extracted, makeVector(0, 26));
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::begin(list_), 25) };
        checkList(extracted, makeVector(0, 25));
        checkList(list_, makeVector(25, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(++std::begin(list_), 25) };
        checkList(extracted, makeVector(1, 26));
        checkList(list_, makeVector(0, 1));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(++std::begin(list_), 24) };
        checkList(extracted, makeVector(1, 25));
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::begin(list_), 0) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extract(std::end(list_), 0) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

    } };

    //-------------------------------------------------------------------------
    auto checkExtractFromStartToPos{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromStartToPos(std::end(list_)) };
        checkList(extracted, makeVector(0, 26));
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromStartToPos(--std::end(list_)) };
        checkList(extracted, makeVector(0, 25));
        checkList(list_, makeVector(25, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromStartToPos(std::begin(list_)) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromStartToPos(++std::begin(list_)) };
        checkList(extracted, makeVector(0, 1));
        checkList(list_, makeVector(1, 26));
      }

    } };

    //-------------------------------------------------------------------------
    auto checkExtractFromStartToPosCount{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromStartToPos(26) };
        checkList(extracted, makeVector(0, 26));
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromStartToPos(25) };
        checkList(extracted, makeVector(0, 25));
        checkList(list_, makeVector(25, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromStartToPos(0) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromStartToPos(1) };
        checkList(extracted, makeVector(0, 1));
        checkList(list_, makeVector(1, 26));
      }

    } };

    //-------------------------------------------------------------------------
    auto checkExtractFromPosToEnd{ [&]() noexcept(false) {
      
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromPosToEnd(std::begin(list_)) };
        checkList(extracted, makeVector(0, 26));
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromPosToEnd(++std::begin(list_)) };
        checkList(extracted, makeVector(1, 26));
        checkList(list_, makeVector(0, 1));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromPosToEnd(std::end(list_)) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ zax::extractFromPosToEnd(--std::end(list_)) };
        checkList(extracted, makeVector(25, 26));
        checkList(list_, makeVector(0, 25));
      }
    } };


    //-------------------------------------------------------------------------
    auto checkExtractFromPosToEndCount{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromPosToEnd(0) };
        checkList(extracted, makeVector(0, 26));
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromPosToEnd(1) };
        checkList(extracted, makeVector(1, 26));
        checkList(list_, makeVector(0, 1));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromPosToEnd(26) };
        checkEmpty(extracted);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto extracted{ list_.extractFromPosToEnd(25) };
        checkList(extracted, makeVector(25, 26));
        checkList(list_, makeVector(0, 25));
      }
    } };

    //-------------------------------------------------------------------------
    auto checkErase{ [&]() noexcept(false) {
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::begin(list_));
        checkList(list_, makeVector(1, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::end(list_));
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(++std::begin(list_));
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(2, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--std::end(list_));
        checkList(list_, makeVector(0, 25));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--(--std::end(list_)));
        auto vec1{ makeVector(0, 24) };
        auto vec2{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }
    } };

    //-------------------------------------------------------------------------
    auto checkEraseCount{ [&]() noexcept(false) {
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        list_.erase(0);
        checkList(list_, makeVector(1, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        list_.erase(26);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        list_.erase(1);
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(2, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        list_.erase(25);
        checkList(list_, makeVector(0, 25));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        list_.erase(24);
        auto vec1{ makeVector(0, 24) };
        auto vec2{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }
    } };

    //-------------------------------------------------------------------------
    auto checkEraseFirstLast{ [&]() noexcept(false) {
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::begin(list_), std::begin(list_));
        checkList(list_, makeVector(0, 26));
      }
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::begin(list_), ++std::begin(list_));
        checkList(list_, makeVector(1, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::end(list_), std::end(list_));
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(++std::begin(list_), ++(++std::begin(list_)));
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(2, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(++std::begin(list_), ++std::begin(list_));
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--std::end(list_), std::end(list_));
        checkList(list_, makeVector(0, 25));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--std::begin(list_), --std::begin(list_));
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--(--std::end(list_)), --std::end(list_));
        auto vec1{ makeVector(0, 24) };
        auto vec2{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--(--std::end(list_)), --(--std::end(list_)));
        checkList(list_, makeVector(0, 26));
      }

    } };

    //-------------------------------------------------------------------------
    auto checkErasePosCount{ [&]() noexcept(false) {
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::begin(list_), 0);
        checkList(list_, makeVector(0, 26));
      }
      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::begin(list_), 1);
        checkList(list_, makeVector(1, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(std::end(list_), 0);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(++std::begin(list_), 1);
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(2, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(++std::begin(list_), 0);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--std::end(list_), 1);
        checkList(list_, makeVector(0, 25));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--std::begin(list_), 0);
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--(--std::end(list_)), 1);
        auto vec1{ makeVector(0, 24) };
        auto vec2{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::erase(--(--std::end(list_)), 0);
        checkList(list_, makeVector(0, 26));
      }

    } };

    //-------------------------------------------------------------------------
    auto checkInsertBefore{ [&]() noexcept(false) {

      auto newToken{ makeToken("new") };

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertBefore(std::begin(list_), newToken);
        auto vec1{ new_ };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertBefore(std::end(list_), newToken);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ new_ };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertBefore(--std::end(list_), newToken);
        auto vec1{ makeVector(0, 25) };
        auto vec2{ new_ };
        auto vec3{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertBefore(++std::begin(list_), newToken);
        auto vec1{ makeVector(0, 1) };
        auto vec2{ new_ };
        auto vec3{ makeVector(1, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

    } };

    //-------------------------------------------------------------------------
    auto checkInsert{ [&]() noexcept(false) {

      auto newToken{ makeToken("new") };

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insert(std::begin(list_), newToken);
        auto vec1{ new_ };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insert(std::end(list_), newToken);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ new_ };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insert(--std::end(list_), newToken);
        auto vec1{ makeVector(0, 25) };
        auto vec2{ new_ };
        auto vec3{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insert(++std::begin(list_), newToken);
        auto vec1{ makeVector(0, 1) };
        auto vec2{ new_ };
        auto vec3{ makeVector(1, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

    } };

    //-------------------------------------------------------------------------
    auto checkInsertAfter{ [&]() noexcept(false) {

      auto newToken{ makeToken("new") };

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertAfter(std::end(list_), newToken);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ new_ };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertAfter(std::begin(list_), newToken);
        auto vec1{ makeVector(0, 1) };
        auto vec2{ new_ };
        auto vec3{ makeVector(1, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertAfter(--std::end(list_), newToken);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ new_ };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        zax::insertAfter(++std::begin(list_), newToken);
        auto vec1{ makeVector(0, 2) };
        auto vec2{ new_ };
        auto vec3{ makeVector(2, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

    } };

    //-------------------------------------------------------------------------
    auto checkInsertCopyBefore{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyBefore(std::begin(list_), alphaCopy);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyBefore(++std::begin(list_), alphaCopy);
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(0, 26) };
        auto vec3{ makeVector(1, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyBefore(std::end(list_), alphaCopy);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyBefore(--std::end(list_), alphaCopy);
        auto vec1{ makeVector(0, 25) };
        auto vec2{ makeVector(0, 26) };
        auto vec3{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }
    } };

    //-------------------------------------------------------------------------
    auto checkInsertCopyAfter{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyAfter(std::begin(list_), alphaCopy);
        auto vec1{ makeVector(0, 1) };
        auto vec2{ makeVector(0, 26) };
        auto vec3{ makeVector(1, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyAfter(++std::begin(list_), alphaCopy);
        auto vec1{ makeVector(0, 2) };
        auto vec2{ makeVector(0, 26) };
        auto vec3{ makeVector(2, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyAfter(std::end(list_), alphaCopy);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyAfter(--std::end(list_), alphaCopy);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(0, 26));
        zax::insertCopyAfter(--(--std::end(list_)), alphaCopy);
        auto vec1{ makeVector(0, 25) };
        auto vec2{ makeVector(0, 26) };
        auto vec3{ makeVector(25, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        vec1.insert(vec1.end(), vec3.begin(), vec3.end());
        checkList(list_, vec1);
      }
    } };

    //-------------------------------------------------------------------------
    auto checkExtractThenPush{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        list_.erase(std::begin(list_));
        list_.erase(--std::end(list_));

        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(1, 25));
        checkList(alphaCopy, makeVector(0, 26));
        list_.extractThenPushFront(alphaCopy);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ makeVector(1, 25) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
        checkEmpty(alphaCopy);
      }

      {
        reset();
        list_ = alphabetList_;
        list_.erase(std::begin(list_));
        list_.erase(--std::end(list_));

        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(1, 25));
        checkList(alphaCopy, makeVector(0, 26));
        list_.extractThenPushBack(alphaCopy);
        auto vec1{ makeVector(1, 25) };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
        checkEmpty(alphaCopy);
      }

    } };

    //-------------------------------------------------------------------------
    auto checkExtractThenPushMove{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        list_.erase(std::begin(list_));
        list_.erase(--std::end(list_));

        checkList(list_, makeVector(1, 25));
        list_.extractThenPushFront(TokenList{ alphabetList_ });
        auto vec1{ makeVector(0, 26) };
        auto vec2{ makeVector(1, 25) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        list_.erase(std::begin(list_));
        list_.erase(--std::end(list_));

        checkList(list_, makeVector(1, 25));
        list_.extractThenPushBack(TokenList{ alphabetList_ });
        auto vec1{ makeVector(1, 25) };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

    } };

    //-------------------------------------------------------------------------
    auto checkCopyThenPush{ [&]() noexcept(false) {

      {
        reset();
        list_ = alphabetList_;
        list_.erase(std::begin(list_));
        list_.erase(--std::end(list_));

        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(1, 25));
        checkList(alphaCopy, makeVector(0, 26));
        list_.copyPushFront(alphaCopy);
        auto vec1{ makeVector(0, 26) };
        auto vec2{ makeVector(1, 25) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
        checkList(alphaCopy, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        list_.erase(std::begin(list_));
        list_.erase(--std::end(list_));

        auto alphaCopy{ alphabetList_ };
        checkList(list_, makeVector(1, 25));
        checkList(alphaCopy, makeVector(0, 26));
        list_.copyPushBack(alphaCopy);
        auto vec1{ makeVector(1, 25) };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
        checkList(alphaCopy, makeVector(0, 26));
      }

    } };

    //-------------------------------------------------------------------------
    auto checkPushFront{ [&]() noexcept(false) {

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        auto tmp{ makeToken("foo") };
        list_.pushFront(tmp);
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ makeToken("foo") };
        list_.pushFront(tmp);
        std::vector<std::string_view> vec1{ "foo" };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        auto vec2{ makeVector(0, 26) };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

    } };

    //-------------------------------------------------------------------------
    auto checkPushBack{ [&]() noexcept(false) {

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        auto tmp{ makeToken("foo") };
        list_.pushBack(tmp);
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushBack(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ makeToken("foo") };
        list_.pushBack(tmp);
        auto vec1{ makeVector(0, 26) };
        std::vector<std::string_view> vec2{ "foo" };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        list_.pushBack(makeToken("foo"));
        auto vec1{ makeVector(0, 26) };
        std::vector<std::string_view> vec2{ "foo" };
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
        checkList(list_, vec1);
      }

    } };

    //-------------------------------------------------------------------------
    auto checkPopFront{ [&]() noexcept(false) {

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        auto tmp{ list_.popFront() };
        TEST(!tmp);
        checkEmpty(list_);
      }

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
        auto tmp{ list_.popFront() };
        TEST(tmp->token_ == "foo");
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ list_.popFront() };
        TEST(tmp->token_ == "A");
        checkList(list_, makeVector(1, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp1{ list_.popFront() };
        TEST(tmp1->token_ == "A");
        auto tmp2{ list_.popFront() };
        TEST(tmp2->token_ == "B");
        checkList(list_, makeVector(2, 26));
      }
    } };

    //-------------------------------------------------------------------------
    auto checkPopBack{ [&]() noexcept(false) {

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        auto tmp{ list_.popBack() };
        TEST(!tmp);
        checkEmpty(list_);
      }

      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
        auto tmp{ list_.popBack() };
        TEST(tmp->token_ == "foo");
        checkEmpty(list_);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ list_.popBack() };
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 25));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp1{ list_.popBack() };
        TEST(tmp1->token_ == "Z");
        auto tmp2{ list_.popBack() };
        TEST(tmp2->token_ == "Y");
        checkList(list_, makeVector(0, 24));
      }
    } };

    //-------------------------------------------------------------------------
    auto checkOperatorIndex{ [&]() noexcept(false) {
      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
        auto tmp{ list_[0] };
        TEST(tmp->token_ == "foo");
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ list_[0] };
        TEST(tmp->token_ == "A");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ list_[25] };
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ list_[26] };  // getting end() is safe
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp1{ list_[25] };
        TEST(tmp1->token_ == "Z");
        auto tmp2{ list_[24] };
        TEST(tmp2->token_ == "Y");
        checkList(list_, makeVector(0, 26));
      }
    } };
    
    //-------------------------------------------------------------------------
    auto checkAt{ [&]() noexcept(false) {
      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
        auto tmp{ *list_.at(0) };
        TEST(tmp->token_ == "foo");
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ *list_.at(0) };
        TEST(tmp->token_ == "A");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ *list_.at(25) };
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp{ *list_.at(26) };  // getting end() is safe
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        auto tmp1{ *list_.at(25) };
        TEST(tmp1->token_ == "Z");
        auto tmp2{ *list_.at(24) };
        TEST(tmp2->token_ == "Y");
        checkList(list_, makeVector(0, 26));
      }
    } };

    //-------------------------------------------------------------------------
    auto checkOperatorIndexConst{ [&]() noexcept(false) {
      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
        const auto& clist{ list_ };
        auto tmp{ clist[0] };
        TEST(tmp->token_ == "foo");
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp{ clist[0] };
        TEST(tmp->token_ == "A");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp{ clist[25] };
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp{ clist[26] };  // getting end() is safe
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp1{ clist[25] };
        TEST(tmp1->token_ == "Z");
        auto tmp2{ clist[24] };
        TEST(tmp2->token_ == "Y");
        checkList(list_, makeVector(0, 26));
      }
    } };

    //-------------------------------------------------------------------------
    auto checkAtConst{ [&]() noexcept(false) {
      {
        reset();
        list_ = {};
        checkEmpty(list_);
        list_.pushFront(makeToken("foo"));
        std::vector<std::string_view> vec1{ "foo" };
        checkList(list_, vec1);
        const auto& clist{ list_ };
        auto tmp{ *clist.at(0) };
        TEST(tmp->token_ == "foo");
        checkList(list_, vec1);
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp{ *clist.at(0) };
        TEST(tmp->token_ == "A");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp{ *clist.at(25) };
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp{ *clist.at(26) };  // getting end() is safe
        TEST(tmp->token_ == "Z");
        checkList(list_, makeVector(0, 26));
      }

      {
        reset();
        list_ = alphabetList_;
        checkList(list_, makeVector(0, 26));
        const auto& clist{ list_ };
        auto tmp1{ *clist.at(25) };
        TEST(tmp1->token_ == "Z");
        auto tmp2{ *clist.at(24) };
        TEST(tmp2->token_ == "Y");
        checkList(list_, makeVector(0, 26));
      }
    } };

    //-------------------------------------------------------------------------
    auto checkFor{ [&]() noexcept(false) {
      
      {
        reset();
        list_ = alphabetList_;
        size_t count{};
        for (auto value : list_) {
          TEST(!value->token_.empty());
          ++count;
        }
        TEST(count == 26);
      }

      {
        reset();
        list_ = alphabetList_;
        const auto& clist{ list_ };

        size_t count{};
        for (auto value : clist) {
          TEST(!value->token_.empty());
          ++count;
        }
        TEST(count == 26);
      }

      {
        reset();
        list_ = alphabetList_;
        size_t count{};
        for (auto iter{ std::begin(list_) }; iter != std::end(list_); ++iter) {
          TEST(!(*iter)->token_.empty());
          ++count;
        }
        TEST(count == 26);
      }

      {
        reset();
        list_ = alphabetList_;
        size_t count{};
        for (auto iter{ std::cbegin(list_) }; iter != std::cend(list_); ++iter) {
          TEST(!(*iter)->token_.empty());
          ++count;
        }
        TEST(count == 26);
      }

      {
        reset();
        list_ = alphabetList_;
        const auto& clist{ list_ };

        size_t count{};
        for (auto iter{ std::begin(clist) }; iter != std::end(clist); ++iter) {
          TEST(!(*iter)->token_.empty());
          ++count;
        }
        TEST(count == 26);
      }

      {
        reset();
        list_ = alphabetList_;
        const auto& clist{ list_ };

        size_t count{};
        for (auto iter{ std::cbegin(clist) }; iter != std::cend(clist); ++iter) {
          TEST(!(*iter)->token_.empty());
          ++count;
        }
        TEST(count == 26);
      }

    } };


    checkExtract();
    checkExtractCount();
    checkExtractFromStartToPos();
    checkExtractFromStartToPosCount();
    checkExtractFromPosToEnd();
    checkExtractFromPosToEndCount();

    checkErase();
    checkEraseCount();
    checkEraseFirstLast();
    checkErasePosCount();

    checkInsertBefore();
    checkInsert();
    checkInsertAfter();

    checkInsertCopyBefore();
    checkInsertCopyAfter();

    checkExtractThenPush();
    checkExtractThenPushMove();

    checkCopyThenPush();

    checkPushFront();
    checkPushBack();

    checkPopFront();
    checkPopBack();

    checkOperatorIndex();
    checkAt();
    checkOperatorIndexConst();
    checkAtConst();

    checkFor();

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
