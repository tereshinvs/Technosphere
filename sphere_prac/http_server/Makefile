CXX = g++
#CXXFLAGS = -O2 -g -Wall -std=c++0x

# Strict compiler options
CXXFLAGS += -Werror -Wformat-security -Wignored-qualifiers -Winit-self \
		-Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith \
		-Wtype-limits -Wempty-body -Wlogical-op \
		-Wmissing-field-initializers -Wctor-dtor-privacy \
		-Wnon-virtual-dtor -Wstrict-null-sentinel \
		-Woverloaded-virtual -Wsign-promo -Weffc++ -std=gnu++0x

BOOSTFLAGS = -lboost_system -lboost_date_time -lboost_thread -lboost_program_options

# Default target (make without specified target).
.DEFAULT_GOAL := all

all: main.cpp Request.hpp Request.cpp Response.hpp Response.cpp Handler.hpp Handler.cpp test.cpp
	$(CXX) -o http_server main.cpp Request.cpp Response.cpp Handler.cpp $(CXXFLAGS) $(BOOSTFLAGS)

clean:
	rm http_server
