CXX = g++
#CXXFLAGS = -O2 -g -Wall -std=c++0x

# Strict compiler options
CXXFLAGS += -Werror -Wformat-security -Wignored-qualifiers -Winit-self \
		-Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith \
		-Wtype-limits -Wempty-body -Wlogical-op \
		-Wmissing-field-initializers -Wctor-dtor-privacy \
		-Wnon-virtual-dtor -Wstrict-null-sentinel \
		-Woverloaded-virtual -Wsign-promo -Weffc++ -std=gnu++0x


# Default target (make without specified target).
.DEFAULT_GOAL := all

main.o: main.cpp Instruction.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

Call.o: Call.cpp Call.hpp
	$(CXX) $(CXXFLAGS) -c Call.cpp

Instruction.o: Instruction.cpp Instruction.hpp
	$(CXX) $(CXXFLAGS) -c Instruction.cpp

all: main.o Call.o Instruction.o
	$(CXX) -o shell main.o Call.o Instruction.o $(CXXFLAGS) 

clean:
	rm shell main.o Call.o Instruction.o
