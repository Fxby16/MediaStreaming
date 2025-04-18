#pragma once

extern void setup_endpoints();
extern int handle_video(struct mg_connection *conn, void *data);
extern int get_files(struct mg_connection *conn, void *data);
extern int handle_auth(struct mg_connection *conn, void *data);
extern int get_state(struct mg_connection *conn, void *data);