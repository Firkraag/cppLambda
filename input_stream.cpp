#include "input_stream.hpp"
#include <iostream>
using namespace std;
const string InputStream::error_msg(const string &msg)
{
    return msg + " " + to_string(line) + ":" + to_string(col);
}
char InputStreamStr::next(void)
{
    char c;
    try
    {
        c = input->at(pos);
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
char InputStreamStr::peek(void)
{
    char c;
    try
    {
        c = input->at(pos);
    }
    catch (out_of_range const &exc)
    {
        c = '\0';
    }
    return c;
}
bool InputStreamStr::eof(void)
{
    return peek() == '\0';
}

char InputStreamStdin::next(void)
{
    auto result = current_char;

    auto c = getchar();
    if (c == EOF)
    {
        current_char = '\0';
    }
    else {
        current_char = c;
    }
    pos++;
    if (current_char == '\n')
    {
        line++;
        col = 0;
    }
    else
    {
        col++;
    }
    return result;
}

char InputStreamStdin::peek(void)
{
    return current_char;
}

bool InputStreamStdin::eof(void)
{
    return current_char == '\0';
}