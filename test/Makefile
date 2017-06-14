# Define comple related values
CXX = clang++
CXXFLAGS = -O2 -g -Wall -std=c++11 -fmessage-length=0
INCLUDES := -I../src/include -I /opt/local/include

# Define linker related values
LD := clang++
LIBS := -L /opt/local/lib -lboost_unit_test_framework-mt
 # -lboost_test_exec_monitor-mt

# Create lists of the elements of our build
SOURCES := $(shell find . -depth 1 -name '*.c*' -print | sort)
OBJECTS := $(SOURCES:.cpp=.o)
TESTS := String Flags
BINS := $(addsuffix .test, $(TESTS))

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCLUDES)

all: $(TESTS)

String: String.o StringTest.o argument.o
	$(LD) -o $@.test $^ $(LIBS)
	chmod 755 $@.test

Flags: Flags.o FlagsTest.o LoggingDummy.o
	$(LD) -o $@.test $^ $(LIBS)
	chmod 755 $@.test

# Run all tests
run:
	for bin in $(BINS) ; do ./$$bin ; done

# Clean up deleting all temporary files
clean:
	rm -f $(BIN) $(OBJECTS)
