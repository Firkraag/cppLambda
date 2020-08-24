#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "object.hpp"
#include <memory>
#include <any>
using std::any_cast;
using std::make_shared;
using std::shared_ptr;
TEST_CASE("test object")
{
    SECTION("test object equality")
    {
        shared_ptr<Object> d1 = make_shared<Double>(1.0);
        shared_ptr<Object> d2 = make_shared<Double>(2.0);
        shared_ptr<Object> d3 = make_shared<Double>(1.0);
        REQUIRE(*d1 == *d3);
        REQUIRE_FALSE(*d1 == *d2);
    }
}