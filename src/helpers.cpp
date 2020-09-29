
#include "pch.h"
#include "helpers.h"

#include <fstream>
#include <filesystem>

using namespace zax;

//-----------------------------------------------------------------------------
Puid zax::puid() noexcept {
  static std::atomic<Puid> singleton{ 1 };
  return singleton++;
}

//-----------------------------------------------------------------------------
StringView zax::makeStringView(const char* start, const char* end) noexcept {
  if ((!start) || (!end))
    return {};
  assert(end >= start);
  return StringView{ start, SafeInt<size_t>(end - start) };
}

//-----------------------------------------------------------------------------
std::pair<std::unique_ptr<std::byte[]>, size_t> zax::readBinaryFile(const StringView fileName) noexcept
{
  std::ifstream binFile;
  std::uintmax_t size{};

  std::error_code ec;

  size = std::filesystem::file_size(fileName, ec);
  if (ec)
    return {};

  binFile.open(fileName, std::ifstream::in | std::ifstream::binary);
  if (!binFile.is_open())
    return {};

  auto dest{ std::make_unique<std::byte[]>(SafeInt<size_t>(size)) };
  static_assert(sizeof(char) == sizeof(std::byte));
  static_assert(alignof(char) == alignof(std::byte));
  binFile.read(reinterpret_cast<char*>(dest.get()), size);
  if (binFile.fail())
    return {};

  return { std::move(dest), SafeInt<size_t>(size) };
}

//-----------------------------------------------------------------------------
bool zax::writeBinaryFile(
  const StringView fileName,
  const std::byte* source,
  size_t length) noexcept
{
  std::ofstream binFile;
  binFile.open(fileName, std::ifstream::out | std::ifstream::binary);

  if (!binFile.is_open())
    return false;

  // scope: write the bin file
  {
    static_assert(sizeof(char) == sizeof(std::byte));
    static_assert(alignof(char) == alignof(std::byte));
    binFile.write(reinterpret_cast<const char*>(source), length);
    if (binFile.fail())
      goto abandon;

    binFile.close();
    return true;
  }
abandon:
  {
    binFile.close();
  }
  return false;
}

//-----------------------------------------------------------------------------
String zax::makeIncludeFile(
  const StringView inCurrentFile,
  const StringView inNewFile,
  String& outFullFilePath) noexcept
{
  String currentFile{ inCurrentFile };
  String newFile{ inNewFile };

  std::error_code ec{};
  outFullFilePath = {};

#ifdef _WIN32
  std::replace(currentFile.begin(), currentFile.end(), '/', '\\');
  std::replace(newFile.begin(), newFile.end(), '/', '\\');
#endif //_WIN32

  Path newFilePath{ newFile };
  if (newFilePath.has_root_path()) {
    outFullFilePath = std::filesystem::absolute(newFilePath, ec).string();
    if (ec)
      return outFullFilePath;
  }

  auto absFilePath = std::filesystem::absolute(currentFile, ec);
  if (ec)
    return outFullFilePath;
  absFilePath.remove_filename();

  auto newPath = absFilePath / newFilePath;
  auto fullPath{ std::filesystem::absolute(newPath, ec) };
  if (ec)
    return outFullFilePath;


  auto relPath = std::filesystem::proximate(fullPath, absFilePath, ec);
  if (ec)
    return outFullFilePath;

  auto currentPath = std::filesystem::current_path(ec);
  if (ec)
    return outFullFilePath;

  auto newRelPath = std::filesystem::proximate(newPath, currentPath, ec);
  if (ec)
    return outFullFilePath;

  outFullFilePath = fullPath.string();
  return newRelPath.string();
}

//-----------------------------------------------------------------------------
String zax::fileAndPathFromFilePath(
  const StringView filePath,
  String& outParentFilePath) noexcept
{
  Path path{ filePath };

  outParentFilePath = path.parent_path().string();
  return path.filename().string();
}

