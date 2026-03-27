#pragma once

#include <string>
#include <iostream>
#include <format>
#include <functional>

class Logger
{
public:
    static inline std::function<void(std::string)> logHandler = [](const std::string& message)
    {
        std::cout << message << std::endl;
    };

    template <typename SourceObj>
    static void warn(SourceObj *src, const std::string& msg)
    {
        logHandler(std::format("[WARN] {}: {}", typeid(SourceObj).name(), msg));
    }

    template <typename SourceObj>
    static void info(SourceObj *src, const std::string& msg)
    {
        logHandler(std::format("[INFO] {}: {}", typeid(SourceObj).name(), msg));
    }

    template <typename SourceObj>
    static void error(SourceObj *src, const std::string& msg)
    {
        logHandler(std::format("[ERROR] {}: {}", typeid(SourceObj).name(), msg));
    }
};
