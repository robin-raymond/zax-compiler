
#include <pch.h>

#include "common.h"

#include "../src/Tokenizer.h"
#include "../src/Source.h"
#include "../src/CompileState.h"
#include "../src/CompilerException.h"
#include "../src/OperatorLut.h"

using TokenizerTypes = zax::TokenizerTypes;
using Tokenizer = zax::Tokenizer;
using Token = zax::Token;
using TokenPtr = zax::TokenPtr;

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
    TEST(pos_.location_.line_ == line);
    TEST(pos_.location_.column_ == column);
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
      TEST(result->originalToken_ == "\'something to see here");
      TEST(result->token_ == "something to see here");
      TEST(!result->foundEnding_);
      expect(1, 43 - 20 + 1 - 1);
      TEST(pos_.pos_ == "\n\n \vhello");
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
      TEST(result->originalToken_ == "\'something \"to\" see here");
      TEST(result->token_ == "something \"to\" see here");
      TEST(!result->foundEnding_);
      expect(1, 47-20+1-3);
      TEST(pos_.pos_ == "\n\'hello");
    }

    {
      reset();
      pos_.pos_ = "\"\nsomething \'to\' see here\"hello";
      auto result = Tokenizer::consumeQuote(pos_);
      TEST(result.has_value());
      TEST(result->originalToken_ == "\"");
      TEST(result->token_ == "");
      TEST(!result->foundEnding_);
      expect(1, 2);
      TEST(pos_.pos_ == "\nsomething \'to\' see here\"hello");
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

    reset();
  }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct TokenizerInstance
{
  std::optional<Tokenizer> tokenizer_;

  zax::SourceTypes::FilePathPtr filePath_{ std::make_shared<zax::SourceTypes::FilePath>() };
  zax::CompileStatePtr compileState_{ std::make_shared<zax::CompileState>() };
  zax::OperatorLutPtr operatorLut_{ std::make_shared<zax::OperatorLut>() };

  struct ExpectedFailures
  {
    std::variant<zax::ErrorTypes::Error, zax::WarningTypes::Warning> type_{};
    int line_{};
    int column_{};
    ExpectedFailures(zax::ErrorTypes::Error error, int line, int column) noexcept(false) :
      type_(error),
      line_(line),
      column_(column)
    {}
    ExpectedFailures(zax::WarningTypes::Warning warning, int line, int column) noexcept(false) :
      type_(warning),
      line_(line),
      column_(column)
    {}
  };

  std::list<ExpectedFailures> failures_;

  //-------------------------------------------------------------------------
  void reset() noexcept
  {
    TEST(failures_.size() < 1);
    tokenizer_.reset();
  }

  //-------------------------------------------------------------------------
  void prepare(std::string_view value) noexcept(false)
  {
    reset();
    static_assert(sizeof(char) == sizeof(std::byte));
    std::pair<std::unique_ptr<std::byte[]>, size_t> content;
    content.first = std::make_unique<std::byte[]>(value.size());
    content.second = value.size();
    memcpy(content.first.get(), value.data(), sizeof(char) * value.size());
    tokenizer_.emplace(filePath_, std::move(content), compileState_, operatorLut_);

    tokenizer_->errorCallback_ = [&](zax::ErrorTypes::Error error, const zax::TokenConstPtr token, const zax::StringMap&) noexcept(false) {
      handleException(error, token);
    };
    tokenizer_->warningCallback_ = [&](zax::WarningTypes::Warning warning, const zax::TokenConstPtr token, const zax::StringMap&) noexcept(false) {
      handleException(warning, token);
    };
  }

  //-------------------------------------------------------------------------
  Tokenizer& get() noexcept(false)
  {
    TEST(tokenizer_.has_value());
    return *tokenizer_;
  }

  //-------------------------------------------------------------------------
  const Tokenizer& get_const() const noexcept(false)
  {
    TEST(tokenizer_.has_value());
    return *tokenizer_;
  }

  //-------------------------------------------------------------------------
  void validate(const zax::TokenConstPtr& token) noexcept(false)
  {
    TEST(token->origin_.filePath_ == filePath_);
    TEST(token->compileState_ == compileState_);
  }

  //-------------------------------------------------------------------------
  void validate(
    const zax::TokenConstPtr& token,
    int line,
    int column) noexcept(false)
  {
    validate(token);
    TEST(token->origin_.location_.line_ == line);
    TEST(token->origin_.location_.column_ == column);
  }

  //-------------------------------------------------------------------------
  void handleException(zax::ErrorTypes::Error error, const zax::TokenConstPtr& token) noexcept(false)
  {
    TEST(failures_.size() > 0);
    
    auto& front{ failures_.front() };
    
    auto ptr{ std::get_if<zax::ErrorTypes::Error>(&front.type_) };
    TEST(ptr);

    TEST(*ptr == error);
    TEST(front.line_ == token->origin_.location_.line_);
    TEST(front.column_ == token->origin_.location_.column_);
    failures_.pop_front();
  }

  //-------------------------------------------------------------------------
  void handleException(zax::WarningTypes::Warning warning, const zax::TokenConstPtr& token) noexcept(false)
  {
    TEST(failures_.size() > 0);

    auto& front{ failures_.front() };

    auto ptr{ std::get_if<zax::WarningTypes::Warning>(&front.type_) };
    TEST(ptr);

    TEST(*ptr == warning);
    TEST(front.line_ == token->origin_.location_.line_);
    TEST(front.column_ == token->origin_.location_.column_);
    failures_.pop_front();
  }

  //-------------------------------------------------------------------------
  void expect(zax::ErrorTypes::Error error, int line, int column) noexcept(false)
  {
    failures_.emplace_back(error, line, column);
  }

  //-------------------------------------------------------------------------
  void expect(zax::WarningTypes::Warning warning, int line, int column) noexcept(false)
  {
    failures_.emplace_back(warning, line, column);
  }

  //-------------------------------------------------------------------------
  void simple() noexcept(false)
  {
    {
      prepare("// comment");
      auto iter{ std::begin(get()) };

      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Comment);
      TEST(token->originalToken_ == "// comment");
      TEST(token->token_ == " comment");
      ++iter;
      TEST(iter == std::end(get()));
    }
    {
      prepare("/* comment");
      expect(zax::ErrorTypes::Error::MissingEndOfComment, 1, 1);

      auto iter{ std::begin(get()) };

      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Comment);
      TEST(token->originalToken_ == "/* comment");
      TEST(token->token_ == " comment");
      ++iter;
      TEST(iter == std::end(get()));
    }
    {
      prepare("/** comment");
      expect(zax::ErrorTypes::Error::MissingEndOfComment, 1, 1);

      auto iter{ std::begin(get()) };

      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Comment);
      TEST(token->originalToken_ == "/** comment");
      TEST(token->token_ == " comment");
      ++iter;
      TEST(iter == std::end(get()));
    }
    {
      prepare(
        "// comment\n"
        "abc"
      );
      auto iter{ std::begin(get()) };

      {
        auto token{ *iter };
        validate(token, 1, 1);
        TEST(token->type_ == zax::Token::Type::Comment);
        TEST(token->originalToken_ == "// comment");
        TEST(token->token_ == " comment");
      }
      {
        ++iter;
        auto token{ *iter };
        validate(token, 1, 50 - 40 + 1);
        TEST(token->type_ == zax::Token::Type::Separator);
        TEST(token->originalToken_ == "\n");
        TEST(token->token_ == "\n");
      }
      {
        ++iter;
        auto token{ *iter };
        validate(token, 2, 1);
        TEST(token->type_ == zax::Token::Type::Literal);
        TEST(token->originalToken_ == "abc");
        TEST(token->token_ == "abc");
      }
      {
        ++iter;
        TEST(iter == std::end(get()));
      }
    }
  }

  //-------------------------------------------------------------------------
  void simple2() noexcept(false)
  {
    prepare(
      "/* comment\n"
      "*/abc"
    );
    auto iter{ std::begin(get()) };

    {
      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Comment);
      TEST(token->originalToken_ == "/* comment\n*/");
      TEST(token->token_ == " comment\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Separator);
      TEST(token->originalToken_ == "/* comment\n*/");
      TEST(token->token_ == " comment\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 3);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "abc");
      TEST(token->token_ == "abc");
    }
    {
      ++iter;
      TEST(iter == std::end(get()));
    }
  }

  //-------------------------------------------------------------------------
  void simple3() noexcept(false)
  {
    prepare(
      "abc 134.4e+5+=foo\n"
      "+++"
    );
    auto iter{ std::begin(get()) };

    {
      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "abc");
      TEST(token->token_ == "abc");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 5);
      TEST(token->type_ == zax::Token::Type::Number);
      TEST(token->originalToken_ == "134.4e+5");
      TEST(token->token_ == "134.4e+5");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 20-8+1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::PlusAssign);
      TEST(token->originalToken_ == "+=");
      TEST(token->token_ == "+=");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 22-8+1);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "foo");
      TEST(token->token_ == "foo");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 25-8+1);
      TEST(token->type_ == zax::Token::Type::Separator);
      TEST(token->originalToken_ == "\n");
      TEST(token->token_ == "\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::Constructor);
      TEST(token->originalToken_ == "+++");
      TEST(token->token_ == "+++");
    }
    {
      ++iter;
      TEST(iter == std::end(get()));
    }
  }

  //-------------------------------------------------------------------------
  void simple4() noexcept(false)
  {
    prepare(
      "abc 134.4e.5+=foo\n"
      "+++'\"hello\"'"
    );
    expect(zax::ErrorTypes::Error::ConstantSyntax, 1, 5);
    auto iter{ std::begin(get()) };

    {
      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "abc");
      TEST(token->token_ == "abc");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 5);
      TEST(token->type_ == zax::Token::Type::Number);
      TEST(token->originalToken_ == "134.4e");
      TEST(token->token_ == "134.4e");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 18 - 8 + 1);
      TEST(token->type_ == zax::Token::Type::Number);
      TEST(token->originalToken_ == ".5");
      TEST(token->token_ == ".5");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 20 - 8 + 1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::PlusAssign);
      TEST(token->originalToken_ == "+=");
      TEST(token->token_ == "+=");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 22 - 8 + 1);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "foo");
      TEST(token->token_ == "foo");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 25 - 8 + 1);
      TEST(token->type_ == zax::Token::Type::Separator);
      TEST(token->originalToken_ == "\n");
      TEST(token->token_ == "\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::Constructor);
      TEST(token->originalToken_ == "+++");
      TEST(token->token_ == "+++");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 4);
      TEST(token->type_ == zax::Token::Type::Quote);
      TEST(token->originalToken_ == "'\"hello\"'");
      TEST(token->token_ == "\"hello\"");
    }
    {
      ++iter;
      TEST(iter == std::end(get()));
    }
  }

  //-------------------------------------------------------------------------
  void simple5() noexcept(false)
  {
    prepare(
      "abc 134[[foo\n"
      "+++\"\'hello\n"
      "---``#%"
    );
    expect(zax::ErrorTypes::Error::MissingEndOfQuote, 2, 4);
    expect(zax::ErrorTypes::Error::Syntax, 3, 4);
    expect(zax::ErrorTypes::Error::Syntax, 3, 6);
    auto iter{ std::begin(get()) };
    auto iterBogus{ std::begin(get()) };
    TEST(iter == iterBogus);

    {
      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "abc");
      TEST(token->token_ == "abc");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 5);
      TEST(token->type_ == zax::Token::Type::Number);
      TEST(token->originalToken_ == "134");
      TEST(token->token_ == "134");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 15 - 8 + 1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::DirectiveOpen);
      TEST(token->originalToken_ == "[[");
      TEST(token->token_ == "[[");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 17 - 8 + 1);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "foo");
      TEST(token->token_ == "foo");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 20 - 8 + 1);
      TEST(token->type_ == zax::Token::Type::Separator);
      TEST(token->originalToken_ == "\n");
      TEST(token->token_ == "\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::Constructor);
      TEST(token->originalToken_ == "+++");
      TEST(token->token_ == "+++");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 4);
      TEST(token->type_ == zax::Token::Type::Quote);
      TEST(token->originalToken_ == "\"\'hello");
      TEST(token->token_ == "\'hello");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 20 - 8 + 1 - 2);
      TEST(token->type_ == zax::Token::Type::Separator);
      TEST(token->originalToken_ == "\n");
      TEST(token->token_ == "\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 3, 1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::Destructor);
      TEST(token->originalToken_ == "---");
      TEST(token->token_ == "---");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 3, 14-8+1);
      TEST(token->type_ == zax::Token::Type::Operator);
      TEST(token->operator_ == zax::Token::Operator::Modulus);
      TEST(token->originalToken_ == "%");
      TEST(token->token_ == "%");
    }
    {
      ++iter;
      TEST(iter == std::end(get()));
    }
  }

  //-------------------------------------------------------------------------
  void simple6() noexcept(false)
  {
    prepare(
      "/* comment\n"
      "*/abc\xE2\x82\xAC"
    );
    auto iter{ std::begin(get_const()) };

    {
      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Comment);
      TEST(token->originalToken_ == "/* comment\n*/");
      TEST(token->token_ == " comment\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 1, 1);
      TEST(token->type_ == zax::Token::Type::Separator);
      TEST(token->originalToken_ == "/* comment\n*/");
      TEST(token->token_ == " comment\n");
    }
    {
      ++iter;
      auto token{ *iter };
      validate(token, 2, 3);
      TEST(token->type_ == zax::Token::Type::Literal);
      TEST(token->originalToken_ == "abc\xE2\x82\xAC");
      TEST(token->token_ == "abc\xE2\x82\xAC");
    }
    {
      ++iter;
      TEST(iter == std::end(get()));
    }
  }

  //-------------------------------------------------------------------------
  void simple7() noexcept(false)
  {
    prepare(
      "/* comment\n"
      "*/abc"
    );
    auto iter{ std::end(get()) };
    TEST(iter == std::end(get()));
    ++iter;
    TEST(iter == std::end(get()));
  }

  //-------------------------------------------------------------------------
  void simple8() noexcept(false)
  {
    prepare(
      "/* comment\n"
      "*/abc"
    );
    auto iter{ std::end(get_const()) };
    TEST(iter == std::cend(get_const()));
    ++iter;
    TEST(iter == std::end(get_const()));

    TEST(get_const().hasAhead(std::cbegin(get_const()), 2));
    TEST(!get().hasAhead(std::begin(get()), 3));
    TEST(std::begin(get_const()) == std::cbegin(get_const()));
  }

  //-------------------------------------------------------------------------
  TokenPtr makeToken(std::string_view str) noexcept
  {
    auto result{ std::make_shared<Token>() };
    result->token_ = str;
    return result;
  }

  std::vector<std::string_view> compareVector_{ "A", "1.1", "C", "--", "E", "&&", "G", "+++", "++", "J", "K", "L", "M", "(", ")", "P", "^^", "R", "$", "T", "]]", "@@", "W", "X", ",", "Z" };

  //-------------------------------------------------------------------------
  std::vector<std::string_view>  makeVector(size_t firstLet, size_t lastLet) noexcept(false)
  {
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
  }

  //-------------------------------------------------------------------------
  void checkEmpty(Tokenizer& tokens) noexcept(false)
  {
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

    zax::TokenList newTokens;

    tokens.extractThenPushFront(newTokens);
    tokens.extractThenPushBack(newTokens);

    tokens.copyPushFront(newTokens);
    tokens.copyPushBack(newTokens);

    TEST(!tokens[0]);
    TEST(tokens.at(0) == tokens.end());

    TEST(tokens.begin() == tokens.end());
    TEST(tokens.cbegin() == tokens.cend());

    TEST(tokens.empty());
    TEST(tokens.size() == 0);
  }

  //-------------------------------------------------------------------------
  void checkList(Tokenizer& list, std::vector<std::string_view>&& vector) noexcept(false)
  {
    if (vector.size() < 1) {
      checkEmpty(list);
      return;
    }
    TEST(!list.empty());
    TEST(zax::hasAhead(std::begin(list), vector.size() - 1));
    TEST(list.hasAhead(std::begin(list), vector.size() - 1));
    TEST(!list.hasAhead(std::begin(list), vector.size()));
    TEST(zax::hasBehind(std::end(list), vector.size() - 1));
    TEST(list.hasBehind(std::end(list), vector.size() - 1));
    TEST(!zax::hasAhead(std::begin(list), vector.size()));
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
  }

  //-------------------------------------------------------------------------
  void checkEmpty(zax::TokenList& tokens) noexcept(false)
  {
    TEST(tokens.empty());
    TEST(tokens.size() == 0);
  }

  //-------------------------------------------------------------------------
  void checkList(zax::TokenList& list, std::vector<std::string_view>&& vector) noexcept(false)
  {
    if (vector.size() < 1) {
      checkEmpty(list);
      return;
    }
    TEST(!list.empty());
    TEST(zax::hasAhead(std::begin(list), vector.size() - 1));
    TEST(list.hasAhead(std::begin(list), vector.size() - 1));
    TEST(!list.hasAhead(std::begin(list), vector.size()));
    TEST(zax::hasBehind(std::end(list), vector.size() - 1));
    TEST(list.hasBehind(std::end(list), vector.size() - 1));
    TEST(!zax::hasAhead(std::begin(list), vector.size()));
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
  }

  //-------------------------------------------------------------------------
  void simple9() noexcept(false)
  {
    prepare("");
    auto iter{ std::end(get()) };
    TEST(std::begin(get()) == std::end(get()));
    ++iter;
    TEST(iter == std::end(get()));
    checkEmpty(get());
  }

  //-------------------------------------------------------------------------
  void simple10() noexcept(false)
  {
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto result1{ list.extractFromStartToPos(4) };
      checkList(list, makeVector(4, 26));
      checkList(result1, makeVector(0, 4));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto result1{ list.extractFromPosToEnd(4) };
      checkList(list, makeVector(0, 4));
      checkList(result1, makeVector(4, 26));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );

      auto& list{ get() };

      // lazy iteration so only the current and "next"
      // prepared
      auto result1{ list.extractFromPosToEnd(4) };
      checkList(result1, makeVector(4, 5));

      auto vec1{ makeVector(0, 4) };
      auto vec2{ makeVector(5, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());

      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      list.erase(4);
      auto vec1{ makeVector(0, 4) };
      auto vec2{ makeVector(5, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      list.pushFront(makeToken("new"));
      std::vector<std::string_view> vec1{ "new" };
      auto vec2{ makeVector(0, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto tmp{ makeToken("new") };
      list.pushFront(tmp);
      std::vector<std::string_view> vec1{ "new" };
      auto vec2{ makeVector(0, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      list.pushBack(makeToken("new"));
      auto vec1{ makeVector(0, 26) };
      std::vector<std::string_view> vec2{ "new" };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto tmp{ makeToken("new") };
      list.pushBack(tmp);
      auto vec1{ makeVector(0, 26) };
      std::vector<std::string_view> vec2{ "new" };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto result{ list.popFront() };
      TEST(result->token_ == "A");
      checkList(list, makeVector(1, 26));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.extractThenPushFront(tokens);
      checkEmpty(tokens);

      auto vec2{ makeVector(0, 26) };
      std::vector<std::string_view> vec1{ "a", "b" };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.extractThenPushFront(std::move(tokens));
      checkEmpty(tokens);

      auto vec2{ makeVector(0, 26) };
      std::vector<std::string_view> vec1{ "a", "b" };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.extractThenPushBack(tokens);
      checkEmpty(tokens);

      auto vec1{ makeVector(0, 26) };
      std::vector<std::string_view> vec2{ "a", "b" };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.extractThenPushBack(std::move(tokens));
      checkEmpty(tokens);

      auto vec1{ makeVector(0, 26) };
      std::vector<std::string_view> vec2{ "a", "b" };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.copyPushFront(tokens);

      auto vec2{ makeVector(0, 26) };
      std::vector<std::string_view> vec1{ "a", "b" };
      checkList(tokens, std::move(vec1));
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.copyPushBack(tokens);

      auto vec1{ makeVector(0, 26) };
      std::vector<std::string_view> vec2{ "a", "b" };
      checkList(tokens, std::move(vec2));
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      auto& list{ get() };
      TEST(list[3]->token_ == "--");
      checkList(list, makeVector(0,26));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      auto& list{ get() };
      TEST((*list.at(3))->token_ == "--");
      checkList(list, makeVector(0, 26));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      auto& list{ get_const() };
      TEST((*list.at(3))->token_ == "--");
      checkList(get(), makeVector(0, 26));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto extracted{ list.extract(list.at(1), list.at(4)) };

      auto vec1{ makeVector(0, 1) };
      auto vec2{ makeVector(4, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
      checkList(extracted, makeVector(1, 4));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto extracted{ list.extract(list.at(1), 3) };

      auto vec1{ makeVector(0, 1) };
      auto vec2{ makeVector(4, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
      checkList(extracted, makeVector(1, 4));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto extracted{ list.extractFromStartToPos(list.at(3)) };

      checkList(list, makeVector(3, 26));
      checkList(extracted, makeVector(0, 3));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto extracted{ list.extractFromPosToEnd(list.at(3)) };

      checkList(list, makeVector(0, 3));
      checkList(extracted, makeVector(3, 26));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      list.erase(list.at(1), list.at(3));

      auto vec1{ makeVector(0, 1) };
      auto vec2{ makeVector(3, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      list.erase(list.at(1));

      auto vec1{ makeVector(0, 1) };
      auto vec2{ makeVector(2, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      list.erase(list.at(1), 2);

      auto vec1{ makeVector(0, 1) };
      auto vec2{ makeVector(3, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto tmp{ makeToken("a") };
      list.insertBefore(list.at(2), tmp);

      auto vec1{ makeVector(0, 2) };
      std::vector<std::string_view> vec2{ "a" };
      auto vec3{ makeVector(2, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      vec1.insert(vec1.end(), vec3.begin(), vec3.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto tmp{ makeToken("a") };
      list.insert(list.at(2), tmp);

      auto vec1{ makeVector(0, 2) };
      std::vector<std::string_view> vec2{ "a" };
      auto vec3{ makeVector(2, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      vec1.insert(vec1.end(), vec3.begin(), vec3.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      auto& list{ get() };
      auto tmp{ makeToken("a") };
      list.insertAfter(list.at(2), tmp);

      auto vec1{ makeVector(0, 3) };
      std::vector<std::string_view> vec2{ "a" };
      auto vec3{ makeVector(3, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      vec1.insert(vec1.end(), vec3.begin(), vec3.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.insertCopyBefore(list.at(3), tokens);

      auto vec1{ makeVector(0, 3) };
      std::vector<std::string_view> vec2{ "a", "b" };
      auto vec3{ makeVector(3, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      vec1.insert(vec1.end(), vec3.begin(), vec3.end());
      checkList(list, std::move(vec1));
    }
    {
      prepare(
        "A 1.1 C--E&&G+++++J K L M ()P^^R$T ]]@@ W X,Z"
      );
      checkList(get(), makeVector(0, 26));

      zax::TokenList tokens;
      tokens.pushBack(makeToken("a"));
      tokens.pushBack(makeToken("b"));

      auto& list{ get() };
      list.insertCopyAfter(list.at(3), tokens);

      auto vec1{ makeVector(0, 4) };
      std::vector<std::string_view> vec2{ "a", "b" };
      auto vec3{ makeVector(4, 26) };
      vec1.insert(vec1.end(), vec2.begin(), vec2.end());
      vec1.insert(vec1.end(), vec3.begin(), vec3.end());
      checkList(list, std::move(vec1));
    }
  }

  //-------------------------------------------------------------------------
  void continuation() noexcept(false)
  {
    {
      prepare("\\");
      expect(zax::WarningTypes::Warning::NewlineAfterContinuation, 1, 2);

      auto iter{ std::begin(get()) };

      auto token{ *iter };
      TEST(!token);
      TEST(iter == std::end(get()));
    }
    {
      prepare(
        "\\\n"
        "hello"
      );

      auto iter{ std::begin(get()) };

      {
        auto token{ *iter };
        validate(token, 2, 1);
        TEST(token->type_ == zax::Token::Type::Literal);
        TEST(token->originalToken_ == "hello");
        TEST(token->token_ == "hello");
        ++iter;
        TEST(iter == std::end(get()));
      }
    }

    {
      prepare(
        "\\ // ignore me\n"
        "hello"
      );

      auto iter{ std::begin(get()) };

      {
        auto token{ *iter };
        validate(token, 2, 1);
        TEST(token->type_ == zax::Token::Type::Literal);
        TEST(token->originalToken_ == "hello");
        TEST(token->token_ == "hello");
        ++iter;
        TEST(iter == std::end(get()));
      }
    }
    {
      prepare(
        "\\ /* ignore me\n"
        "*/hello"
      );

      auto iter{ std::begin(get()) };

      {
        auto token{ *iter };
        validate(token, 2, 3);
        TEST(token->type_ == zax::Token::Type::Literal);
        TEST(token->originalToken_ == "hello");
        TEST(token->token_ == "hello");
        ++iter;
        TEST(iter == std::end(get()));
      }
    }
    {
      prepare(
        "\\ problem\n"
        "hello"
      );

      expect(zax::WarningTypes::Warning::NewlineAfterContinuation, 1, 3);

      auto iter{ std::begin(get()) };

      {
        auto token{ *iter };
        validate(token, 1, 3);
        TEST(token->type_ == zax::Token::Type::Literal);
        TEST(token->originalToken_ == "problem");
        TEST(token->token_ == "problem");
        ++iter;
        TEST(iter != std::end(get()));
      }

      {
        auto token{ *iter };
        validate(token, 1, 20-11+1);
        TEST(token->type_ == zax::Token::Type::Separator);
        ++iter;
        TEST(iter != std::end(get()));
      }

      {
        auto token{ *iter };
        validate(token, 2, 1);
        TEST(token->type_ == zax::Token::Type::Literal);
        TEST(token->originalToken_ == "hello");
        TEST(token->token_ == "hello");
        ++iter;
        TEST(iter == std::end(get()));
      }
    }

    {
      prepare(
        "\\\n"
        "\\\n"
        "\\\n"
        "\\\n"
        "\\\n"
        "\\\n"
        "\\\n"
        "hello"
      );

      auto iter{ std::begin(get()) };

      {
        auto token{ *iter };
        validate(token, 8, 1);
        TEST(token->type_ == zax::Token::Type::Literal);
        TEST(token->originalToken_ == "hello");
        TEST(token->token_ == "hello");
        ++iter;
        TEST(iter == std::end(get()));
      }
    }
  }

  //-------------------------------------------------------------------------
  void runAll() noexcept(false)
  {
    auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

    runner([&]() { simple(); });
    runner([&]() { simple2(); });
    runner([&]() { simple3(); });
    runner([&]() { simple4(); });
    runner([&]() { simple5(); });
    runner([&]() { simple6(); });
    runner([&]() { simple7(); });
    runner([&]() { simple8(); });
    runner([&]() { simple9(); });
    runner([&]() { simple10(); });
    runner([&]() { continuation(); });

    reset();
  }
};

//---------------------------------------------------------------------------
void testTokenizer() noexcept(false)
{
  TokenizerBasics{}.runAll();
  TokenizerInstance{}.runAll();
}

} // namespace zaxTest
