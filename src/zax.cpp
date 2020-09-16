
#include "pch.h"
#include "types.h"
#include "version.h"
#include "Config.h"
#include "CompilerException.h"

using namespace zax;
using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

#ifdef ZAX_INCLUDE_TESTS
int runAllTests() noexcept;
#endif // ZAX_INCLUDE_TESTS

namespace
{

struct Output
{
  int errorNumber_{};

  void write(const StringStream& ss) noexcept
  {
    std::cout << ss.str();
  }
  int error() noexcept { return errorNumber_; }
  void error(int errorNumber) noexcept { if (0 == errorNumber_) errorNumber_ = errorNumber; }
};

//-----------------------------------------------------------------------------
Output& singleton() noexcept
{
  static Output value{};
  return value;
}

//-----------------------------------------------------------------------------
void showVersion() noexcept
{
  StringStream ss;
  ss << Version::name() << " " << Version::copyright() << " version " << Version::version() << "\n";
  singleton().write(ss);
}

//-----------------------------------------------------------------------------
void showHelp() noexcept
{
  StringStream ss;
  ss << "\n";
  ss << "Command line options are as follows:\n";
  ss << "\n";
  ss << " GENERAL OPTIONS:\n";
  ss << "\n";
  ss << "  -help\n";
  ss << "  -h\n";
  ss << "  -?                        display help\n";
  ss << "\n";
  ss << "  -version\n";
  ss << "  -v                        display version\n";
  ss << "\n";
  ss << "  -q\n";
  ss << "  -quiet                    suppress displaying output\n";
  ss << "\n";
  ss << "  -in <file>                input file\n";
  ss << "\n";
  ss << "  -output <file>            output file path\n";
  ss << "\n";
  ss << "  -listing <file>           listing file\n";
  ss << "\n";
  ss << "  -tab <size>               specifies default input file tab size\n";
  ss << "\n";
#ifdef ZAX_INCLUDE_TESTS
  ss << "  -test                     run unit tests\n";
  ss << "\n";
#endif //ZAX_INCLUDE_TESTS
  ss << "\n";
  ss << " META-DATA OPTIONS:\n";
  ss << "\n";
  ss << "  -metadata <file-prefix>   generate file(s) containing metadata\n";
  ss << "\n";
  ss << "  -metadata-types <...>     specify the output meta data types, choose from:\n";
  ss << "                            json\n";
  ss << "                            bson\n";
  ss << "                            cbor\n";
  ss << "                            msgpack\n";
  ss << "                            ubjson\n";
  ss << "\n";
  ss << "\n";
  singleton().write(ss);
}

//-----------------------------------------------------------------------------
void showError(const std::string& message) noexcept
{
  StringStream ss;
  std::cout << "\n";
  std::cout << "[ERROR] " << message << "\n";
  std::cout << "\n";
  singleton().error(-3);
  singleton().write(ss);
}

//-----------------------------------------------------------------------------
void showIllegalOption(const std::string& arg) noexcept
{
  showError("Command line argument issue: "s + arg);
}

} // namespace

//-----------------------------------------------------------------------------
void zax::output(const CompilerException& exception) noexcept
{
  StringStream ss;
  ss << exception.what();
  switch (exception.type_) {
    case CompilerException::ErrorType::Info:    break;
    case CompilerException::ErrorType::Warning: break;
    case CompilerException::ErrorType::Error:   singleton().error(-1); break;
    case CompilerException::ErrorType::Fatal:   singleton().error(-2); break;
  }
  singleton().write(ss);
}

//-----------------------------------------------------------------------------
class IllegalOption : std::exception {

  std::string what_;

private:
  IllegalOption(const std::string& what) noexcept { what_ = what; }
  IllegalOption(IllegalOption&&) = delete;

  IllegalOption& operator=(const IllegalOption&) = delete;
  IllegalOption& operator=(IllegalOption&&) = delete;

public:
  static void throwError(const std::string_view what) noexcept(false)
  {
    throw IllegalOption{ std::string(what) };
  }

  const char* what() const noexcept final
  {
    return what_.c_str();
  }
};

