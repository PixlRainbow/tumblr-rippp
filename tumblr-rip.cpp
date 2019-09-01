#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <string>
#include <chrono>

constexpr char HOST[15] = "api.tumblr.com";
constexpr char BASE_PATH[10] = "/v2/blog/";
namespace http = httplib;

inline std::string construct_PATH(const char* api_key, const char* blog, int limit = 20, int offset = 0){
    return std::string(BASE_PATH)+blog+"/posts?api_key="+api_key+"&limit="+std::to_string(limit)+"&offset="+std::to_string(offset);
}

int main(int argc, char const *argv[])
{
    if(argc < 3) {
        std::cerr << "usage: tumblr-rip API_KEY TUMBLR_BLOG" << std::endl;
        return 1;
    }

    http::SSLClient cli(HOST);
    Json::StreamWriterBuilder builder;
    Json::Value root;
    auto res = cli.Get(
        construct_PATH(argv[1], argv[2]).c_str()
    );
    if(!res) {
        std::cerr << "Failed to connect" << std::endl;
        return 1;
    }
    
    //std::cerr << "HTTP " << res->status << std::endl;
    //std::cerr << "Body:\n" << res->body << std::endl;
    std::stringstream body;
    body << res->body;
    body >> root;
    //std::cerr << Json::writeString(builder, root) << std::endl;
    if(root["meta"]["status"] == 200){
        fprintf(stderr, "There are %i posts\n", root["response"]["blog"]["posts"].asInt());
        Json::Value posts(root["response"]["posts"]);
        for(int i = 0; i < posts.size(); i++){
            Json::Value post(posts[i]);
            fprintf(
                stderr, "Post %u, at %s\nSummary: %s\n\n",
                post["id"].asUInt64(),
                post["date"].asCString(),
                post["summary"].asCString()
            );
        }
    }
    else{
        std::cerr << Json::writeString(builder, root) << std::endl;
        return 1;
    }
    
    return 0;
}
