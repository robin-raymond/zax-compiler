
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

  try {
    size = std::filesystem::file_size(fileName);
  }
  catch (const std::filesystem::filesystem_error&) {
    return {};
  }

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

  std::filesystem::path newFilePath{ newFile };
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
  std::filesystem::path path{ filePath };

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
  std::filesystem::path currentFilePath{ currentFile };
  std::filesystem::path parentCurrentFile{ currentFilePath.parent_path() };
  if (useAbsolutePath) {
    parentCurrentFile = std::filesystem::absolute(parentCurrentFile, ec);
    if (ec)
      return outFullFilePath;
  }

  while (true) {
    String fullPath;
    auto usePath{ parentCurrentFile / currentFilePath.filename() };
    auto result{ makeIncludeFile(usePath.string(), newFile, fullPath) };
    if (std::filesystem::is_regular_file(std::filesystem::path{ result }, ec)) {
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
  std::list<std::pair<String, String>>& outFoundFilePaths,
  const StringView currentFile,
  const StringView newFileWithWildCards,
  bool useAbsolutePath) noexcept
{
  using Path = std::filesystem::path;
  using PathList = std::list<Path>;

  struct HasWild {
    static bool hasWild(const StringView str) noexcept {
      return (!((StringView::npos == str.find("*")) &&
                (StringView::npos == str.find("?"))));
    }
  };

  if (!HasWild::hasWild(newFileWithWildCards)) {
    String outFullFilePath;
    String result{ locateFile(currentFile, newFileWithWildCards, outFullFilePath) };
    outFoundFilePaths.push_back(std::make_pair(result, outFullFilePath));
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
        PathList& outFoundFiles,
        Path basePath,
        PathList exploreComponents) noexcept {
        std::error_code ec{};

        if (exploreComponents.empty()) {
          if (std::filesystem::is_regular_file(basePath, ec))
            outFoundFiles.push_back(basePath);
          return;
        }

        Path matchEntry{ exploreComponents.front() };
        exploreComponents.pop_front();

        if (!HasWild::hasWild(matchEntry.string())) {
          basePath /= matchEntry;
          return wildMatch(outFoundFiles, basePath, exploreComponents);
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
            PathList& outFoundFiles,
            PathList& exploreComponents,
            const Path& basePath,
            const Path& matchEntry,
            const Path& entry,
            decltype(String::npos) greediness) {
            String wildStr{ matchEntry.string() };
            auto posQuestion{ wildStr.find("?") };
            auto posStar{ wildStr.find("*") };
            auto posFirst{ String::npos == posQuestion ? posStar : ( String::npos == posStar ? posQuestion : std::min(posStar, posQuestion) ) };
            if (String::npos == posFirst) {
              if (0 != matchEntry.compare(entry))
                return false;
              // no more wild card, direct match check
              auto useBasePath{ basePath / entry };
              wildMatch(outFoundFiles, useBasePath, exploreComponents);
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
              if (humbleMatch(outFoundFiles, exploreComponents, basePath, Path{ useMatchStr }, Path{ useEsStr }, 0))
                return true;
              if (!increaseGreed)
                return false;
              return humbleMatch(outFoundFiles, exploreComponents, basePath, matchEntry, entry, useGreed + 1);
            } };

            if (posFirst == posQuestion)
              return rehumble(1, false);
            return rehumble(greediness, true);
          }
        };

        for (auto& checkEntry : entries) {
          HumbleMatcher::humbleMatch(outFoundFiles, exploreComponents, basePath, matchEntry, checkEntry , 0);
        }
      }
    };

    PathList foundResults;
    WildMatcher::wildMatch(foundResults, basePath, exploreComponents);

    if (foundResults.size() > 0) {
      for (auto& path : foundResults) {

        assert(std::filesystem::is_regular_file(path, ec));
        auto absPath = std::filesystem::absolute(path, ec);

        outFoundFilePaths.push_back(std::make_pair(path.string(), absPath.string()));
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
