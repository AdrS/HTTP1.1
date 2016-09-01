CXX = g++
CXXFLAGS = -Wall -Werror -pedantic -std=c++11 -g

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

test: TestConnection TestURL
	./TestURL
	./TestConnection

clean:
	rm -rf *.o TestConnection TestURL
