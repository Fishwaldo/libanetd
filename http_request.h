#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>

#include <exception>
#include <map>
#include <string>
#include <vector>

#include "http_response.h"


class http_request
{
public:
	http_request(http_response *response, boost::asio::io_service *postback);
	~http_request();
	void reset();
	bool Starttransfer(std::string url);
	typedef boost::function<void (http_response *)> t_callbackFunc;
	bool setCallback(t_callbackFunc);
	boost::unique_future<http_response*> Status;
	class connection_exception: public std::exception { };
	class server_connection_exception: public std::exception { };
	class policy_file_request_exception: public std::exception { };
private:
	size_t socksend(std::string data);
	size_t sockget(char *data, size_t size);
	bool verify_callback(bool preverified, boost::asio::ssl::verify_context &vctx);
	void send();
	void send(std::string absolute_url);
	void receive();
	void disconnect();
	void callback(http_response *res);
	void clear();
	http_response *connect(std::string url);

	std::string method;
	std::string host;
	std::string url;
	std::string targeturl;
	std::string version;
	int status;
	std::string description;
	std::map<std::string, std::string> arguments;
	std::map<std::string, std::string> headers;
	std::string body;
	http_response *response;
	t_callbackFunc CallbackFunction;
	enum http_response_parser_state { VERSION, STATUS, DESCRIPTION, HEADER_KEY, HEADER_VALUE, BODY, OK };
	enum http_proxy_enum { NONE, HTTP_PROXY, HTTPS_PROXY};
	enum http_type_enum { PLAIN_HTTP, SSL_HTTPS};

	http_proxy_enum http_proxy;
	http_type_enum http_type;
	boost::asio::io_service io;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ssl::context ctx;
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> sslsocket;
	boost::asio::io_service *postbackio;
};


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string NowTime()
{
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in NowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
    return result;
}

#else

#include <sys/time.h>

inline std::string NowTime()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
    return result;
}

#endif //WIN32


typedef enum {LOG_TRACE, LOG_DEBUG, LOG_WARN, LOG_ERROR, LOG_CRITICAL} TLogLevels;

template <typename T>
class LogClass {
public:
	LogClass() {  };
    virtual ~LogClass() {
    	os << std::endl;
    	T::Output(os.str());
    }
    std::ostringstream& Get(TLogLevels level = LOG_TRACE) {
    	os << "- " << NowTime();
    	os << " " << ToString(level) << ": ";
    	os << std::string(level > LOG_CRITICAL ? level - LOG_CRITICAL : 0, '\t');
    	return os;
    }
    static TLogLevels& ReportingLevel() {
        static TLogLevels reportingLevel = LOG_TRACE;
        return reportingLevel;
    }
    static std::string ToString(TLogLevels level) {
   		static const char* const buffer[] = {"TRACE", "DEBUG", "WARN", "ERROR", "CRITICAL"};
   	    return buffer[level];
    }
    static TLogLevels FromString(const std::string& level) {
        if (level == "TRACE")
            return LOG_TRACE;
        if (level == "DEBUG")
            return LOG_DEBUG;
        if (level == "WARN")
            return LOG_WARN;
        if (level == "ERROR")
            return LOG_ERROR;
        if (level == "CRITICAL")
            return LOG_CRITICAL;
        LogClass<T>().Get(LOG_WARN) << "Unknown logging level '" << level << "'. Using WARN level as default.";
        return LOG_WARN;
    }
protected:
    std::ostringstream os;
private:
    LogClass(const LogClass&);
    LogClass& operator =(const LogClass&);
};

class Output2StdErr
{
public:
    static FILE*& Stream() {
        static FILE* pStream = stderr;
        return pStream;
    }
    static void Output(const std::string& msg) {
    	std::cerr << msg;
    }
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   if defined (BUILDING_HTTPLOG_DLL)
#       define HTTPLOG_DECLSPEC   __declspec (dllexport)
#   elif defined (USING_HTTPLOG_DLL)
#       define HTTPLOG_DECLSPEC   __declspec (dllimport)
#   else
#       define HTTPLOG_DECLSPEC
#   endif // BUILDING_HTTPLOG_DLL
#else
#   define HTTPLOG_DECLSPEC
#endif // _WIN32

class HTTPLOG_DECLSPEC HTTPLog : public LogClass<Output2StdErr> {};

#define LogFormat() << boost::this_thread::get_id() << ": "  << __FILE__ << ":" <<  __LINE__ << "(" << __FUNCTION__ << "): "

#define LogTrace() if (LOG_TRACE >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_TRACE) LogFormat()

#define LogDebug() if (LOG_DEBUG >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_DEBUG) LogFormat()
#define LogWarn() if (LOG_WARN >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_WARN) LogFormat()
#define LogError() if (LOG_ERROR >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_ERROR) LogFormat()
#define LogCritical() if (LOG_CRITICAL >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_CRITICAL) LogFormat()


#endif // HTTP_REQUEST_H
