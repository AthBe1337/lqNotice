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
#include "JsonFetcher.h"
#include "MailSender.h"

class AlertMonitor {
public:
    struct Config {
        int check_interval;  // 检查间隔（秒）
        std::string target_url;  // 目标URL
        MailSender::SmtpConfig smtp;  // 邮件配置
        std::string trigger_key;  // 触发关键词
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
     * @param trigger_key 触发关键词
     * @return std::optional<std::string> 包含触发关键词的新闻项（如果有）
     */
    static std::optional<nlohmann::json> checkForTrigger(
        const nlohmann::json& json_data,
        const std::string& trigger_key
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
     * @param news_item 新闻项JSON对象
     * @return std::string 格式化后的邮件内容
     */
    static std::string generateEmailContent(const nlohmann::json& news_item);
};
#endif //ALERTMONITOR_H
