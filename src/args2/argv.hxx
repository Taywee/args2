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
template <typename CharT, std::input_iterator It>
  requires std::convertible_to<std::iter_value_t<It>,
                               std::basic_string_view<CharT>>
class Iterator {
private:
  It it;

  // We need the end iterator on hand so we can properly return an ExpectedValue
  // without moving past the end.
  It end;

  const std::unordered_set<CharT> *short_flags;
  const std::unordered_set<CharT> *short_value_flags;
  const std::unordered_set<std::basic_string_view<CharT>> *long_flags;
  const std::unordered_set<std::basic_string_view<CharT>> *long_value_flags;

  // Triggered after a -- is encountered
  bool positional_only = false;

  // If this is not empty, we are in short flags and still might have short flag
  // arguments left to return.
  // When this is not empty, *it still refers to its full set, because it == end
  // always signifies the end, regardless of everything else.
  std::basic_string_view<CharT> short_flags_block;

  std::optional<Result<CharT>> current_item;

  /** Load in the next item to be fetched on a dereference, like a Rust
   * Iterator.
   */
  std::optional<Result<CharT>> next() noexcept {
    constexpr auto positional_separator =
        Separators<CharT>::positional_separator;
    constexpr auto long_prefix = Separators<CharT>::long_prefix;
    constexpr auto short_prefix = Separators<CharT>::short_prefix;
    constexpr auto long_value_separator =
        Separators<CharT>::long_value_separator;

    if (it == end) {
      // Reached the end, clear out the current item.
      // Dereferencing is now UB.
      return std::nullopt;
    } else {
      // Always make sure we only access *it as a string_view.  Don't want to
      // accidentally copy it.
      const std::basic_string_view<CharT> arg = *it;

      if (positional_only) {
        auto output = Positional<CharT>(arg);
        ++it;
        return output;
      } else if (!short_flags_block.empty()) {
        // currently processing short flags.
        const auto flag = short_flags_block.front();

        if (short_flags_block.size() > 1) {
          short_flags_block = short_flags_block.substr(1);
        } else {
          short_flags_block = std::basic_string_view<CharT>{};
          ++it;
        }

        if (short_flags->count(flag)) {
          // no value
          return ShortFlag<CharT>(flag);
        } else if (short_value_flags->count(flag)) {
          if (!short_flags_block.empty()) {
            // need value, attached to short flag block
            auto value = short_flags_block;
            short_flags_block = std::basic_string_view<CharT>{};
            ++it;
            return ShortValueFlag<CharT>(flag, value);
          } else if (it == end) {
            // need value, but no value available
            return ExpectedValueError<CharT>(ShortFlag<CharT>(flag));
          } else {
            // need value, take from next arg
            const std::basic_string_view<CharT> arg = *it;
            ++it;
            return ShortValueFlag<CharT>(flag, arg);
          }
        } else {
          short_flags_block = std::basic_string_view<CharT>{};
          it = end;
          return UnknownFlagError<CharT>(ShortFlag<CharT>(flag));
        }
      } else if (arg == positional_separator) {
        positional_only = true;
        ++it;
        return next();
      } else if (arg.size() > long_prefix.size() &&
                 arg.substr(0, long_prefix.size()) == long_prefix) {

        // Either a long flag or a long flag and a value.
        const auto long_block = arg.substr(long_prefix.size());
        ++it;

        const auto value_separator_position =
            long_block.find(long_value_separator);

        std::basic_string_view<CharT> flag;
        std::optional<std::basic_string_view<CharT>> value;

        if (value_separator_position == long_block.npos) {
          flag = long_block;
        } else {
          flag = long_block.substr(0, value_separator_position);
          value = long_block.substr(value_separator_position +
                                    long_value_separator.size());
        }

        if (long_flags->count(flag)) {
          if (value) {
            // got attached value that we didn't want
            short_flags_block = std::basic_string_view<CharT>{};
            it = end;
            return UnexpectedValueError<CharT>(
                LongValueFlag<CharT>(flag, *value));
          } else {
            // no value
            return LongFlag<CharT>(flag);
          }
        } else if (long_value_flags->count(flag)) {
          if (value) {
            // attached value
            return LongValueFlag<CharT>(flag, *value);
          } else if (it == end) {
            // needed a value but none was attached or available
            return ExpectedValueError<CharT>(LongFlag<CharT>(flag));
          } else {
            // get value from next arg
            const std::basic_string_view<CharT> arg = *it;
            ++it;
            return LongValueFlag<CharT>(flag, arg);
          }
        } else {
          short_flags_block = std::basic_string_view<CharT>{};
          it = end;
          return UnknownFlagError<CharT>(LongFlag<CharT>(flag));
        }
      } else if (arg.size() > short_prefix.size() &&
                 arg.substr(0, short_prefix.size()) == short_prefix) {
        short_flags_block = arg.substr(short_prefix.size());
        return next();
      } else {
        auto output = Positional<CharT>(arg);
        ++it;
        return output;
      }
    }
  }

public:
  Iterator(It it, It end, const std::unordered_set<CharT> *short_flags,
           const std::unordered_set<CharT> *short_value_flags,
           const std::unordered_set<std::basic_string_view<CharT>> *long_flags,
           const std::unordered_set<std::basic_string_view<CharT>>
               *long_value_flags) noexcept
      : it(it), end(end), short_flags(short_flags), long_flags(long_flags),
        short_value_flags(short_value_flags),
        long_value_flags(long_value_flags) {
    current_item = next();
  }
  Iterator() noexcept { current_item = next(); }

  bool operator==(const Iterator &other) const noexcept {
    // Special case for default-constructed end iterator
    return (it == end && other.it == other.end && !current_item &&
            !other.current_item) ||
           (it == other.it && short_flags_block == other.short_flags_block &&
            current_item == other.current_item);
  }
  bool operator!=(const Iterator &other) const noexcept {
    return !(*this == other);
  }

  const Result<CharT> &operator*() const noexcept { return *current_item; }

  Iterator &operator++() noexcept {
    current_item = next();
    return *this;
  }
  Iterator operator++(int) noexcept {
    auto prev = *this;
    current_item = next();
    return prev;
  }
};

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
template <typename CharT, std::input_iterator It>
  requires std::convertible_to<std::iter_value_t<It>,
                               std::basic_string_view<CharT>>
struct iterator_traits<args2::argv::Iterator<CharT, It>> {
  using iterator_concept = std::forward_iterator_tag;
  using iterator_category = std::forward_iterator_tag;
  using value_type = args2::argv::Result<CharT>;
  using difference_type = ssize_t;
  using pointer = const args2::argv::Result<CharT> *;
  using reference = args2::argv::Result<CharT>;
};
} // namespace std
