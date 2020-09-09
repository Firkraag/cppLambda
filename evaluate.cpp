#include "input_stream.hpp"
#include "token_stream.hpp"
#include "parse.hpp"
#include "primitive.hpp"
#include <functional>
#include <memory>
#include <fstream>
#include <streambuf>
int main(int argc, char const *argv[])
{
    shared_ptr<Environment> environment = std::make_shared<Environment>();
    add_primitive(*environment);
    if (argc == 1)
    {
        std::cout << "ready> ";
        Parser parser(new TokenStream(new InputStreamStdin()));
        while (!parser.eof())
        {
            auto ast = parser.parse_expression();
            parser.skip_punc(';');
            auto result = ast->evaluate(environment);
            std::cout << *result << endl;
            std::cout << "ready> ";
        }
    }
    else
    {
        std::ifstream lambda_source_file(argv[1]);
        std::string code((std::istreambuf_iterator<char>(lambda_source_file)),
                         std::istreambuf_iterator<char>());
        auto result = Parser(code)()->evaluate(environment);
        std::cout << *result << endl;
    }
    return 0;
}
