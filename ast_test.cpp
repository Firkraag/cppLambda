#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "ast.hpp"
TEST_CASE("test ast")
{
    SECTION("test equality") {
        NumberAst ast1(1.0);
        NumberAst ast2(1.0);
        REQUIRE(ast1 == ast2);
        BooleanAst ast3(false);
        BooleanAst ast4(false);
        REQUIRE(ast3 == ast4);
        StringAst ast5("abc");
        StringAst ast6("abc");
        REQUIRE(ast5 == ast6);
        NumberAst &ref3 = ast1;
        NumberAst &ref4 = ast2;
        REQUIRE(ref3 == ref4);
        Ast &ref1 = ast1;
        Ast &ref2 = ast2;
        REQUIRE(ref1 == ref2);
    }
}