//
// Created by Gegel85 on 15/01/2019.
//

#ifndef THFGAME_LOGGER_HPP
#define THFGAME_LOGGER_HPP


#include <fstream>
#include <string>

class Logger {
private:
	std::ofstream	file;
public:
	explicit Logger(const std::string &filepath = "./latest.log") noexcept;
	~Logger() noexcept;

	void	msg(const std::string &content, const std::string &prepend = "") noexcept;
	void	info(const std::string &content) noexcept;
	void	warn(const std::string &content) noexcept;
	void	error(const std::string &content) noexcept;
	void	fatal(const std::string &content) noexcept;
};

extern Logger	logger;


#endif //THFGAME_LOGGER_HPP
