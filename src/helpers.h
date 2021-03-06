
#pragma once

#include "types.h"

namespace zax
{

Puid puid() noexcept;
StringView makeStringView(const char* start, const char* end) noexcept;

std::pair< std::unique_ptr<std::byte[]>, size_t> readBinaryFile(const StringView fileName) noexcept;

bool writeBinaryFile(
  const StringView fileName,
  const std::byte* source,
  size_t length) noexcept;

inline bool writeBinaryFile(
  const StringView fileName,
  const StringView contents) noexcept
{
  // short cut to prevent memory duplication as likely to be true
  static_assert(sizeof(std::byte) == sizeof(char));
  static_assert(alignof(std::byte) == alignof(char));
  return writeBinaryFile(fileName, reinterpret_cast<const std::byte*>(contents.data()), contents.length());
}

inline bool writeBinaryFile(
  const StringView fileName,
  const std::uint8_t* source,
  size_t length) noexcept
{
  // short cut to prevent memory duplication as likely to be true
  static_assert(sizeof(std::byte) == sizeof(std::uint8_t));
  static_assert(alignof(std::byte) == alignof(std::uint8_t));
  return writeBinaryFile(fileName, reinterpret_cast<const std::byte*>(source), length);
}

String makeIncludeFile(
  const StringView currentFile,
  const StringView newFile,
  String& outFullFilePath) noexcept;

String fileAndPathFromFilePath(
  const StringView filePath,
  String& outParentFilePath) noexcept;

String locateFile(
  const StringView currentFile,
  const StringView newFile,
  String& outFullFilePath,
  bool useAbsolutePath = false) noexcept;

struct LocateWildCardFilesResult
{
  String path_;
  String fullPath_;
  StringList foundMatches_;

  LocateWildCardFilesResult(
    const StringView path,
    const StringView fullPath,
    const StringList& foundMatches) noexcept :
    path_(path),
    fullPath_(fullPath),
    foundMatches_(foundMatches)
  {}

  LocateWildCardFilesResult() noexcept = default;
  LocateWildCardFilesResult(const LocateWildCardFilesResult&) noexcept = default;
  LocateWildCardFilesResult(LocateWildCardFilesResult&&) noexcept = default;

  LocateWildCardFilesResult& operator=(const LocateWildCardFilesResult&) noexcept = default;
  LocateWildCardFilesResult& operator=(LocateWildCardFilesResult&&) noexcept = default;
};

void locateWildCardFiles(
  std::list<LocateWildCardFilesResult>& outFoundFilePaths,
  const StringView currentFile,
  const StringView newFileWithWildCards,
  bool useAbsolutePath = false) noexcept;

std::map<size_t, StringView> stringSplitView(
  const StringView input,
  const StringView splitStr) noexcept;

std::map<size_t, String> stringSplit(
  const StringView input,
  const StringView splitStr) noexcept;

String stringMerge(
  const std::map<size_t, String>& input,
  const StringView splitStr) noexcept;

String stringMerge(
  const std::map<size_t, StringView>& input,
  const StringView splitStr) noexcept;

String stringReplace(
  const StringView input,
  const StringMap& mapping) noexcept;

String stringReplace(
  const StringView input,
  const StringViewMap& mapping) noexcept;

String stringReplace(
  const StringView input,
  const StringView search,
  const StringView replace) noexcept;

std::optional<int> toInt(const String& input) noexcept;
std::optional<int> toInt(StringView input) noexcept;

// see https://semver.org/
// The rules are fully followed except the "pre-release" and "build" are
// relaxed rules for strings that are not strict by the parse rules.
struct SemanticVersion
{
  int major_{};
  int minor_{};
  int patch_{};
  String preRelease_;
  String build_;

  SemanticVersion(StringView value, bool *outSuccess = nullptr) noexcept;

  SemanticVersion() = default;
  SemanticVersion(const SemanticVersion&) = default;
  SemanticVersion(SemanticVersion&&) = default;

  SemanticVersion& operator=(const SemanticVersion&) = default;
  SemanticVersion& operator=(SemanticVersion&&) = default;

  bool operator==(const SemanticVersion& op2) const noexcept { return std::strong_ordering::equivalent == (*this <=> op2); }
  std::strong_ordering operator<=>(const SemanticVersion& op2) const noexcept;
  std::strong_ordering fullCompare(const SemanticVersion& op2) const noexcept;

  static std::optional<SemanticVersion> convert(const StringView value) noexcept;
};

} // namespace zax
