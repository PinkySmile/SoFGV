//
// Created by PinkySmile on 15/01/2019.
//

#ifndef THFGAME_LOGGER_HPP
#define THFGAME_LOGGER_HPP


#include <fstream>
#include <string>
#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace SpiralOfFate
{
	class Logger {
	private:
		std::ofstream	file;
	public:
		explicit Logger(const std::string &filepath = "./latest.log") noexcept;
		~Logger() noexcept;

#ifdef __ANDROID__
		void msg(const std::string &content, int prio = ANDROID_LOG_DEFAULT) noexcept;
#else
		void msg(const std::string &content, const std::string &prepend = "") noexcept;
#endif
		void debug(const std::string &content) noexcept;
		void info(const std::string &content) noexcept;
		void warn(const std::string &content) noexcept;
		void error(const std::string &content) noexcept;
		void fatal(const std::string &content) noexcept;
	};
}


#endif //THFGAME_LOGGER_HPP
