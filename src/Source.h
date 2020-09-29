
#pragma once

#include "types.h"
#include "helpers.h"

namespace zax
{

struct SourceTypes
{
  ZAX_DECLARE_STRUCT_PTR(FilePath);
  ZAX_DECLARE_STRUCT_PTR(Location);

  struct FilePath
  {
    SourceWeakPtr source_;
    String filePath_;
    String fullFilePath_;
  };

  struct Location
  {
    int line_{ 1 };
    int column_{ 1 };

    bool operator==(const Location& rhs) const noexcept = default;
    bool operator!=(const Location& rhs) const noexcept = default;
  };

  struct Origin
  {
    FilePathPtr filePath_;
    Location location_;
  };
};

struct Source : public SourceTypes
{
  const Puid id_{ puid() };
  ContextPtr context_;

  FilePathPtr realPath_;
  FilePathPtr effectivePath_;

  TokenizerPtr tokenizer_;
};

} // namespace zax
