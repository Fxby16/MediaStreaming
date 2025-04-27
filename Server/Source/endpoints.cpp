#include "endpoints.hpp"
#include "auth.hpp"
#include "common.hpp"
#include "chats.hpp"
#include "app_data.hpp"
#include "session.hpp"
#include "random.hpp"
#include "tmdb.hpp"
#include "db.hpp"

#include <civetweb.h>
#include <iostream>
#include <fstream>
#include <algorithm>

void setup_endpoints()
{
    const char* options[] = {
        "listening_ports", "10000",
        nullptr 
    };

    mg_callbacks callbacks = {};
    mg_context *ctx = mg_start(&callbacks, nullptr, options);

    //telegram
    mg_set_request_handler(ctx, "/get_files", get_files, nullptr);
    mg_set_request_handler(ctx, "/video", handle_video, nullptr);
    mg_set_request_handler(ctx, "/auth", handle_auth, nullptr);
    mg_set_request_handler(ctx, "/auth/get_state", get_state, nullptr);
    mg_set_request_handler(ctx, "/logout", logout, nullptr);
    mg_set_request_handler(ctx, "/get_chats", handle_chats, nullptr);

    //tmdb
    mg_set_request_handler(ctx, "/search_movie", search_movie_handler, nullptr);
    mg_set_request_handler(ctx, "/search_tv_show", search_tv_show_handler, nullptr);
    mg_set_request_handler(ctx, "/get_movie_details", get_movie_details_handler, nullptr);
    mg_set_request_handler(ctx, "/get_tv_show_details", get_tv_show_details_handler, nullptr);
    mg_set_request_handler(ctx, "/get_tv_show_season", get_tv_show_season_handler, nullptr);
    mg_set_request_handler(ctx, "/get_movie_cast", get_movie_cast_handler, nullptr);
    mg_set_request_handler(ctx, "/get_tv_show_cast", get_tv_show_cast_handler, nullptr);
    mg_set_request_handler(ctx, "/get_person_details", get_person_details_handler, nullptr);

    mg_set_request_handler(ctx, "/set_movie_data", set_movie_data_handler, nullptr);
    mg_set_request_handler(ctx, "/set_tv_show_data", set_tv_show_data_handler, nullptr);

    mg_set_request_handler(ctx, "/get_videos_data", get_videos_data_handler, nullptr);

    add_genres("it");
}

void add_genres(const std::string& language)
{
    json result = db_select("SELECT * FROM genres");
    if(!result.empty()){
        std::cout << "Genres already present in the database." << std::endl;
        return;
    }

    json genres = get_movie_genres(language);
    json genres_tv = get_tv_show_genres(language);

    for(const auto& genre : genres["genres"]){
        db_execute("INSERT IGNORE INTO genres (genre_id, name) VALUES (" + std::to_string(genre["id"].get<int>()) + ", '" + genre["name"].get<std::string>() + "')");
    }

    for(const auto& genre : genres_tv["genres"]){
        db_execute("INSERT IGNORE INTO genres (genre_id, name) VALUES (" + std::to_string(genre["id"].get<int>()) + ", '" + genre["name"].get<std::string>() + "')");
    }
}

int handle_chats(struct mg_connection* conn, void*)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    uint32_t session_id = 0;

    // Extract the session_id parameter
    if(params.find("session_id") != params.end()){
        session_id = std::stoul(params["session_id"]);
    }else{
        std::cerr << "[ERROR] Missing session_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    std::vector<json> chats = get_chats(getSession(session_id));
    std::string json_str = json(chats).dump();

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");
    mg_write(conn, json_str.c_str(), json_str.size());
    return 200;
}

int handle_video(struct mg_connection* conn, void *)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    const char* range_header = mg_get_header(conn, "Range");
    
    if(range_header){
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
            std::cerr << "[ERROR] Missing file_id parameter in request." << std::endl;
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 400;
        }

        // Extract the session_id parameter
        if(params.find("session_id") != params.end()){
            session_id = std::stoul(params["session_id"]);
        }else{
            std::cerr << "[ERROR] Missing session_id parameter in request." << std::endl;
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 400;
        }

        // Extract the range values
        int matched = sscanf(range_header, "bytes=%zu-%zu", &start, &end);

        if(matched == 1){ // If only start is provided
            end = start + 1024 * 1024 - 1; // Default to 1 MB range
        }else if(matched != 2){ // If the format is incorrect
            std::cerr << "❌ Invalid Range header format" << std::endl;
            std::cerr << "📥 Range Header: " << range_header << std::endl;

            mg_printf(conn, "HTTP/1.1 416 Range Not Satisfiable\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 416;
        }

        std::shared_ptr<ClientSession> session = getSession(session_id);

        // Request chunk from TDLib
        session->send({{"@type", "downloadFile"},
                    {"file_id", file_id},
                    {"priority", 1},
                    {"offset", start},
                    {"limit", end - start + 1},
                    {"synchronous", false}});

        //{{file_id, session_id}, response_id}
        static std::map<std::pair<unsigned int, uint32_t>,uint32_t> last_checked_map;

        std::pair<unsigned int, uint32_t> key = std::make_pair(file_id, session_id);

        while(true){
            // Get new responses
            auto responses = session->getResponses()->get_all(last_checked_map[key]);

            for(auto r = responses.begin(); r != responses.end(); r++){
                last_checked_map[key] = r->first; // Save last checked

                json response = r->second;
                if(!response.is_null() && response["@type"] == "updateFile"){
                    // If it's the requested chunk and it has been downloaded
                    if(response["file"]["id"] == file_id && response["file"]["local"]["is_downloading_active"] == false){
                        size_t available_start = response["file"]["local"]["download_offset"];
                        size_t available_size = response["file"]["local"]["downloaded_size"];
                        size_t available_end = available_start + available_size - 1;

                        // Handle file smaller than the requested range
                        end = std::min(std::min(end, (size_t) response["file"]["expected_size"] - 1), available_end);

                        std::string file_path = response["file"]["local"]["path"];
                        int file_size = response["file"]["expected_size"];
                        std::ifstream file(file_path, std::ios::binary);

                        if(file){
                            size_t length = end - start + 1;
                            mg_printf(conn, "HTTP/1.1 206 Partial Content\r\n");
                            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
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

                            return 200;
                        }else{
                            std::cerr << "[ERROR] Failed to open file: " << file_path << std::endl;
                            mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n");
                            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                            mg_printf(conn, "\r\n");
                            return 500;
                        }
                    }
                }
            }
        }
    }else{
        std::cerr << "[ERROR] Range header not found" << std::endl;
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 405;
    }

    return 200;
}

