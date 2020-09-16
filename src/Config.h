
#pragma once

#include "types.h"

namespace zax
{

struct Config
{
  bool quiet_{};
  String inputFilePath_;
  String outputPath_;
  String listingFilePath_;
  int tabStopWidth_{ 8 };


  struct MetaData final
  {
    String outputPath_;
    bool json_{};
    bool bson_{};
    bool cbor_{};
    bool msgPack_{};
    bool ubjson_{};
  } metaData_;
};

} // namespace zax
