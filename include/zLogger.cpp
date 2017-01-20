#include "engine.h"
#include <stdarg.h>
#define MAX_LOG_SIZE 512

zLogger::zLogger(const std::string& name, const std::string& logPath) :append_(new DailyRollingFileAppender(logPath.c_str(), HOURLY, true, 0, true, false)), name(name)
{
	_logger = Logger::getInstance(name.c_str());
	_logger.addAppender(append_);
	
	//this->_logger = _logger;
}

zLogger::~zLogger()
{

}

void zLogger::debug(const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int count = vsnprintf(NULL, 0, format, argptr);
	va_end(argptr);

	va_start(argptr, format);
	char* buf = (char*)malloc(count*sizeof(char));
	vsnprintf(buf, count, format, argptr);
	va_end(argptr);

	std::string str(buf, count);
	free(buf);


	
	LOG4CPLUS_DEBUG(_logger, str.c_str());
	
}
void zLogger::error(const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int count = vsnprintf(NULL, 0, format, argptr);
	va_end(argptr);

	va_start(argptr, format);
	char* buf = (char*)malloc(count*sizeof(char));
	vsnprintf(buf, count, format, argptr);
	va_end(argptr);

	std::string str(buf, count);
	free(buf);
	LOG4CPLUS_ERROR(_logger, str.c_str());
	
}
void zLogger::info(const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int count = vsnprintf(NULL, 0, format, argptr);
	va_end(argptr);

	va_start(argptr, format);
	char* buf = (char*)malloc(count*sizeof(char));
	vsnprintf(buf, count, format, argptr);
	va_end(argptr);

	std::string str(buf, count);
	free(buf);
	LOG4CPLUS_INFO(_logger, str.c_str());
	
}
void zLogger::fatal(const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int count = vsnprintf(NULL, 0, format, argptr);
	va_end(argptr);

	va_start(argptr, format);
	char* buf = (char*)malloc(count*sizeof(char));
	vsnprintf(buf, count, format, argptr);
	va_end(argptr);

	std::string str(buf, count);
	free(buf);
	LOG4CPLUS_FATAL(_logger, str.c_str());
	
}
void zLogger::warn(const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int count = vsnprintf(NULL, 0, format, argptr);
	va_end(argptr);

	va_start(argptr, format);
	char* buf = (char*)malloc(count*sizeof(char));
	vsnprintf(buf, count, format, argptr);
	va_end(argptr);

	std::string str(buf, count);
	free(buf);
	LOG4CPLUS_WARN(_logger, str.c_str());
	
}