#include "evaluator.hpp"
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
    std::ifstream lambda_source_file(argv[1]);
    std::string code((std::istreambuf_iterator<char>(lambda_source_file)),
                     std::istreambuf_iterator<char>());
    InputStream input_stream(code);
    TokenStream token_stream(input_stream);
    Parser parser(token_stream);
    shared_ptr<Environment> environment = std::make_shared<Environment>();
    add_primitive(*environment);
    auto result = parser()->evaluate(environment);
    // auto result = evaluate(*parser(), environment);
    // auto type_info = result.type();
    // std::cout << result.type() << std::endl;
    std::cout << *result << endl;
    // std::cout << std::any_cast<double>(result) << std::endl;
    return 0;
}
