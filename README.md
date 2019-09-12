# tumblr-rip++
C++ tumblr downloader
## Note
This program targets C++11.

Also, g++ 4.8 cannot build this program since `<regex>` in g++4.8 is broken.
## Usage
```sh
./tumblr-rip API_KEY someblog.tumblr.com
```
Make sure that you have the run-time dependencies installed first. `libssl1.1` may be replaced by other packages as long as they are compatible with OpenSSL 1.1.0 and above.
```sh
apt-get install libjsoncpp1 libssl1.1
```
## Building on Linux
```sh
g++ -O3 -std=c++11 tumblr-rip.cpp -o tumblr-rip -ljsoncpp -lcrypto -lssl -pthread
```
Make sure you have build-time dependencies installed first. `libssl-dev` may be replaced by other packages as long as they are compatible with OpenSSL 1.1.0 and above.
```sh
apt-get install libssl-dev libjsoncpp-dev
```
`httplib.h` is not in standard repos, so we have to download it seperately. You don't necessarily have to use git.

The following example demonstrates creating a symbolic link in `/usr/include` so that the file can be found by the compiler, but there are other ways to add it as an included file.
```sh
git clone https://github.com/yhirose/cpp-httplib.git && ln -s cpp-httplib/httplib.h /usr/include
```
## Dependencies
- [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)
- libssl1.1
- ~~zlib1g~~
- [open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp/)
