# CC=clang++-10
CC=clang++-10
CFLAGS= --std=c++17 -g -Wall -I/usr/lib/llvm-10/include 
FLAGS=$(shell llvm-config-10 --cxxflags --ldflags --system-libs --libs all)
CXX=$(CC)
CXXFLAGS=$(CFLAGS)
TESTFILES= token_stream_test input_stream_test utils_test parser_test ast_test\
environment_test object_test ast_evaluate_test
EXECUTABLES= $(TESTFILES) evaluate compile evaluate_callback cps_transform lambda llvm_code_gen
LambdaLib= liblambda.a
# TESTFILES_OBJS=$(TESTFILES:%=%.o)

# TESTFILES_OBJS= token_stream_test.o input_stream_test.o parse_test.o
OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
# OBJS= $($(wildcard *.cpp):%.cpp=%.o)
DEPS=$(patsubst %.cpp,%.d,$(wildcard *.cpp))

all: build
build: $(EXECUTABLES) $(LambdaLib)
$(LambdaLib): parse.o token_stream.o input_stream.o ast_evaluate.o ast_to_js.o ast_evaluate_callback.o ast_equality.o ast_to_cps.o ast_make_scope.o ast_optimize.o ast_code_gen.o utils.o object.o
	ar rcs $@ $^
input_stream_test: input_stream_test.o input_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
token_stream_test: token_stream_test.o token_stream.o input_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
utils_test: utils_test.o utils.o object.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
parser_test: parser_test.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
ast_test: ast_test.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
object_test: object_test.o object.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
ast_evaluate_test: ast_evaluate_test.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
cps_transform: cps_transform.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
evaluate: evaluate.o primitive.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
evaluate_callback: evaluate_callback.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
environment_test: environment_test.o object.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
compile: compile.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
lambda: lambda.o optimizer.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10
llvm_code_gen: llvm_code_gen.o $(LambdaLib)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib/llvm-10/lib -lLLVM-10 -rdynamic
$(DEPS): %.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM $(subst .d,.cpp,$@) -MF $@
	sed -i 's,\($*\)\.o[ :]*,\1.o $@ : ,g' $@
test: $(TESTFILES)
	for i in $(TESTFILES);do ./$$i; done
clean:
	rm -rf $(EXECUTABLES) $(OBJS) $(DEPS)
echo:
	for obj in $(EXECUTABLES); do ls -lh ./$$obj; done
flags:
	@echo $(FLAGS)
.PHONY: all
include $(DEPS)