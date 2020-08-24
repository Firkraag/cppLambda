#ifndef UTILS_H_
#define UTILS_H_
#include <string>
#include <any>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "object.hpp"
using std::any;
using std::string;

shared_ptr<Object> apply_op(string op, shared_ptr<Object> left, shared_ptr<Object> right);
const string generate_unique_symbol(const string &name);
template <typename Arg>
bool instanceof(Arg &object) {
    return false;
}
template <typename Arg, typename TypeFirst, typename... Types>
bool instanceof(Arg& object) {
    const TypeFirst *ptr = dynamic_cast<const TypeFirst *>(&object);
    if (ptr != nullptr)
    {
        return true;
    }
    return instanceof <Arg, Types...>(object);
}
template <typename Iter>
std::string join(Iter begin, Iter end, std::string const &separator)
{
    std::ostringstream result;
    if (begin != end)
        result << *begin++;
    while (begin != end)
        result << separator << *begin++;
    return result.str();
}
#endif