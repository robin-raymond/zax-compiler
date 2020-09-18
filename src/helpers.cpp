
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
  const String fileName,
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
  const String& inCurrentFile,
  const String& inNewFile,
  String& outFullPathFileName) noexcept
{
  String currentFile = inCurrentFile;
  String newFile = inNewFile;

#ifdef _WIN32
  std::replace(currentFile.begin(), currentFile.end(), '/', '\\');
  std::replace(newFile.begin(), newFile.end(), '/', '\\');
#endif //_WIN32

  auto absFilePath = std::filesystem::absolute(currentFile);
  absFilePath.remove_filename();

  auto relPath = std::filesystem::proximate(newFile, absFilePath);
  auto newPath = absFilePath / relPath;

  auto currentPath = std::filesystem::current_path();
  auto newRelPath = std::filesystem::proximate(newPath, currentPath);

  outFullPathFileName = std::filesystem::absolute(newRelPath).string();

  return newRelPath.string();
}


//-----------------------------------------------------------------------------
std::map<size_t, StringView> zax::stringSplitView(
  const StringView& input,
  const StringView& splitStr) noexcept
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
std::map<size_t, String> stringSplit(
  const StringView& input,
  const StringView& splitStr) noexcept
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
  const StringView& splitStr) noexcept
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
  const StringView& splitStr) noexcept
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
  const StringView& input,
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
