#include "declares.h"
#include <thread>
#include <queue>
#include <functional>
#include <future>
#include <iostream>

void batch_download(protocol p, std::string host, std::vector<http::Request> requests){
    //http::Client *cli;
    std::unique_ptr<http::Client> cli(
        p == HTTPS 
        ? new http::SSLClient(host.c_str())
        : new http::Client(host.c_str())
    );
    cli->set_keep_alive_max_count(64);
    cli->follow_location(true);

    std::vector<http::Response> responses;
    if(cli->send(requests, responses)){
        const size_t len = requests.size();
        try{
            for (size_t i = 0; i < len; i++){
                const http::Response &res = responses.at(i);
                const http::Request &req = requests.at(i);
                if(res.status != 200)
                    continue;
                std::string filename = "data/assets"
                    + req.path.substr(req.path.rfind('/'));
                std::ofstream asset(filename.c_str(), std::ofstream::binary);
                asset.write(res.body.c_str(), res.body.size());
                asset.close();
                std::cerr << "saved " << filename << std::endl;
            }
        } catch(std::out_of_range){
            return;
        }
    }
}