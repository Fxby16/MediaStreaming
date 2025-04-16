#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

inline constexpr float TDLIB_TIMEOUT = 10.0f;

extern void td_send(const json &j, void* client);
extern json td_recv(void* client);