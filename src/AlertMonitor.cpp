//
// Created by athbe on 2025/6/17.
//
#include "AlertMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype> // for std::tolower

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

    // 读取关键词列表
    config.trigger_keywords = config_json["trigger_keywords"].get<std::vector<std::string>>();

    // 读取收件人列表
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

    // 输出关键词列表
    std::cout << "触发关键词: ";
    for (const auto& keyword : config.trigger_keywords) {
        std::cout << "\"" << keyword << "\" ";
    }
    std::cout << std::endl;

    // 输出收件人列表
    std::cout << "收件人: ";
    for (const auto& recipient : config.recipients) {
        std::cout << recipient << "; ";
    }
    std::cout << std::endl;

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

                // 检查所有匹配关键词的通知
                auto triggeredItems = checkForTrigger(jsonData.value(), config.trigger_keywords);

                if (!triggeredItems.empty()) {
                    std::cout << "检测到 " << triggeredItems.size()
                              << " 条包含所有关键词的通知" << std::endl;

                    // 发送邮件
                    std::string subject = "蓝桥杯大赛通知提醒";
                    std::string content = generateEmailContent(triggeredItems, config.trigger_keywords);

                    for (const auto& recipient : config.recipients) {
                        std::cout << "发送邮件到: " << recipient << std::endl;
                        if (MailSender::send(config.smtp, recipient, subject, content)) {
                            std::cout << "邮件发送成功" << std::endl;
                        } else {
                            std::cerr << "邮件发送失败" << std::endl;
                        }
                    }

                    alertTriggered = true; // 发送后停止监控
                } else {
                    std::cout << "未检测到包含所有关键词的通知" << std::endl;
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

// 检查标题是否包含所有关键词（不区分大小写）
bool AlertMonitor::containsAllKeywords(
    const std::string& title,
    const std::vector<std::string>& keywords
) {
    // 空关键词列表视为匹配所有
    if (keywords.empty()) {
        return true;
    }

    // 将标题转换为小写用于不区分大小写的匹配
    std::string lowerTitle = title;
    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    // 检查每个关键词
    for (const auto& keyword : keywords) {
        // 将关键词转换为小写
        std::string lowerKeyword = keyword;
        std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        // 如果标题中不包含关键词，返回false
        if (lowerTitle.find(lowerKeyword) == std::string::npos) {
            return false;
        }
    }

    return true;
}

std::vector<nlohmann::json> AlertMonitor::checkForTrigger(
    const nlohmann::json& json_data,
    const std::vector<std::string>& trigger_keywords
) {
    std::vector<nlohmann::json> matchedItems;

    // 检查数据是否包含datalist数组
    if (!json_data.contains("datalist") || !json_data["datalist"].is_array()) {
        return matchedItems;
    }

    // 遍历所有新闻项，收集匹配所有关键词的项
    for (const auto& item : json_data["datalist"]) {
        if (item.contains("title") && item["title"].is_string()) {
            std::string title = item["title"].get<std::string>();

            // 检查标题是否包含所有关键词
            if (containsAllKeywords(title, trigger_keywords)) {
                matchedItems.push_back(item);
            }
        }
    }

    // 按创建时间排序（最新在前）
    std::sort(matchedItems.begin(), matchedItems.end(), [](const auto& a, const auto& b) {
        std::string timeA = a.value("creatTime", "");
        std::string timeB = b.value("creatTime", "");
        return timeA > timeB; // 降序排序
    });

    return matchedItems;
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

std::string AlertMonitor::generateEmailContent(
    const std::vector<nlohmann::json>& news_items,
    const std::vector<std::string>& trigger_keywords
) {
    std::ostringstream content;

    content << "蓝桥杯大赛通知提醒\n\n";
    content << "检测到以下重要通知（包含所有关键词: ";
    for (size_t i = 0; i < trigger_keywords.size(); ++i) {
        content << "\"" << trigger_keywords[i] << "\"";
        if (i < trigger_keywords.size() - 1) {
            content << ", ";
        }
    }
    content << "）:\n\n";

    int count = 1;
    for (const auto& item : news_items) {
        content << "通知 #" << count++ << ":\n";
        content << "----------------------------\n";

        // 标题
        if (item.contains("title") && item["title"].is_string()) {
            content << "标题: " << item["title"].get<std::string>() << "\n";
        }

        // 创建时间
        if (item.contains("creatTime") && item["creatTime"].is_string()) {
            content << "发布时间: " << utcToBeijingTime(item["creatTime"].get<std::string>()) << "\n";
        }

        // 栏目名称
        if (item.contains("programaName") && item["programaName"].is_string()) {
            content << "栏目: " << item["programaName"].get<std::string>() << "\n";
        }

        // 简介
        if (item.contains("synopsis") && item["synopsis"].is_string()) {
            content << "内容简介: " << item["synopsis"].get<std::string>() << "\n";
        }

        // 通知链接
        if (item.contains("nnid")) {
            content << "通知链接: https://dasai.lanqiao.cn/notices/"
                    << item["nnid"].get<int>() << "\n";
        }

        content << "\n";
    }

    content << "----------------------------\n";
    content << "请及时登录蓝桥杯大赛官网查看完整信息。\n";
    content << "此邮件由自动监控系统生成，请勿直接回复。\n";

    return content.str();
}
