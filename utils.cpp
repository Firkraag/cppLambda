#include "utils.hpp"

const string generate_unique_symbol(const string &name) {
    static int GENSYM = 0;
    GENSYM++;
    return name + to_string(GENSYM);
}