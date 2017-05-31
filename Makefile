CXX = g++
CXXFLAGS = -Wall -Werror -pedantic -std=c++11 -g
#CXXFLAGS = -Wall -Werror -pedantic -std=c++11 -g -lssl -lcrypto

all: TestHeaderMap TestURL TestConnection TestTcpConnection TestTlsConnection TestBufferedConnection

#TestHTTP: TestHTTP.o HTTP.o Connection.o HeaderMap.o URL.o
#	$(CXX) $(CXXFLAGS) TestHTTP.o HTTP.o Connection.o HeaderMap.o URL.o -o TestHTTP

#TestHTTP.o: TestHTTP.cpp HTTP.hpp HeaderMap.hpp
#	$(CXX) $(CXXFLAGS) -c TestHTTP.cpp

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

TcpConnection.o: TcpConnection.cpp TcpConnection.hpp BaseConnection.hpp
	$(CXX) $(CXXFLAGS) -c TcpConnection.cpp

TestTcpConnection.o: TestTcpConnection.cpp TcpConnection.hpp BaseConnection.hpp
	$(CXX) $(CXXFLAGS) -c TestTcpConnection.cpp

TestTcpConnection: TestTcpConnection.o TcpConnection.o
	$(CXX) $(CXXFLAGS) TestTcpConnection.o TcpConnection.o -o TestTcpConnection

TlsConnection.o: TlsConnection.cpp TlsConnection.hpp BaseConnection.hpp
	$(CXX) $(CXXFLAGS) -c TlsConnection.cpp

TestTlsConnection.o: TestTlsConnection.cpp TlsConnection.hpp BaseConnection.hpp
	$(CXX) $(CXXFLAGS) -c TestTlsConnection.cpp

TestTlsConnection: TestTlsConnection.o TlsConnection.o
	$(CXX) $(CXXFLAGS) TestTlsConnection.o TlsConnection.o -lssl -lcrypto -o TestTlsConnection

BufferedConnection.o: BufferedConnection.cpp BufferedConnection.hpp BaseConnection.hpp TlsConnection.hpp TcpConnection.hpp
	$(CXX) $(CXXFLAGS) -c BufferedConnection.cpp

TestBufferedConnection.o: TestBufferedConnection.cpp BufferedConnection.hpp
	$(CXX) $(CXXFLAGS) -c TestBufferedConnection.cpp

TestBufferedConnection: TestBufferedConnection.o BufferedConnection.o TlsConnection.o TcpConnection.o
	$(CXX) $(CXXFLAGS) TestBufferedConnection.o BufferedConnection.o TlsConnection.o TcpConnection.o -lssl -lcrypto -o TestBufferedConnection

test: TestConnection TestURL TestHTTP TestHeaderMap
	./TestHeaderMap
	#./TestHTTP
	./TestURL
	./TestConnection

example.o: example.cpp HTTP.hpp
	$(CXX) $(CXXFLAGS) -c example.cpp

demo: example.o BufferedConnection.o HTTP.o TcpConnection.o TlsConnection.o URL.o HeaderMap.o
	$(CXX) $(CXXFLAGS) example.o HTTP.o URL.o HeaderMap.o BufferedConnection.o TlsConnection.o TcpConnection.o -lssl -lcrypto -o example

clean:
	rm -rf *.o TestHTTP TestHeaderMap TestURL TestConnection TestTcpConnection TestTlsConnection TestBufferedConnection
