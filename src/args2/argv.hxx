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
/** An argv iterator type.
    When passed argc and argv, it will iterate argv as string_views.
    All end Iterators compare equal, even default-constructed ones.
*/
template <typename CharT> class Iterator {
private:
  int argc;
  const CharT **argv;
  std::basic_string_view<CharT> value;

public:
  Iterator(int argc, const CharT **argv) noexcept
      : argc(argc), argv(argv), value(*argv) {}
  Iterator() noexcept : argc(0), argv(nullptr) {}

  const std::basic_string_view<CharT> &operator*() const noexcept {
    return value;
  }

  Iterator &operator++() noexcept {
    --argc;
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
    ++argc;
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

/** An argv range type.
 */
template <typename CharT> class Argv {
public:
  using iterator = Iterator<CharT>;

private:
  int argc;
  const CharT **argv;

public:
  Argv(int argc, const CharT **argv) noexcept : argc(argc), argv(argv) {}

  iterator begin() const noexcept { return iterator(argc, argv); }

  iterator end() const noexcept { return iterator(0, argv + argc); }
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
