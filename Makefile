# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall

# Source files
SRCS = main.cpp message.cpp ber.cpp file.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = isa-ldapserver

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run:
	./$(TARGET) -f ./lidi.csv
