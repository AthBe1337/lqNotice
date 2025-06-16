cmake_minimum_required(VERSION 3.15)
project(lqNotice)

# 设置消息级别为详细
set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找 OpenSSL
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found. Install with: sudo apt install libssl-dev")
endif()

# 查找 CURL
find_package(CURL REQUIRED)
if(CURL_FOUND)
    message(STATUS "Found CURL: ${CURL_VERSION}")
    message(STATUS "CURL include dir: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL libraries: ${CURL_LIBRARIES}")
else()
    message(FATAL_ERROR "CURL not found. Install with: sudo apt install libcurl4-openssl-dev")
endif()

# 处理 nlohmann/json
find_package(nlohmann_json 3.11.2 QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Found nlohmann_json: ${nlohmann_json_VERSION}")
else()
    message(STATUS "nlohmann_json not found, using FetchContent")

    include(FetchContent)
    FetchContent_Declare(
            json
            URL https://github.com/nlohmann/json/releases/download/v3.11.2/include.zip
            URL_HASH SHA256=e5c7a9f49a16814be27e4ed0ee900ecd0092bfb7dbfca65b5a421b774dccbfa3
    )

    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
        FetchContent_Populate(json)
    endif()

    # 创建接口库而不是常规库
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json INTERFACE ${json_SOURCE_DIR})
endif()

# 添加可执行文件
add_executable(lqNotice
        src/main.cpp
        src/JsonFetcher.cpp
        src/MailSender.cpp
        src/AlertMonitor.cpp
        #src/MailSenderTest.cpp
)

message(STATUS "current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

# 包含目录
target_include_directories(lqNotice PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CURL_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)

# 链接库
target_link_libraries(lqNotice PRIVATE
        # nlohmann_json 是头文件库，只需包含路径，不链接二进制
        ${CURL_LIBRARIES}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        CURL::libcurl
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
        dl
)

# 添加头文件库的包含路径
if(TARGET nlohmann_json)
    target_link_libraries(lqNotice PRIVATE nlohmann_json)
else()
    target_include_directories(lqNotice PRIVATE ${nlohmann_json_INCLUDE_DIRS})
endif()

# 添加编译定义（如果需要）
#target_compile_definitions(lqNotice PRIVATE
        #-D_GLIBCXX_USE_CXX11_ABI=1
#)

# 设置链接器选项
set_target_properties(lqNotice PROPERTIES
        LINK_FLAGS "-Wl,--as-needed"
)

execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-search-dirs
        OUTPUT_VARIABLE COMPILER_SEARCH_DIRS)
message(STATUS "Compiler search dirs:\n${COMPILER_SEARCH_DIRS}")
