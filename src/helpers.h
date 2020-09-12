
#pragma once

#include "types.h"

namespace zax
{

Puid puid() noexcept;


std::pair< std::unique_ptr<std::byte[]>, size_t> readBinaryFile(const std::string_view fileName) noexcept;

bool writeBinaryFile(
  const std::string fileName,
  const std::byte* source,
  size_t length) noexcept;

inline bool writeBinaryFile(
  const std::string fileName,
  const std::string_view contents) noexcept
{
  // short cut to prevent memory duplication as likely to be true
  static_assert(sizeof(std::byte) == sizeof(char));
  static_assert(alignof(std::byte) == alignof(char));
  return writeBinaryFile(fileName, reinterpret_cast<const std::byte*>(contents.data()), contents.length());
}

inline bool writeBinaryFile(
  const std::string fileName,
  const std::uint8_t* source,
  size_t length) noexcept
{
  // short cut to prevent memory duplication as likely to be true
  static_assert(sizeof(std::byte) == sizeof(std::uint8_t));
  static_assert(alignof(std::byte) == alignof(std::uint8_t));
  return writeBinaryFile(fileName, reinterpret_cast<const std::byte*>(source), length);
}

std::string makeIncludeFile(
  const std::string& currentFile,
  const std::string& newFile,
  std::string& outFullPathFileName) noexcept;

std::map<size_t, std::string_view> stringSplitView(
  const std::string_view& input,
  const std::string_view& splitStr) noexcept;

std::map<size_t, std::string> stringSplit(
  const std::string_view& input,
  const std::string_view& splitStr) noexcept;

std::string stringMerge(
  const std::map<size_t, std::string>& input,
  const std::string_view& splitStr) noexcept;

std::string stringMerge(
  const std::map<size_t, std::string_view>& input,
  const std::string_view& splitStr) noexcept;

} // namespace zax
