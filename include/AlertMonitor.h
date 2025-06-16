//
// Created by athbe on 2025/6/17.
//

#ifndef ALERTMONITOR_H
#define ALERTMONITOR_H

#include <nlohmann/json.hpp>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <vector>
#include "JsonFetcher.h"
#include "MailSender.h"

class AlertMonitor {
public:
    struct Config {
        int check_interval;  // 检查间隔（秒）
        std::string target_url;  // 目标URL
        MailSender::SmtpConfig smtp;  // 邮件配置
        std::vector<std::string> trigger_keywords;  // 触发关键词列表
        std::vector<std::string> recipients;  // 收件人列表
    };

    /**
     * @brief 从配置文件加载配置
     *
     * @param config_path 配置文件路径
     * @return Config 解析后的配置对象
     */
    static Config loadConfig(const std::string& config_path);

    /**
     * @brief 启动监控循环
     *
     * @param config 监控配置
     */
    static void run(const Config& config);

private:
    /**
     * @brief 检查JSON数据是否包含触发关键词
     *
     * @param json_data JSON数据
     * @param trigger_keywords 触发关键词数组
     * @return std::optional<std::string> 包含触发关键词的新闻项（如果有）
     */
    static std::vector<nlohmann::json> checkForTrigger(
        const nlohmann::json& json_data,
        const std::vector<std::string>& trigger_keywords
    );

    /**
     * @brief 将UTC时间转换为北京时间字符串
     *
     * @param utc_time UTC时间字符串（格式：2025-06-16T09:11:44）
     * @return std::string 北京时间字符串
     */
    static std::string utcToBeijingTime(const std::string& utc_time);

    /**
     * @brief 生成邮件内容
     *
     * @param news_items 新闻项JSON数组
     * @param trigger_keywords 触发关键词数组
     * @return std::string 格式化后的邮件内容
     */
    static std::string generateEmailContent(
        const std::vector<nlohmann::json>& news_items,
        const std::vector<std::string>& trigger_keywords
    );

    /**
     * @brief 检查标题是否包含所有关键词
     *
     * @param title 新闻标题
     * @param keywords 关键词数组
     * @return true 如果标题包含所有关键词
     * @return false 如果标题不包含所有关键词
     */
    static bool containsAllKeywords(
        const std::string& title,
        const std::vector<std::string>& keywords
    );
};
#endif //ALERTMONITOR_H
