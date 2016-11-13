CXX = g++
#CXXFLAGS = -O2 -g -Wall -std=c++0x

# Strict compiler options
CXXFLAGS += -Werror -Wformat-security -Wignored-qualifiers -Winit-self \
		-Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith \
		-Wtype-limits -Wempty-body -Wlogical-op \
		-Wmissing-field-initializers -Wctor-dtor-privacy \
		-Wnon-virtual-dtor -Wstrict-null-sentinel \
		-Woverloaded-virtual -Wsign-promo -Weffc++ -std=gnu++0x

LDFLAGS = -lconfig++ -ldl

# Default target (make without specified target).
.DEFAULT_GOAL := all

chatsrv: chatsrv.cpp
	$(CXX) $(CXXFLAGS) -o chatsrv chatsrv.cpp

chatclient: chatclient.cpp
	$(CXX) $(CXXFLAGS) -o chatclient chatclient.cpp

all: test.py chatsrv chatclient
	python test.py

clean:
	rm chatclient chatsrv
