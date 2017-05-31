# HTTP1.1
A C++ implementation of HTTP/1.1 that support TLS

# Requirements
- Version of g++ new enough to support C++11
- OpenSSL >= 1.0.2
Note: I have only tested this on Linux


#Usage example:
```c++
#include <iostream>
#include <fstream>
#include <string>
#include "HTTP.hpp"

using namespace std;

int main(const int argc, const char **argv) {
	if(argc != 4) {
		cout << "Usage: " << argv[0] << " host target output" << endl;
		cout << "\tExample: ./get https://www.google.com / index.html" << endl;
		return 0;
	}
	cout << "Connection to host " << argv[1] << "..." << endl;
	Client c(argv[1]);
	c.headers.insert("User-Agent", "Adrian's HTTP client");
	c.headers.insert("Connection", "close");

	cout << "Fetching page " << argv[2] << "..." << endl;
	Reply r = c.get(argv[2]);

	cout << "Status: " << r.status << endl;
	cout << "Responce Headers: " << endl;
	for(auto &h : r.headers) {
		cout << h.first << ": " << h.second << endl;
	}

	cout << "Saving to file " << argv[3] << "..." << endl;
	ofstream output(argv[3], ios::out | ios::binary);
	output.write(r.body.get(), r.length);

	cout << "Done!!!" << endl;
	return 0;
}

```
