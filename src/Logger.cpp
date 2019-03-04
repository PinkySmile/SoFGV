//
// Created by Gegel85 on 15/01/2019.
//

#include <iostream>
#include <iomanip>
#include <ctime>
#include "Logger.hpp"

Logger::Logger(const std::string &filepath) noexcept :
	file(std::ofstream("./latest.log", std::ios::out))
{
}

Logger::~Logger() noexcept
{
	this->file.close();
}

void Logger::msg(const std::string &content, const std::string &prepend) noexcept
{
	std::stringstream	logged_str;
	time_t			timestamp = std::time(nullptr);
	struct tm		*tm = std::localtime(&timestamp);

	logged_str << std::put_time(tm, "[%d-%m-%Y][%H:%M:%S]") << prepend << ": " << content << std::endl;
	this->file << logged_str.str();
	std::cout << logged_str.str();
}

void Logger::warn(const std::string &content) noexcept
{
	this->msg(content, "[WARNING]");
}

void Logger::info(const std::string &content) noexcept
{
	this->msg(content, "[INFO]");
}

void Logger::error(const std::string &content) noexcept
{
	this->msg(content, "[ERROR]");
}

void Logger::fatal(const std::string &content) noexcept
{
	this->msg(content, "[FATAL]");
}
