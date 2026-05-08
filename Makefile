CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
TARGET = compiler
SRCS = src/main.cpp src/dfa.cpp src/lexer.cpp src/grammar.cpp src/semantic_ir.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-demo: all
	mkdir -p output
	./compiler dfa input/dfa_in.dfa 4 abba > output/exp1_dfa.txt
	./compiler lexer input/test.src output/tokens.txt > output/exp2_lexer_console.txt
	./compiler lr0 input/grammar_expr.txt output/lr0_items.txt
	./compiler slr input/grammar_expr.txt output/slr_table.txt
	./compiler ir input/test.src output
	./compiler all input/test.src input/grammar_expr.txt output/all || true

clean:
	rm -f $(TARGET) src/*.o
	rm -rf output/*
