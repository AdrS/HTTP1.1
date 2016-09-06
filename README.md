# HTTP1.1
An implementation of HTTP/1.1 (Written in C++11 for Linux)

Currently a work in progress

Usage example:
```c++
#include <iostream>
#include <fstream>
#include <string>
#include "HTTP.hpp"

using namespace std;

int main(const int argc, const char **argv) {
	if(argc != 4) {
		cout << "Usage: " << argv[0] << " host target output" << endl;
		cout << "\tExample: ./get www.google.com / index.html" << endl;
		return 0;
	}
	cout << "Connection to host " << argv[1] << "..." << endl;
	
	//To create a client all you need is the hostname
	Client c(argv[1]);
	//Add whatever headers you want, except for Host and Content-Length which are automatically sent
	c.headers.insert("User-Agent", "Adrian's HTTP client");

	cout << "Fetching page " << argv[2] << "..." << endl;
	//to issue a get request, just pass a non percent encoded target url
	Reply r = c.get(argv[2]);

	cout << "Status: " << r.status << endl;
	cout << "Responce Headers: " << endl;
	//the headers are returned in a wrapper for a std::map
	for(auto &h : r.headers) {
		cout << h.first << ": " << h.second << endl;
	}

	cout << "Saving to file " << argv[3] << "..." << endl;
	ofstream output(argv[3], ios::out | ios::binary);
	//the body is returned wrapped in a unque_ptr
	output.write(r.body.get(), r.length);

	cout << "Done!!!" << endl;
	return 0;
}

```
