/*******************************************************
** class name:  Logger
**
** description: Log to file:
**                LError(), LInfo(), LDebug() ...
**
** last change: 2019-12-20
*******************************************************/
#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <iostream>


class Logger {
public:
	Logger() :
			logger(spdlog::basic_logger_mt("basic", "logs/basic-logger.txt")) {
		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::err);
		spdlog::flush_every(std::chrono::seconds(1));
	}
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	~Logger() { spdlog::drop_all(); }
public:
	static Logger& GetInstance() {
		static Logger m_instance;
		return m_instance;
	}
	auto GetLogger() { return logger; }

public:
private:
	std::shared_ptr<spdlog::logger> logger;
};


#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

#ifndef suffixsth
#define suffixsth(msg) std::string(msg).append(" <")\
	.append(__FILENAME__).append("> <").append(__func__)\
	.append("> <").append(std::to_string(__LINE__))\
	.append(">").c_str()
#endif // suffix


// print in console
#ifdef NDEBUG
#define Log(x)
#define LogF(fmt, ...)
#define LogEx(fmt, ...)
#else
#include <iostream>
#define Log(x) std::cout << x << std::endl
#define LogF(fmt, ...) printf("%s"##fmt"\n", ##__VA_ARGS__)
#define LogEx(fmt, ...) printf("%s(%d)-<%s>: "##fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif


// output to file
#define LTrace(msg, ...) Logger::GetInstance().GetLogger()->trace(suffixsth(msg), __VA_ARGS__)
#define LDebug(...) Logger::GetInstance().GetLogger()->debug(__VA_ARGS__)
#define LInfo(...) Logger::GetInstance().GetLogger()->info(__VA_ARGS__)
#define LWarn(...) Logger::GetInstance().GetLogger()->warn(__VA_ARGS__)
#define LError(...) Logger::GetInstance().GetLogger()->error(__VA_ARGS__)
#define LCritical(...) Logger::GetInstance().GetLogger()->critical(__VA_ARGS__)