//-----------------------------------------------------------------------------
String zax::locateFile(
  const StringView currentFile,
  const StringView newFile,
  String& outFullFilePath,
  bool useAbsolutePath) noexcept
{
  outFullFilePath = {};
  std::error_code ec{};
  Path currentFilePath{ currentFile };
  Path parentCurrentFile{ currentFilePath.parent_path() };
  if (useAbsolutePath) {
    parentCurrentFile = std::filesystem::absolute(parentCurrentFile, ec);
    if (ec)
      return outFullFilePath;
  }

  while (true) {
    String fullPath;
    auto usePath{ parentCurrentFile / currentFilePath.filename() };
    auto result{ makeIncludeFile(usePath.string(), newFile, fullPath) };
    if (std::filesystem::is_regular_file(Path{ result }, ec)) {
      outFullFilePath = fullPath;
      return result;
    }
    if (!parentCurrentFile.has_relative_path()) {
      if (!useAbsolutePath)
        return locateFile(currentFile, newFile, outFullFilePath, true);
      break;
    }

    parentCurrentFile = parentCurrentFile.parent_path();
  }
  return {};
}

//-----------------------------------------------------------------------------
void zax::locateWildCardFiles(
  std::list<LocateWildCardFilesResult>& outFoundFilePaths,
  const StringView currentFile,
  const StringView newFileWithWildCards,
  bool useAbsolutePath) noexcept
{
  using PathList = std::list<Path>;
  using ResultList = std::list<LocateWildCardFilesResult>;

  struct HasWild {
    static bool hasWild(const StringView str) noexcept {
      return (!((StringView::npos == str.find("*")) &&
                (StringView::npos == str.find("?"))));
    }
  };

  if (!HasWild::hasWild(newFileWithWildCards)) {
    String outFullFilePath;
    String result{ locateFile(currentFile, newFileWithWildCards, outFullFilePath) };
    if (!result.empty())
      outFoundFilePaths.emplace_back(result, outFullFilePath, StringList{});
    return;
  }

  std::error_code ec{};
  Path currentFilePath{ currentFile };
  Path parentCurrentFile{ currentFilePath.parent_path() };
  if (useAbsolutePath) {
    parentCurrentFile = std::filesystem::absolute(parentCurrentFile, ec);
    if (ec)
      return;
  }

  while (true) {
    String fullPath;
    auto usePath{ parentCurrentFile / currentFilePath.filename() };
    auto result{ makeIncludeFile(usePath.string(), newFileWithWildCards, fullPath) };

    // break out all wild cards into components
    Path basePath{ result };
    PathList exploreComponents;
    while (true) {
      Path parent{ basePath.parent_path() };
      Path relative{ std::filesystem::proximate(basePath, parent, ec) };
      exploreComponents.push_front(relative);
      basePath = parent;
      if (!HasWild::hasWild(basePath.string()))
        break;
    }

    struct WildMatcher {
      static void wildMatch(
        ResultList& outFoundFiles,
        Path basePath,
        PathList exploreComponents,
        const StringList& foundMatches) noexcept {
        std::error_code ec{};

        if (exploreComponents.empty()) {
          if (std::filesystem::is_regular_file(basePath, ec))
            outFoundFiles.emplace_back(basePath.string(), String{}, foundMatches);
          return;
        }

        Path matchEntry{ exploreComponents.front() };
        exploreComponents.pop_front();

        if (!HasWild::hasWild(matchEntry.string())) {
          basePath /= matchEntry;
          return wildMatch(outFoundFiles, basePath, exploreComponents, foundMatches);
        }

        // wild card matching is required

        PathList entries;
        for (auto& p : std::filesystem::directory_iterator(basePath, ec)) {
          auto foundPath{ p.path() };
          auto usePath{ std::filesystem::proximate(foundPath, basePath, ec) };
          if (usePath.empty())
            continue;
          entries.push_back(usePath);
        }

        struct HumbleMatcher {
          static bool humbleMatch(
            ResultList& outFoundFiles,
            const PathList& exploreComponents,
            StringList foundMatches,
            const Path& basePath,
            const Path& matchEntry,
            const Path& entry,
            std::remove_const_t<decltype(String::npos)> greediness) {

            String wildStr{ matchEntry.string() };
            auto posQuestion{ wildStr.find("?") };
            auto posStar{ wildStr.find("*") };
            auto posFirst{ String::npos == posQuestion ? posStar : ( String::npos == posStar ? posQuestion : std::min(posStar, posQuestion) ) };
            if (String::npos == posFirst) {
              if (0 != matchEntry.compare(entry))
                return false;
              // no more wild card, direct match check
              auto useBasePath{ basePath / entry };
              wildMatch(outFoundFiles, useBasePath, exploreComponents, foundMatches);
              return true;
            }

            String entryStr{ entry.string() };

            StringView wp{ wildStr };
            StringView es{ entryStr };

            StringView prefixWp{ wp.substr(0, posFirst) };
            if (es.length() < prefixWp.length())
              return false;

            // prefixes must match or not a match
            if (prefixWp.size() > 0) {
              if (0 != Path{ prefixWp }.compare(Path{ es.substr(0, prefixWp.length()) }))
                return false;
            }

            auto rehumble{ [&](decltype(String::npos) useGreed, bool increaseGreed) noexcept -> bool {
              // impossible match?
              if (es.length() < posFirst + useGreed)
                return false;
              if (wp.length() < posFirst + 1)
                return false;

              StringView borrowStr{ useGreed > 0 ? es.substr(posFirst, useGreed) : StringView{} };
              StringView postfixWp{ wp.substr(posFirst + 1) }; // account for the extracted wild character
              StringView postfixEs{ es.substr(posFirst + useGreed) }; // account for the borrowed string

              foundMatches.emplace_back(borrowStr);

              String useMatchStr;
              useMatchStr.reserve(prefixWp.size() + borrowStr.size() + postfixWp.size());
              String useEsStr;
              useEsStr.reserve(prefixWp.size() + borrowStr.size() + postfixEs.size());

              useMatchStr += prefixWp;
              useMatchStr += borrowStr;
              useMatchStr += postfixWp;

              useEsStr += prefixWp;
              useEsStr += borrowStr;
              useEsStr += postfixEs;
              if (humbleMatch(outFoundFiles, exploreComponents, foundMatches, basePath, Path{ useMatchStr }, Path{ useEsStr }, 0))
                return true;
              if (!increaseGreed)
                return false;
              foundMatches.pop_back();
              return humbleMatch(outFoundFiles, exploreComponents, foundMatches, basePath, matchEntry, entry, useGreed + 1);
            } };

            if (posFirst == posQuestion)
              return rehumble(1, false);
            return rehumble(greediness, true);
          }
        };

        for (auto& checkEntry : entries) {
          HumbleMatcher::humbleMatch(outFoundFiles, exploreComponents, foundMatches, basePath, matchEntry, checkEntry , 0);
        }
      }
    };

    ResultList foundResults;
    WildMatcher::wildMatch(foundResults, basePath, exploreComponents, StringList{});

    if (foundResults.size() > 0) {
      for (auto& pathResult : foundResults) {
        Path path{ pathResult.path_ };

        assert(std::filesystem::is_regular_file(path, ec));
        auto absPath = std::filesystem::absolute(path, ec);

        outFoundFilePaths.emplace_back(pathResult.path_, absPath.string(), pathResult.foundMatches_);
      }
      return;
    }

    if (!parentCurrentFile.has_relative_path()) {
      if (!useAbsolutePath) {
        locateWildCardFiles(outFoundFilePaths, currentFile, newFileWithWildCards, true);
        return;
      }
      break;
    }

    parentCurrentFile = parentCurrentFile.parent_path();
  }
}

