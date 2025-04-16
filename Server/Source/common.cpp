#include "common.hpp"

#include <td/telegram/td_json_client.h>

#include <string>
#include <iostream>

void td_send(const json &j, void* client)
{
    if(!client){
        std::cerr << "[ERROR] Client is null. Cannot send message.\n";
        return;
    }

    std::string s = j.dump();

    std::cout << "[MESSAGE] Sending message: " << s << std::endl;

    td_json_client_send(client, s.c_str());
}

json td_recv(void* client)
{
    if(!client){
        std::cerr << "[ERROR] Client is null. Cannot receive message.\n";
        return nullptr;
    }

    const char* res = td_json_client_receive(client, TDLIB_TIMEOUT);
    
    if(!res) return nullptr;
    try{
        json j = json::parse(res);
        //std::cout << "[MESSAGE] Received message: " << j.dump(4) << std::endl;
        return j;
    }catch(const std::exception &e){
        std::cerr << "[ERROR] Failed to parse JSON: " << e.what() << std::endl;
        return nullptr;
    }
    return nullptr;
}