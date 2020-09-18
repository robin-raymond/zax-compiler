
#include "pch.h"
#include "CompilerException.h"
#include "Token.h"
#include "SourceFilePath.h"
#include "helpers.h"

using namespace zax;

//-----------------------------------------------------------------------------
void zax::output(InformationalTypes::Informational informational, TokenPtr token, const StringMap& params) noexcept
{
  assert(token);
  assert(token->location_.filePath_);

  zax::output(
    CompilerException{
      CompilerException::ErrorType::Informational,
      token->location_.filePath_->filePath_,
      token->location_.line_,
      token->location_.column_,
      String{ InformationalTypes::InformationalTraits::toString(informational) },
      stringReplace(InformationalTypes::InformationalHumanReadableTraits::toString(informational), params)
    }
  );
}

//-----------------------------------------------------------------------------
void zax::output(WarningTypes::Warning warning, bool treatAsError, TokenPtr token, const StringMap& params) noexcept
{
  assert(token);
  assert(token->location_.filePath_);

  zax::output(
    CompilerException{
      treatAsError ? CompilerException::ErrorType::Error : CompilerException::ErrorType::Warning,
      token->location_.filePath_->filePath_,
      token->location_.line_,
      token->location_.column_,
      String{ WarningTypes::WarningTraits::toString(warning) },
      stringReplace(WarningTypes::WarningHumanReadableTraits::toString(warning), params)
    }
  );
}

//-----------------------------------------------------------------------------
void zax::output(ErrorTypes::Error error, TokenPtr token, const StringMap& params) noexcept
{
  assert(token);
  assert(token->location_.filePath_);

  zax::output(
    CompilerException{
      CompilerException::ErrorType::Error,
      token->location_.filePath_->filePath_,
      token->location_.line_,
      token->location_.column_,
      String{ ErrorTypes::ErrorTraits::toString(error) },
      stringReplace(ErrorTypes::ErrorHumanReadableTraits::toString(error), params)
    }
  );
}

//-----------------------------------------------------------------------------
void zax::throwException(ErrorTypes::Error error, TokenPtr token, const StringMap& params) noexcept(false)
{
  assert(token);
  assert(token->location_.filePath_);

  throwException(
    CompilerException{
      CompilerException::ErrorType::Fatal,
      token->location_.filePath_->filePath_,
      token->location_.line_,
      token->location_.column_,
      String{ ErrorTypes::ErrorTraits::toString(error) },
      stringReplace(ErrorTypes::ErrorHumanReadableTraits::toString(error), params)
    }
  );
}