//-----------------------------------------------------------------------------
std::map<size_t, StringView> zax::stringSplitView(
  const StringView input,
  const StringView splitStr) noexcept
{
  std::map<size_t, StringView> outResult;

  if (input.size() < 1)
    return outResult;

  if (splitStr.size() < 1) {
    outResult[0] = input;
    return outResult;
  }

  size_t count{};
  size_t searchFrom{};

  do
  {
    auto found = input.find(splitStr, searchFrom);
    if (String::npos == found) {
      StringView sub = input.substr(searchFrom);

      if (sub.size() > 0) {
        outResult[count] = sub;
        ++count;
      }
      break;
    }

    StringView sub = input.substr(searchFrom, found - searchFrom);
    searchFrom = found + splitStr.size();

    outResult[count] = sub;
    ++count;

  } while (true);

  return outResult;
}

//-----------------------------------------------------------------------------
std::map<size_t, String> zax::stringSplit(
  const StringView input,
  const StringView splitStr) noexcept
{
  std::map<size_t, String> outResult;

  if (input.size() < 1)
    return outResult;

  if (splitStr.size() < 1) {
    outResult[0] = input;
    return outResult;
  }

  size_t count{};
  size_t searchFrom{};

  do
  {
    auto found = input.find(splitStr, searchFrom);
    if (String::npos == found) {
      StringView sub = input.substr(searchFrom);

      if (sub.size() > 0) {
        outResult[count] = sub;
        ++count;
      }
      break;
    }

    StringView sub = input.substr(searchFrom, found - searchFrom);
    searchFrom = found + splitStr.size();

    outResult[count] = sub;
    ++count;

  } while (true);

  return outResult;
}

