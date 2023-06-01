#include <compare>
#include <concepts>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <uchar.h>
#include <unordered_set>
#include <variant>

namespace args2 {
namespace argv {
template <typename CharT> class Iterator {
private:
  const CharT **argv;
  int argc;
  std::basic_string_view<CharT> value;

public:
  Iterator(const char **argv, int argc) noexcept
      : argv(argv), argc(argc), value(*argv) {}
  Iterator() noexcept : argv(nullptr), argc(0) {}

  constexpr bool operator==(const Iterator &other) const noexcept {
    // Special case for default-constructed end iterator
    return (argc == 0 && other.argc == 0) ||
           (argv == other.argv && argc == other.argc);
  }

  constexpr bool operator!=(const Iterator &other) const noexcept = default;

  const std::basic_string_view<CharT> &operator*() const noexcept {
    return value;
  }

  Iterator &operator++() noexcept {
    ++argv;
    value = *argv;
    return *this;
  }
  Iterator operator++(int) noexcept {
    auto prev = *this;
    ++(*this);
    return prev;
  }
  Iterator &operator--() noexcept {
    --argv;
    value = *argv;
    return *this;
  }
  Iterator operator--(int) noexcept {
    auto prev = *this;
    --(*this);
    return prev;
  }

  Iterator &operator+=(const int n) noexcept {
    argv += n;
    argc -= n;
    value = *argv;
  }

  Iterator operator+(const int n) const noexcept {
    Iterator other;
    other += n;
    return other;
  }
  Iterator &operator-=(const int n) noexcept {
    argv -= n;
    argc += n;
    value = *argv;
  }
  Iterator operator-(const int n) const noexcept {
    Iterator other;
    other -= n;
    return other;
  }

  int operator-(const Iterator &other) const noexcept {
    // Order is reversed, because argc decrements.
    return other.argc - argc;
  }
  std::basic_string_view<CharT> operator[](const int n) const noexcept {
    return argv[n];
  }
  std::strong_ordering operator<=>(const Iterator &other) const noexcept {
    // Order is reversed, because argc decrements.
    return other.argc <=> argc;
  }
};
template <typename CharT>
Iterator<CharT> operator+(const int n, const Iterator<CharT> &it) noexcept {
  return it + n;
}

template <typename CharT, std::input_iterator It>
  requires std::convertible_to<std::iter_value_t<It>,
                               std::basic_string_view<CharT>>
class Argv {
public:
  using iterator = Iterator<CharT, It>;

private:
  It begin_;
  It end_;

  std::unordered_set<CharT> short_flags;
  std::unordered_set<CharT> short_value_flags;
  std::unordered_set<std::basic_string_view<CharT>> long_flags;
  std::unordered_set<std::basic_string_view<CharT>> long_value_flags;

public:
  Argv(It begin, It end, std::unordered_set<CharT> short_flags,
       std::unordered_set<CharT> short_value_flags,
       std::unordered_set<std::basic_string_view<CharT>> long_flags,
       std::unordered_set<std::basic_string_view<CharT>>
           long_value_flags) noexcept
      : begin_(begin), end_(end), short_flags(short_flags),
        long_flags(long_flags), short_value_flags(short_value_flags),
        long_value_flags(long_value_flags) {}

  Argv(const std::ranges::range auto &range,
       std::unordered_set<CharT> short_flags,
       std::unordered_set<CharT> short_value_flags,
       std::unordered_set<std::basic_string_view<CharT>> long_flags,
       std::unordered_set<std::basic_string_view<CharT>>
           long_value_flags) noexcept
      : begin_(std::ranges::begin(range)), end_(std::ranges::end(range)),
        short_flags(short_flags), long_flags(long_flags),
        short_value_flags(short_value_flags),
        long_value_flags(long_value_flags) {}

  iterator begin() const noexcept {
    return iterator(begin_, end_, &short_flags, &short_value_flags, &long_flags,
                    &long_value_flags);
  }

  iterator end() const noexcept {
    return iterator(end_, end_, &short_flags, &short_value_flags, &long_flags,
                    &long_value_flags);
  }
};
} // namespace argv
} // namespace args2

namespace std {
template <typename CharT> struct iterator_traits<args2::argv::Iterator<CharT>> {
  using iterator_concept = std::random_access_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::basic_string_view<CharT>;
  using difference_type = int;
  using pointer = const std::basic_string_view<CharT> *;
  using reference = std::basic_string_view<CharT>;
};
} // namespace std
