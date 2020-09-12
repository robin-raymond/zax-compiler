
#pragma once

#include "types.h"

namespace zax
{

template <typename TEnum, typename TEnumTraits>
struct Faults
{
  struct State
  {
    bool defaultEnabled_{ true };
    bool enabled_{ true };
    bool locked_{ false };
    Puid lockedBy_{};
    bool forceError_{ false };
  };

  inline static constexpr size_t Size{ TEnumTraits::Total() };
  using StateArray = std::array<State, Size>;

  StateArray current_;
  std::stack<StateArray> stack_;

  void push() noexcept
  {
    stack_.push(current_);
  }

  bool pop() noexcept
  {
    if (stack_.size() < 1)
      return false;

    stack_.pop();
    return true;
  }

  State& at(TEnum value) noexcept
  {
    return current_[value];
  }

  const State& at(TEnum value) const noexcept
  {
    return current_[value];
  }
 
  bool enableForceError(TEnum value) noexcept
  {
      auto& state{ current_[value] };
      if (state.locked_)
        return false;
      state.enabled_ = true;
      state.forceError_ = true;
      return true;
  }

  bool enable(TEnum value) noexcept
  {
      auto& state{ current_[value] };
      if (state.locked_)
        return false;
      state.enabled_ = true;
      state.forceError_ = false;
      return true;
  }

  bool disable(TEnum value) noexcept
  {
      auto& state{ current_[value] };
      if (state.locked_)
        return false;
      state.enabled_ = false;
      state.forceError_ = false;
      return true;
  }

  bool lock(
    TEnum value,
    Puid locker) noexcept
  {
      auto& state{ current_[value] };
      if (state.locked_)
        return false;
      if ((0 != locker) &&
          (state.lockedBy_ != locker))
        return false;
      state.locked_ = true;
      state.lockedBy_ = locker;
      return true;
  }

  bool unlock(
    TEnum value,
    Puid locker) noexcept
  {    
      auto& state{ current_[value] };
      if (!state.locked_)
        return false;
      if (state.lockedBy_ != locker)
        return false;
      state.locked_ = false;
      state.lockedBy_ = 0;
  }

  size_t lockAll(Puid locker) noexcept
  {
    size_t totalLocked{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (lock(value, locker))
        ++totalLocked;
    }
    return totalLocked;
  }

  size_t unlockAll(Puid locker) noexcept
  {
    size_t totalLocked{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (unlock(value, locker))
        ++totalLocked;
    }
    return totalLocked;
  }
};

} // namespace zax
