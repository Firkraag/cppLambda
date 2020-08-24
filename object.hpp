#ifndef OBJECT_H
#define OBJECT_H
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>
using std::cout;
using std::endl;
using std::function;
using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
class Object
{
private:
    virtual void print(ostream &o) const = 0;

public:
    virtual ~Object() {}
    virtual bool operator==(const Object &o) const = 0;
    virtual operator bool() const;
    virtual shared_ptr<Object> operator()(const vector<shared_ptr<Object>> &args)
    {
        abort();
    };
    virtual void operator()(function<void(shared_ptr<Object>)> callback, const vector<shared_ptr<Object>> &args)
    {
        abort();
    };
    friend ostream &operator<<(ostream &output, const Object &o);
    virtual string to_string() const = 0;
};
class Function : public Object
{
private:
    using FunctionType = function<shared_ptr<Object>(const vector<shared_ptr<Object>> &)>;
    FunctionType value;
    virtual void print(ostream &o) const;

public:
    Function(FunctionType value) : value(value) {}
    virtual bool operator==(const Object &o) const;
    virtual shared_ptr<Object> operator()(const vector<shared_ptr<Object>> &args);
    virtual string to_string() const
    {
        return "function";
    };
};
class FunctionCallback : public Object
{
private:
    using FunctionType = function<void(function<void(shared_ptr<Object>)> callback, const vector<shared_ptr<Object>> &)>;
    FunctionType value;
    virtual void print(ostream &o) const;

public:
    FunctionCallback(FunctionType value) : value(value) {}
    virtual bool operator==(const Object &o) const;
    virtual void operator()(function<void(shared_ptr<Object>)> callback, const vector<shared_ptr<Object>> &args);
    virtual string to_string() const
    {
        return "function_callback";
    };
};
class Boolean : public Object
{
private:
    bool value;
    virtual void print(ostream &o) const;

public:
    Boolean(bool value) : value(value) {}
    virtual bool operator==(const Object &o) const;
    virtual operator bool() const;
    virtual string to_string() const
    {
        return value ? "true" : "false";
    };
    const bool get_value() const {
        return value;
    }
};
class Double : public Object
{
private:
    double value;
    virtual void print(ostream &o) const;

public:
    Double(double value) : value(value) {}
    // bool operator==(double x);
    virtual bool operator==(const Object &o) const;
    shared_ptr<Double> operator+(const Double &x) const;
    shared_ptr<Double> operator-(const Double &x) const;
    shared_ptr<Double> operator*(const Double &x) const;
    shared_ptr<Double> operator/(const Double &x) const;
    shared_ptr<Boolean> operator==(const Double &x) const;
    shared_ptr<Boolean> operator<(const Double &x) const;
    shared_ptr<Boolean> operator>(const Double &x) const;
    shared_ptr<Boolean> operator<=(const Double &x) const;
    shared_ptr<Boolean> operator>=(const Double &x) const;
    const double get_value() const {
        return value;
    }
    virtual string to_string() const
    {
        return std::to_string(value);
    }
};

class String : public Object
{
private:
    string value;
    virtual void print(ostream &o) const;

public:
    String(string value) : value(value) {}
    virtual bool operator==(const Object &o) const;
    virtual string to_string() const {
        return value;
    };
    const string get_value() const {
        return value;
    }
};
extern shared_ptr<Boolean> TRUE;
extern shared_ptr<Boolean> FALSE;
#endif