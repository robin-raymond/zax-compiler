
#include "pch.h"
#include "CompilerException.h"
#include "Token.h"
#include "CompileState.h"
#include "helpers.h"

using namespace zax;

//-----------------------------------------------------------------------------
void zax::output(InformationalTypes::Informational informational, TokenPtr token, const StringMap& params) noexcept
{
  assert(token);

  auto useOrigin{ &(token->origin_) };
  if (InformationalTypes::Informational::ActualOrigin == informational) {
    if ((token->origin_.filePath_ == token->actualOrigin_.filePath_) &&
        (token->origin_.location_ == token->actualOrigin_.location_))
      return;
    useOrigin = &(token->actualOrigin_);
  }
  assert(useOrigin);
  assert(useOrigin->filePath_);

  zax::output(
    CompilerException{
      CompilerException::ErrorType::Informational,
      useOrigin->filePath_->filePath_,
      useOrigin->location_.line_,
      useOrigin->location_.column_,
      String{ InformationalTypes::InformationalTraits::toString(informational) },
      stringReplace(InformationalTypes::InformationalHumanReadableTraits::toString(informational), params)
    }
  );
}

//-----------------------------------------------------------------------------
void zax::output(WarningTypes::Warning warning, TokenPtr token, const StringMap& params) noexcept
{
  assert(token);
  assert(token->origin_.filePath_);
  assert(token->compileState_);
  bool treatAsError{ token->compileState_->isWarningAnError(warning) };

  zax::output(
    CompilerException{
      treatAsError ? CompilerException::ErrorType::Error : CompilerException::ErrorType::Warning,
      token->origin_.filePath_->filePath_,
      token->origin_.location_.line_,
      token->origin_.location_.column_,
      String{ WarningTypes::WarningTraits::toString(warning) },
      stringReplace(WarningTypes::WarningHumanReadableTraits::toString(warning), params)
    }
  );
  output(InformationalTypes::Informational::ActualOrigin, token);
}

//-----------------------------------------------------------------------------
void zax::output(ErrorTypes::Error error, TokenPtr token, const StringMap& params) noexcept
{
  assert(token);
  assert(token->origin_.filePath_);

  zax::output(
    CompilerException{
      CompilerException::ErrorType::Error,
      token->origin_.filePath_->filePath_,
      token->origin_.location_.line_,
      token->origin_.location_.column_,
      String{ ErrorTypes::ErrorTraits::toString(error) },
      stringReplace(ErrorTypes::ErrorHumanReadableTraits::toString(error), params)
    }
  );
  output(InformationalTypes::Informational::ActualOrigin, token);
}

//-----------------------------------------------------------------------------
void zax::throwException(ErrorTypes::Error error, TokenPtr token, const StringMap& params) noexcept(false)
{
  assert(token);
  assert(token->origin_.filePath_);

  throwException(
    CompilerException{
      CompilerException::ErrorType::Fatal,
      token->origin_.filePath_->filePath_,
      token->origin_.location_.line_,
      token->origin_.location_.column_,
      String{ ErrorTypes::ErrorTraits::toString(error) },
      stringReplace(ErrorTypes::ErrorHumanReadableTraits::toString(error), params)
    }
  );
}
