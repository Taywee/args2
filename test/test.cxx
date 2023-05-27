#include <args2/parser.hxx>
#include <bits/ranges_algobase.h>
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <algorithm>
#include <ranges>
#include <iterator>

TEST_CASE("Parser can iterate short flags", "[shortflags]") {
    const std::vector<std::string> args{"-abc", "-def"};
    using Parser = args2::parser::Parser<char, decltype(args.begin())>;

    Parser parser(
        args,
        {'a', 'b', 'c', 'd', 'e', 'f', 'g'},
        {},
        {},
        {}
        );

    using Iterator = Parser::iterator;

    std::vector<std::iterator_traits<Iterator>::value_type> collection;
    std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
    REQUIRE(collection == std::vector<std::iterator_traits<Iterator>::value_type>{
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('a')),
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('b')),
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('c')),
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('d')),
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('e')),
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('f')),
    }
    );
}

TEST_CASE("Parser can get values from shortflags", "[shortflags]") {
    const std::vector<std::string> args{"-abc", "-def"};
    using Parser = args2::parser::Parser<char, decltype(args.begin())>;

    Parser parser(
        args,
        {'a', 'b', 'c'},
        {'d'},
        {},
        {}
        );

    using Iterator = Parser::iterator;

    std::vector<std::iterator_traits<Iterator>::value_type> collection;
    std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
    REQUIRE(collection == std::vector<std::iterator_traits<Iterator>::value_type>{
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('a')),
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('b')),
        args2::parser::Result<char>(args2::parser::ShortFlag<char>('c')),
        args2::parser::Result<char>(args2::parser::ShortValueFlag<char>('d', "ef")),
    }
    );
}

TEST_CASE("Parser can parse long flags", "[longflags]") {
    const std::vector<std::string> args{"--alpha", "--beta"};
    using Parser = args2::parser::Parser<char, decltype(args.begin())>;

    Parser parser(
        args,
        {'a', 'b', 'g'},
        {'d'},
        {"alpha", "beta", "gamma"},
        {"delta"}
        );

    using Iterator = Parser::iterator;

    std::vector<std::iterator_traits<Iterator>::value_type> collection;
    std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
    REQUIRE(collection == std::vector<std::iterator_traits<Iterator>::value_type>{
        args2::parser::Result<char>(args2::parser::LongFlag<char>("alpha")),
        args2::parser::Result<char>(args2::parser::LongFlag<char>("beta")),
    }
    );
}
TEST_CASE("Parser can parse values from long flags", "[longflags]") {
    const std::vector<std::string> args{"--alpha", "one", "--gamma", "--beta=two"};
    using Parser = args2::parser::Parser<char, decltype(args.begin())>;

    Parser parser(
        args,
        {'a', 'b', 'g'},
        {'d'},
        {"gamma"},
        {"alpha", "beta", "delta"}
        );

    using Iterator = Parser::iterator;

    std::vector<std::iterator_traits<Iterator>::value_type> collection;
    std::copy(parser.begin(), parser.end(), std::back_inserter(collection));
    REQUIRE(collection == std::vector<std::iterator_traits<Iterator>::value_type>{
        args2::parser::Result<char>(args2::parser::LongValueFlag<char>("alpha", "one")),
        args2::parser::Result<char>(args2::parser::LongFlag<char>("gamma")),
        args2::parser::Result<char>(args2::parser::LongValueFlag<char>("beta", "two")),
    }
    );
}
