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
#include "endpoints.hpp"

int main()
{
    setup_endpoints();

    std::cout << "📺 Server running at http://localhost:10000\n";
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(60));
}
