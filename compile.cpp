#include "ast.hpp"
#include "input_stream.hpp"
#include "token_stream.hpp"
#include "parse.hpp"
#include <fstream>
#include <string>
#include <memory>

using std::make_shared;
using std::shared_ptr;

int main(int argc, char const *argv[])
{
    
    std::ifstream lambda_source_file(argv[1]);
    std::string code((std::istreambuf_iterator<char>(lambda_source_file)),
                     std::istreambuf_iterator<char>());
    InputStream input_stream(code);
    TokenStream token_stream(input_stream);
    Parser parser(token_stream);
    auto result = parser()->to_js();
    cout << result << endl;
    return 0;
}
