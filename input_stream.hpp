#ifndef INPUT_STREAM_H_
#define INPUT_STREAM_H_
#include <iostream>
#include <memory>
using namespace std;
class InputStream
{
private:
    int pos = 0;
    int line = 1;
    int col = 0;
    unique_ptr<string> input;

public:
    InputStream(const char* input) : input(new string(input)) {} 
    InputStream(string *input) : input(input) {} 
    const string error_msg(const string &msg);
    char next(void);
    char peek(void) const;
    bool eof(void) const;
};
#endif