int get_files(struct mg_connection* conn, void* data)
{
    const mg_request_info *req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    uint32_t session_id = 0;
    std::string chat_id;

    // Extract the session_id parameter
    if(params.find("session_id") != params.end()){
        session_id = std::stoul(params["session_id"]);
    }else{
        std::cerr << "[ERROR] Missing session_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the chat_id parameter
    if(params.find("chat_id") != params.end()){
        chat_id = params["chat_id"];
    }else{
        std::cerr << "[ERROR] Missing chat_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    std::cout << "Chat ID: " << chat_id << std::endl;

    std::shared_ptr<ClientSession> session = getSession(session_id);
    std::vector<json> videos = get_videos_from_channel(session, chat_id, 0, 100);

    json files_json;
    for(const auto &video : videos){
        files_json.push_back(video);
    }

    std::string json_str = files_json.dump();

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");

    mg_write(conn, json_str.c_str(), json_str.size());

    return 200;
}

int handle_auth(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    if(req_info->request_method && std::string(req_info->request_method) == "POST"){
        char post_data[1024];
        int post_data_len = mg_read(conn, post_data, sizeof(post_data) - 1);

        if(post_data_len > 0){
            post_data[post_data_len] = '\0'; // Null-terminate the data

            json request_json = json::parse(post_data);

            if(request_json.contains("start_auth")){ // Initiate authentication process
                uint32_t session_id = 0;

                if(request_json.contains("session_id") && request_json["session_id"] != 0){ // Client already logged in
                    session_id = request_json["session_id"];
                }else{ // New client, generate session id
                    session_id = rand_uint32();
                }

                std::shared_ptr<ClientSession> session = getSession(session_id);
                std::string directory = std::to_string(session_id);

                session->send({{"@type", "getAuthorizationState"}});
                td_auth_send_parameters(session, APP_API_ID, APP_API_HASH, directory);

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"},
                                      {"session_id", session_id}}; // Send session id to client for future requests

                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }

            // To continue the authentication, the client must have a session open
            if(!request_json.contains("session_id")){
                std::cerr << "[ERROR] Missing session_id in JSON." << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 400;
            }

            uint32_t session_id = request_json["session_id"];
            std::shared_ptr<ClientSession> session = getSession(session_id);
            
            if(request_json.contains("phone_number")){
                std::string phone_number = request_json["phone_number"];

                td_auth_send_number(session, phone_number);

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"}};
                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }else if(request_json.contains("code")){
                std::string code = request_json["code"];

                td_auth_send_code(session, code);

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"}};
                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }else if(request_json.contains("password")){
                std::string password = request_json["password"];

                td_auth_send_password(session, password);

                mg_printf(conn, "HTTP/1.1 200 OK\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "Content-Type: application/json\r\n");
                mg_printf(conn, "\r\n");

                json response_json = {{"status", "success"}};
                std::string response_str = response_json.dump();
                mg_write(conn, response_str.c_str(), response_str.size());

                return 200;
            }else{
                std::cerr << "[ERROR] Missing required fields in JSON." << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 400;
            }
        }else{
            std::cerr << "[ERROR] No POST data received." << std::endl;
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 400;
        }
    }else{
        std::cerr << "[ERROR] Unsupported request method: " << (req_info->request_method ? req_info->request_method : "null") << std::endl;
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 405;
    }
}

int get_state(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    uint32_t session_id = 0;

    // Extract the session_id parameter
    if(params.find("session_id") != params.end()){
        session_id = std::stoul(params["session_id"]);
    }else{
        std::cerr << "[ERROR] Missing session_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    std::shared_ptr<ClientSession> session = getSession(session_id);

    session->send({{"@type", "getAuthorizationState"}});
    json state_json = {{"state", td_auth_get_state(session)}};

    std::string json_str = state_json.dump();

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");
    mg_write(conn, json_str.c_str(), json_str.size());

    return 200;
}

int logout(struct mg_connection *conn, void *data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    uint32_t session_id = 0;

    if(params.find("session_id") != params.end()){
        session_id = std::stoul(params["session_id"]);
    }else{
        std::cerr << "[ERROR] Missing session_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    std::shared_ptr<ClientSession> session = getSession(session_id);

    session->send({{"@type", "logOut"}});
    
    std::cout << "[MESSAGE] Logging out..." << std::endl;

    while(td_auth_get_state(session) != "authorizationStateClosed"){
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    std::cout << "[MESSAGE] Logged out successfully!" << std::endl;

    session->send({{"@type", "close"}});

    closeSession(session_id);

    std::cout << "[MESSAGE] Session closed!" << std::endl;

    std::filesystem::path session_dir = std::filesystem::path("UserData") / std::to_string(session_id);

    if(std::filesystem::exists(session_dir)){
        std::filesystem::remove_all(session_dir);
    }

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "\r\n");
    json response_json = {{"status", "success"}};
    std::string response_str = response_json.dump();
    mg_write(conn, response_str.c_str(), response_str.size());

    return 200;
}

int search_movie_handler(struct mg_connection* conn, void* data)
{
    std::cout << "[MESSAGE] Search movie handler called" << std::endl;

    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    std::string title;
    std::string language;

    // Extract the title parameter
    if(params.find("title") != params.end()){
        title = params["title"];
    }else{
        std::cerr << "[ERROR] Missing title parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    json result = search_movie(title, language);
    
    std::string json_str = result.dump();

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");

    mg_write(conn, json_str.c_str(), json_str.size());

    return 200;
}

int search_tv_show_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    std::string title;
    std::string language;

    // Extract the title parameter
    if(params.find("title") != params.end()){
        title = params["title"];
    }else{
        std::cerr << "[ERROR] Missing title parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    json result = search_tv_show(title, language);
    
    std::string json_str = result.dump();

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");

    mg_write(conn, json_str.c_str(), json_str.size());

    return 200;
}

int get_movie_details_(int id, const std::string& language, json& result)
{
    result = db_select("SELECT * FROM movies WHERE movie_id = " + std::to_string(id));
    if(result.empty()){
        json tmdb_result = get_movie_details(id, language);
        if(tmdb_result.empty()){
            return 404;
        }

        std::string insert_query = "INSERT INTO movies (movie_id, title, overview, release_date, runtime, status, popularity, vote_average, vote_count, poster_path, backdrop_path, adult, imdb_id, homepage, video, last_update) VALUES (" +
                       std::to_string(tmdb_result["id"].get<int>()) + ", '" +
                       escape_string(tmdb_result["title"].get<std::string>()) + "', '" +
                       escape_string(tmdb_result["overview"].get<std::string>()) + "', '" +
                       tmdb_result["release_date"].get<std::string>() + "', " +
                       std::to_string(tmdb_result["runtime"].get<int>()) + ", '" +
                       tmdb_result["status"].get<std::string>() + "', " +
                       std::to_string(tmdb_result["popularity"].get<double>()) + ", " +
                       std::to_string(tmdb_result["vote_average"].get<double>()) + ", " +
                       std::to_string(tmdb_result["vote_count"].get<int>()) + ", '" +
                       tmdb_result["poster_path"].get<std::string>() + "', '" +
                       tmdb_result["backdrop_path"].get<std::string>() + "', " +
                       (tmdb_result["adult"].get<bool>() ? "1" : "0") + ", '" +
                       tmdb_result["imdb_id"].get<std::string>() + "', '" +
                       tmdb_result["homepage"].get<std::string>() + "', " +
                       (tmdb_result["video"].get<bool>() ? "1" : "0") + ", " +
                       "CURRENT_TIMESTAMP);";

        db_execute(insert_query);

        for(const auto& genre : tmdb_result["genres"]){
            db_execute("INSERT INTO movie_genres (movie_id, genre_id) VALUES (" + std::to_string(tmdb_result["id"].get<int>()) + ", " + std::to_string(genre["id"].get<int>()) + ")");
        }

        result = db_select("SELECT * FROM movies WHERE movie_id = " + std::to_string(id)).front();
        result["genres"] = db_select("SELECT genre_id FROM movie_genres WHERE movie_id = " + std::to_string(id));
    }

    return 200;
}

int get_movie_details_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    int id;
    std::string language;

    // Extract the id parameter
    if(params.find("id") != params.end()){
        id = std::stoi(params["id"]);
    }else{
        std::cerr << "[ERROR] Missing id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    int status_code = 0;
    json result;

    status_code = get_movie_details_(id, language, result);

    if(status_code == 404){
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 404;
    }

    if(status_code == 200){
        std::string json_str = result.dump();

        mg_printf(conn, "HTTP/1.1 200 OK\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "Content-Type: application/json\r\n");
        mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
        mg_printf(conn, "\r\n");

        mg_write(conn, json_str.c_str(), json_str.size());

        return 200;
    }

    mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "\r\n");
    return 500;
}

int get_tv_show_details_(int id, const std::string& language, json& result)
{
    result = db_select("SELECT * FROM tv_shows WHERE tv_id = " + std::to_string(id));
    if(result.empty()){
        json tmdb_result = get_tv_show_details(id, language);
        if(tmdb_result.empty()){
            return 404;
        }

        std::string insert_query = "INSERT INTO tv_shows (tv_id, name, overview, first_air_date, last_air_date, number_of_seasons, number_of_episodes, status, popularity, vote_average, vote_count, poster_path, backdrop_path, homepage, in_production, type, last_update) VALUES (" +
                       std::to_string(tmdb_result["id"].get<int>()) + ", '" +
                       escape_string(tmdb_result["name"].get<std::string>()) + "', '" +
                       escape_string(tmdb_result["overview"].get<std::string>()) + "', '" +
                       tmdb_result["first_air_date"].get<std::string>() + "', '" +
                       tmdb_result["last_air_date"].get<std::string>() + "', " +
                       std::to_string(tmdb_result["number_of_seasons"].get<int>()) + ", " +
                       std::to_string(tmdb_result["number_of_episodes"].get<int>()) + ", '" +
                       tmdb_result["status"].get<std::string>() + "', " +
                       std::to_string(tmdb_result["popularity"].get<double>()) + ", " +
                       std::to_string(tmdb_result["vote_average"].get<double>()) + ", " +
                       std::to_string(tmdb_result["vote_count"].get<int>()) + ", '" +
                       tmdb_result["poster_path"].get<std::string>() + "', '" +
                       tmdb_result["backdrop_path"].get<std::string>() + "', '" +
                       tmdb_result["homepage"].get<std::string>() + "', " +
                       (tmdb_result["in_production"].get<bool>() ? "1" : "0") + ", '" +
                       tmdb_result["type"].get<std::string>() + "', " +
                       "CURRENT_TIMESTAMP);";

        db_execute(insert_query);

        for(const auto& genre : tmdb_result["genres"]){
            db_execute("INSERT INTO tv_show_genres (tv_id, genre_id) VALUES (" + std::to_string(tmdb_result["id"].get<int>()) + ", " + std::to_string(genre["id"].get<int>()) + ")");
        }

        result = db_select("SELECT * FROM tv_shows WHERE tv_id = " + std::to_string(id)).front();
        result["genres"] = db_select("SELECT genre_id FROM tv_show_genres WHERE tv_id = " + std::to_string(id));
    }

    return 200;
}

int get_tv_show_details_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    int id;
    std::string language;

    // Extract the id parameter
    if(params.find("id") != params.end()){
        id = std::stoi(params["id"]);
    }else{
        std::cerr << "[ERROR] Missing id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    int status_code = 0;
    json result;
    status_code = get_tv_show_details_(id, language, result);

    if(status_code == 404){
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 404;
    }

    if(status_code == 200){
        std::string json_str = result.dump();

        mg_printf(conn, "HTTP/1.1 200 OK\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "Content-Type: application/json\r\n");
        mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
        mg_printf(conn, "\r\n");

        mg_write(conn, json_str.c_str(), json_str.size());

        return 200;
    }

    mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "\r\n");
    return 500;
}

int get_tv_show_season_handler(struct mg_connection* conn, void* data)
{
    std::cout << "[MESSAGE] Get TV show season handler called" << std::endl;

    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    int tv_id;
    int season_number;
    std::string language;

    // Extract the tv_id parameter
    if(params.find("id") != params.end()){
        tv_id = std::stoi(params["id"]);
    }else{
        std::cerr << "[ERROR] Missing id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the season_number parameter
    if(params.find("season_number") != params.end()){
        season_number = std::stoi(params["season_number"]);
    }else{
        std::cerr << "[ERROR] Missing season_number parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    json result;
    result["episodes"] = db_select("SELECT * FROM episodes, seasons WHERE episodes.season_id = seasons.season_id AND episodes.tv_id = " + std::to_string(tv_id) + " AND seasons.season_number = " + std::to_string(season_number) + ";");

    if(result["episodes"].empty()){
        json tmp;
        get_tv_show_details_(tv_id, language, tmp);

        json tmdb_result = get_tv_show_season(tv_id, season_number, language);

        if(tmdb_result.empty()){
            mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 404;
        }

        json season_result = db_select("SELECT * FROM seasons WHERE tv_id = " + std::to_string(tv_id) + " AND season_id = " + std::to_string(tmdb_result["id"].get<int>()));
        if(season_result.empty()){
            int episodes_count = tmdb_result["episodes"].size();

            std::string insert_season_query = "INSERT INTO seasons (season_id, tv_id, season_number, name, overview, air_date, episode_count, poster_path, last_update) VALUES (" +
                                              std::to_string(tmdb_result["id"].get<int>()) + ", " +
                                              std::to_string(tv_id) + ", " +
                                              std::to_string(tmdb_result["season_number"].get<int>()) + ", '" +
                                              escape_string(tmdb_result["name"].get<std::string>()) + "', '" +
                                              escape_string(tmdb_result["overview"].get<std::string>()) + "', '" +
                                              tmdb_result["air_date"].get<std::string>() + "', " +
                                              std::to_string(episodes_count) + ", '" +
                                              tmdb_result["poster_path"].get<std::string>() + "', " +
                                              "CURRENT_TIMESTAMP);";

            db_execute(insert_season_query);
        }

        for(const auto& episode : tmdb_result["episodes"]){
            json episode_result = db_select("SELECT * FROM episodes WHERE episode_id = " + std::to_string(episode["id"].get<int>()));
            if(episode_result.empty()){
                std::string insert_episode_query = "INSERT INTO episodes (episode_id, season_id, episode_number, name, overview, air_date, runtime, vote_average, vote_count, still_path, last_update, tv_id) VALUES (" +
                                                    std::to_string(episode["id"].get<int>()) + ", " +
                                                    std::to_string(tmdb_result["id"].get<int>()) + ", " +
                                                    std::to_string(episode["episode_number"].get<int>()) + ", '" +
                                                    escape_string(episode["name"].get<std::string>()) + "', '" +
                                                    escape_string(episode["overview"].get<std::string>()) + "', '" +
                                                    episode["air_date"].get<std::string>() + "', " +
                                                    std::to_string(episode["runtime"].get<int>()) + ", " +
                                                    std::to_string(episode["vote_average"].get<double>()) + ", " +
                                                    std::to_string(episode["vote_count"].get<int>()) + ", '" +
                                                    episode["still_path"].get<std::string>() + "', " +
                                                    "CURRENT_TIMESTAMP, " +
                                                    std::to_string(tv_id) + ");";

                db_execute(insert_episode_query);
            }
        }

        result["episodes"] = db_select("SELECT * FROM episodes, seasons WHERE episodes.season_id = seasons.season_id AND episodes.tv_id = " + std::to_string(tv_id) + " AND seasons.season_number = " + std::to_string(season_number) + ";");
        result["season"] = db_select("SELECT * FROM seasons WHERE tv_id = " + std::to_string(tv_id) + " AND season_number = " + std::to_string(season_number));
    }

    std::string json_str = result.dump();
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");
    mg_write(conn, json_str.c_str(), json_str.size());
    return 200;
}

int get_movie_cast_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    int id;
    std::string language;

    // Extract the id parameter
    if(params.find("id") != params.end()){
        id = std::stoi(params["id"]);
    }else{
        std::cerr << "[ERROR] Missing id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    json result;
    json tmdb_result = get_movie_cast(id);

    if(tmdb_result.empty()){
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 404;
    }

    for(const auto& cast : tmdb_result["cast"]){
        json cast_result = db_select("SELECT * FROM movie_cast WHERE credit_id = '" + cast["credit_id"].get<std::string>() + "'");

        if(!cast_result.empty()){
            continue;
        }

        json person = db_select("SELECT * FROM people WHERE person_id = " + std::to_string(cast["id"].get<int>()));
        if(person.empty()){
            json tmdb_person = get_person_details(cast["id"].get<int>(), language);
            if(tmdb_person.empty()){
                mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 404;
            }

            std::string homepage = tmdb_person["homepage"].is_null() ? "NULL" : "'" + tmdb_person["homepage"].get<std::string>() + "'";
            std::string profile_path = tmdb_person["profile_path"].is_null() ? "NULL" : "'" + tmdb_person["profile_path"].get<std::string>() + "'";
            std::string imdb_id = tmdb_person["imdb_id"].is_null() ? "NULL" : "'" + tmdb_person["imdb_id"].get<std::string>() + "'";
            std::string insert_query = "INSERT INTO people (person_id, name, gender, biography, birthday, deathday, known_for_department, popularity, profile_path, imdb_id, homepage, last_update) VALUES (" +
                                        std::to_string(tmdb_person["id"].get<int>()) + ", '" +
                                        escape_string(tmdb_person["name"].get<std::string>()) + "', " +
                                        std::to_string(tmdb_person["gender"].get<int>()) + ", '" +
                                        escape_string(tmdb_person["biography"].get<std::string>()) + "', " +
                                        (tmdb_person["birthday"].is_null() ? "NULL" : "'" + tmdb_person["birthday"].get<std::string>() + "'") + ", " +
                                        (tmdb_person["deathday"].is_null() ? "NULL" : "'" + tmdb_person["deathday"].get<std::string>() + "'") + ", '" +
                                        tmdb_person["known_for_department"].get<std::string>() + "', " +
                                        std::to_string(tmdb_person["popularity"].get<double>()) + ", " +
                                        profile_path + ", " +
                                        imdb_id + "," +
                                        homepage + ", " +
                                        "CURRENT_TIMESTAMP);";

            db_execute(insert_query);
        }

        std::string job = !cast.contains("job") ? "NULL" : "'" + cast["job"].get<std::string>() + "'";
        std::string insert_cast_query = "INSERT INTO movie_cast (movie_id, person_id, `character`, credit_id, `order`, department, job, last_update) VALUES (" +
                                        std::to_string(id) + ", " +
                                        std::to_string(cast["id"].get<int>()) + ", '" +
                                        escape_string(cast["character"].get<std::string>()) + "', '" +
                                        cast["credit_id"].get<std::string>() + "', " +
                                        std::to_string(cast["order"].get<int>()) + ", '" +
                                        cast["known_for_department"].get<std::string>() + "', " +
                                        job + ", " +
                                        "CURRENT_TIMESTAMP);";

        db_execute(insert_cast_query);
    }

    for(const auto& cast : tmdb_result["crew"]){
        json cast_result = db_select("SELECT * FROM movie_cast WHERE credit_id = '" + cast["credit_id"].get<std::string>() + "'");

        if(!cast_result.empty()){
            continue;
        }

        json person = db_select("SELECT * FROM people WHERE person_id = " + std::to_string(cast["id"].get<int>()));
        if(person.empty()){
            json tmdb_person = get_person_details(cast["id"].get<int>(), language);
            if(tmdb_person.empty()){
                mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 404;
            }

            std::string homepage = tmdb_person["homepage"].is_null() ? "NULL" : "'" + tmdb_person["homepage"].get<std::string>() + "'";
            std::string profile_path = tmdb_person["profile_path"].is_null() ? "NULL" : "'" + tmdb_person["profile_path"].get<std::string>() + "'";
            std::string imdb_id = tmdb_person["imdb_id"].is_null() ? "NULL" : "'" + tmdb_person["imdb_id"].get<std::string>() + "'";
            std::string insert_query = "INSERT INTO people (person_id, name, gender, biography, birthday, deathday, known_for_department, popularity, profile_path, imdb_id, homepage, last_update) VALUES (" +
                                        std::to_string(tmdb_person["id"].get<int>()) + ", '" +
                                        escape_string(tmdb_person["name"].get<std::string>()) + "', " +
                                        std::to_string(tmdb_person["gender"].get<int>()) + ", '" +
                                        escape_string(tmdb_person["biography"].get<std::string>()) + "', " +
                                        (tmdb_person["birthday"].is_null() ? "NULL" : "'" + tmdb_person["birthday"].get<std::string>() + "'") + ", " +
                                        (tmdb_person["deathday"].is_null() ? "NULL" : "'" + tmdb_person["deathday"].get<std::string>() + "'") + ", '" +
                                        tmdb_person["known_for_department"].get<std::string>() + "', " +
                                        std::to_string(tmdb_person["popularity"].get<double>()) + ", " +
                                        profile_path + ", " +
                                        imdb_id + "," +
                                        homepage + ", " +
                                        "CURRENT_TIMESTAMP);";

            db_execute(insert_query);
        }

        std::string job = !cast.contains("job") ? "NULL" : "'" + cast["job"].get<std::string>() + "'";
        int order = !cast.contains("order") ? -1 : cast["order"].get<int>();
        std::string insert_cast_query = "INSERT INTO movie_cast (movie_id, person_id, `character`, credit_id, `order`, department, job, last_update) VALUES (" +
                                        std::to_string(id) + ", " +
                                        std::to_string(cast["id"].get<int>()) + ", '" +
                                        "NULL" + "', '" +
                                        cast["credit_id"].get<std::string>() + "', " +
                                        std::to_string(order) + ", '" +
                                        cast["known_for_department"].get<std::string>() + "', " +
                                        job + ", " +
                                        "CURRENT_TIMESTAMP);";

        db_execute(insert_cast_query);
    }

    result = db_select("SELECT * FROM movie_cast WHERE movie_id = " + std::to_string(id) + " ORDER BY `order` ASC;");

    std::string json_str = result.dump();
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");
    mg_write(conn, json_str.c_str(), json_str.size());
    return 200;
}

int get_tv_show_cast_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    int id;
    std::string language;

    // Extract the id parameter
    if(params.find("id") != params.end()){
        id = std::stoi(params["id"]);
    }else{
        std::cerr << "[ERROR] Missing id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    json result;
    
    json tmdb_result = get_tv_show_cast(id);
    if(tmdb_result.empty()){
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 404;
    }

    for(const auto& cast : tmdb_result["cast"]){
        json cast_result = db_select("SELECT * FROM tv_show_cast WHERE credit_id = '" + cast["credit_id"].get<std::string>() + "'");

        if(!cast_result.empty()){
            continue;
        }

        json person = db_select("SELECT * FROM people WHERE person_id = " + std::to_string(cast["id"].get<int>()));
        if(person.empty()){
            json tmdb_person = get_person_details(cast["id"].get<int>(), language);
            if(tmdb_person.empty()){
                mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 404;
            }

            std::string homepage = tmdb_person["homepage"].is_null() ? "NULL" : "'" + tmdb_person["homepage"].get<std::string>() + "'";
            std::string profile_path = tmdb_person["profile_path"].is_null() ? "NULL" : "'" + tmdb_person["profile_path"].get<std::string>() + "'";
            std::string imdb_id = tmdb_person["imdb_id"].is_null() ? "NULL" : "'" + tmdb_person["imdb_id"].get<std::string>() + "'";
            std::string insert_query = "INSERT INTO people (person_id, name, gender, biography, birthday, deathday, known_for_department, popularity, profile_path, imdb_id, homepage, last_update) VALUES (" +
                                        std::to_string(tmdb_person["id"].get<int>()) + ", '" +
                                        escape_string(tmdb_person["name"].get<std::string>()) + "', " +
                                        std::to_string(tmdb_person["gender"].get<int>()) + ", '" +
                                        escape_string(tmdb_person["biography"].get<std::string>()) + "', " +
                                        (tmdb_person["birthday"].is_null() ? "NULL" : "'" + tmdb_person["birthday"].get<std::string>() + "'") + ", " +
                                        (tmdb_person["deathday"].is_null() ? "NULL" : "'" + tmdb_person["deathday"].get<std::string>() + "'") + ", '" +
                                        tmdb_person["known_for_department"].get<std::string>() + "', " +
                                        std::to_string(tmdb_person["popularity"].get<double>()) + ", " +
                                        profile_path + ", " +
                                        imdb_id + ", " +
                                        homepage + ", " +
                                        "CURRENT_TIMESTAMP);";

            db_execute(insert_query);
        }

        std::string job = !cast.contains("job") ? "NULL" : "'" + cast["job"].get<std::string>() + "'";
        std::string insert_cast_query = "INSERT INTO tv_show_cast (tv_id, person_id, `character`, credit_id, `order`, department, job, last_update) VALUES (" +
                                        std::to_string(id) + ", " +
                                        std::to_string(cast["id"].get<int>()) + ", '" +
                                        escape_string(cast["character"].get<std::string>()) + "', '" +
                                        cast["credit_id"].get<std::string>() + "', " +
                                        std::to_string(cast["order"].get<int>()) + ", '" +
                                        cast["known_for_department"].get<std::string>() + "', " +
                                        job + ", " +
                                        "CURRENT_TIMESTAMP);";

        db_execute(insert_cast_query);
    }

    result = db_select("SELECT * FROM tv_show_cast WHERE tv_id = " + std::to_string(id) + " ORDER BY `order` ASC;");

    std::string json_str = result.dump();
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");
    mg_write(conn, json_str.c_str(), json_str.size());
    return 200;
}

int get_person_details_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Parse query string
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    std::map<std::string, std::string> params = parse_query_string(query_string);

    int id;
    std::string language;

    // Extract the id parameter
    if(params.find("id") != params.end()){
        id = std::stoi(params["id"]);
    }else{
        std::cerr << "[ERROR] Missing id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(params.find("language") != params.end()){
        language = params["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    json result = db_select("SELECT * FROM people WHERE person_id = " + std::to_string(id));
    if(result.empty()){
        json tmdb_result = get_person_details(id, language);
        if(tmdb_result.empty()){
            mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 404;
        }

        std::string homepage = tmdb_result["homepage"].is_null() ? "NULL" : "'" + tmdb_result["homepage"].get<std::string>() + "'";
        std::string profile_path = tmdb_result["profile_path"].is_null() ? "NULL" : "'" + tmdb_result["profile_path"].get<std::string>() + "'";
        std::string imdb_id = tmdb_result["imdb_id"].is_null() ? "NULL" : "'" + tmdb_result["imdb_id"].get<std::string>() + "'";
        std::string insert_person_query = "INSERT INTO people (person_id, name, gender, biography, birthday, deathday, known_for_department, popularity, profile_path, imdb_id, homepage, last_update) VALUES (" +
                                          std::to_string(tmdb_result["id"].get<int>()) + ", '" +
                                          escape_string(tmdb_result["name"].get<std::string>()) + "', " +
                                          std::to_string(tmdb_result["gender"].get<int>()) + ", '" +
                                          escape_string(tmdb_result["biography"].get<std::string>()) + "', " +
                                          (tmdb_result["birthday"].is_null() ? "NULL" : "'" + tmdb_result["birthday"].get<std::string>() + "'") + ", " +
                                          (tmdb_result["deathday"].is_null() ? "NULL" : "'" + tmdb_result["deathday"].get<std::string>() + "'") + ", '" +
                                          tmdb_result["known_for_department"].get<std::string>() + "', " +
                                          std::to_string(tmdb_result["popularity"].get<double>()) + ", " +
                                          profile_path + ", " +
                                          imdb_id + ", " +
                                          homepage + "', " +
                                          "CURRENT_TIMESTAMP);";

        db_execute(insert_person_query);
        result = db_select("SELECT * FROM people WHERE person_id = " + std::to_string(id));
    }

    std::string json_str = result.dump();
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", json_str.size());
    mg_printf(conn, "\r\n");
    mg_write(conn, json_str.c_str(), json_str.size());

    return 200;
}

int set_movie_data_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    // Ensure the request method is POST
    if (strcmp(req_info->request_method, "POST") != 0) {
        std::cerr << "[ERROR] Unsupported request method: " << req_info->request_method << std::endl;
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "Allow: POST\r\n");
        mg_printf(conn, "\r\n");
        return 405;
    }

    // Read the POST data
    char post_data[1024];
    int post_data_len = mg_read(conn, post_data, sizeof(post_data) - 1);
    if (post_data_len <= 0) {
        std::cerr << "[ERROR] No POST data received." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Null-terminate the POST data
    post_data[post_data_len] = '\0';

    // Parse the JSON payload
    json request_json;
    try {
        request_json = json::parse(post_data);
    } catch (const json::parse_error& e) {
        std::cerr << "[ERROR] Failed to parse JSON: " << e.what() << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }
    
    int movie_id;
    int64_t message_id;
    int64_t chat_id;
    std::string language;

    // Extract the movie_id parameter
    if(request_json.contains("movie_id")){
        movie_id = request_json["movie_id"];
    }else{
        std::cerr << "[ERROR] Missing movie_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the video_id parameter
    if(request_json.contains("video_id")){
        message_id = std::stoll(request_json["video_id"].get<std::string>());
    }else{
        std::cerr << "[ERROR] Missing video_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the chat_id parameter
    if(request_json.contains("chat_id")){
        chat_id = std::stoll(request_json["chat_id"].get<std::string>());
    }else{
        std::cerr << "[ERROR] Missing chat_id parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    // Extract the language parameter
    if(request_json.contains("language")){
        language = request_json["language"];
    }else{
        std::cerr << "[ERROR] Missing language parameter in request." << std::endl;
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "\r\n");
        return 400;
    }

    json tmp;
    get_movie_details_(movie_id, language, tmp);

    json res = db_select("SELECT * FROM telegram_video WHERE chat_id = " + std::to_string(chat_id) + " AND video_id = " + std::to_string(message_id) + ";");
    if(res.empty()){
        std::string insert_query = "INSERT INTO telegram_video (chat_id, video_id, movie_id) VALUES (" +
                                   std::to_string(chat_id) + ", " +
                                   std::to_string(message_id) + ", " +
                                   std::to_string(movie_id) + ");";
        db_execute(insert_query);
    }else{
        std::string update_query = "UPDATE telegram_video SET movie_id = " + std::to_string(movie_id) + " WHERE chat_id = " + std::to_string(chat_id) + " AND video_id = " + std::to_string(message_id) + ";";
        db_execute(update_query);
    }

    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "\r\n");
    return 200;
}

int set_tv_show_data_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    if(strcmp("POST", req_info->request_method) == 0){
        char post_data[10000];
        int post_data_len = mg_read(conn, post_data, sizeof(post_data) - 1);
        post_data[post_data_len] = '\0';

        if(post_data_len > 0){
            json request_json = json::parse(post_data);

            int id;
            json episodes;
            int64_t chat_id;
            std::string video_id;

            if(request_json.contains("episodes")){
                episodes = request_json["episodes"];
            }else{
                std::cerr << "[ERROR] Missing episodes parameter in request." << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 400;
            }

            if(request_json.contains("chat_id")){
                chat_id = std::stoll(request_json["chat_id"].get<std::string>());
            }else{
                std::cerr << "[ERROR] Missing chat_id parameter in request." << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 400;
            }

            for(const auto& episode : episodes){
                if(episode.contains("episode_id") && episode.contains("video_id")){
                    int episode_id = episode["episode_id"];
                    video_id = episode["video_id"];

                    json res = db_select("SELECT * FROM telegram_video WHERE chat_id = " + std::to_string(chat_id) + " AND video_id = '" + video_id + "';");
                    int telegram_video_id = 0;
                    if(res.empty()){
                        std::string insert_query = "INSERT INTO telegram_video (chat_id, video_id, episode_id) VALUES (" +
                                                   std::to_string(chat_id) + ", '" +
                                                   video_id + "', " +
                                                    std::to_string(episode_id) + ");";
                        db_execute(insert_query);
                    }else{
                        std::string update_query = "UPDATE telegram_video SET episode_id = " + std::to_string(episode_id) + " WHERE chat_id = " + std::to_string(chat_id) + " AND video_id = '" + video_id + "';";
                        db_execute(update_query);
                    }
                }else{
                    std::cerr << "[ERROR] Missing episode_id or video_id in request." << std::endl;
                    mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
                    mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                    mg_printf(conn, "\r\n");
                    return 400;
                }
            }

            mg_printf(conn, "HTTP/1.1 200 OK\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "Content-Type: application/json\r\n");
            mg_printf(conn, "\r\n");
            return 200;
        }else{
            std::cerr << "[ERROR] No data received in request." << std::endl;
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 400;
        }
    }else{
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "Allow: POST\r\n");
        mg_printf(conn, "Content-Type: application/json\r\n");
        mg_printf(conn, "\r\n");
        return 405;
    }
    return 200;
}

int get_videos_data_handler(struct mg_connection* conn, void* data)
{
    const mg_request_info* req_info = mg_get_request_info(conn);

    if(strcmp("POST", req_info->request_method) == 0){
        char post_data[10000];
        int post_data_len = mg_read(conn, post_data, sizeof(post_data) - 1);
        post_data[post_data_len] = '\0';

        if(post_data_len > 0){
            json request_json = json::parse(post_data);

            if(!request_json.contains("videos")){
                std::cerr << "[ERROR] Missing videos parameter in request." << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 400;
            }

            if(!request_json.contains("chat_id")){
                std::cerr << "[ERROR] Missing chat_id parameter in request." << std::endl;
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
                mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
                mg_printf(conn, "\r\n");
                return 400;
            }

            int64_t chat_id = request_json["chat_id"];

            json out;
            json movies = json::array();
            json episodes = json::array();
            json no_data = json::array();
            json tv_shows;

            for(const auto& video : request_json["videos"]){
                int64_t video_id = video["message_id"];

                json res = db_select("SELECT * FROM telegram_video WHERE chat_id = " + std::to_string(chat_id) + " AND video_id = " + std::to_string(video_id) + ";");

                if(res.empty()){
                    no_data.push_back(video);
                }else{
                    if(res[0]["movie_id"].is_null() || res[0]["movie_id"] == "NULL"){
                        json episode = video;
                        episode["episode_id"] = std::stoi(res[0]["episode_id"].get<std::string>());
                        episode["telegram_video_id"] = video["id"];
                        episode["mime_type"] = video["mime_type"];
                        episodes.push_back(episode);
                    }else{
                        json movie = db_select("SELECT * FROM movies WHERE movie_id = " + res[0]["movie_id"].get<std::string>() + ";");
                        if(movie.empty()){
                            no_data.push_back(video);
                        }else{
                            movie = movie[0];
                            movie["telegram_video_id"] = video["id"];
                            movie["mime_type"] = video["mime_type"];
                            movies.push_back(movie);
                        }
                    }
                }
            }

            out["movies"] = movies;
            out["no_data"] = no_data;

            if(!episodes.empty()){
                std::vector<int> episode_ids;
                std::unordered_map<int, json> episode_video_map;

                for(const auto& episode : episodes){
                    episode_ids.push_back(episode["episode_id"]);
                    json tmp;
                    tmp["telegram_video_id"] = episode["telegram_video_id"];
                    tmp["mime_type"] = episode["mime_type"];
                    episode_video_map[episode["episode_id"]] = tmp;
                }

                std::string episode_ids_str = "";
                for(size_t i = 0; i < episode_ids.size(); i++){
                    episode_ids_str += std::to_string(episode_ids[i]);
                    if(i < episode_ids.size() - 1){
                        episode_ids_str += ",";
                    }
                }

                std::string query = R"(
                    SELECT 
                        t.tv_id, t.name AS tv_name, t.overview AS tv_overview, t.first_air_date, t.last_air_date, t.number_of_seasons,
                        t.number_of_episodes, t.status, t.popularity, t.vote_average AS tv_vote_average, t.vote_count AS tv_vote_count,
                        t.poster_path AS tv_poster_path, t.backdrop_path, t.homepage, t.in_production, t.type, t.last_update AS tv_last_update,
                        s.season_id, s.season_number, s.name AS season_name, s.overview AS season_overview, s.air_date AS season_air_date,
                        s.poster_path AS season_poster_path, s.episode_count, s.last_update AS season_last_update,
                        e.episode_id, e.season_id AS episode_season_id, e.episode_number, e.name AS episode_name, e.overview AS episode_overview,
                        e.air_date AS episode_air_date, e.runtime, e.vote_average AS episode_vote_average, e.vote_count AS episode_vote_count,
                        e.still_path, e.last_update AS episode_last_update, e.tv_id AS episode_tv_id
                    FROM episodes e
                    JOIN seasons s ON e.season_id = s.season_id
                    JOIN tv_shows t ON s.tv_id = t.tv_id
                    WHERE e.episode_id IN ()" + episode_ids_str + R"()
                    ORDER BY t.tv_id, s.season_number, e.episode_number
                    )";             
                    
                json episodes_data = db_select(query); 

                out["tv_shows"] = json::array();
                
                // Mappa per tenere traccia dei tv_shows e seasons già inseriti
                std::unordered_map<std::string, size_t> tv_show_index;
                std::unordered_map<std::string, std::unordered_map<std::string, size_t>> season_index;
                
                for (const auto& row : episodes_data) {
                    std::string tv_id = row["tv_id"];
                    std::string season_id = row["season_id"];
                    std::string episode_id = row["episode_id"];
                
                    // Se il tv_show non è ancora stato aggiunto
                    if (tv_show_index.find(tv_id) == tv_show_index.end()) {
                        json tv_show;
                        tv_show["tv_id"] = tv_id;
                        tv_show["name"] = row["tv_name"];
                        tv_show["overview"] = row["tv_overview"];
                        tv_show["first_air_date"] = row["first_air_date"];
                        tv_show["last_air_date"] = row["last_air_date"];
                        tv_show["number_of_seasons"] = row["number_of_seasons"];
                        tv_show["number_of_episodes"] = row["number_of_episodes"];
                        tv_show["status"] = row["status"];
                        tv_show["popularity"] = row["popularity"];
                        tv_show["vote_average"] = row["tv_vote_average"];
                        tv_show["vote_count"] = row["tv_vote_count"];
                        tv_show["poster_path"] = row["tv_poster_path"];
                        tv_show["backdrop_path"] = row["backdrop_path"];
                        tv_show["homepage"] = row["homepage"];
                        tv_show["in_production"] = row["in_production"];
                        tv_show["type"] = row["type"];
                        tv_show["last_update"] = row["tv_last_update"];
                        tv_show["seasons"] = json::array();
                
                        tv_show_index[tv_id] = out["tv_shows"].size();
                        out["tv_shows"].push_back(tv_show);
                    }
                
                    auto& tv_show = out["tv_shows"][tv_show_index[tv_id]];
                
                    // Se la season non è ancora stata aggiunta
                    if (season_index[tv_id].find(season_id) == season_index[tv_id].end()) {
                        json season;
                        season["season_id"] = season_id;
                        season["season_number"] = row["season_number"];
                        season["name"] = row["season_name"];
                        season["overview"] = row["season_overview"];
                        season["air_date"] = row["season_air_date"];
                        season["poster_path"] = row["season_poster_path"];
                        season["episode_count"] = row["episode_count"];
                        season["last_update"] = row["season_last_update"];
                        season["episodes"] = json::array();
                
                        season_index[tv_id][season_id] = tv_show["seasons"].size();
                        tv_show["seasons"].push_back(season);
                    }
                
                    auto& season = tv_show["seasons"][season_index[tv_id][season_id]];
                
                    // Aggiungiamo l'episodio
                    json episode;
                    episode["episode_id"] = episode_id;
                    episode["season_id"] = row["episode_season_id"];
                    episode["episode_number"] = row["episode_number"];
                    episode["name"] = row["episode_name"];
                    episode["overview"] = row["episode_overview"];
                    episode["air_date"] = row["episode_air_date"];
                    episode["runtime"] = row["runtime"];
                    episode["vote_average"] = row["episode_vote_average"];
                    episode["vote_count"] = row["episode_vote_count"];
                    episode["still_path"] = row["still_path"];
                    episode["last_update"] = row["episode_last_update"];
                    episode["tv_id"] = row["episode_tv_id"];
                    episode["telegram_video_id"] = episode_video_map[std::stoi(episode_id)]["telegram_video_id"];
                    episode["mime_type"] = episode_video_map[std::stoi(episode_id)]["mime_type"];
                
                    season["episodes"].push_back(episode);
                }             
            }                   

            mg_printf(conn, "HTTP/1.1 200 OK\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "Content-Type: application/json\r\n");
            mg_printf(conn, "Content-Length: %zu\r\n", out.dump().size());
            mg_printf(conn, "\r\n");
            mg_write(conn, out.dump().c_str(), out.dump().size());
            return 200;
        }else{
            std::cerr << "[ERROR] No data received in request." << std::endl;
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n");
            mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
            mg_printf(conn, "\r\n");
            return 400;
        }
    }else{
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n");
        mg_printf(conn, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(conn, "Allow: POST\r\n");
        mg_printf(conn, "Content-Type: application/json\r\n");
        mg_printf(conn, "\r\n");
        return 405;
    }
}