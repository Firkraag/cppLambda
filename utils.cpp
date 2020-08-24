#include "utils.hpp"
#include "object.hpp"
#include <string>
using std::string, std::to_string;
static const Double &num(shared_ptr<Object> operand);
static const Double &div(shared_ptr<Object> operand);

const string generate_unique_symbol(const string &name)
{
    static int GENSYM = 0;
    GENSYM++;
    return "$_" + name + to_string(GENSYM);
}

shared_ptr<Object> apply_op(string op, shared_ptr<Object> left, shared_ptr<Object> right)
{
    if (op == "+")
    {
        return num(left) + num(right);
    }
    if (op == "-")
    {
        return num(left) - num(right);
    }
    if (op == "*")
    {
        return num(left) * num(right);
    }
    if (op == "/")
    {
        return num(left) / div(right);
    }
    // if (op == "%") {
    //     return num(left) % div(right);
    // }
    if (op == "&&")
    {
        const Boolean *value = dynamic_cast<const Boolean *>(left.get());
        if (value == nullptr)
        {
            return right;
        }
        else if (*value)
        {
            return right;
        }
        else
        {
            return FALSE;
        }
    }
    if (op == "||")
    {
        const Boolean *value = dynamic_cast<const Boolean *>(left.get());
        if (value == nullptr)
        {
            return left;
        }
        else if (*value) {
            return left;
        }
        else {
            return right;
        }
    }
    if (op == "<")
    {
        return num(left) < num(right);
    }
    if (op == ">")
    {
        return num(left) > num(right);
    }
    if (op == "<=")
    {
        return num(left) <= num(right);
    }
    if (op == "<=")
    {
        return num(left) <= num(right);
    }
    // if (op == "==")
    // {
    //     return left == right;
    // }
    // if (op == "!=")
    // {
    //     return left != right;
    // }
    throw "Can't apply operator " + op;
}
static const Double &num(shared_ptr<Object> operand)
{
    const Double *number = dynamic_cast<const Double *>(operand.get());
    if (number == nullptr)
    {
        throw "Expected double";
    }
    return *number;
}

static const Double &div(shared_ptr<Object> operand)
{
    const Double &value = num(operand);
    if (*(value == Double(0.0)))
    {
        throw "Divide by zero";
    }
    return value;
}