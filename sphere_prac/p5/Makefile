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

main.o: main.cpp HashTable.hpp Request.hpp Coworker.hpp sock_ops.h map_file.h
	$(CXX) $(CXXFLAGS) -c main.cpp $(BOOSTFLAGS)

HashTable.o: HashTable.cpp HashTable.hpp Semaphore.hpp
	$(CXX) $(CXXFLAGS) -c HashTable.cpp $(BOOSTFLAGS)

Request.o: Request.cpp Request.hpp HashTable.hpp
	$(CXX) $(CXXFLAGS) -c Request.cpp $(BOOSTFLAGS)

Coworker.o: Coworker.cpp Coworker.hpp sock_ops.h
	$(CXX) $(CXXFLAGS) -c Coworker.cpp $(BOOSTFLAGS)

Semaphore.o: Semaphore.cpp Semaphore.hpp
	$(CXX) $(CXXFLAGS) -c Semaphore.cpp $(BOOSTFLAGS)

sock_ops.o: sock_ops.c sock_ops.h
	$(CXX) $(CXXFLAGS) -c sock_ops.c

map_file.o: map_file.c map_file.h
	$(CXX) $(CXXFLAGS) -c map_file.c

all: main.o Request.o HashTable.o Coworker.o Semaphore.o sock_ops.o map_file.o
	$(CXX) -o cache main.o HashTable.o Request.o Coworker.o Semaphore.o sock_ops.o map_file.o $(CXXFLAGS) $(BOOSTFLAGS)

clean:
	rm cache main.o HashTable.o Request.o Coworker.o Semaphore.o sock_ops.o map_file.o

clean_cache:
	dd if=/dev/zero of=cache_data.data bs=10000000 count=1