//-----------------------------------------------------------------------------
String zax::stringMerge(
  const std::map<size_t, String>& input,
  const StringView splitStr) noexcept
{
  if (input.size() < 1)
    return {};

  if (input.size() == 1)
    return String{ input.begin()->second };

  size_t count{};
  for (auto& mapping : input) {
    count += mapping.second.size();
  }
  count += splitStr.size() * input.size();

  String result;
  result.reserve(count);
  for (auto iter = input.begin(); iter != input.end(); ++iter) {
    if (result.size() > 0)
      result += splitStr;
    result += (*iter).second;
  }
  return result;
}

//-----------------------------------------------------------------------------
String zax::stringMerge(
  const std::map<size_t, StringView>& input,
  const StringView splitStr) noexcept
{
  if (input.size() < 1)
    return {};

  if (input.size() == 1)
    return String{ input.begin()->second };

  size_t count{};
  for (auto& mapping : input) {
    count += mapping.second.size();
  }
  count += splitStr.size() * input.size();

  String result;
  result.reserve(count);
  for (auto iter = input.begin(); iter != input.end(); ++iter) {
    if (result.size() > 0)
      result += splitStr;
    result += (*iter).second;
  }
  return result;
}

//-----------------------------------------------------------------------------
String zax::stringReplace(
  const StringView input,
  const StringMap& mapping) noexcept
{
  if (mapping.size() < 1)
    return String{ input };

  String output{ input };

  for (auto& [search, replace] : mapping) {
    String::size_type lastPos{};
    while (true) {
      auto pos{ output.find(search, lastPos) };
      if (pos == String::npos)
        break;

      output.replace(pos, search.length(), replace);
      lastPos = pos + search.length();
    }
  }

  return output;
}

//-----------------------------------------------------------------------------
String zax::stringReplace(
  const StringView input,
  const StringViewMap& mapping) noexcept
{
  if (mapping.size() < 1)
    return String{ input };

  String output{ input };

  for (auto& [search, replace] : mapping) {
    String::size_type lastPos{};
    while (true) {
      auto pos{ output.find(search, lastPos) };
      if (pos == String::npos)
        break;

      output.replace(pos, search.length(), replace);
      lastPos = pos + search.length();
    }
  }

  return output;
}

//-----------------------------------------------------------------------------
String zax::stringReplace(
  const StringView input,
  const StringView search,
  const StringView replace) noexcept
{
  if (search.size() < 1)
    return String{ input };

  String output{ input };

  String::size_type lastPos{};
  while (true) {
    auto pos{ output.find(search, lastPos) };
    if (pos == String::npos)
      break;

    output.replace(pos, search.length(), replace);
    lastPos = pos + search.length();
  }

  return output;
}

//-----------------------------------------------------------------------------
std::optional<int> zax::toInt(const String& input) noexcept
{
  if (input.empty())
    return {};
  try {
    // filter out decimals and exponents
    if (String::npos != input.find("."))
      return {};
    if (String::npos != input.find("e"))
      return {};
    return std::stoi(input);
  }
  catch (const std::invalid_argument&) {
  }
  catch (const std::out_of_range&) {
  }
  return {};
}

//-----------------------------------------------------------------------------
std::optional<int> zax::toInt(StringView input) noexcept
{
  return toInt(String{input});
}

namespace
{
  //---------------------------------------------------------------------------
  std::strong_ordering ordering(const String& value1, const String& value2) noexcept
  {
    auto compare{ value1.compare(value2) };
    if (compare < 0)
      return std::strong_ordering::less;
    if (compare > 0)
      return std::strong_ordering::greater;
    return std::strong_ordering::equal;
  }
} // namespace

