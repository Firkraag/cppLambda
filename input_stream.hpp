#ifndef INPUT_STREAM_H_
#define INPUT_STREAM_H_
#include <iostream>
using namespace std;
class InputStream
{
private:
    int pos;
    int line;
    int col;
    string input;

public:
    InputStream(string input); 
    const string error_msg(const string &msg);
    char next(void);
    char peek(void) const;
    bool eof(void) const;
};
#endif