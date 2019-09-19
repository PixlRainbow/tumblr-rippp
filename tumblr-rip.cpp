#include "declares.h"
#include "thread_pool.h"
#include <jsoncpp/json/json.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <regex>
#include <queue>
#include <map>
#include <unordered_map>

inline std::string construct_PATH(const char* api_key, const char* blog, size_t limit = 20, size_t offset = 0){
    return std::string(BASE_PATH)+blog+"/posts?api_key="+api_key+"&limit="+std::to_string(limit)+"&offset="+std::to_string(offset);
}

inline const std::smatch get_URL_parts(std::string &url){
    const auto pat = std::regex(R"s((https?|ftp)://([^/\r\n]+)(/[^\r\n]*)?)s",
        std::regex_constants::icase);
    std::smatch m;
    std::regex_search(url, m, pat);
    return m;
}

void add_download(
        std::string &url,
        const http::Headers &headers,
        std::map<std::string, std::vector<http::Request>> &hosts_downloads,
        std::unordered_map<std::string, protocol> &hosts_downloads_p
    ){
    std::smatch URL_parts = get_URL_parts(url);
    http::Get(hosts_downloads[URL_parts[2].str()], URL_parts[3].str().c_str(), headers);
    hosts_downloads_p[URL_parts[2].str()] = URL_parts[1] == "https" ? HTTPS : HTTP;
}

int main(int argc, char const *argv[])
{
    if(argc < 3) {
        std::cerr << "usage: tumblr-rip API_KEY TUMBLR_BLOG" << std::endl;
        return 1;
    }
    if(mkdir("data", 0770) && errno != EEXIST) {
        std::cerr << "Failed to create data directory" << std::endl;
        return 1;
    }
    if(mkdir("data/posts", 0770) && errno != EEXIST) {
        std::cerr << "Failed to create posts directory" << std::endl;
        return 1;
    }
    if(mkdir("data/assets", 0770) && errno != EEXIST) {
        std::cerr << "Failed to create assets directory" << std::endl;
        return 1;
    }

    http::SSLClient cli(HOST);
    cli.follow_location(true);
    int retry_count = 0;
    size_t offset = 0;
    const std::regex url_pat("((ht|f)tp(s?):\\/\\/|www\\.)"
        "(([\\w\\-]+\\.){1,}?([\\w\\-.~]+\\/?)*"
        "[\\p{Alnum}.,%_=?&#\\-+()\\[\\]\\*$~@!:/{};']*)",
        std::regex_constants::icase | std::regex_constants::optimize);
    const http::Headers headers({
        {"Connection", "Keep-Alive"}
    });
    for(;;) {
        Json::StreamWriterBuilder builder;
        Json::Value root;
        fprintf(stderr, "Getting %lu posts at offset %lu\n", LIMIT, offset);
        auto res = cli.Get(
            construct_PATH(argv[1], argv[2], LIMIT, offset).c_str(),
            headers
        );
        if(!res) {
            std::cerr << "Failed to connect... ";
            if(retry_count < 3) {
                std::cerr << "retrying... attempt " << ++retry_count << std::endl;
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

        std::map<std::string, std::vector<http::Request>> hosts_downloads;
        std::unordered_map<std::string, protocol> hosts_downloads_p;

        if(root["meta"]["status"] == 200){
            if(offset == 0)
                fprintf(stderr, "There are %u posts\n", root["response"]["blog"]["posts"].asUInt());
            Json::Value posts(root["response"]["posts"]);
            if(posts.size() <= 0){
                std::cerr << "Reached end of blog" << std::endl;
                return 0;
            }
            for(auto &post : posts){
                //Json::Value post(posts[i]);
                fprintf(
                    stderr, "Post %lu, at %s\nSummary: %s\n\n",
                    post["id"].asUInt64(),
                    post["date"].asCString(),
                    post["summary"].asCString()
                );

                std::string post_filename = "data/posts/" + std::to_string(post["id"].asUInt64()) + ".json";
                std::ofstream post_file(post_filename.c_str());
                post_file << Json::writeString(builder, post).c_str();
                post_file.close();

                //post images
                if(post.isMember("photos")){
                    Json::Value images(post["photos"]);
                    for(auto &image : images){
                        std::string URL = image["original_size"]["url"].asString();
                        if(URL.empty())
                            continue;
                        add_download(URL, headers, hosts_downloads, hosts_downloads_p);
                    }
                }

                //post videos
                if(post.isMember("video_url")){
                    std::string URL = post["video_url"].asString();
                    add_download(URL, headers, hosts_downloads, hosts_downloads_p);
                }

                //caption embed images
                std::string content;
                if (post.isMember("body"))
                    content = post["body"].asString();
                else if (post.isMember("caption"))
                    content = post["caption"].asString();
                else if (post.isMember("description"))
                    content = post["description"].asString();
                else
                    continue;

                for (auto it = std::sregex_iterator(content.begin(), content.end(), url_pat);
                    it != std::sregex_iterator();
                    ++it)
                {
                    std::smatch match = *it;
                    std::string match_s = match.str();
                    if(http::detail::find_content_type(match_s)){
                        //std::cerr << match_s << std::endl;
                        add_download(match_s,headers,hosts_downloads,hosts_downloads_p);
                    }
                }
            }
            offset += LIMIT;
        }
        else{
            std::cerr << Json::writeString(builder, root) << std::endl;
            return 1;
        }
        for(auto& download_queue : hosts_downloads){
            const std::string &host = download_queue.first;
            std::vector<http::Request> &queue = download_queue.second;
            std::thread(batch_download, hosts_downloads_p[host], host, queue).detach();
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    
    return 0;
}
