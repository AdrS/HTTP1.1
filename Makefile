CXX = g++
CXXFLAGS = -Wall -Werror -pedantic -std=c++11

TestConnection: TestConnection.o Connection.o
	$(CXX) $(CXXFLAGS) TestConnection.o Connection.o -o TestConnection

TestConnection.o: TestConnection.cpp Connection.hpp
	$(CXX) $(CXXFLAGS) -c TestConnection.cpp

Connection.o: Connection.cpp Connection.hpp
	$(CXX) $(CXXFLAGS) -c Connection.cpp

test: TestConnection
	./TestConnection

clean:
	rm -rf *.o TestConnection
