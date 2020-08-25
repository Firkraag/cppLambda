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
    Parser parser(new TokenStream(new InputStream(new string(std::move(code)))));
    shared_ptr<Environment> environment = std::make_shared<Environment>();
    // add_primitive(*environment);
    parser()->evaluate_callback(environment, [](shared_ptr<Object> result) {
        cout << *result << endl;
    });
    return 0;
}
