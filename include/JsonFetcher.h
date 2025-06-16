//
// Created by athbe on 2025/6/17.
//

#ifndef JSONFETCHER_H
#define JSONFETCHER_H
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <curl/curl.h>

class JsonFetcher {
public:
    /**
     * @brief 从指定URL获取并解析JSON数据
     *
     * @param url 要获取的JSON数据URL
     * @return std::optional<nlohmann::json> 解析后的JSON对象，如果失败则返回std::nullopt
     */
    static std::optional<nlohmann::json> fetchFromUrl(const std::string& url);

    /**
     * @brief 获取最后一次错误信息
     *
     * @return std::string 错误描述
     */
    static std::string getLastError();

private:
    /**
     * @brief libcurl 写回调函数
     *
     * @param contents 接收到的数据指针
     * @param size 数据块大小
     * @param nmemb 数据块数量
     * @param userp 用户指针（指向std::string）
     * @return size_t 实际处理的数据大小
     */
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

    // 错误信息缓冲区
    static thread_local std::string lastError;
};

#endif //JSONFETCHER_H
