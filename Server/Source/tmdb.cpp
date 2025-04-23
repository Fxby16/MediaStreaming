#include "app_data.hpp"
#include "tmdb.hpp"
#include "curl.hpp"

#include <curl/curl.h>
#include <string>

json search_movie(const std::string& title, const std::string& language)
{
    std::string encoded_title = encode_url(title);   
    std::string url = "https://api.themoviedb.org/3/search/movie?query=" + encoded_title + "&language=" + language + "&include_adult=true&page=1";
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}

json search_tv_show(const std::string& title, const std::string& language)
{
    std::string encoded_title = encode_url(title);   
    std::string url = "https://api.themoviedb.org/3/search/tv?query=" + encoded_title + "&language=" + language + "&include_adult=true&page=1";
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}

json get_movie_details(int id, const std::string& language)
{
    std::string url = "https://api.themoviedb.org/3/movie/" + std::to_string(id) + "?language=" + language;
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}

json get_tv_show_details(int id, const std::string& language)
{
    std::string url = "https://api.themoviedb.org/3/tv/" + std::to_string(id) + "?language=" + language;
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}

json get_tv_show_season(int tv_id, int season_number, const std::string& language)
{
    std::string url = "https://api.themoviedb.org/3/tv/" + std::to_string(tv_id) + "/season/" + std::to_string(season_number) + "?language=" + language;
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}

json get_movie_cast(int movie_id)
{
    std::string url = "https://api.themoviedb.org/3/movie/" + std::to_string(movie_id) + "/credits";
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    json result = curl_get(url, headers_str);
    json out;

    out["id"] = result["id"];
    out["cast"] = result["cast"];

    json crew = json::array();
    for(const auto& item : result["crew"]){
        if(item["job"] == "Director"){
            crew.push_back(item);
        }
    }

    out["crew"] = crew;

    return out;
}

json get_tv_show_cast(int tv_show_id)
{
    std::string url = "https://api.themoviedb.org/3/tv/" + std::to_string(tv_show_id) + "/credits";
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    json result = curl_get(url, headers_str);
    json out;

    out["id"] = result["id"];
    out["cast"] = result["cast"];

    return out;
}

json get_person_details(int person_id, const std::string& language)
{
    std::string url = "https://api.themoviedb.org/3/person/" + std::to_string(person_id) + "?language=" + language;
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}

json get_movie_genres(const std::string& language)
{
    std::string url = "https://api.themoviedb.org/3/genre/movie/list?language=" + language;
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}

json get_tv_show_genres(const std::string& language)
{
    std::string url = "https://api.themoviedb.org/3/genre/tv/list?language=" + language;
    std::string header_key = "Authorization: Bearer " + std::string(TMDB_API_HEADER_KEY);

    std::vector<std::string> headers_str;
    headers_str.push_back("accept: application/json");
    headers_str.push_back(header_key);

    return curl_get(url, headers_str);
}