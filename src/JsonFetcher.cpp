//
// Created by athbe on 2025/6/17.
//
#include "JsonFetcher.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>

// 初始化线程本地错误信息
thread_local std::string JsonFetcher::lastError = "";

size_t JsonFetcher::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::string JsonFetcher::getLastError() {
    return lastError;
}

std::optional<nlohmann::json> JsonFetcher::fetchFromUrl(const std::string& url) {
    lastError.clear();

    CURL* curl = curl_easy_init();
    if (!curl) {
        lastError = "Failed to initialize CURL";
        return std::nullopt;
    }

    std::string response;
    long http_code = 0;
    CURLcode res = CURLE_OK;

    // 设置CURL选项
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "JsonFetcher/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // 执行请求
    res = curl_easy_perform(curl);

    // 获取HTTP状态码
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    // 检查错误
    if (res != CURLE_OK) {
        lastError = "CURL error: ";
        lastError += curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return std::nullopt;
    }

    // 清理CURL资源
    curl_easy_cleanup(curl);

    // 检查HTTP状态码
    if (http_code != 200) {
        std::ostringstream oss;
        oss << "HTTP error: " << http_code;
        lastError = oss.str();
        return std::nullopt;
    }

    // 尝试解析JSON
    try {
        nlohmann::json jsonData = nlohmann::json::parse(response);
        return jsonData;
    } catch (const nlohmann::json::parse_error& e) {
        lastError = "JSON parse error: ";
        lastError += e.what();
        return std::nullopt;
    } catch (const std::exception& e) {
        lastError = "Error: ";
        lastError += e.what();
        return std::nullopt;
    }
}
