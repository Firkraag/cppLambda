#include "object.hpp"
#include <memory>
shared_ptr<Boolean> TRUE = make_shared<Boolean>(true);
shared_ptr<Boolean> FALSE = make_shared<Boolean>(false);
ostream &operator<<(ostream &output, const Object &o)
{
    o.print(output);
    return output;
}
Object::operator bool() const
{
    return true;
}

void Double::print(ostream &o) const
{
    cout << value;
}
bool Double::operator==(const Object &o) const
{
    if (this == &o)
    {
        return true;
    }

    const Double *double2 = dynamic_cast<const Double *>(&o);
    if (double2 == nullptr)
    {
        return false;
    }
    return this->value == double2->value;
}
// bool Double::operator==(double x)
// {
//     return value == x;
// }
// bool Double::operator==(const Object &o) const
// {
//     const Double *d = dynamic_cast<const Double *>(&o);
//     if (d == nullptr)
//     {
//         return false;
//     }
//     return d->value == value;
// }
shared_ptr<Double> Double::operator+(const Double &x) const
{
    return make_shared<Double>(value + x.value);
}
shared_ptr<Double> Double::operator-(const Double &x) const
{
    return make_shared<Double>(value - x.value);
}
shared_ptr<Double> Double::operator*(const Double &x) const
{
    return make_shared<Double>(value * x.value);
}
shared_ptr<Double> Double::operator/(const Double &x) const
{
    return make_shared<Double>(value / x.value);
}
shared_ptr<Boolean> Double::operator==(const Double &x) const
{
    return value == x.value ? TRUE : FALSE;
}
shared_ptr<Boolean> Double::operator<(const Double &x) const
{
    return value < x.value ? TRUE : FALSE;
}
shared_ptr<Boolean> Double::operator>(const Double &x) const
{
    return value > x.value ? TRUE : FALSE;
}
shared_ptr<Boolean> Double::operator<=(const Double &x) const
{
    return value <= x.value ? TRUE : FALSE;
}
shared_ptr<Boolean> Double::operator>=(const Double &x) const
{
    return value >= x.value ? TRUE : FALSE;
}

void Boolean::print(ostream &o) const
{

    cout << (value ? "true" : "false");
}
Boolean::operator bool() const
{
    return value;
}

bool Boolean::operator==(const Object &o) const
{
    if (this == &o)
    {
        return true;
    }

    const Boolean *bool2 = dynamic_cast<const Boolean *>(&o);
    if (bool2 == nullptr)
    {
        return false;
    }
    return this->value == bool2->value;
}
void String::print(ostream &o) const
{
    cout << value;
}
bool String::operator==(const Object &o) const
{
    if (this == &o)
    {
        return true;
    }

    const String *string2 = dynamic_cast<const String *>(&o);
    if (string2 == nullptr)
    {
        return false;
    }
    return this->value == string2->value;
}
void Function::print(ostream &o) const
{
    cout << "function()";
}
bool Function::operator==(const Object &o) const
{
    return false;
}
shared_ptr<Object> Function::operator()(const vector<shared_ptr<Object>> &args)
{
    return value(args);
}

void FunctionCallback::print(ostream &o) const
{
    cout << "functioncallback()";
}
bool FunctionCallback::operator==(const Object &o) const
{
    return false;
}
void FunctionCallback::operator()(function<void(shared_ptr<Object>)> callback, const vector<shared_ptr<Object>> &args)
{
    value(callback, args);
}