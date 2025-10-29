#pragma once

#include <string>
#include <iostream>
#include <format>

class Logger
{
public:
	template<typename SourceObj>
	static void warn(SourceObj *src, const std::string &msg)
	{
		std::cout << std::format("[WARN] {}: {}", typeid(SourceObj).name(), msg) << std::endl;
	}

	template<typename SourceObj>
	static void info(SourceObj *src, const std::string &msg)
	{
		std::cout << std::format("[INFO] {}: {}", typeid(SourceObj).name(), msg) << std::endl;
	}

	template<typename SourceObj>
	static void error(SourceObj *src, const std::string &msg)
	{
		std::cout << std::format("[ERROR] {}: {}", typeid(SourceObj).name(), msg) << std::endl;
	}
};