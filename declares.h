#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
constexpr char HOST[15] = "api.tumblr.com";
constexpr char BASE_PATH[10] = "/v2/blog/";
constexpr size_t LIMIT = 50;
namespace http = httplib;
enum protocol { HTTPS, HTTP };