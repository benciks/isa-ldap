# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall

# Include
INCDIR = -I./include

# SRC
SRCDIR = ./src

# Source files
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/message.cpp $(SRCDIR)/ber.cpp $(SRCDIR)/search.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = isa-ldapserver

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf doc/

run:
	./$(TARGET) -f ./resources/lidi.csv

doxygen:
	doxygen Doxyfile

manual:
	pandoc manual.md -o manual.pdf

pack: clean
	tar -cf xbenci01.tar src/ include/ resources/ Makefile README doxyfile manual.md manual.pdf