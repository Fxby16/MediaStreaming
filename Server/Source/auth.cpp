#include "auth.hpp"
#include "common.hpp"

#include <td/telegram/td_json_client.h>

#include <iostream>
#include <string>
#include <filesystem>

void td_auth(void* client, const std::string &api_id, const std::string &api_hash, const std::string &directory)
{
    td_send({{"@type", "getOption"},
             {"name", "version"}}, client);

    json response = td_recv(client);

    if(!response.is_null() && response["@type"] == "updateOption"){
        std::cout << "[MESSAGE] TDLib version: " << response["value"]["value"] << std::endl;
    }else{
        std::cerr << "[WARNING] Failed to retrieve TDLib version. \nResponse: " << response.dump(4) << std::endl;
    }

    td_send({{"@type", "setLogVerbosityLevel"},
             {"new_verbosity_level", 1}}, client);

    // Start authentication process
    td_send({{"@type", "getAuthorizationState"}}, client);

    std::string phone;

    while(true){
        json r = td_recv(client);

        if(r.is_null() || !r.contains("@type")){
            continue;
        }

        if(r["@type"] == "updateAuthorizationState"){
            std::string state = r["authorization_state"]["@type"];

            // Send TDLib parameters
            if(state == "authorizationStateWaitTdlibParameters"){
                td_send({{"@type", "setTdlibParameters"},
                         {"database_directory", directory},
                         {"use_message_database", true},
                         {"use_secret_chats", false},
                         {"api_id", std::stoi(api_id)},
                         {"api_hash", api_hash},
                         {"system_language_code", "en"},
                         {"device_model", "Server"},
                         {"system_version", "Linux"},
                         {"application_version", "1.0"},
                         {"enable_storage_optimizer", true},
                         {"use_test_dc", false}

                }, client);
            // Send phone number
            }else if (state == "authorizationStateWaitPhoneNumber"){
                std::cout << "Enter phone number: ";
                std::getline(std::cin, phone);
                td_send({{"@type", "setAuthenticationPhoneNumber"},
                         {"phone_number", phone}}, client);
            // Send code
            }else if (state == "authorizationStateWaitCode"){
                std::cout << "📲 Waiting for the code to be sent via the Telegram app...\n";
                // Now wait for the user to input the code received in the
                std::string code;
                std::cout << "Enter authentication code from Telegram app: ";
                std::getline(std::cin, code);
                td_send({{"@type", "checkAuthenticationCode"},
                         {"code", code}}, client);
            // Send password
            }else if (state == "authorizationStateWaitPassword"){
                std::string pass;
                std::cout << "Enter 2FA password: ";
                std::getline(std::cin, pass);
                td_send({{"@type", "checkAuthenticationPassword"},
                         {"password", pass}}, client);
            // Authentication success
            }else if (state == "authorizationStateReady"){
                std::cout << "[MESSAGE] Authorized successfully!\n";
                break;
            }
        }
    }
}

void td_auth_send_parameters(void* client, const std::string& api_id, const std::string& api_hash, const std::string& directory)
{
    td_send({{"@type", "getOption"},
        {"name", "version"}}, client);

    json response = td_recv(client);

    if(!response.is_null() && response["@type"] == "updateOption"){
        std::cout << "[MESSAGE] TDLib version: " << response["value"]["value"] << std::endl;
    }else{
        std::cerr << "[WARNING] Failed to retrieve TDLib version. \nResponse: " << response.dump(4) << std::endl;
    }

    td_send({{"@type", "setLogVerbosityLevel"},
            {"new_verbosity_level", 1}}, client);

    std::cout << "[MESSAGE] TDLib log verbosity level set to 1" << std::endl;

    // Start authentication process

    std::cout << "[MESSAGE] TDLib authentication started" << std::endl;

    while(true){
        json r = td_recv(client);

        if(r.is_null() || !r.contains("@type")){
            continue;
        }

        if(r["@type"] == "updateAuthorizationState"){
            std::string state = r["authorization_state"]["@type"];

            std::cout << "[MESSAGE] Current state: " << state << std::endl;

            std::string final_dir = "UserData/" + directory;
            std::filesystem::create_directories(final_dir);

            // Send TDLib parameters
            if(state == "authorizationStateWaitTdlibParameters"){
                td_send({{"@type", "setTdlibParameters"},
                            {"database_directory", final_dir},
                            {"use_message_database", true},
                            {"use_secret_chats", false},
                            {"api_id", std::stoi(api_id)},
                            {"api_hash", api_hash},
                            {"system_language_code", "en"},
                            {"device_model", "Server"},
                            {"system_version", "Linux"},
                            {"application_version", "1.0"},
                            {"enable_storage_optimizer", true},
                            {"use_test_dc", false}

                }, client);

                std::cout << "[MESSAGE] TDLib parameters sent successfully!\n";

                break;
            }
        }
    }

    std::string state;
    //td_send({{"@type", "getAuthorizationState"}}, client);
    while((state = td_auth_get_state(client)) == "authorizationStateWaitTdlibParameters");
}

