//
// Created by athbe on 2025/6/17.
//
#include "AlertMonitor.h"
#include <iostream>
#include <cstdlib>

int main() {
    try {
        // 加载配置文件
        AlertMonitor::Config config = AlertMonitor::loadConfig("config/settings.json");

        // 启动监控
        AlertMonitor::run(config);

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "\n错误: " << e.what() << std::endl;
        std::cerr << "程序异常终止" << std::endl;
        return EXIT_FAILURE;
    }
}


