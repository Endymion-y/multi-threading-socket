CXX = c++
BOOSTPATH = /usr/local/boost_1_63_0/
HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
ifeq ($(SYSTEM),Darwin)
# On Mac
CXXFALGS = -I $(BOOSTPATH) -std=c++11 -stdlib=libc++
else
# On linux
CXXFALGS = -I $(BOOSTPATH) -std=c++11 std=gnu++11 -pthread
endif

all: server client

server: server.cpp
	$(CXX) $(CXXFALGS) server.cpp -o server

client: client.cpp
	$(CXX) $(CXXFALGS) client.cpp -o client

clean:
	rm server client