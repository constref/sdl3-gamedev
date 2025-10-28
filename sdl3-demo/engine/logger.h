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
		//std::cout << std::format("[INFO] {}: {}", src, msg) << std::endl;
		std::cout << std::format("{}", typeid(SourceObj).name()) << std::endl;
	}
};