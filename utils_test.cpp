#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "utils.hpp"
TEST_CASE("test utils")
{
    SECTION("test generate unique symbol") {
        REQUIRE(generate_unique_symbol("abc") == "abc1");
        REQUIRE(generate_unique_symbol("cba") == "cba2");
    }
}