void td_auth_send_number(void* client, const std::string &phone)
{
    std::string state = "";

    td_send({{"@type", "getAuthorizationState"}}, client);
    state = td_auth_get_state(client);

    if(state == "authorizationStateReady"){
        std::cout << "[MESSAGE] Already authorized!\n";
    }else if(state == "authorizationStateWaitPhoneNumber"){
        std::cout << "[MESSAGE] Sending phone number: " << phone << std::endl;

        td_send({{"@type", "setAuthenticationPhoneNumber"},
                 {"phone_number", phone}}, client);

        std::cout << "[MESSAGE] Phone number sent successfully!" << std::endl;
    }

    //td_send({{"@type", "getAuthorizationState"}}, client);
    while((state = td_auth_get_state(client)) == "authorizationStateWaitPhoneNumber");

    std::cout << "Last state " << state << std::endl;
}

void td_auth_send_code(void* client, const std::string &code)
{
    std::string state = "";

    td_send({{"@type", "getAuthorizationState"}}, client);
    state = td_auth_get_state(client);

    if(state == "authorizationStateReady"){
        std::cout << "[MESSAGE] Already authorized!\n";
    }else if(state == "authorizationStateWaitCode"){
        std::cout << "[MESSAGE] Sending code: " << code << std::endl;

        td_send({{"@type", "checkAuthenticationCode"},
                 {"code", code}}, client);

        std::cout << "[MESSAGE] Code sent successfully!\n";
    }

    //td_send({{"@type", "getAuthorizationState"}}, client);
    while((state = td_auth_get_state(client)) == "authorizationStateWaitCode");
}

void td_auth_send_password(void* client, const std::string &password)
{
    std::string state = "";

    td_send({{"@type", "getAuthorizationState"}}, client);
    state = td_auth_get_state(client);

    if(state == "authorizationStateReady"){
        std::cout << "[MESSAGE] Already authorized!\n";
    }else if(state == "authorizationStateWaitPassword"){
        std::cout << "[MESSAGE] Sending password: " << password << std::endl;
        td_send({{"@type", "checkAuthenticationPassword"},
            {"password", password}}, client);
    }

    //td_send({{"@type", "getAuthorizationState"}}, client);
    while((state = td_auth_get_state(client)) == "authorizationStateWaitPassword");
}

void td_auth_verify(void* client)
{
    std::string state = "";

    td_send({{"@type", "getAuthorizationState"}}, client);

    int max_attempts = 5;
    while(true){
        json r = td_recv(client);

        if(r.is_null() || !r.contains("@type")){
            continue;
        }

        if(r["@type"] == "updateAuthorizationState"){
            state = r["authorization_state"]["@type"];

            if(state == "authorizationStateReady"){
                std::cout << "[MESSAGE] Authorized successfully!\n";
                break;
            }else if(state == "authorizationStateClosed"){
                std::cout << "[MESSAGE] Authorization closed!\n";
                break;
            }
        }
    }
}

std::string td_auth_get_state(void* client)
{
    std::string state = "";

    while(true){
        json r = td_recv(client);

        if(r.is_null() || !r.contains("@type")){
            continue;
        }

        std::cout << "Response for state\n" << r.dump(4) << std::endl;

        if(r["@type"] == "updateAuthorizationState"){
            state = r["authorization_state"]["@type"];
            return state;
        }else if(r["@type"] == "authorizationStateWaitTdlibParameters"){
            state = "authorizationStateWaitTdlibParameters";
            return state;
        }else if(r["@type"] == "authorizationStateWaitPhoneNumber"){
            state = "authorizationStateWaitPhoneNumber";
            return state;
        }else if(r["@type"] == "authorizationStateWaitCode"){
            state = "authorizationStateWaitCode";
            return state;
        }else if(r["@type"] == "authorizationStateWaitPassword"){
            state = "authorizationStateWaitPassword";
            return state;
        }else if(r["@type"] == "authorizationStateReady"){
            state = "authorizationStateReady";
            return state;
        }
    }

    return state;
}