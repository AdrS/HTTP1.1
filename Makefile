CXX = g++
CXXFLAGS = -Wall -Werror -pedantic -std=c++11 -g

all: TestHTTP TestHeaderMap TestURL TestConnection

TestHTTP: TestHTTP.o HTTP.o Connection.o HeaderMap.o URL.o
	$(CXX) $(CXXFLAGS) TestHTTP.o HTTP.o Connection.o HeaderMap.o URL.o -o TestHTTP

TestHTTP.o: TestHTTP.cpp HTTP.hpp HeaderMap.hpp
	$(CXX) $(CXXFLAGS) -c TestHTTP.cpp

HTTP.o: HTTP.cpp HTTP.hpp Connection.hpp URL.hpp
	$(CXX) $(CXXFLAGS) -c HTTP.cpp

TestHeaderMap: TestHeaderMap.o HeaderMap.o
	$(CXX) $(CXXFLAGS) TestHeaderMap.o HeaderMap.o -o TestHeaderMap

TestHeaderMap.o: TestHeaderMap.cpp HeaderMap.hpp
	$(CXX) $(CXXFLAGS) -c TestHeaderMap.cpp

HeaderMap.o: HeaderMap.hpp HeaderMap.cpp
	$(CXX) $(CXXFLAGS) -c HeaderMap.cpp

TestURL: TestURL.o URL.o
	$(CXX) $(CXXFLAGS) TestURL.o URL.o -o TestURL

TestURL.o: TestURL.cpp URL.hpp
	$(CXX) $(CXXFLAGS) -c TestURL.cpp

URL.o: URL.hpp URL.cpp
	$(CXX) $(CXXFLAGS) -c URL.cpp

TestConnection: TestConnection.o Connection.o
	$(CXX) $(CXXFLAGS) TestConnection.o Connection.o -o TestConnection

TestConnection.o: TestConnection.cpp Connection.hpp
	$(CXX) $(CXXFLAGS) -c TestConnection.cpp

Connection.o: Connection.cpp Connection.hpp
	$(CXX) $(CXXFLAGS) -c Connection.cpp

test: TestConnection TestURL TestHTTP TestHeaderMap
	./TestHeaderMap
	./TestHTTP
	./TestURL
	./TestConnection

clean:
	rm -rf *.o TestConnection TestURL TestHTTP TestHeaderMap
