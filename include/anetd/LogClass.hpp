/* libanetd - LogClass.hpp
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

/** @file LogClass.hpp
 *  @brief
 */



#ifndef LOGCLASS_HPP_
#define LOGCLASS_HPP_

#include <string>
#include <boost/format.hpp>

namespace DynamX {
	namespace Logging {

		enum LogLevel
		{
			LogLevel_None,		/**< Disable all logging */
			LogLevel_Fatal,		/**< A likely fatal issue in the library */
			LogLevel_Error,		/**< A serious issue with the library or the network */
			LogLevel_Warning,	/**< A minor issue from which the library should be able to recover */
			LogLevel_Info,		/**< Everything's working fine...these messages provide streamlined feedback on each message */
			LogLevel_Debug		/**< Very detailed information on progress that will create a huge log file quickly
										But this level (as others) can be queued and sent to the log only on an error or warning */
		};

		class i_LogImpl
		{
			public:
				i_LogImpl(std::string const& _filename, bool const _bConsoleOutput, LogLevel const _saveLevel) {}
				virtual ~i_LogImpl() {};
				virtual void Write( LogLevel _level, std::string ) = 0;
				virtual void SetLoggingState( LogLevel _saveLevel) = 0;
				virtual void SetLogFileName( std::string _filename ) = 0;
			private:
				i_LogImpl() {};
		};

		/** \brief Implements a platform-independent log...written to the console and, optionally, a file.
		 */
		class Log
		{
			public:
				/**
				 * Create a log.
				 * Creates the cross-platform logging singleton.
				 * Any previous log will be cleared.
				 * \return a pointer to the logging object.
				 * \see Destroy, Write
				 */
				static Log* Create( std::string const& _filename, bool const _bConsoleOutput, LogLevel const _LogLevel);


				/**
				 * Destroys the log.
				 * Destroys the logging singleton.  The log can no longer
				 * be written to without another call to Create.
				 * \see Create, Write
				 */
				static void Destroy();

				/**
				 * \brief Set the Logging Implmentation Class to replace the standard File/Console Loggin
				 * \param LogClass A Logging Class that inherits the i_LogImpl Class used to Log to
				 * \return Bool Value indicating success or failure
				 */
				static bool SetLoggingClass(i_LogImpl *LogClass );

				/**
				 * \brief Enable or disable library logging.  To disable, set _saveLevel and _queueLevel
				 * to LogLevel_None.
				 * \param _saveLevel	LogLevel of messages to write in real-time
				 */
				static void SetLoggingState( LogLevel _saveLevel);

				/**
				 * \brief Determine whether logging is enabled or not (retained for backward compatibility)
				 * \return The LogLevel Set
				 */
				static LogLevel GetLoggingState();

				/**
				 * \brief Change the log file name.  This will start a new log file (or potentially start appending
				 * information to an existing one.  Developers might want to use this function, together with a timer
				 * in the controlling application, to create timestamped log file names.
				 * \param _filename Name of the new (or existing) file to use for log output.
				 */
				static void SetLogFileName( std::string _filename );

				/**
				 * Write an entry to the log.
				 * Writes a formatted string to the log.
				 * \param _level	Specifies the type of log message (Error, Warning, Debug, etc.)
				 * \param _format.  A string formatted in the same manner as used with printf etc.
				 * \param ... a variable number of arguments, to be included in the formatted string.
				 * \see Create, Destroy
				 */
				static void Write( LogLevel _level, std::string);


			private:
				Log( std::string const& _filename, bool const _bConsoleOutput, LogLevel _saveLevel);
				~Log();

				static i_LogImpl*	m_pImpl;		/**< Pointer to an object that encapsulates the platform-specific logging implementation. */
				static Log*	s_instance;
				static LogLevel currentlevel;
		};

		class LogImpl : public i_LogImpl
		{
		private:
			friend class Log;

			LogImpl( std::string const& _filename, bool const _bConsoleOutput, LogLevel const _saveLevel);
			~LogImpl();

			void Write( LogLevel _level, std::string message );
			void SetLoggingState( LogLevel _saveLevel);
			void SetLogFileName( std::string _filename );

			std::string GetTimeStampString();

			LogLevel m_saveLevel;
			bool m_bConsoleOutput;
		};

#define LogFormat(MSG) boost::str(boost::format("%1% - %2% (%3%:%4%) - %5%") % boost::lexical_cast<std::string>(boost::this_thread::get_id()) % __FUNCTION__ % __FILE__ % __LINE__ % MSG )
		/*! \def LogDebug()
		 * \relates LogClass
		 * Log a Message at LogDebug Level
		 */
#define LogDebug(Y)   Log::Write(LogLevel_Debug, LogFormat(Y))
		/*! \def LogInfo()
		 * \relates LogClass
		 * Log a Message at LogInfo level
		 */
#define LogInfo(Y) Log::Write(LogLevel_Info, LogFormat(Y))
		/*! \def LogWarn()
		 * \relates LogClass
		 * Log a Message at the Warn Level
		 */
#define LogWarn(Y)  Log::Write(LogLevel_Warning, LogFormat(Y))
		/*! \def LogError()
		 * \relates LogClass
		 * Log a Message at the LogError Level
		 */
#define LogError(Y) Log::Write(LogLevel_Error, LogFormat(Y))
		/*! \def LogFatal()
		 * Log a Message at the LogFatal Level
		 * \relates LogClass
		 */
#define LogFatal(Y) Log::Write(LogLevel_Fatal, LogFormat(Y))

	}
}


#endif /* LOGCLASS_HPP_ */
