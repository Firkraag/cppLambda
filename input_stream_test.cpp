#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "input_stream.hpp"
TEST_CASE("test input stream")
{
    SECTION("test next")
    {
        InputStream input_stream1("");
        REQUIRE(input_stream1.next() == '\0');
        InputStream input_stream2("ab");
        REQUIRE(input_stream2.next() == 'a');
        REQUIRE(input_stream2.next() == 'b');
        REQUIRE(input_stream2.next() == '\0');
        REQUIRE(input_stream2.next() == '\0');
    }
    SECTION("test peek")
    {
        InputStream input_stream3("ab");
        REQUIRE(input_stream3.peek() == 'a');
        input_stream3.next();
        REQUIRE(input_stream3.peek() == 'b');
        input_stream3.next();
        REQUIRE(input_stream3.next() == '\0');
        input_stream3.next();
        REQUIRE(input_stream3.next() == '\0');
    }
    SECTION("test eof")
    {
        InputStream input_stream4("ab");
        REQUIRE(!input_stream4.eof());
        input_stream4.next();
        REQUIRE(!input_stream4.eof());
        input_stream4.next();
        REQUIRE(input_stream4.eof());
        input_stream4.next();
        REQUIRE(input_stream4.eof());
    }
}