#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "ast.hpp"
#include "input_stream.hpp"
#include "token_stream.hpp"
#include "parse.hpp"
#include <memory>
#include <functional>
#include <any>
#include <string>
using std::any_cast;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
TEST_CASE("test evaluate")
{
    auto code1 = "(lambda foo(n) if n > 0 then n + foo(n - 1) else 0)(10);";
    auto result1 = Parser(code1)()->evaluate(std::make_shared<Environment>());
    CHECK(*result1 == Double(55.0));
    auto code2 = "let (x = 2, y = x * 2, z = x + y) x + y + z";
    auto result2 = Parser(code2)()->evaluate(std::make_shared<Environment>());
    CHECK(*result2 == Double(12));
    auto code3 = "let (x = 3, x = x * 3) x";
    auto result3 = Parser(code3)()->evaluate(std::make_shared<Environment>());
    CHECK(*result3 == Double(9));
    // variable foo is defined in the lambda scope, not in the scope enclosing the lambda
    auto code4 = "lambda foo(x) x * x; foo(10)";
    CHECK_THROWS(Parser(code4)()->evaluate(std::make_shared<Environment>()));
}