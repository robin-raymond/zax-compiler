
#include <pch.h>

#include "common.h"

#include "../src/Tokenizer.h"
#include "../src/OperatorLut.h"

using TokenizerTypes = zax::TokenizerTypes;
using Tokenizer = zax::Tokenizer;

namespace zaxTest
{

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct TokenizerBasics
{
  TokenizerTypes::ParserPos pos_;
  TokenizerTypes::ParserPos oldPos_;

  //-------------------------------------------------------------------------
  void reset() noexcept
  {
    pos_ = {};
    oldPos_ = {};
  }

  //-------------------------------------------------------------------------
  void expect(int line, int column) noexcept(false)
  {
    TEST(pos_.line_ == line);
    TEST(pos_.column_ == column);
  }

  //-------------------------------------------------------------------------
  void expect(int line, int column, int overrideLine) noexcept(false)
  {
    TEST(pos_.line_ == line);
    TEST(pos_.column_ == column);
  }

  //-------------------------------------------------------------------------
  void testUtf8Bom() noexcept(false)
  {
    {
      reset();
      auto result = Tokenizer::consumeUtf8Bom(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "hello // nothing to see here";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeUtf8Bom(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xF0\x90\x8D\x88";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeUtf8Bom(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xef\xbb\xbf";
      auto result = Tokenizer::consumeUtf8Bom(pos_);
      TEST(result);
      expect(1, 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "\xef\xbb\xbfhello";
      auto result = Tokenizer::consumeUtf8Bom(pos_);
      TEST(result);
      expect(1, 1);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\xef\xbbhello";
      auto result = Tokenizer::consumeUtf8Bom(pos_);
      TEST(!result);
      expect(1, 1);
      TEST(pos_.pos_ == "\xef\xbbhello");
    }

    {
      reset();
      pos_.pos_ = "\xef\xbf\xbbhello";
      auto result = Tokenizer::consumeUtf8Bom(pos_);
      TEST(!result);
      expect(1, 1);
      TEST(pos_.pos_ == "\xef\xbf\xbbhello");
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testSingleLineComments() noexcept(false)
  {
    {
      reset();
      auto result = Tokenizer::consumeComment(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xF0\x90\x8D\x88";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeComment(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "hello // nothing to see here";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeComment(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "// something to see here";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "// something to see here");
      TEST(result->token_ == " something to see here");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 44 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "// something to see here\r\nhello";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "// something to see here");
      TEST(result->token_ == " something to see here");
      expect(1, 44 - 20 + 1);
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      TEST(pos_.pos_ == "\r\nhello");
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testCStyleComments() noexcept(false)
  {
    {
      reset();
      pos_.pos_ = "hello /* nothing to see here */";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeComment(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "/* something to see here";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/* something to see here");
      TEST(result->token_ == " something to see here");
      TEST(!result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 44 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/* something to see here\r\nhello";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/* something to see here\r\nhello");
      TEST(result->token_ == " something to see here\r\nhello");
      TEST(!result->foundEnding_);
      TEST(result->addNewLine_);
      expect(2, 62 - 57 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/* something to see here\rhello";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/* something to see here\rhello");
      TEST(result->token_ == " something to see here\rhello");
      TEST(!result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 62 - 57 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/* something to see here */";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/* something to see here */");
      TEST(result->token_ == " something to see here ");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 66 - 39 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/* something to see here */afterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/* something to see here */");
      TEST(result->token_ == " something to see here ");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 66 - 39 + 1);
      TEST(pos_.pos_ == "afterjunk");
    }

    {
      reset();
      pos_.pos_ = "/* something to \vsee here */afterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/* something to \vsee here */");
      TEST(result->token_ == " something to \vsee here ");
      TEST(result->foundEnding_);
      TEST(result->addNewLine_);
      expect(2, 66 - 39 + 1);
      TEST(pos_.pos_ == "afterjunk");
    }

    {
      reset();
      pos_.pos_ = "/* something to see here */\nafterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/* something to see here */");
      TEST(result->token_ == " something to see here ");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 66 - 39 + 1);
      TEST(pos_.pos_ == "\nafterjunk");
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testNestedCStyleComments() noexcept(false)
  {
    {
      reset();
      pos_.pos_ = "hello /** nothing to see here **/";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeComment(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "/** something to see here";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see here");
      TEST(result->token_ == " something to see here");
      TEST(!result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 45 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something to see here */";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see here */");
      TEST(result->token_ == " something to see here */");
      TEST(!result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 48 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something to see here\r\nhello";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see here\r\nhello");
      TEST(result->token_ == " something to see here\r\nhello");
      TEST(!result->foundEnding_);
      TEST(result->addNewLine_);
      expect(2, 62 - 57 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something to see */here\r\nhello";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see */here\r\nhello");
      TEST(result->token_ == " something to see */here\r\nhello");
      TEST(!result->foundEnding_);
      TEST(result->addNewLine_);
      expect(2, 62 - 57 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something to see here\rhello";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see here\rhello");
      TEST(result->token_ == " something to see here\rhello");
      TEST(!result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 62 - 57 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something to see here **/";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see here **/");
      TEST(result->token_ == " something to see here ");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 68 - 39 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something /**to see here **/";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something /**to see here **/");
      TEST(result->token_ == " something /**to see here **/");
      TEST(!result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 71 - 39 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something to see**/ here **/";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see**/");
      TEST(result->token_ == " something to see");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 43 - 20 + 1);
      TEST(pos_.pos_ ==  " here **/");
    }

    {
      reset();
      pos_.pos_ = "/** something to see/** here **/ with something else**/";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see/** here **/ with something else**/");
      TEST(result->token_ == " something to see/** here **/ with something else");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 75 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "/** something to see/** here **/ with something else**/afterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see/** here **/ with something else**/");
      TEST(result->token_ == " something to see/** here **/ with something else");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 75 - 20 + 1);
      TEST(pos_.pos_ == "afterjunk");
    }

    {
      reset();
      pos_.pos_ = "/**\n something to see/** here **/ with something else**/afterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/**\n something to see/** here **/ with something else**/");
      TEST(result->token_ == "\n something to see/** here **/ with something else");
      TEST(result->foundEnding_);
      TEST(result->addNewLine_);
      expect(2, 72 - 20 + 1);
      TEST(pos_.pos_ == "afterjunk");
    }

    {
      reset();
      pos_.pos_ = "/** something to see here **/afterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see here **/");
      TEST(result->token_ == " something to see here ");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 68 - 39 + 1);
      TEST(pos_.pos_ == "afterjunk");
    }

    {
      reset();
      pos_.pos_ = "/** something to \vsee here **/afterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to \vsee here **/");
      TEST(result->token_ == " something to \vsee here ");
      TEST(result->foundEnding_);
      TEST(result->addNewLine_);
      expect(2, 68 - 39 + 1);
      TEST(pos_.pos_ == "afterjunk");
    }

