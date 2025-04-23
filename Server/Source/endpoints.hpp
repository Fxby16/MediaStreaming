#pragma once

#include <string>

extern void setup_endpoints();
extern int handle_video(struct mg_connection* conn, void* data);
extern int get_files(struct mg_connection* conn, void* data);
extern int handle_auth(struct mg_connection* conn, void* data);
extern int get_state(struct mg_connection* conn, void* data);
extern int logout(struct mg_connection* conn, void* data);
extern int handle_chats(struct mg_connection* conn, void* data);

extern int search_movie_handler(struct mg_connection* conn, void* data);
extern int search_tv_show_handler(struct mg_connection* conn, void* data);
extern int get_movie_details_handler(struct mg_connection* conn, void* data);
extern int get_tv_show_details_handler(struct mg_connection* conn, void* data);
extern int get_tv_show_season_handler(struct mg_connection* conn, void* data);
extern int get_movie_cast_handler(struct mg_connection* conn, void* data);
extern int get_tv_show_cast_handler(struct mg_connection* conn, void* data);
extern int get_person_details_handler(struct mg_connection* conn, void* data);

extern int set_movie_data_handler(struct mg_connection* conn, void* data);
extern int set_tv_show_data_handler(struct mg_connection* conn, void* data);

extern void add_genres(const std::string& language);