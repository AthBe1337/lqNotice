//
// Created by athbe on 2025/6/17.
//
#include "AlertMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>

AlertMonitor::Config AlertMonitor::loadConfig(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file) {
        throw std::runtime_error("无法打开配置文件: " + config_path);
    }

    nlohmann::json config_json;
    try {
        config_file >> config_json;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON解析错误: " + std::string(e.what()));
    }

    Config config;
    config.check_interval = config_json["check_interval"];
    config.target_url = config_json["target_url"];
    config.trigger_key = config_json["trigger_key"];
    config.recipients = config_json["recipients"].get<std::vector<std::string>>();

    // 解析SMTP配置
    auto& smtp_json = config_json["smtp"];
    config.smtp.server = smtp_json["server"];
    config.smtp.port = smtp_json["port"];
    config.smtp.username = smtp_json["username"];
    config.smtp.password = smtp_json["password"];

    // 根据"security"字段设置SSL
    std::string security = smtp_json["security"];
    config.smtp.useSsl = (security == "ssl" || security == "tls");

    return config;
}

void AlertMonitor::run(const Config& config) {
    std::cout << "启动监控服务..." << std::endl;
    std::cout << "监控URL: " << config.target_url << std::endl;
    std::cout << "检查间隔: " << config.check_interval << "秒" << std::endl;
    std::cout << "触发关键词: " << config.trigger_key << std::endl;

    bool alertTriggered = false;
    int checkCount = 0;

    while (!alertTriggered) {
        checkCount++;
        std::cout << "\n=== 检查 #" << checkCount << " ===" << std::endl;
        std::cout << "获取JSON数据..." << std::endl;

        try {
            auto jsonData = JsonFetcher::fetchFromUrl(config.target_url);

            if (jsonData) {
                std::cout << "成功获取JSON数据" << std::endl;

                // 检查是否包含触发关键词
                auto triggeredItem = checkForTrigger(jsonData.value(), config.trigger_key);

                if (triggeredItem) {
                    std::cout << "检测到触发关键词: " << config.trigger_key << std::endl;
                    std::cout << "标题: " << triggeredItem.value()["title"].get<std::string>() << std::endl;

                    // 发送邮件
                    std::string subject = "蓝桥杯大赛通知提醒";
                    std::string content = generateEmailContent(triggeredItem.value());

                    for (auto recipient : config.recipients) {
                        std::cout << "发送邮件到: " << recipient << std::endl;
                        if (MailSender::send(config.smtp, recipient, subject, content)) {
                            std::cout << "邮件发送成功" << std::endl;
                            alertTriggered = true;
                        } else {
                            std::cerr << "邮件发送失败" << std::endl;
                        }
                    }
                } else {
                    std::cout << "未检测到触发关键词" << std::endl;
                }
            } else {
                std::cerr << "获取JSON数据失败: " << JsonFetcher::getLastError() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "发生异常: " << e.what() << std::endl;
        }

        // 如果不是最后一次检查，等待指定时间
        if (!alertTriggered) {
            std::cout << "等待下一次检查..." << std::endl;
            for (int i = 0; i < config.check_interval; i += 10) {
                if (i % 60 == 0 && i > 0) {
                    std::cout << "已等待 " << i / 60 << " 分钟..." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
    }

    std::cout << "\n监控服务已停止" << std::endl;
}

std::optional<nlohmann::json> AlertMonitor::checkForTrigger(
    const nlohmann::json& json_data,
    const std::string& trigger_key
) {
    // 检查数据是否包含datalist数组
    if (!json_data.contains("datalist") || !json_data["datalist"].is_array()) {
        return std::nullopt;
    }

    // 遍历所有新闻项
    for (const auto& item : json_data["datalist"]) {
        if (item.contains("title") && item["title"].is_string()) {
            std::string title = item["title"].get<std::string>();

            // 检查标题是否包含关键词
            if (title.find(trigger_key) != std::string::npos) {
                return item;
            }
        }
    }

    return std::nullopt;
}

std::string AlertMonitor::utcToBeijingTime(const std::string& utc_time) {
    // 解析UTC时间字符串
    std::tm tm = {};
    std::istringstream ss(utc_time);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail()) {
        return "时间解析失败";
    }

    // 转换为time_t (UTC时间)
    time_t utc_t = timegm(&tm);

    // 转换为北京时间 (UTC+8)
    utc_t += 8 * 3600;

    // 转换回tm结构
    std::tm* beijing_tm = gmtime(&utc_t);

    // 格式化为字符串
    std::ostringstream oss;
    oss << std::put_time(beijing_tm, "%Y-%m-%d %H:%M:%S");
    oss << " (北京时间)";

    return oss.str();
}

std::string AlertMonitor::generateEmailContent(const nlohmann::json& news_item) {
    std::ostringstream content;

    content << "蓝桥杯大赛通知提醒\n\n";
    content << "检测到新的重要通知：\n\n";

    // 标题
    if (news_item.contains("title") && news_item["title"].is_string()) {
        content << "标题: " << news_item["title"].get<std::string>() << "\n";
    }

    // 创建时间
    if (news_item.contains("creatTime") && news_item["creatTime"].is_string()) {
        content << "发布时间: " << utcToBeijingTime(news_item["creatTime"].get<std::string>()) << "\n";
    }

    // 栏目名称
    if (news_item.contains("programaName") && news_item["programaName"].is_string()) {
        content << "栏目: " << news_item["programaName"].get<std::string>() << "\n";
    }

    // 简介
    if (news_item.contains("synopsis") && news_item["synopsis"].is_string()) {
        content << "\n内容简介:\n";
        content << news_item["synopsis"].get<std::string>() << "\n";
    }
    // 链接
    if (news_item.contains("nnid")) {
        content << "\n通知链接: https://dasai.lanqiao.cn/notices/" << news_item["nnid"] << "\n";
    }
    content << "\n请及时登录蓝桥杯大赛官网查看完整信息。\n";
    content << "此邮件由自动监控系统生成，请勿直接回复。\n";

    return content.str();
}
