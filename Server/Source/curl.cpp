#include "curl.hpp"
#include <curl/curl.h>

#include <string>
#include <iostream>

std::string encode_url(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if(!curl){
        return url;
    }

    char* encoded_url = curl_easy_escape(curl, url.c_str(), url.length());
    std::string encoded_string(encoded_url);
    curl_free(encoded_url);
    curl_easy_cleanup(curl);

    return encoded_string;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out)
{
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

json curl_get(const std::string& url, const std::vector<std::string>& headers_str)
{
    CURL* hnd = curl_easy_init();
    if(!hnd){
        std::cerr << "curl_easy_init() failed" << std::endl;
        return json();
    }

    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());

    struct curl_slist* headers = NULL;
    for(const auto& header : headers_str){
        headers = curl_slist_append(headers, header.c_str());
    }
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    // Stringa per catturare la risposta
    std::string response_string;
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response_string);

    CURLcode ret = curl_easy_perform(hnd);

    if(ret != CURLE_OK){
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(ret) << "\n";
        curl_easy_cleanup(hnd);
        curl_slist_free_all(headers);
        return json();
    }

    long response_code;
    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(hnd);
    curl_slist_free_all(headers);

    if(response_code != 200){
        std::cerr << "HTTP request failed with response code: " << response_code << "\n";
        return json();
    }

    try{
        return json::parse(response_string);
    } catch(const json::parse_error& e){
        std::cerr << "JSON parsing failed: " << e.what() << "\n";
        return json();
    }
}