//
// Created by athbe on 2025/6/17.
//
#include "MailSender.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>

// 用于libcurl读取邮件内容数据的回调函数
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
    std::string *text = static_cast<std::string*>(userp);
    size_t total = size * nmemb;

    if (text->empty())
        return 0;

    size_t to_copy = std::min(total, text->size());
    memcpy(ptr, text->c_str(), to_copy);
    text->erase(0, to_copy);

    return to_copy;
}

// 初始化全局CURL环境（线程安全）
void MailSender::globalInit() {
    static bool initialized = false;
    if (!initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        initialized = true;
    }
}

// 清理全局CURL环境
void MailSender::globalCleanup() {
    curl_global_cleanup();
}

// 获取libcurl版本信息
std::string MailSender::getCurlVersion() {
    return curl_version();
}

// 构建邮件头部信息
std::string MailSender::buildEmailHeader(const std::string& from,
                                       const std::vector<std::string>& recipients,
                                       const std::string& subject) {
    std::ostringstream header;

    // 发件人
    header << "From: " << from << "\r\n";

    // 收件人
    header << "To: ";
    for (size_t i = 0; i < recipients.size(); ++i) {
        if (i > 0) header << ", ";
        header << recipients[i];
    }
    header << "\r\n";

    // 主题
    header << "Subject: " << subject << "\r\n";

    // MIME版本和内容类型
    header << "MIME-Version: 1.0\r\n";
    header << "Content-Type: text/plain; charset=UTF-8\r\n";
    header << "Content-Transfer-Encoding: 8bit\r\n";

    // 空行分隔头部和正文
    header << "\r\n";

    return header.str();
}

bool MailSender::send(const SmtpConfig& config,
                     const std::string& recipient,
                     const std::string& subject,
                     const std::string& message) {
    return send(config, std::vector<std::string>{recipient}, subject, message);
}

bool MailSender::send(const SmtpConfig& config,
                     const std::vector<std::string>& recipients,
                     const std::string& subject,
                     const std::string& message) {
    // 初始化CURL全局环境（线程安全）
    globalInit();

    // 初始化CURL会话
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }

    // 构建完整的邮件内容
    std::string header = buildEmailHeader(config.username, recipients, subject);
    std::string email_text = header + message + "\r\n";

    bool result = performSend(curl, config, recipients, email_text);
    curl_easy_cleanup(curl);
    return result;
}

bool MailSender::performSend(CURL* curl,
                            const SmtpConfig& config,
                            const std::vector<std::string>& recipients,
                            const std::string& email_text) {
    CURLcode res = CURLE_OK;
    struct curl_slist *recipient_list = nullptr;

    // 设置收件人列表
    for (const auto& recipient : recipients) {
        recipient_list = curl_slist_append(recipient_list, recipient.c_str());
    }

    // 复制邮件内容，因为回调函数会修改字符串
    std::string email_copy = email_text;

    // 设置服务器地址和端口
    std::string url = (config.useSsl ? "smtps://" : "smtp://") + config.server;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_PORT, config.port);

    // 设置用户名和密码
    curl_easy_setopt(curl, CURLOPT_USERNAME, config.username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, config.password.c_str());

    // 设置发件人
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, config.username.c_str());

    // 设置收件人列表
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipient_list);

    // 设置邮件内容回调函数
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &email_copy);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    // SSL/TLS 配置
    if (config.useSsl) {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL); // 强制使用SSL
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);      // 验证SSL证书
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);      // 严格主机验证

        // 设置CA证书路径（可选，如果系统证书不可用）
        // curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
    }

    // 设置超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // 启用TCP保活
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);

    // 开启详细日志（调试时使用）
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // 执行发送
    res = curl_easy_perform(curl);

    // 清理收件人列表
    curl_slist_free_all(recipient_list);

    // 检查结果
    if (res != CURLE_OK) {
        std::cerr << "Mail sending failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    return true;
}
