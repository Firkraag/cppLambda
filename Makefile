# CC=clang++-10
CC=clang++-10
CFLAGS= --std=c++17 -g -Wall
CXX=$(CC)
CXXFLAGS=$(CFLAGS)
TESTFILES= token_stream_test input_stream_test utils_test parser_test ast_test
EXECUTABLES= $(TESTFILES) test_lambda
# TESTFILES_OBJS=$(TESTFILES:%=%.o)

# TESTFILES_OBJS= token_stream_test.o input_stream_test.o parse_test.o
OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
# OBJS= $($(wildcard *.cpp):%.cpp=%.o)
DEPS=$(patsubst %.cpp,%.d,$(wildcard *.cpp))

all: build
build: $(EXECUTABLES)
test_lambda: test_lambda.o input_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^
input_stream_test: input_stream_test.o input_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^
token_stream_test: token_stream_test.o token_stream.o input_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^
utils_test: utils_test.o utils.o
	$(CXX) $(CXXFLAGS) -o $@ $^
parser_test: parser_test.o utils.o token_stream.o input_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^
$(DEPS): %.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM $(subst .d,.cpp,$@) -MF $@
	sed -i 's,\($*\)\.o[ :]*,\1.o $@ : ,g' $@
test: $(TESTFILES)
	for i in $(TESTFILES);do ./$$i; done
clean:
	rm -rf $(EXECUTABLES) $(OBJS) $(DEPS)
echo:
	echo $(OBJS)
	echo $(DEPS)
.PHONY: all
include $(DEPS)