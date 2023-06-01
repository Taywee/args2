#include <algorithm>
#include <args2/parser.hxx>
#include <bits/ranges_algobase.h>
#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <ranges>
#include <vector>

TEST_CASE("Parser can iterate short flags", "[shortflags]") {
  const std::vector<std::string> args{"-abc", "-def"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'c', 'd', 'e', 'f', 'g'}, {}, {}, {});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::ranges::copy(parser, std::back_inserter(collection));
  REQUIRE(collection ==
          std::vector<std::iterator_traits<Iterator>::value_type>{
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('a')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('b')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('c')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('d')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('e')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('f')),
          });
}

TEST_CASE("Parser fails on unknown short flag", "[shortflags]") {
  const std::vector<std::string> args{"-abxc", "-def"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'c', 'd', 'e', 'f', 'g'}, {}, {}, {});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(collection ==
          std::vector<std::iterator_traits<Iterator>::value_type>{
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('a')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('b')),
              args2::parser::Result<char>(args2::parser::UnknownFlagError<char>(
                  args2::parser::ShortFlag<char>('x'))),
          });
}

TEST_CASE("Parser fails short flag needing value but getting none",
          "[shortflags]") {
  const std::vector<std::string> args{"-ab"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a'}, {'b'}, {}, {});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(
      collection ==
      std::vector<std::iterator_traits<Iterator>::value_type>{
          args2::parser::Result<char>(args2::parser::ShortFlag<char>('a')),
          args2::parser::Result<char>(args2::parser::ExpectedValueError<char>(
              args2::parser::ShortFlag<char>('b'))),
      });
}

TEST_CASE("Parser can get values from shortflags", "[shortflags]") {
  const std::vector<std::string> args{"-abc", "-def"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'c'}, {'d'}, {}, {});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(collection ==
          std::vector<std::iterator_traits<Iterator>::value_type>{
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('a')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('b')),
              args2::parser::Result<char>(args2::parser::ShortFlag<char>('c')),
              args2::parser::Result<char>(
                  args2::parser::ShortValueFlag<char>('d', "ef")),
          });
}

TEST_CASE("Parser can parse long flags", "[longflags]") {
  const std::vector<std::string> args{"--alpha", "--beta"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'g'}, {'d'}, {"alpha", "beta", "gamma"},
                {"delta"});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(
      collection ==
      std::vector<std::iterator_traits<Iterator>::value_type>{
          args2::parser::Result<char>(args2::parser::LongFlag<char>("alpha")),
          args2::parser::Result<char>(args2::parser::LongFlag<char>("beta")),
      });
}
TEST_CASE("Parser can parse values from long flags", "[longflags]") {
  const std::vector<std::string> args{"--alpha", "one", "--gamma",
                                      "--beta=two"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'g'}, {'d'}, {"gamma"},
                {"alpha", "beta", "delta"});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(
      collection ==
      std::vector<std::iterator_traits<Iterator>::value_type>{
          args2::parser::Result<char>(
              args2::parser::LongValueFlag<char>("alpha", "one")),
          args2::parser::Result<char>(args2::parser::LongFlag<char>("gamma")),
          args2::parser::Result<char>(
              args2::parser::LongValueFlag<char>("beta", "two")),
      });
}

TEST_CASE("Parser fails on unknown long flag", "[longflags]") {
  const std::vector<std::string> args{"--alpha", "--beta"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'g'}, {'d'}, {"alpha", "gamma"}, {"delta"});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(
      collection ==
      std::vector<std::iterator_traits<Iterator>::value_type>{
          args2::parser::Result<char>(args2::parser::LongFlag<char>("alpha")),
          args2::parser::Result<char>(args2::parser::UnknownFlagError<char>(
              args2::parser::LongFlag<char>("beta"))),
      });
}

TEST_CASE("Parser fails when an unexpected value is present", "[longflags]") {
  const std::vector<std::string> args{"--gamma", "--alpha=one", "--beta=two"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'g'}, {'d'}, {"alpha", "gamma"},
                {"beta", "delta"});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(
      collection ==
      std::vector<std::iterator_traits<Iterator>::value_type>{
          args2::parser::Result<char>(args2::parser::LongFlag<char>("gamma")),
          args2::parser::Result<char>(args2::parser::UnexpectedValueError<char>(
              args2::parser::LongValueFlag<char>("alpha", "one"))),
      });
}

TEST_CASE("Parser fails on missing value", "[longflags]") {
  const std::vector<std::string> args{"--alpha", "--delta"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b', 'g'}, {'d'}, {"alpha", "gamma"}, {"delta"});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
  REQUIRE(
      collection ==
      std::vector<std::iterator_traits<Iterator>::value_type>{
          args2::parser::Result<char>(args2::parser::LongFlag<char>("alpha")),
          args2::parser::Result<char>(args2::parser::ExpectedValueError<char>(
              args2::parser::LongFlag<char>("delta"))),
      });
}

TEST_CASE("Parser can iterate all types in all orders", "[alltypes]") {
  const std::vector<std::string> args{"-acepsilon",  "--alpha", "zeta",
                                      "--gamma=eta", "-d",      "-btheta",
                                      "--delta",     "-abcd",   "iota"};
  using Parser = args2::parser::Parser<char, decltype(args.begin())>;

  Parser parser(args, {'a', 'b'}, {'c', 'd'}, {"alpha", "beta"},
                {"gamma", "delta"});

  using Iterator = Parser::iterator;

  std::vector<std::iterator_traits<Iterator>::value_type> collection;
  std::ranges::copy(parser, std::back_inserter(collection));
  REQUIRE(
      collection ==
      std::vector<std::iterator_traits<Iterator>::value_type>{
          args2::parser::Result<char>(args2::parser::ShortFlag<char>('a')),
          args2::parser::Result<char>(
              args2::parser::ShortValueFlag<char>('c', "epsilon")),
          args2::parser::Result<char>(args2::parser::LongFlag<char>("alpha")),
          args2::parser::Result<char>(args2::parser::Positional<char>("zeta")),
          args2::parser::Result<char>(
              args2::parser::LongValueFlag<char>("gamma", "eta")),
          args2::parser::Result<char>(
              args2::parser::ShortValueFlag<char>('d', "-btheta")),
          args2::parser::Result<char>(
              args2::parser::LongValueFlag<char>("delta", "-abcd")),
          args2::parser::Result<char>(args2::parser::Positional<char>("iota")),
      });
}
