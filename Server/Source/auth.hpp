#pragma once

#include <string>

extern void td_auth(void* client, const std::string &api_id, const std::string &api_hash, const std::string &directory);
extern void td_auth_send_parameters(void* client, const std::string &api_id, const std::string &api_hash, const std::string &directory);
extern void td_auth_send_number(void* client, const std::string &phone);
extern void td_auth_send_code(void* client, const std::string &code);
extern void td_auth_send_password(void* client, const std::string &password);
extern void td_auth_verify(void* client);
extern std::string td_auth_get_state(void* client);