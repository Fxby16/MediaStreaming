#include <td/telegram/td_json_client.h>
#include <civetweb.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <mutex>

#include "auth.hpp"
#include "common.hpp"
#include "chats.hpp"
#include "app_data.hpp"

using json = nlohmann::json;

void *client = nullptr;

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

            // Parse the query string to extract parameters
            std::string query_string = req_info->query_string ? req_info->query_string : "";
            std::map<std::string, std::string> params;
            size_t pos = 0;
            while ((pos = query_string.find('&')) != std::string::npos) {
                std::string param = query_string.substr(0, pos);
                size_t eq_pos = param.find('=');
                if (eq_pos != std::string::npos) {
                    params[param.substr(0, eq_pos)] = param.substr(eq_pos + 1);
                }
                query_string.erase(0, pos + 1);
            }
            if (!query_string.empty()) {
                size_t eq_pos = query_string.find('=');
                if (eq_pos != std::string::npos) {
                    params[query_string.substr(0, eq_pos)] = query_string.substr(eq_pos + 1);
                }
            }

            // Extract the file_id parameter
            if (params.find("file_id") != params.end()) {
                file_id = std::stoul(params["file_id"]);
                std::cout << "📥 Received file_id: " << file_id << std::endl;
            } else {
                std::cerr << "❌ Missing file_id parameter in request" << std::endl;
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

            //std::cout << "📥 Range: " << start << "-" << end << std::endl;
            //std::cout << "📥 File ID: " << file_id << std::endl;

            td_send({{"@type", "downloadFile"},
                     {"file_id", file_id},
                     {"priority", 1},
                     {"offset", start},
                     {"limit", end - start + 1},
                     {"synchronous", false}}, client);

            while(true){
                json response = td_recv(client);
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

void set_tdlib_log_file(const std::string &log_file_path)
{
    td_send({{"@type", "setLogStream"},
             {"log_stream", {{"@type", "logStreamFile"}, {"path", log_file_path}, {"max_file_size", 10485760}, // 10 MB
                             {"redirect_stderr", true}}}}, client);

    json r = td_recv(client);
    if (r["@type"] == "ok")
    {
        std::cout << "✅ TDLib logging to file: " << log_file_path << std::endl;
    }
    else
    {
        std::cerr << "❌ Failed to set TDLib log file: " << r.dump(4) << std::endl;
    }
}

int get_files(struct mg_connection *conn, void *data)
{
    const mg_request_info *req_info = mg_get_request_info(conn);

    std::cout << "📩 Received request for files" << std::endl;

    std::vector<Video> videos = get_videos_from_channel(client, "-1002405503349", 0, 100);

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

            if(request_json.contains("directory"))
            {
                std::string directory = request_json["directory"];

                std::cout << "🔑 Authenticating with TDLib...\n";

                td_send({{"@type", "getAuthorizationState"}}, client);

                td_auth_send_parameters(client, API_ID, API_HASH, directory);

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

                td_auth_send_number(client, phone_number);

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

                td_auth_send_code(client, code);

                std::vector<json> chats = get_chats(client, 9223372036854775807, 0, 100);

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

                td_auth_send_password(client, password);

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

    td_send({{"@type", "getAuthorizationState"}}, client);

    json state_json = {{"state", td_auth_get_state(client)}};

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

int main()
{
    client = td_json_client_create();

    if(!client){
        std::cerr << "❌ Failed to create TDLib client\n";
        return 1;
    }

    const char *options[] = {
        "listening_ports", "10000",
        nullptr};

    mg_callbacks callbacks = {};
    mg_context *ctx = mg_start(&callbacks, nullptr, options);

    mg_set_request_handler(ctx, "/get_files", get_files, nullptr);
    mg_set_request_handler(ctx, "/video", handle_video, nullptr);
    mg_set_request_handler(ctx, "/auth", handle_auth, nullptr);
    mg_set_request_handler(ctx, "/auth/get_state", get_state, nullptr);

    std::cout << "📺 Server running at http://localhost:10000\n";
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(60));
}
