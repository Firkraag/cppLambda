#include "input_stream.hpp"
#include <iostream>
using namespace std;
InputStream::InputStream(string input) : input(input)
{
    pos = 0;
    line = 1;
    col = 0;
}
const string InputStream::error_msg(const string &msg)
{
    return msg + " " + to_string(line) + ":" + to_string(col);
}
char InputStream::next(void)
{
    char c;
    try
    {
        c = input.at(pos);
    }
    catch (out_of_range const &exc)
    {
        c = '\0';
    }

    pos++;
    if (c == '\n')
    {
        line++;
        col = 0;
    }
    else
    {
        col++;
    }
    return c;
}
char InputStream::peek(void) const
{
    char c;
    try
    {
        c = input.at(pos);
    }
    catch (out_of_range const &exc)
    {
        c = '\0';
    }
    return c;
}
bool InputStream::eof(void) const
{
    return peek() == '\0';
}