#include "common.hpp"

#include <td/telegram/td_json_client.h>

#include <string>
#include <iostream>

void td_send(const json& j, void* client)
{
    if(!client){
        std::cerr << "[ERROR] Client is null. Cannot send message." << std::endl;
        return;
    }

    std::string s = j.dump();
    td_json_client_send(client, s.c_str());
}

json td_recv(void* client)
{
    if(!client){
        std::cerr << "[ERROR] Client is null. Cannot receive message." << std::endl;
        return nullptr;
    }

    const char* res = td_json_client_receive(client, TDLIB_TIMEOUT);
    
    if(!res) return nullptr;

    try{
        json j = json::parse(res);
        return j;
    }catch(const std::exception& e){
        std::cerr << "[ERROR] Failed to parse JSON: " << e.what() << std::endl;
        return nullptr;
    }

    return nullptr;
}

std::map<std::string, std::string> parse_query_string(const std::string& query_string)
{
    std::map<std::string, std::string> params;
    std::string query_string_copy = query_string;
    size_t pos = 0;

    while((pos = query_string_copy.find('&')) != std::string::npos){
        std::string param = query_string_copy.substr(0, pos);
        size_t eq_pos = param.find('=');

        if(eq_pos != std::string::npos){
            params[param.substr(0, eq_pos)] = param.substr(eq_pos + 1);
        }

        query_string_copy.erase(0, pos + 1);
    }

    if(!query_string_copy.empty()){
        size_t eq_pos = query_string_copy.find('=');
        
        if(eq_pos != std::string::npos){
            params[query_string_copy.substr(0, eq_pos)] = query_string_copy.substr(eq_pos + 1);
        }
    }

    return std::move(params);
}
