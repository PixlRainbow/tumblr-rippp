# tumblr-rip++
C++ tumblr downloader
## Note
This program targets C++11.

Also, g++ 4.8 cannot build this program since `<regex>` in g++4.8 is broken.
## Usage
```sh
./tumblr-rip API_KEY someblog.tumblr.com
```
## Building on Linux
```sh
g++ -O3 -std=c++11 tumblr-rip.cpp -o tumblr-rip -ljsoncpp -lcrypto -lssl -pthread
```
## Dependencies
- [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)
- libssl1.1
- ~~zlib1g~~
- [open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp/)
