#include "common.hpp"
#include "chats.hpp"

#include <iostream>

std::vector<json> get_chats(void* client, uint64_t offset_order, uint64_t offset_chat_id, size_t limit)
{
    td_send({
        {"@type", "getChats"},
        {"offset_order", std::to_string(offset_order)}, 
        {"offset_chat_id", offset_chat_id},
        {"limit", limit}
    }, client);

    std::vector<json> chats;

    while (true){
        json response = td_recv(client);
        if(!response.is_null()){
            // Check if the response is the expected "chats" type
            if(response["@type"] == "chats"){
                for (const auto& chat_id : response["chat_ids"]){
                    td_send({{"@type", "getChat"},
                             {"chat_id", chat_id}}, client);

                    size_t null_ctr = 0;

                    while(true){
                        json chat_response = td_recv(client);

                        if(chat_response.is_null()){
                            null_ctr++;
                            if(null_ctr > 5){
                                std::cerr << "[ERROR] Failed to retrieve chat details for chat_id: " << chat_id << std::endl;
                                break;
                            }
                        }else{
                            null_ctr = 0;
                        }

                        if(!chat_response.is_null() && chat_response["@type"] == "updateNewChat"){
                            chat_response = chat_response["chat"];
                            chats.push_back(chat_response);
                            break;
                        }else if (!chat_response.is_null() && chat_response["@type"] == "chat"){
                            chats.push_back(chat_response);
                            break;
                        }else{
                            std::cerr << "[ERROR] Failed to retrieve chat details for chat_id: " << chat_id << std::endl;
                        }
                    }
                }
                break;
            }
        }
    }

    return std::move(chats);
}

std::vector<Video> get_videos_from_channel(void* client, const std::string &chat_id, int64_t from_message_id, int limit)
{
    std::vector<Video> videos;

    td_send({{"@type", "getChatHistory"},
        {"chat_id", std::stoll(chat_id)},
        {"from_message_id", from_message_id},
        {"offset", 0},
        {"limit", limit},
        {"only_local", false}}, client);

    while (true){
        json response = td_recv(client);
        if(!response.is_null() && (response["@type"] == "messages" || response["@type"] == "updateNewMessage" || response["@type"] == "message")){
            // Controlla se il messaggio è un video
            if(response["@type"] == "updateNewMessage"){
                if(response["message"]["content"]["@type"] == "messageVideo"){
                    std::string file_id = response["message"]["content"]["video"]["video"]["id"];

                    std::cout << "Video id: " << file_id << std::endl;

                    videos.push_back(Video(std::stoul(file_id), response["message"]["content"]["video"]["video"]["mime_type"], response["message"]["content"]["video"]["video"]["file_name"]));
                }
            }

            if(response["@type"] == "messages"){
                // Scorri i messaggi e cerca i video
                for(const auto &message : response["messages"]){
                    if(message["content"]["@type"] == "messageVideo"){
                        unsigned int file_id = message["content"]["video"]["video"]["id"];

                        std::cout << "Video id: " << file_id << std::endl;

                        videos.push_back(Video(file_id, message["content"]["video"]["mime_type"], message["content"]["video"]["file_name"]));
                    }
                }

                // Se non ci sono più messaggi, esci dal loop
                if(response["messages"].empty()){
                    break;
                }

                std::cout << "Number of messages: " << response["messages"].size() << std::endl;

                // Aggiorna `from_message_id` per continuare a scorrere i messaggi
                from_message_id = response["messages"].back()["id"];

                td_send({{"@type", "getChatHistory"},
                            {"chat_id", std::stoll(chat_id)},
                            {"from_message_id", from_message_id},
                            {"offset", 0},
                            {"limit", limit},
                            {"only_local", false}}, client);
            }
        }
    }

    return std::move(videos);
}