    {
      reset();
      pos_.pos_ = "/** something to see here **/\nafterjunk";
      auto result = Tokenizer::consumeComment(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "/** something to see here **/");
      TEST(result->token_ == " something to see here ");
      TEST(result->foundEnding_);
      TEST(!result->addNewLine_);
      expect(1, 68 - 39 + 1);
      TEST(pos_.pos_ == "\nafterjunk");
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testWhitespace() noexcept(false)
  {
    {
      reset();
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xF0\x90\x8D\x88";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "hello /** nothing to see here **/";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "  hello";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "  ");
      TEST(result->token_ == "  ");
      TEST(!result->addNewLine_);
      expect(1, 3);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\thello";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\t");
      TEST(result->token_ == "\t");
      TEST(!result->addNewLine_);
      expect(1, 9);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\t\bhello";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\t\b");
      TEST(result->token_ == "\t\b");
      TEST(!result->addNewLine_);
      expect(1, 8);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "  \b\b\bhello";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "  \b\b\b");
      TEST(result->token_ == "  \b\b\b");
      TEST(!result->addNewLine_);
      expect(1, 1);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\t\vhello";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\t\v");
      TEST(result->token_ == "\v");
      TEST(result->addNewLine_);
      expect(2, 9);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\n hello";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\n ");
      TEST(result->token_ == "\n");
      TEST(result->addNewLine_);
      expect(2, 2);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\n\n\n\n\n\n\n\n ";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeWhitespace(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\n\n\n\n\n\n\n\n ");
      TEST(result->token_ == "\n");
      TEST(result->addNewLine_);
      expect(9, 2);
      TEST(pos_.pos_.empty());
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testQuotes() noexcept(false)
  {
    {
      reset();
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xF0\x90\x8D\x88";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "hello // nothing to see here";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\"something to see here";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\"something to see here");
      TEST(result->token_ == "something to see here");
      TEST(!result->foundEnding_);
      expect(1, 43 - 21 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "\'something to see here";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\'something to see here");
      TEST(result->token_ == "something to see here");
      TEST(!result->foundEnding_);
      expect(1, 43 - 21 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "\'something to see here\n\n \vhello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\'something to see here\n\n \vhello");
      TEST(result->token_ == "something to see here\n\n \vhello");
      TEST(!result->foundEnding_);
      expect(4, 7);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "\'something to see here\'";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\'something to see here\'");
      TEST(result->token_ == "something to see here");
      TEST(result->foundEnding_);
      expect(1, 44 - 21 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "\"something to see here\"";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\"something to see here\"");
      TEST(result->token_ == "something to see here");
      TEST(result->foundEnding_);
      expect(1, 44 - 21 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "\'something to see here\'hello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\'something to see here\'");
      TEST(result->token_ == "something to see here");
      TEST(result->foundEnding_);
      expect(1, 44 - 21 + 1);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\"something to see here\"hello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\"something to see here\"");
      TEST(result->token_ == "something to see here");
      TEST(result->foundEnding_);
      expect(1, 44 - 21 + 1);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\"something \'to\' see here\"hello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\"something \'to\' see here\"");
      TEST(result->token_ == "something \'to\' see here");
      TEST(result->foundEnding_);
      expect(1, 46 - 21 + 1);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\'something \"to\" see here\'hello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\'something \"to\" see here\'");
      TEST(result->token_ == "something \"to\" see here");
      TEST(result->foundEnding_);
      expect(1, 46 - 21 + 1);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\'something \"to\" see here\n\'hello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\'something \"to\" see here\n\'");
      TEST(result->token_ == "something \"to\" see here\n");
      TEST(result->foundEnding_);
      expect(2, 2);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\"\nsomething \'to\' see here\"hello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\"\nsomething \'to\' see here\"");
      TEST(result->token_ == "\nsomething \'to\' see here");
      TEST(result->foundEnding_);
      expect(2, 45 - 21 + 1);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "\"\"";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\"\"");
      TEST(result->token_ == "");
      TEST(result->foundEnding_);
      expect(1, 3);
      TEST(pos_.pos_.empty());
    }
    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testLiteral() noexcept(false)
  {
    {
      reset();
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\nhello // nothing to see here";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "something";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "something");
      expect(1, 29 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "a";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "a");
      expect(1, 21 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "1";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "something to see here";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "something");
      expect(1, 29 - 20 + 1);
      TEST(pos_.pos_ == " to see here");
    }

    {
      reset();
      pos_.pos_ = "something\n to see here";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "something");
      expect(1, 29 - 20 + 1);
      TEST(pos_.pos_ == "\n to see here");
    }

    {
      reset();
      pos_.pos_ = "_";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "_");
      expect(1, 21 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "_1";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "_1");
      expect(1, 22 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "1_";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(!result);
      expect(1, 1);
      TEST(pos_.pos_ == "1_");
    }

    {
      reset();
      pos_.pos_ = "_\xF0\x90\x8D\x88z";
      auto result = Tokenizer::consumeLiteral(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "_\xF0\x90\x8D\x88z");
      expect(1, 23 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testNumeric() noexcept(false)
  {
    {
      reset();
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xF0\x90\x8D\x88";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "hello // nothing to see here";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "5";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5");
      TEST(!result->illegalSequence_);
      expect(1, 21 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.");
      TEST(!result->illegalSequence_);
      expect(1, 22 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.0";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.0");
      TEST(!result->illegalSequence_);
      expect(1, 23 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.0e";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.0e");
      TEST(result->illegalSequence_);
      expect(1, 24 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.0e1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.0e1");
      TEST(!result->illegalSequence_);
      expect(1, 25 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.0e+1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.0e+1");
      TEST(!result->illegalSequence_);
      expect(1, 26 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.e1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.e1");
      TEST(!result->illegalSequence_);
      expect(1, 24 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.e+1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.e+1");
      TEST(!result->illegalSequence_);
      expect(1, 25 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.e-1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.e-1");
      TEST(!result->illegalSequence_);
      expect(1, 25 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "5.e+e1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5.e+");
      TEST(result->illegalSequence_);
      expect(1, 24 - 20 + 1);
      TEST(pos_.pos_ == "e1");
    }

    {
      reset();
      pos_.pos_ = "5e+.1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "5e+");
      TEST(result->illegalSequence_);
      expect(1, 23 - 20 + 1);
      TEST(pos_.pos_ == ".1");
    }

    {
      reset();
      pos_.pos_ = ".e-1";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(!result.has_value());
      expect(1, 1);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = ".";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(!result.has_value());
      expect(1, 1);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = ".2e-1";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == ".2e-1");
      TEST(!result->illegalSequence_);
      expect(1, 25 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = ".2e2";
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == ".2e2");
      TEST(!result->illegalSequence_);
      expect(1, 24 - 20 + 1);
      TEST(pos_.pos_.empty());
    }

    {
      reset();
      pos_.pos_ = "1+2";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "1");
      TEST(!result->illegalSequence_);
      expect(1, 21 - 20 + 1);
      TEST(pos_.pos_ == "+2");
    }

    {
      reset();
      pos_.pos_ = "1e+2+2";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "1e+2");
      TEST(!result->illegalSequence_);
      expect(1, 24 - 20 + 1);
      TEST(pos_.pos_ == "+2");
    }

    {
      reset();
      pos_.pos_ = "1e+2\xF0\x90\x8D\x88";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeNumeric(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "1e+2");
      TEST(!result->illegalSequence_);
      expect(1, 24 - 20 + 1);
      TEST(pos_.pos_ == "\xF0\x90\x8D\x88");
    }

    output(__FILE__ "::" __FUNCTION__);
  }


  //-------------------------------------------------------------------------
  void testOperator() noexcept(false)
  {
    zax::OperatorLut lut;

    {
      reset();
      auto result = Tokenizer::consumeOperator(lut, pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xF0\x90\x8D\x88";
      oldPos_ = pos_;
      auto result = Tokenizer::consumeOperator(lut, pos_);
      TEST(!result);
      expect(1, 1);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "+++hello";
      auto result = Tokenizer::consumeOperator(lut, pos_);
      TEST(result.has_value());
      TEST(result->token_ == "+++");
      TEST(result->operator_ == zax::Token::Operator::Constructor);
      expect(1, 4);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "++++hello";
      auto result = Tokenizer::consumeOperator(lut, pos_);
      TEST(result.has_value());
      TEST(result->token_ == "+++");
      TEST(result->operator_ == zax::Token::Operator::Constructor);
      expect(1, 4);
      TEST(pos_.pos_ == "+hello");
    }
    {
      reset();
      pos_.pos_ = "+=/hello";
      auto result = Tokenizer::consumeOperator(lut, pos_);
      TEST(result.has_value());
      TEST(result->token_ == "+=");
      TEST(result->operator_ == zax::Token::Operator::PlusAssign);
      expect(1, 3);
      TEST(pos_.pos_ == "/hello");
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void testKnownIllegalToken() noexcept(false)
  {
    {
      reset();
      auto result = Tokenizer::consumeKnownIllegalToken(pos_);
      TEST(!result);
      TEST(pos_ == oldPos_);
    }

    {
      reset();
      pos_.pos_ = "\xF0\x90\x8D\x88";
      auto result = Tokenizer::consumeKnownIllegalToken(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "\xF0");
      expect(1, 2);
      TEST(pos_.pos_ == "\x90\x8D\x88");
    }

    {
      reset();
      pos_.pos_ = "`hello";
      auto result = Tokenizer::consumeKnownIllegalToken(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "`");
      expect(1, 2);
      TEST(pos_.pos_ == "hello");
    }

    {
      reset();
      pos_.pos_ = "````hello";
      auto result = Tokenizer::consumeKnownIllegalToken(pos_);
      TEST(result.has_value());
      TEST(result->token_ == "````");
      expect(1, 5);
      TEST(pos_.pos_ == "hello");
    }

    output(__FILE__ "::" __FUNCTION__);
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { testUtf8Bom(); });
    runner([&]() { testSingleLineComments(); });
    runner([&]() { testCStyleComments(); });
    runner([&]() { testNestedCStyleComments(); });
    runner([&]() { testWhitespace(); });
    runner([&]() { testQuotes(); });
    runner([&]() { testLiteral(); });
    runner([&]() { testNumeric(); });
    runner([&]() { testOperator(); });
    runner([&]() { testKnownIllegalToken(); });
  }
};

//---------------------------------------------------------------------------
void testTokenizer() noexcept(false)
{
  TokenizerBasics{}.runAll();
}

} // namespace zaxTest
