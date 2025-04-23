#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string encode_url(const std::string& url);
json curl_get(const std::string& url, const std::vector<std::string>& headers_str);