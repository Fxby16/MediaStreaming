#pragma once

#include "common.hpp"

struct Video
{
    unsigned int file_id;
    std::string mime_type;
    std::string path;

    Video(unsigned int id, const std::string &type, const std::string &p)
        : file_id(id), mime_type(type), path(p){}
    Video() : file_id(0), mime_type(""), path("") {}
};

extern std::vector<json> get_chats(void* client, uint64_t offset_order, uint64_t offset_chat_id, size_t limit);
extern std::vector<Video> get_videos_from_channel(void* client, const std::string &chat_id, int64_t from_message_id, int limit);