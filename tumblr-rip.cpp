#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

constexpr char HOST[15] = "api.tumblr.com";
constexpr char BASE_PATH[10] = "/v2/blog/";
constexpr size_t LIMIT = 50;
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
    int retry_count = 0;
    size_t offset = 0;
    for(;;) {
        Json::StreamWriterBuilder builder;
        Json::Value root;
        fprintf(stderr, "Getting %u posts at offset %u\n", LIMIT, offset);
        auto res = cli.Get(
            construct_PATH(argv[1], argv[2], LIMIT, offset).c_str()
        );
        if(!res) {
            std::cerr << "Failed to connect... ";
            if(retry_count < 3) {
                std::cerr << "retrying... attempt " << retry_count++ << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }
            else {
                std::cerr << "giving up" << std::endl;
                return 1;
            }
        }
        std::stringstream body;
        body << res->body;
        body >> root;

        if(root["meta"]["status"] == 200){
            if(offset == 0)
                fprintf(stderr, "There are %i posts\n", root["response"]["blog"]["posts"].asInt());
            Json::Value posts(root["response"]["posts"]);
            if(posts.size() <= 0){
                std::cerr << "Reached end of blog" << std::endl;
                return 0;
            }
            for(int i = 0; i < posts.size(); i++){
                Json::Value post(posts[i]);
                fprintf(
                    stderr, "Post %u, at %s\nSummary: %s\n\n",
                    post["id"].asUInt64(),
                    post["date"].asCString(),
                    post["summary"].asCString()
                );
            }
            offset += LIMIT;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        else{
            std::cerr << Json::writeString(builder, root) << std::endl;
            return 1;
        }
    }
    
    return 0;
}
