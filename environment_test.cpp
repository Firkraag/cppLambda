#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include <any>
#include <memory>
#include "environment.hpp"
#include "object.hpp"
using std::make_shared;
TEST_CASE("test environment")
{
    SECTION("test extend")
    {
        shared_ptr<Environment> environment1(new Environment);
        REQUIRE(environment1->is_global());
        auto subscope = environment1->extend();
        REQUIRE(&(subscope->get_parent()) == environment1.get());
        // REQUIRE(subscope->parent.get() == environment1.get());
    }
    SECTION("test define and get")
    {
        shared_ptr<Environment> global_env(new Environment);
        global_env->define("a", make_shared<Double>(1.0));
        REQUIRE(*(global_env->get("a")) == Double(1.0));
        // auto subscope = global_env->extend();
        // subscope->define("b", false);
        // REQUIRE(std::any_cast<double>(subscope->get("a")) == 1.0);
        // REQUIRE(std::any_cast<bool>(subscope->get("b")) == false);
        // REQUIRE_THROWS(subscope->get("c"));
    }
    SECTION("test set")
    {
        // shared_ptr<Environment> global_env(new Environment);
        // global_env->define("a", 1.0);
        // auto subscope = global_env->extend();
        // subscope->define("b", false);
        // subscope->set("b", true);
        // REQUIRE(std::any_cast<bool>(subscope->get("b")) == true);
        // REQUIRE_THROWS(subscope->set("c", "foo"));
        // global_env->set("c", false);
        // REQUIRE(std::any_cast<bool>(global_env->get("c")) == false);
    }
}