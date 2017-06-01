# Define comple related values
CXX = clang++
CXXFLAGS = -O2 -g -Wall -std=c++11 -fmessage-length=0
INCLUDES := -I../src/include -I /opt/local/include

# Define linker related values
LD := clang++
LIBS := -L /opt/local/lib -lboost_unit_test_framework-mt
 # -lboost_test_exec_monitor-mt

# Create lists of the elements of our build
SOURCES := $(shell find . -depth 1 -name '*.cpp' -print | sort)
OBJECTS := $(SOURCES:.cpp=.o)
BIN := tests

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCLUDES)

all: $(OBJECTS)
	$(LD) -o $(BIN) $^ $(LIBS)
	chmod 755 $(BIN)

# Run all tests
run:
	./$(BIN)

# Clean up deleting all temporary files
clean:
	rm -f $(BIN) $(OBJECTS)
