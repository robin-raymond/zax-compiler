
#pragma once

#include "types.h"
#include "Config.h"

namespace zax
{

struct CompilerTypes
{
  using ModuleMap = std::map<String, ModulePtr>;
  using ModuleList = std::list<ModulePtr>;
  using SourceList = std::list<SourcePtr>;

  enum class SoureAssetRequired {
    Yes,
    No,
    Warn
  };

  struct SoureAssetRequiredDeclare final : public zs::EnumDeclare<SoureAssetRequired, 3>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {SoureAssetRequired::Yes, "yes"},
        {SoureAssetRequired::No, "no"},
        {SoureAssetRequired::Warn, "warn"}
      } };
    }
  };

  using SoureAssetRequiredTraits = zs::EnumTraits<SoureAssetRequired, SoureAssetRequiredDeclare>;

  struct SourceAsset {
    String filePath_;
    String fullFilePath_;
    String renameFilePath_;
    SoureAssetRequired required_{};
    bool generated_{};
  };

  using SourceAssetList = std::list<SourceAsset>;
};

struct Compiler : public CompilerTypes
{
  Config config_;
  CompileStatePtr activeState_;

  ModulePtr rootModule_;
  ModuleMap imports_;
  ModuleList anonymousImports_;

  SourceAssetList pendingSources_;
  SourceAssetList pendingAssets_;

  SourceList sources_;

  Compiler(const Config& config) noexcept;

  Compiler() noexcept = delete;
  Compiler(const Compiler&) noexcept = delete;
  Compiler(Compiler&&) noexcept = delete;

  Compiler& operator=(const Compiler&) noexcept = delete;
  Compiler& operator=(Compiler&&) noexcept = delete;

  void compile() noexcept(false);   // throws CompilerException

private:
  void prime() noexcept;
};

} // namespace zax
