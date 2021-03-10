/**
 *
 * cc sample.cpp -lcurl
 * 
 */
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct curl_fetch_st
{
    std::stringstream payload;
};

size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t length = size * nmemb;
    struct curl_fetch_st *p = (struct curl_fetch_st *)userp;
    p->payload.write((const char *)contents, length);
    return length;
}

CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch)
{
    fetch->payload.str("");

    curl_easy_setopt(ch, CURLOPT_URL, url);
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *)fetch);
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 60);
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);
    return curl_easy_perform(ch);
}

json curl_post_json(const char *url, json &request)
{
    json response = {
        {"code", 0},
        {"data", NULL}};
    CURL *ch = curl_easy_init();
    if (ch == NULL)
    {
        response["code"] = -1;
        response["info"] = "初始化 cURL 失败";
        return response;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);

    auto body = request.dump();
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, body.c_str());

    struct curl_fetch_st curl_fetch;
    struct curl_fetch_st *cf = &curl_fetch;
    CURLcode rcode = curl_fetch_url(ch, url, cf);

    curl_easy_cleanup(ch);
    curl_slist_free_all(headers);

    if (rcode != CURLE_OK)
    {
        response["code"] = rcode;
        response["info"] = curl_easy_strerror(rcode);
        return response;
    }
    response = json::parse(cf->payload.str());
    return response;
}

int main(int argc, char *argv[])
{
    const char *url = "http://192.168.3.218/public/robot/action";

    json robot_data = {
        {"cmd", "robot_data"}};
    std::cout << curl_post_json(url, robot_data) << std::endl;

    json movej = {
        {"cmd", "movej"},
        {"data", {
                     {"pose_to", {0, 0.2, 0.3, -0.4, 0.5, 0.6}},
                     {"is_joint_angle", true},
                     {"acceleration", 0},
                     {"velocity", 0},
                     {"time", 2},
                     {"smooth_move_to_next", 1},
                 }}};
    std::cout << curl_post_json(url, movej) << std::endl;

    return 0;
}