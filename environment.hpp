#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_
#include <map>
#include <memory>
#include <variant>
#include <string>
#include <any>
#include "object.hpp"
using std::any;
using std::enable_shared_from_this;
using std::map;
using std::shared_ptr;
using std::string;
// typedef variant<string, double> EnvValue;
template <typename T>
class EnvironmentBase : public enable_shared_from_this<EnvironmentBase<T>>
{
private:
    map<string, T> vars;
    shared_ptr<EnvironmentBase<T>> parent;
public:
    EnvironmentBase<T>* lookup(const string &var_name);
    EnvironmentBase<T>(shared_ptr<EnvironmentBase<T>> parent = nullptr) : parent(parent){};
    bool is_global(void);
    const EnvironmentBase<T>& get_parent() const;
    shared_ptr<EnvironmentBase<T>> extend();
    T define(const string &var_name, T value);
    T set(const string &name, T value);
    T get(const string &name);
    bool contains(const string &name);
    const map<string, T> & get_vars() const {
        return vars;
    }
};
template <typename T>
bool EnvironmentBase<T>::is_global(void)
{
    return parent == nullptr;
}

template <typename T>
const EnvironmentBase<T> &EnvironmentBase<T>::get_parent() const
{
    return *parent;
}
template <typename T>
shared_ptr<EnvironmentBase<T>> EnvironmentBase<T>::extend()
{
    // EnvironmentBase<T> *environment = new EnvironmentBase<T>(shared_from_this());
    // std::shared_ptr<EnvironmentBase<T>> ptr(environment);

    // return ptr;
    return make_shared<EnvironmentBase<T>>(this->shared_from_this());
    // return make_shared<EnvironmentBase<T>>(shared_from_this());
    // return std::make_shared<EnvironmentBase<T>>(std::make_shared<EnvironmentBase<T>>(this));
}
template <typename T>
EnvironmentBase<T> *EnvironmentBase<T>::lookup(const string &var_name)
{
    auto scope = this;
    while (scope != nullptr)
    {
        auto search = scope->vars.find(var_name);
        if (search != scope->vars.end())
        {
            return scope;
        }
        scope = scope->parent.get();
    }
    return nullptr;
}

template <typename T>
T EnvironmentBase<T>::define(const string &var_name, T value)
{
    vars[var_name] = value;
    return value;
}

template <typename T>
T EnvironmentBase<T>::set(const string &name, T value)
{
    auto environment = lookup(name);
    if (environment != nullptr)
    {
        environment->vars[name] = value;
    }
    else if (is_global())
    {
        vars[name] = value;
    }
    else
    {
        throw "Undefined variable " + name;
    }
    return value;
}
template <typename T>
T EnvironmentBase<T>::get(const string &name)
{
    auto environment = lookup(name);
    if (environment == nullptr)
    {
        throw "Undefined variable " + name;
    }
    return environment->vars[name];
}
template <typename T>
bool EnvironmentBase<T>::contains(const string &name) {
    return vars.find(name) != vars.end();
}
using Environment = EnvironmentBase<shared_ptr<Object>>;
#endif