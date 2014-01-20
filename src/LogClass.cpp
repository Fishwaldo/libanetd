/* libanetd - LogClass.cpp
 ** Copyright (c) 2010 Justin Hammond
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation; either version 2 of the License, or
 **  (at your option) any later version.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 **  USA
 **
 ** libanetd SVN Identification:
 ** $Rev$
 */

/** @file LogClass.cpp
 *  @brief
 */

#include <sys/time.h>
#include <stdio.h>
#include <vector>
#include "anetd/LogClass.hpp"



using namespace DynamX::Logging;


inline std::string getLevelString(LogLevel lvl) {
	switch (lvl) {
		case LogLevel_None:
			return "None";
		case LogLevel_Fatal:
			return "Fatal";
		case LogLevel_Error:
			return "Error";
		case LogLevel_Warning:
			return "Warning";
		case LogLevel_Info:
			return "Info";
		case LogLevel_Debug:
			return "Debug";
	}
	return "Unknown";
}


Log* Log::s_instance = NULL;
i_LogImpl* Log::m_pImpl = NULL;
LogLevel Log::currentlevel = LogLevel_Fatal;

//-----------------------------------------------------------------------------
//	<Log::Create>
//	Static creation of the singleton
//-----------------------------------------------------------------------------
Log* Log::Create
(
		std::string const& _filename,
		bool const _bConsoleOutput,
		LogLevel const _saveLevel
)
{
	if( NULL == s_instance )
	{
		s_instance = new Log( _filename, _bConsoleOutput, _saveLevel);
	}

	return s_instance;
}


//-----------------------------------------------------------------------------
//	<Log::Destroy>
//	Static method to destroy the logging singleton.
//-----------------------------------------------------------------------------
void Log::Destroy
(
)
{
	delete s_instance;
	s_instance = NULL;
}

//-----------------------------------------------------------------------------
//	<Log::SetLoggingClass>
//	Set log class
//-----------------------------------------------------------------------------
bool Log::SetLoggingClass
(
		i_LogImpl *LogClass
)
{
	delete m_pImpl;
	m_pImpl = LogClass;
	return true;
}



//-----------------------------------------------------------------------------
//	<Log::SetLoggingState>
//	Set flag to actually write to log or skip it
//-----------------------------------------------------------------------------
void Log::SetLoggingState
(
		LogLevel _saveLevel
)
{
	currentlevel = _saveLevel;
}

//-----------------------------------------------------------------------------
//	<Log::GetLoggingState>
//	Return a flag to indicate whether logging is enabled
//-----------------------------------------------------------------------------
LogLevel Log::GetLoggingState
(
)
{
	currentlevel;
}

//-----------------------------------------------------------------------------
//	<Log::Write>
//	Write to the log
//-----------------------------------------------------------------------------
void Log::Write
(
		LogLevel _level,
		std::string message
)
{
	if( s_instance &&s_instance->m_pImpl )
	{
		s_instance->m_pImpl->Write( _level, message );
	}
}



//-----------------------------------------------------------------------------
//	<Log::SetLogFileName>
//	Change the name of the log file (will start writing a new file)
//-----------------------------------------------------------------------------
void Log::SetLogFileName
(
		std::string _filename
)
{
	if( s_instance && s_instance->m_pImpl )
	{
		s_instance->m_pImpl->SetLogFileName( _filename );
	}
}

//-----------------------------------------------------------------------------
//	<Log::Log>
//	Constructor
//-----------------------------------------------------------------------------
Log::Log
(
		std::string const& _filename,
		bool const _bConsoleOutput,
		LogLevel const _saveLevel
)
{
	if (NULL == m_pImpl)
		m_pImpl = new LogImpl( _filename,_bConsoleOutput, _saveLevel);

}

//-----------------------------------------------------------------------------
//	<Log::~Log>
//	Destructor
//-----------------------------------------------------------------------------
Log::~Log
(
)
{
	delete m_pImpl;
}


//-----------------------------------------------------------------------------
//	<LogImpl::LogImpl>
//	Constructor
//-----------------------------------------------------------------------------
LogImpl::LogImpl
(
		std::string const& _filename,
		bool const _bConsoleOutput,
		LogLevel const _saveLevel
):
i_LogImpl(_filename,_bConsoleOutput, _saveLevel)
{
	m_saveLevel = _saveLevel;
	m_bConsoleOutput = _bConsoleOutput;
}

//-----------------------------------------------------------------------------
//	<LogImpl::~LogImpl>
//	Destructor
//-----------------------------------------------------------------------------
LogImpl::~LogImpl
(
)
{
}

//-----------------------------------------------------------------------------
//	<LogImpl::Write>
//	Write to the log
//-----------------------------------------------------------------------------
void LogImpl::Write
(
		LogLevel _logLevel,
		std::string message
)
{
	char lineBuf[1024] = {};
	int lineLen = 0;
	if (!m_bConsoleOutput)
		return;
	//std::cout << this->m_saveLevel << " " << _logLevel << std::endl;
	if (this->m_saveLevel < _logLevel) {
		return;
	}
	if (_logLevel >= LogLevel_Debug +1 ) {
		_logLevel = LogLevel_Debug;
	}
	std::cout << GetTimeStampString() << " " << getLevelString(_logLevel) << " "<<  message << std::endl;

}


//-----------------------------------------------------------------------------
//	<LogImpl::SetLoggingState>
//	Sets the various log state variables
//-----------------------------------------------------------------------------
void LogImpl::SetLoggingState
(
		LogLevel _saveLevel
)
{
	m_saveLevel = _saveLevel;
}

//-----------------------------------------------------------------------------
//	<LogImpl::GetTimeStampString>
//	Generate a string with formatted current time
//-----------------------------------------------------------------------------
std::string LogImpl::GetTimeStampString
(
)
{
	// Get a timestamp
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm *tm;
	tm = localtime( &tv.tv_sec );

	// create a time stamp string for the log message
	char buf[100];
	snprintf( buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d ",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec / 1000 );
	std::string str = buf;
	return str;
}



//-----------------------------------------------------------------------------
//	<LogImpl::SetLogFileName>
//	Provide a new log file name (applicable to future writes)
//-----------------------------------------------------------------------------
void LogImpl::SetLogFileName
(
		std::string _filename
)
{
}


