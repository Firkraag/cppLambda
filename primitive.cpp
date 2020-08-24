#include "primitive.hpp"
int fib(int number)
{
    if (number < 2)
    {
        return number;
    }
    return fib(number - 1) + fib(number - 2);
}
void add_primitive(Environment &environment) {
    // environment.define("fib", fib);
}