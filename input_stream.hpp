#ifndef INPUT_STREAM_H_
#define INPUT_STREAM_H_
#include <iostream>
#include <memory>
using namespace std;
class InputStream
{
protected:
    int pos = 0;
    int line = 1;
    int col = 0;

public:
    virtual ~InputStream(){};
    // get current character and advance to next position
    // when met end of input stream, return '\0'
    virtual char next(void) = 0;
    // get current character
    // when met end of input stream, return '\0'
    virtual char peek(void) = 0;
    virtual bool eof(void) = 0;
    const string error_msg(const string &msg);
};
class InputStreamStr : public InputStream
{
private:
    unique_ptr<string> input;

public:
    InputStreamStr(const char *input) : input(new string(input)) {}
    InputStreamStr(string *input) : input(input) {}
    virtual char next(void);
    virtual char peek(void);
    virtual bool eof(void);
};
class InputStreamStdin : public InputStream
{
private:
    char current_char;

public:
    InputStreamStdin()
    {
        int c;
        if ((c = getchar()) == EOF)
        {
            current_char = '\0';
        }
        else
        {
            current_char = c;
        }
    }
    virtual char next(void);
    virtual char peek(void);
    virtual bool eof(void);
};
#endif