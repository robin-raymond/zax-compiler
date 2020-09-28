
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
    bool defaultForceError_{ false };
    bool enabled_{ true };
    bool locked_{ false };
    Puid lockedBy_{};
    bool forceError_{ false };
  };

  inline static constexpr size_t Size{ TEnumTraits::Total() };
  using StateArray = std::array<State, Size>;

  StateArray current_;
  std::stack<StateArray> stack_;

  Faults fork() const noexcept { return Faults{ .current_ = current_, .stack_ = stack_ }; }

  void push() noexcept
  {
    stack_.push(current_);
  }

  [[nodiscard]] bool pop() noexcept
  {
    if (stack_.size() < 1)
      return false;

    current_ = stack_.top();
    stack_.pop();
    return true;
  }

  State& at(TEnum value) noexcept
  {
    assert(TEnumTraits::toUnderlying(value) < Size);
    return current_[TEnumTraits::toUnderlying(value)];
  }

  const State& at(TEnum value) const noexcept
  {
    assert(TEnumTraits::toUnderlying(value) < Size);
    return current_[TEnumTraits::toUnderlying(value)];
  }
 
  bool enableForceError(TEnum value) noexcept
  {
      auto& state{ at(value) };
      if (state.locked_)
        return false;
      state.enabled_ = true;
      state.forceError_ = true;
      return true;
  }

  size_t enableForceErrorAll() noexcept
  {
    size_t total{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (enableForceError(value))
        ++total;
    }
    return total;
  }

  bool enable(TEnum value) noexcept
  {
      auto& state{ at(value) };
      if (state.locked_)
        return false;
      state.enabled_ = true;
      state.forceError_ = false;
      return true;
  }

  bool disable(TEnum value) noexcept
  {
      auto& state{ at(value) };
      if (state.locked_)
        return false;
      state.enabled_ = false;
      state.forceError_ = false;
      return true;
  }

  size_t enableAll() noexcept
  {
    size_t total{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (enable(value))
        ++total;
    }
    return total;
  }

  size_t disableAll() noexcept
  {
    size_t total{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (disable(value))
        ++total;
    }
    return total;
  }

  bool applyDefault(TEnum value) noexcept
  {
    auto& state{ at(value) };
    if (state.defaultForceError_)
      return enableForceError(value);
    if (state.defaultEnabled_)
      return enable(value);
    return disable(value);
  }

  size_t defaultAll() noexcept
  {
    size_t total{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (applyDefault(value))
        ++total;
    }
    return total;
  }

  bool lock(
    TEnum value,
    Puid locker) noexcept
  {
      auto& state{ at(value) };
      if (state.locked_)
        return false;
      if ((0 != state.lockedBy_) &&
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
      auto& state{ at(value) };
      if (!state.locked_)
        return false;
      if (state.lockedBy_ != locker)
        return false;
      state.locked_ = false;
      state.lockedBy_ = 0;
      return true;
  }

  size_t lockAll(Puid locker) noexcept
  {
    size_t total{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (lock(value, locker))
        ++total;
    }
    return total;
  }

  size_t unlockAll(Puid locker) noexcept
  {
    size_t total{};
    const TEnumTraits traits;
    for (auto [value, name] : traits) {
      if (unlock(value, locker))
        ++total;
    }
    return total;
  }
};

} // namespace zax
