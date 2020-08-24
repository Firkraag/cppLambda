#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "utils.hpp"
#include "object.hpp"
TEST_CASE("test utils")
{
    SECTION("test generate unique symbol") {
        REQUIRE(generate_unique_symbol("abc") == "$_abc1");
        REQUIRE(generate_unique_symbol("cba") == "$_cba2");
    }
    SECTION("test apply binary operator") {
        apply_op("/", make_shared<Double>(2.0), make_shared<Double>(1.0));
        REQUIRE(*apply_op("&&", FALSE, make_shared<Double>(1.0)) == *FALSE);
        REQUIRE(*apply_op("&&", TRUE, make_shared<Double>(1.0)) == Double(1.0));
        REQUIRE(*apply_op("&&", make_shared<String>(""), make_shared<Double>(1.0)) == Double(1.0));
        REQUIRE(*apply_op("||", FALSE, make_shared<Double>(1.0)) == Double(1.0));
        REQUIRE(*apply_op("||", TRUE, make_shared<Double>(1.0)) == *TRUE);
        REQUIRE(*apply_op("||", make_shared<String>(""), make_shared<Double>(1.0)) == String(""));
    }
    SECTION("test instanceof") {
        class Foo {
            public:
            virtual ~Foo(){};
        };
        class Bar {
            public:
            virtual ~Bar(){};
        };
        Foo f;
        Bar b;
        cout << instanceof <Foo, Foo>(f) << endl;
        cout << instanceof <Foo, Bar>(f) << endl;
        cout << instanceof <Foo, Foo, Bar>(f) << endl;
        cout << instanceof <Foo, Bar, Foo>(f) << endl;
    }
}