//
// Created by athbe on 2025/6/17.
//

#ifndef MAILSENDER_H
#define MAILSENDER_H
#include <string>
#include <vector>
#include <curl/curl.h>

class MailSender {
public:
    // SMTP服务器配置结构体
    struct SmtpConfig {
        std::string server;
        int port = 465; // 默认使用465端口
        std::string username;
        std::string password;
        bool useSsl = true; // 默认使用SSL
    };

    /**
     * @brief 发送邮件
     *
     * @param config SMTP服务器配置
     * @param recipient 收件人邮箱地址
     * @param subject 邮件主题
     * @param message 邮件正文
     * @return true 发送成功
     * @return false 发送失败
     */
    static bool send(const SmtpConfig& config,
                    const std::string& recipient,
                    const std::string& subject,
                    const std::string& message);

    /**
     * @brief 发送邮件（支持多个收件人）
     *
     * @param config SMTP服务器配置
     * @param recipients 收件人邮箱地址列表
     * @param subject 邮件主题
     * @param message 邮件正文
     * @return true 发送成功
     * @return false 发送失败
     */
    static bool send(const SmtpConfig& config,
                    const std::vector<std::string>& recipients,
                    const std::string& subject,
                    const std::string& message);

    /**
     * @brief 获取libcurl版本信息
     *
     * @return std::string libcurl版本字符串
     */
    static std::string getCurlVersion();

    /**
     * @brief 初始化CURL环境（可选，线程安全）
     */
    static void globalInit();

    /**
     * @brief 清理CURL环境（可选）
     */
    static void globalCleanup();

private:
    /**
     * @brief 初始化CURL并发送邮件
     *
     * @param curl CURL对象
     * @param config SMTP配置
     * @param recipients 收件人列表
     * @param email_text 完整的邮件内容
     * @return true 发送成功
     * @return false 发送失败
     */
    static bool performSend(CURL* curl,
                           const SmtpConfig& config,
                           const std::vector<std::string>& recipients,
                           const std::string& email_text);

    /**
     * @brief 构建邮件头部信息
     *
     * @param from 发件人
     * @param recipients 收件人列表
     * @param subject 邮件主题
     * @return std::string 邮件头部字符串
     */
    static std::string buildEmailHeader(const std::string& from,
                                      const std::vector<std::string>& recipients,
                                      const std::string& subject);
};


#endif //MAILSENDER_H
