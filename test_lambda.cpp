// #include "input_stream.hpp"
#include "token_stream.hpp"
// #include "parse.hpp"

TokenStream *foo(void) {
    InputStream input_stream("abc");
    // TokenStream token_stream(input_stream);
    TokenStream *ts = new TokenStream(input_stream);
    cout << &(ts->input_stream) << endl;
    return ts;
}
int main(int argc, char const *argv[])
{
    TokenStream *token_stream = foo();
    token_stream->input_stream.next();
    cout << &(token_stream->input_stream) << endl;
    return 0;
}