//-----------------------------------------------------------------------------
int main(int argn, char const* argv[])
{
  Config config;

  try {
    std::string lastOption;

    bool displayVersion{ true };
    bool displayHelp{};
    bool runTests{};

    bool expectingValue{};

    for (int loop = 1; loop < argn; ++loop) {
      std::string arg{ argv[loop] };

      if (expectingValue) {
        expectingValue = false;
      }
      else if (!lastOption.empty()) {
        expectingValue = true;
      }

      if ('-' == *arg.c_str()) {
        if ((expectingValue) &&
            (!lastOption.empty()))
          IllegalOption::throwError(lastOption);

        lastOption = &(arg.c_str()[1]);

        if ((0 == lastOption.compare("help")) ||
            (0 == lastOption.compare("h")) ||
            (0 == lastOption.compare("?"))) {
          displayHelp = true;
          goto resetOption;
        }
#ifdef ZAX_INCLUDE_TESTS
        if (0 == lastOption.compare("test")) {
          runTests = true;
          goto resetOption;
        }
#endif // ZAX_INCLUDE_TESTS

        if ((0 == lastOption.compare("version")) ||
            (0 == lastOption.compare("v"))) {
          displayVersion = true;
          goto resetOption;
        }

        if ((0 == lastOption.compare("q")) ||
            (0 == lastOption.compare("quiet"))) {
          displayVersion = false;
          config.quiet_ = true;
          goto resetOption;
        }

        if (0 == lastOption.compare("in"))
          continue;
        if (0 == lastOption.compare("out"))
          continue;
        if (0 == lastOption.compare("listing"))
          continue;
        if (0 == lastOption.compare("tab"))
          continue;
        if (0 == lastOption.compare("metadata"))
          continue;
        if (0 == lastOption.compare("metadata-types"))
          continue;

        IllegalOption::throwError(lastOption);
      }

      //scope: parseArgument
      {
        if (0 == lastOption.compare("in")) {
          if (config.inputFilePath_.size() > 0)
            IllegalOption::throwError(arg);
          config.inputFilePath_ = arg;
          goto resetOption;
        }
        if ((0 == lastOption.compare("out")) ||
          (lastOption.size() < 1)) {
          if (config.outputPath_.size() > 0)
            IllegalOption::throwError(arg);
          config.outputPath_ = arg;
          goto resetOption;
        }

        if (0 == lastOption.compare("listing")) {
          if (config.listingFilePath_.size() > 0)
            IllegalOption::throwError(arg);
          config.listingFilePath_ = arg;
          goto resetOption;
        }
        if (0 == lastOption.compare("tab")) {
          size_t processed{};
          try {
            auto converted = std::stoll(arg, &processed);
            if (converted < 0)
              IllegalOption::throwError(lastOption);
            if (processed < arg.length())
              IllegalOption::throwError(lastOption);
            config.tabStopWidth_ = SafeInt<decltype(config.tabStopWidth_)>(converted);
          }
          catch (const std::invalid_argument&) {
            IllegalOption::throwError(lastOption);
          }
          catch (const std::out_of_range&) {
            IllegalOption::throwError(lastOption);
          }
          goto resetOption;
        }
        if (0 == lastOption.compare("metadata")) {
          if (config.metaData_.outputPath_.size() > 0)
            IllegalOption::throwError(arg);
          config.metaData_.outputPath_ = arg;
          goto resetOption;
        }
        if (0 == lastOption.compare("metadata-types")) {
          if ("json" == arg) {
            if (config.metaData_.json_)
              IllegalOption::throwError(lastOption);
            config.metaData_.json_ = true;
          }
          else if ("bson" == arg) {
            if (config.metaData_.bson_)
              IllegalOption::throwError(lastOption);
            config.metaData_.bson_ = true;
          }
          else if ("cbor" == arg) {
            if (config.metaData_.cbor_)
              IllegalOption::throwError(lastOption);
            config.metaData_.cbor_ = true;
          }
          else if ("msgpack" == arg) {
            if (config.metaData_.msgPack_)
              IllegalOption::throwError(lastOption);
            config.metaData_.msgPack_ = true;
          }
          else if ("ubjson" == arg) {
            if (config.metaData_.ubjson_)
              IllegalOption::throwError(lastOption);
            config.metaData_.ubjson_ = true;
          }
          continue;
        }
      }

    resetOption:
      {
        lastOption.clear();
      }

    }

    // scope: finalizeOptions
    {
      if (displayVersion) {
        showVersion();
      }

      if (displayHelp) {
        if (!displayVersion)
          showVersion();
        showHelp();
      }

#ifdef ZAX_INCLUDE_TESTS
      if (runTests)
        return runAllTests();
#endif // ZAX_INCLUDE_TESTS

      if (config.inputFilePath_.size() < 1) {
        if (!config.quiet_)
          showError("input file name must be specified");
        return singleton().error();
      }

      if (config.outputPath_.size() < 1) {
        config.outputPath_ = "output";
      }

      auto verifyMetaDataAllow = [&]() noexcept(false) {
        if ((config.metaData_.json_) ||
            (config.metaData_.bson_) ||
            (config.metaData_.cbor_) ||
            (config.metaData_.msgPack_) ||
            (config.metaData_.ubjson_)) {

          if (config.metaData_.outputPath_.empty()) {
            IllegalOption::throwError("metadata-types (requires -metadata)");
          }

        }
        else {
          if (!config.metaData_.outputPath_.empty()) {
            IllegalOption::throwError("metadata (requires -metadata-types)");
          }
        }
      };

      auto verifyMetaDataDeny = [&]() noexcept(false) {
        if (!config.metaData_.outputPath_.empty()) {
          IllegalOption::throwError("metadata");
        }

        if ((config.metaData_.json_) ||
            (config.metaData_.bson_) ||
            (config.metaData_.cbor_) ||
            (config.metaData_.msgPack_) ||
            (config.metaData_.ubjson_)) {
          IllegalOption::throwError("metadata-types");
        }
      };

      try {
        //compile();
      }
      catch (const CompilerException& e) {
        if (!config.quiet_) {
          zax::output(e);
        }
      }
    }
  }
  catch (const IllegalOption& option) {
    showIllegalOption(option.what());
  }
  return singleton().error();
}