//-----------------------------------------------------------------------------
SemanticVersion::SemanticVersion(StringView value, bool* outSuccess) noexcept
{
  if (outSuccess)
    *outSuccess = false;
  if (value.empty())
    return;

  static constexpr int maxVersions{ 3 };

  int dotIndex{ 0 };
  bool lastWasDot{};

  StringView posNumber{};
  StringView posPlus{};
  StringView posMinus{};

  bool foundPlus{};
  bool foundMinus{};

  auto pos{ value };

  auto completePlus{ [&]() noexcept -> bool {
    if (posPlus.empty())
      return true;
    build_ = makeStringView(posPlus.data() + 1, pos.data());
    posPlus = {};
    foundPlus = true;
    return !build_.empty();
  } };

  auto completeMinus{ [&]() noexcept -> bool {
    if (posMinus.empty())
      return true;
    preRelease_ = makeStringView(posMinus.data() + 1, pos.data());
    posMinus = {};
    foundMinus = true;
    return !preRelease_.empty();
  } };

  auto completeNumber{ [&]() noexcept -> bool {
      if (posNumber.empty())
        return true;

      auto asNumber{ toInt(makeStringView(posNumber.data(), pos.data())) };
      if (!asNumber)
        return false;

      if (*asNumber < 0)
        return false;

      switch (dotIndex) {
        case 0: major_ = *asNumber; break;
        case 1: minor_ = *asNumber; break;
        case 2: patch_ = *asNumber; break;
        default: return false;
      }
      ++dotIndex;
      posNumber = {};
      return true;
  } };

  auto foundStrayChar{ [&]() noexcept -> bool {
    if (dotIndex < maxVersions)
      return false;
    return ((posMinus.empty()) && (posPlus.empty()));
  } };

  while (!pos.empty())
  {
    char chr{ *pos.data() };

    switch (chr) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        if (foundStrayChar())
          return;
        if (dotIndex >= maxVersions)
          break;
        if (posNumber.empty())
          posNumber = pos;
        lastWasDot = false;
        break;
      }
      case '.': {
        if (foundStrayChar())
          return;
        if (dotIndex >= maxVersions)
          break;
        if (dotIndex + 1 == maxVersions)
          return;

        if (lastWasDot)
          return;

        if (posNumber.empty())
          return;

        if (!completeNumber())
          return;

        lastWasDot = true;
        break;
      }
      case '+': {
        if (!completeNumber())
          return;
        if (dotIndex < maxVersions)
          return;
        if ((foundPlus) || (!posPlus.empty()))
          return;
        if (!completeMinus())
          return;
        posPlus = pos;
        break;
      }
      case '-': {
        if (!completeNumber())
          return;
        if (dotIndex < maxVersions)
          return;
        if ((foundMinus) || (!posMinus.empty()))
          return;
        if (!completePlus())
          return;
        posMinus = pos;
        break;
      }
      default: {
        if (dotIndex < maxVersions)
          return;
        if (foundStrayChar())
          return;
        break;
      }
    }

    pos = pos.substr(1);
  }

  if (lastWasDot)
    return;
  if (!completeNumber())
    return;
  if (!completePlus())
    return;
  if (!completeMinus())
    return;

  if (dotIndex < maxVersions)
    return;

  if (outSuccess)
    *outSuccess = true;
}

//-----------------------------------------------------------------------------
std::strong_ordering SemanticVersion::operator<=>(const SemanticVersion& op2) const noexcept
{
  { auto check{ major_ <=> op2.major_ }; if (std::strong_ordering::equal != check) return check; }
  { auto check{ minor_ <=> op2.minor_ }; if (std::strong_ordering::equal != check) return check; }
  { auto check{ patch_ <=> op2.patch_ }; if (std::strong_ordering::equal != check) return check; }
  return std::strong_ordering::equivalent;
}

//-----------------------------------------------------------------------------
std::strong_ordering SemanticVersion::fullCompare(const SemanticVersion& op2) const noexcept
{
  { auto check{ *this <=> op2 }; if (std::strong_ordering::equivalent != check) return check; }
  { auto check{ ordering(preRelease_, op2.preRelease_) }; if (std::strong_ordering::equal != check) return check; }
  { auto check{ ordering(build_, op2.build_) }; if (std::strong_ordering::equal != check) return check; }
  return std::strong_ordering::equal;
}

//-----------------------------------------------------------------------------
std::optional<SemanticVersion> SemanticVersion::convert(const StringView value) noexcept
{
  bool success{};
  SemanticVersion result{ value, &success };
  if (!success)
    return {};
  return result;
}
