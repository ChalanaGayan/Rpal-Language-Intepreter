# Makefile for myRpal

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -IOperations

# Source files and object files
SRCS := main.cpp tree.cpp BOP/binaryOP.cpp
OBJS := $(SRCS:.cpp=.o)

# Header files
HDRS := LexicalAnalyzer.h Parser.h CSEMachine.h Token.h TokenController.h TreeNode.h Tree.h BOP/binaryOP.h

# Target executable
TARGET := myrpal

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	rm -f *.o

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Header dependencies
$(OBJS): $(HDRS)

# Clean
clean:
	rm -f *.o myrpal.exe
