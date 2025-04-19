#include "common.hpp"
#include "chats.hpp"

#include <iostream>

std::vector<json> get_chats(std::shared_ptr<ClientSession> session, uint64_t offset_order, uint64_t offset_chat_id, size_t limit)
{
    session->send({
        {"@type", "getChats"},
        {"offset_order", std::to_string(offset_order)}, 
        {"offset_chat_id", offset_chat_id},
        {"limit", limit}
    });

    std::vector<json> chats;

    std::cout << "[MESSAGE] Sending getChats request" << std::endl;

    uint32_t last_checked = 0;
    while (true){
        auto responses = session->getResponses()->get_all(last_checked);
        
        for(auto r = responses.begin(); r != responses.end(); r++){
            if(r->first <= last_checked){
                continue;
            }

            last_checked = r->first;
            json response = r->second;

            if(!response.is_null()){
                // Check if the response is the expected "chats" type
                if(response.contains("@type") && response["@type"] == "chats"){
                    for (const auto& chat_id : response["chat_ids"]){
                        session->send({{"@type", "getChat"},
                                {"chat_id", chat_id}});

                        std::cout << "[MESSAGE] Sending getChat request for chat_id: " << chat_id << std::endl;

                        size_t null_ctr = 0;

                        bool found = false;
                        while(!found){
                            auto responses_2 = session->getResponses()->get_all(last_checked);

                            for(auto r2 = responses_2.begin(); r2 != responses_2.end(); r2++){
                                if(r2->first <= last_checked){
                                    continue;
                                }

                                last_checked = r2->first;
                                json chat_response = r2->second;

                                if(chat_response.is_null()){
                                    null_ctr++;
                                    if(null_ctr > 5){
                                        std::cerr << "[ERROR] Failed to retrieve chat details for chat_id: " << chat_id << std::endl;
                                        found = true;
                                        break;
                                    }
                                }else{
                                    null_ctr = 0;
                                }

                                if(!chat_response.is_null() && chat_response["@type"] == "updateNewChat"){
                                    chat_response = chat_response["chat"];
                                    chats.push_back(chat_response);
                                    found = true;
                                    break;
                                }else if (!chat_response.is_null() && chat_response["@type"] == "chat"){
                                    chats.push_back(chat_response);
                                    found = true;
                                    break;
                                }else{
                                    std::cerr << "[ERROR][NOT_NULL] Failed to retrieve chat details for chat_id: " << chat_id << std::endl;
                                }
                            }
                        }
                    }
                    return std::move(chats);
                }
            }
        }
    }

    return std::move(chats);
}

std::vector<Video> get_videos_from_channel(std::shared_ptr<ClientSession> session, const std::string &chat_id, int64_t from_message_id, int limit)
{
    std::vector<Video> videos;

    session->send({{"@type", "getChatHistory"},
        {"chat_id", std::stoll(chat_id)},
        {"from_message_id", from_message_id},
        {"offset", 0},
        {"limit", limit},
        {"only_local", false}});

    uint32_t last_checked = 0;
    while (true){
        auto responses = session->getResponses()->get_all(last_checked);
        for(auto r = responses.begin(); r != responses.end(); r++){
            if(r->first <= last_checked){
                continue;
            }

            last_checked = r->first;
            json response = r->second;

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

                            std::cout << "Video id 2: " << file_id << std::endl;

                            videos.push_back(Video(file_id, message["content"]["video"]["mime_type"], message["content"]["video"]["file_name"]));
                        }
                    }

                    std::cout << "Number of messages: " << response["messages"].size() << std::endl;

                    // Se non ci sono più messaggi, esci dal loop
                    if(response["messages"].empty()){
                        return std::move(videos);
                    }

                    // Aggiorna `from_message_id` per continuare a scorrere i messaggi
                    from_message_id = response["messages"].back()["id"];

                    session->send({{"@type", "getChatHistory"},
                                {"chat_id", std::stoll(chat_id)},
                                {"from_message_id", from_message_id},
                                {"offset", 0},
                                {"limit", limit},
                                {"only_local", false}});
                }
            }
        }
    }

    return std::move(videos);
}