#include "endpoints.hpp"
#include "auth.hpp"
#include "common.hpp"
#include "chats.hpp"
#include "app_data.hpp"
#include "session.hpp"

#include <civetweb.h>
#include <iostream>
#include <fstream>

void setup_endpoints()
{
    const char *options[] = {
        "listening_ports", "10000",
        nullptr};

    mg_callbacks callbacks = {};
    mg_context *ctx = mg_start(&callbacks, nullptr, options);

    mg_set_request_handler(ctx, "/get_files", get_files, nullptr);
    mg_set_request_handler(ctx, "/video", handle_video, nullptr);
    mg_set_request_handler(ctx, "/auth", handle_auth, nullptr);
    mg_set_request_handler(ctx, "/auth/get_state", get_state, nullptr);
}

int handle_video(struct mg_connection *conn, void *)
{
    const mg_request_info *req_info = mg_get_request_info(conn);

    if (req_info->request_method && std::string(req_info->request_method) == "GET")
    {
        const char *range_header = mg_get_header(conn, "Range");
        if (range_header)
        {
            size_t start, end;
            unsigned int file_id = 0;
            uint32_t session_id = 0;

            // Parse the query string to extract parameters
            std::string query_string = req_info->query_string ? req_info->query_string : "";
            std::map<std::string, std::string> params = parse_query_string(query_string);

            // Extract the file_id parameter
            if(params.find("file_id") != params.end()){
                file_id = std::stoul(params["file_id"]);
            }else{
                std::cerr << "❌ Missing file_id parameter in request" << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
                return 400;
            }

            // Extract the session_id parameter
            if(params.find("session_id") != params.end()){
                session_id = std::stoul(params["session_id"]);
            }else{
                std::cerr << "❌ Missing session_id parameter in request" << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
                return 400;
            }

            // Extract the range values
            int matched = sscanf(range_header, "bytes=%zu-%zu", &start, &end);
            if (matched == 1){
                end = start + 1024 * 1024 - 1; // Default to 1 MB if only start is provided
            }else if (matched != 2)
            {
                std::cerr << "❌ Invalid Range header format" << std::endl;

                std::cout << "📥 Range Header: " << range_header << std::endl;

                mg_printf(conn, "HTTP/1.1 416 Range Not Satisfiable\r\n\r\n");
                return 416;
            }

            std::shared_ptr<ClientSession> session = getSession(session_id);

            session->send({{"@type", "downloadFile"},
                     {"file_id", file_id},
                     {"priority", 1},
                     {"offset", start},
                     {"limit", end - start + 1},
                     {"synchronous", false}});

            //{{file_id, session_id}, response_id}
            static std::map<std::pair<unsigned int, uint32_t>,uint32_t> last_checked_map;

            std::pair<unsigned int, uint32_t> key = std::make_pair(file_id, session_id);

            bool found = false;
            while(!found){
                auto responses = session->getResponses()->get_all();

                for(auto r = responses.begin(); r != responses.end(); ++r){
                    if(r->first <= last_checked_map[key]){
                        continue;
                    }

                    last_checked_map[key] = r->first;

                    json response = r->second;
                    if (!response.is_null() && response["@type"] == "updateFile")
                    {
                        if (response["file"]["id"] == file_id && response["file"]["local"]["is_downloading_active"] == false)
                        {
                            size_t available_start = response["file"]["local"]["download_offset"];
                            size_t available_size = response["file"]["local"]["downloaded_size"];
                            size_t available_end = available_start + available_size - 1;

                            //std::cout << "Downloaded range: " << available_start << "-" << available_end << std::endl;

                            // Verifica che il range richiesto sia disponibile
                            if (start >= available_start && end <= available_end) {
                                std::string file_path = response["file"]["local"]["path"];
                                int file_size = response["file"]["expected_size"];
                                std::ifstream file(file_path, std::ios::binary);
                                if (file) {
                                    size_t length = end - start + 1;
                                    mg_printf(conn, "HTTP/1.1 206 Partial Content\r\n");
                                    mg_printf(conn, "Content-Type: video/mp4\r\n");
                                    mg_printf(conn, "Accept-Ranges: bytes\r\n");
                                    mg_printf(conn, "Content-Length: %zu\r\n", length);
                                    mg_printf(conn, "Content-Range: bytes %zu-%zu/%d\r\n", start, end, file_size);
                                    mg_printf(conn, "\r\n");

                                    file.seekg(start);
                                    std::vector<char> buffer(length);
                                    file.read(buffer.data(), length);
                                    mg_write(conn, buffer.data(), length);
                                    file.close();

                                    //std::cout << "📤 Sent video file: " << file_path << std::endl;
                                    //std::cout << "Content-Length: " << length << std::endl;
                                    //std::cout << "Content-Range: bytes " << start << "-" << end << "/" << file_size << std::endl;
                                    return 200;
                                }
                                else
                                {
                                    std::cerr << "❌ Failed to open file: " << file_path << std::endl;
                                    mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
                                    return 500;
                                }
                            }else{
                                //std::cout << response.dump(4) << std::endl;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            std::cerr << "❌ Unsupported request method: " << (req_info->request_method ? req_info->request_method : "null") << std::endl;
            mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
            return 405;
        }
    }
    else
    {
        std::cerr << "❌ Unsupported request method: " << (req_info->request_method ? req_info->request_method : "null") << std::endl;
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return 405;
    }
    return 200;
}

int get_files(struct mg_connection *conn, void *data)
{
    const mg_request_info *req_info = mg_get_request_info(conn);

    std::cout << "📩 Received request for files" << std::endl;

    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);
    uint32_t session_id = 0;

    // Extract the session_id parameter
    if(params.find("session_id") != params.end()){
        session_id = std::stoul(params["session_id"]);
    }else{
        std::cerr << "❌ Missing session_id parameter in request" << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return 400;
    }

    std::shared_ptr<ClientSession> session = getSession(session_id);

    std::vector<Video> videos = get_videos_from_channel(session, "-1002405503349", 0, 100);

    json files_json;
    for (const auto &video : videos)
    {
        std::cout << "📩 Found video with file_id: " << video.file_id << std::endl;
        files_json.push_back({{"id", video.file_id},
                              {"mime_type", video.mime_type},
                              {"path", video.path}});
    }

    std::string json_str = files_json.dump();
    std::cout << "✅ JSON inviato: " << json_str << std::endl;
    std::cout << "📦 Lunghezza JSON: " << json_str.size() << std::endl;

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "\r\n");

    mg_write(conn, json_str.c_str(), json_str.size());

    return 200;
}

int handle_auth(struct mg_connection *conn, void *data)
{
    const mg_request_info *req_info = mg_get_request_info(conn);

    if (req_info->request_method && std::string(req_info->request_method) == "POST")
    {
        char post_data[1024];
        int post_data_len = mg_read(conn, post_data, sizeof(post_data) - 1);
        if (post_data_len > 0)
        {
            post_data[post_data_len] = '\0'; // Null-terminate the data

            std::cout << "📩 Received POST data: " << post_data << std::endl;

            json request_json = json::parse(post_data);

            std::cout << "Parsing JSON: " << request_json.dump(4) << std::endl;

            if(!request_json.contains("session_id"))
            {
                std::cerr << "❌ Missing session_id in JSON" << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
                return 400;
            }

            uint32_t session_id = request_json["session_id"];
            std::shared_ptr<ClientSession> session = getSession(session_id);

            if(request_json.contains("directory"))
            {
                std::string directory = request_json["directory"];

                std::cout << "🔑 Authenticating with TDLib...\n";

                session->send({{"@type", "getAuthorizationState"}});

                td_auth_send_parameters(session, API_ID, API_HASH, directory);

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"}};
                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }else if(request_json.contains("phone_number"))
            {
                std::string phone_number = request_json["phone_number"];
                std::cout << "📞 Phone number: " << phone_number << std::endl;

                td_auth_send_number(session, phone_number);

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"}};
                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }
            else if(request_json.contains("code"))
            {
                std::string code = request_json["code"];
                std::cout << "🔑 Code: " << code << std::endl;

                td_auth_send_code(session, code);

                std::vector<json> chats = get_chats(session, 9223372036854775807, 0, 100);

                for(const auto &chat : chats)
                {
                    std::cout << "📩 Found chat with ID: " << chat["id"] << std::endl;
                }

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"}};
                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }
            else if(request_json.contains("password"))
            {
                std::string password = request_json["password"];
                std::cout << "🔑 Password: " << password << std::endl;

                td_auth_send_password(session, password);

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"}};
                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }
            
            else
            {
                std::cerr << "❌ Missing required fields in JSON" << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
                return 400;
            }
        }
        else
        {
            std::cerr << "❌ No POST data received" << std::endl;
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
            return 400;
        }
    }
    else
    {
        std::cerr << "❌ Unsupported request method: " << (req_info->request_method ? req_info->request_method : "null") << std::endl;
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return 405;
    }
}

int get_state(struct mg_connection *conn, void *data)
{
    const mg_request_info *req_info = mg_get_request_info(conn);

    std::cout << "📩 Received request for state" << std::endl;

    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    uint32_t session_id = 0;

    // Extract the session_id parameter
    if(params.find("session_id") != params.end()){
        session_id = std::stoul(params["session_id"]);
    }else{
        std::cerr << "❌ Missing session_id parameter in request" << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return 400;
    }

    std::shared_ptr<ClientSession> session = getSession(session_id);

    session->send({{"@type", "getAuthorizationState"}});

    json state_json = {{"state", td_auth_get_state(session)}};

    std::string json_str = state_json.dump();
    std::cout << "✅ JSON inviato: " << json_str << std::endl;
    std::cout << "📦 Lunghezza JSON: " << json_str.size() << std::endl;

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "\r\n");

    mg_write(conn, json_str.c_str(), json_str.size());

    return 200;
}