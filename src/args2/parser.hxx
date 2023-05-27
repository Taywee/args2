#include <concepts>
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
namespace parser {
/** Similar to a type traits object, statically defines separators for each char
 * type.
 */
template <typename CharT> struct Separators;

using namespace std::literals::string_view_literals;

template <> struct Separators<char> {
  static constexpr std::string_view positional_separator = "--"sv;
  static constexpr std::string_view long_prefix = "--"sv;
  static constexpr std::string_view short_prefix = "-"sv;
  static constexpr std::string_view long_value_separator = "="sv;
};

template <> struct Separators<wchar_t> {
  static constexpr std::wstring_view positional_separator = L"--"sv;
  static constexpr std::wstring_view long_prefix = L"--"sv;
  static constexpr std::wstring_view short_prefix = L"-"sv;
  static constexpr std::wstring_view long_value_separator = L"="sv;
};

template <> struct Separators<char8_t> {
  static constexpr std::u8string_view positional_separator = u8"--"sv;
  static constexpr std::u8string_view long_prefix = u8"--"sv;
  static constexpr std::u8string_view short_prefix = u8"-"sv;
  static constexpr std::u8string_view long_value_separator = u8"="sv;
};

template <> struct Separators<char16_t> {
  static constexpr std::u16string_view positional_separator = u"--"sv;
  static constexpr std::u16string_view long_prefix = u"--"sv;
  static constexpr std::u16string_view short_prefix = u"-"sv;
  static constexpr std::u16string_view long_value_separator = u"="sv;
};

template <> struct Separators<char32_t> {
  static constexpr std::u32string_view positional_separator = U"--"sv;
  static constexpr std::u32string_view long_prefix = U"--"sv;
  static constexpr std::u32string_view short_prefix = U"-"sv;
  static constexpr std::u32string_view long_value_separator = U"="sv;
};

template <typename CharT> struct Literals;

template <typename CharT> struct ShortFlag {
  CharT flag;

  auto operator<=>(const ShortFlag &) const noexcept = default;
};

template <typename CharT>
std::ostream &operator<<(std::basic_ostream<CharT> &os,
                         const ShortFlag<CharT> &value) {
  os << Separators<CharT>::short_prefix << value.flag;
  return os;
}

template <typename CharT> struct LongFlag {
  std::basic_string_view<CharT> flag;

  auto operator<=>(const LongFlag &) const noexcept = default;
};

template <typename CharT>
std::ostream &operator<<(std::basic_ostream<CharT> &os,
                         const LongFlag<CharT> &value) {
  os << Separators<CharT>::long_prefix << value.flag;
  return os;
}

template <typename CharT> struct ShortValueFlag {
  CharT flag;
  std::basic_string_view<CharT> value;

  auto operator<=>(const ShortValueFlag &) const noexcept = default;
};

template <typename CharT>
std::ostream &operator<<(std::basic_ostream<CharT> &os,
                         const ShortValueFlag<CharT> &value) {
  os << Separators<CharT>::short_prefix << value.flag << value.value;
  return os;
}

template <typename CharT> struct LongValueFlag {
  std::basic_string_view<CharT> flag;
  std::basic_string_view<CharT> value;

  auto operator<=>(const LongValueFlag &) const noexcept = default;
};

template <typename CharT>
std::ostream &operator<<(std::basic_ostream<CharT> &os,
                         const LongValueFlag<CharT> &value) {
  os << Separators<CharT>::long_value_prefix << value.flag
     << Separators<CharT>::long_value_separator << value.value;
  return os;
}

template <typename CharT> struct Positional {
  std::basic_string_view<CharT> value;

  auto operator<=>(const Positional &) const noexcept = default;
};

template <typename CharT>
std::ostream &operator<<(std::basic_ostream<CharT> &os,
                         const Positional<CharT> &value) {
  os << value.value;
  return os;
}

template <typename CharT>
using Token =
    std::variant<ShortFlag<CharT>, LongFlag<CharT>, ShortValueFlag<CharT>,
                 LongValueFlag<CharT>, Positional<CharT>>;

// If a value was expected but not received.
template <typename CharT> struct ExpectedValueError {
  std::variant<ShortFlag<CharT>, LongFlag<CharT>> flag;

  auto operator<=>(const ExpectedValueError &) const noexcept = default;
};

// If a value was not expected but we got a long flag with an equal sign.
template <typename CharT> struct UnexpectedValueError {
  std::variant<ShortValueFlag<CharT>, LongValueFlag<CharT>> flag;

  auto operator<=>(const UnexpectedValueError &) const noexcept = default;
};

// Got an unknown flag, don't know whether it wanted a value or not.
template <typename CharT> struct UnknownFlagError {
  std::variant<ShortFlag<CharT>, LongFlag<CharT>> flag;

  auto operator<=>(const UnknownFlagError &) const noexcept = default;
};

template <typename CharT>
using Error =
    std::variant<ExpectedValueError<CharT>, UnexpectedValueError<CharT>,
                 UnknownFlagError<CharT>>;

template <typename CharT>
using Result = std::variant<Token<CharT>, Error<CharT>>;

template <typename CharT, std::input_iterator It>
  requires std::convertible_to<std::iter_value_t<It>,
                               std::basic_string_view<CharT>>
class Iterator

{
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

  bool operator==(const Iterator &other) const noexcept {
    return it == other.it && short_flags_block == other.short_flags_block &&
           current_item == other.current_item;
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
class Parser {
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
  Parser(It begin, It end, std::unordered_set<CharT> short_flags,
         std::unordered_set<CharT> short_value_flags,
         std::unordered_set<std::basic_string_view<CharT>> long_flags,
         std::unordered_set<std::basic_string_view<CharT>>
             long_value_flags) noexcept
      : begin_(begin), end_(end), short_flags(short_flags),
        long_flags(long_flags), short_value_flags(short_value_flags),
        long_value_flags(long_value_flags) {}

  Parser(const std::ranges::range auto &range,
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
} // namespace parser
} // namespace args2

namespace std {
template <typename CharT, std::input_iterator It>
  requires std::convertible_to<std::iter_value_t<It>,
                               std::basic_string_view<CharT>>
struct iterator_traits<args2::parser::Iterator<CharT, It>> {
  using iterator_category = std::forward_iterator_tag;
  using value_type = args2::parser::Result<CharT>;
  using difference_type = void;
  using pointer = const args2::parser::Result<CharT> *;
  using reference = args2::parser::Result<CharT>;
};
} // namespace std
