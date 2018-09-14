
//============================================================================
// Name        : ICTogger.h
// Author      : Bergzoll
// Version     :
// Documentation reference :   ICTouch R100 - Application Framework
//  						  3ak_29000_0155_dtzza
//
// Modified : ESTEVE Olivier - 01/06/2010
//            Removed static variable for macro define
//		(compilation problem without optimization O2 with gcc)
//
//            SULYAN Michel  - 2010/10/20
//            fix of crms00342562
//============================================================================

#ifndef ICT_LOGGER_H_
#define ICT_LOGGER_H_
#include <QString>
#include <QFile>
#include <QTime>
#include <unistd.h>
#include <syslog.h>


#include <QMutex>

#ifdef ICT_HOST

#  define LOGGER_IF_EMERG   if( true )
#  define LOGGER_IF_ERROR   if( true )
#  define LOGGER_IF_WARNING if( true )
#  define LOGGER_IF_NOTICE  if( true )
#  define LOGGER_IF_INFO    if( true )
#  define LOGGER_IF_DEBUG   if( true )

#else

#  define LOGGER_IF_EMERG   if( logger()->getConfiguredLevel() >= LOG_EMERG)
#  define LOGGER_IF_ERROR   if( logger()->getConfiguredLevel() >= LOG_ERR)
#  define LOGGER_IF_WARNING if( logger()->getConfiguredLevel() >= LOG_WARNING)
#  define LOGGER_IF_NOTICE  if( logger()->getConfiguredLevel() >= LOG_NOTICE)
#  define LOGGER_IF_INFO    if( logger()->getConfiguredLevel() >= LOG_INFO)
#  define LOGGER_IF_DEBUG   if( logger()->getConfiguredLevel() >= LOG_DEBUG)

#endif

// crms00342562 +++
#define LOGGER_EMERG(...)   do { LOGGER_IF_EMERG   logger()->emergency(__VA_ARGS__); } while (false)
#define LOGGER_ERROR(...)   do { LOGGER_IF_ERROR   logger()->error(__VA_ARGS__); } while (false)
#define LOGGER_WARNING(...) do { LOGGER_IF_WARNING logger()->warning(__VA_ARGS__); } while (false)
#define LOGGER_NOTICE(...)  do { LOGGER_IF_NOTICE  logger()->notice(__VA_ARGS__); } while (false)
#define LOGGER_INFO(...)    do { LOGGER_IF_INFO    logger()->info(__VA_ARGS__); } while (false)
#define LOGGER_DEBUG(...)   do { LOGGER_IF_DEBUG   logger()->debug(__VA_ARGS__); } while (false)
// crms00342562 ---

typedef struct _code
{
	int c_val;
} CODE;


class ICTLogger
{

private :


	QString _facility;
	int _facilityValue;

	//! The Name of the module
	QString _moduleName;

	// memorise the name of the actual facility used
	QString _inuseFacility;

	// programmed level filter by syslog-ng configuration file
	int _configuredLevel;

	// boolean attribut to test if the stderr stream is open or not
	bool stderrOk;


	mutable QMutex mutex;





public :



	static const QString configurationSyslogRootPath;
	static const QString configurationSyslogBranchPath;
	static const QString syslogWorkingDirectory;
	static const QString facilityConfigFilesPath;
	static const QString commandFilesPath;
	static const QString syslogExecutablePath;

	/**
	 * @param functionnality
	 * @param moduleName
	 */
	ICTLogger(QString facility,QString moduleName = "Default");

	/**
	 * destructor of ICTLogger
	 */
	~ICTLogger();

	/*
	 * Get the facility name of a instanciated logger
	 * Used only in developpement mode.
	 */
	QString getLoggerfacility();

	/*
	 * Get the level filter of a instanciated logger
	 * Used in normal debug mode, to log level config
	 * with syslog-ng
	 *
	 */
	int getConfiguredLevel();

	void setConfiguredLevel(int level);

	/**
	 * Send a message to syslog with a "info" level and with a QString as input parameter
	 * @param message
	 */
	void info(const QString& message);


	/**
	 * Send a message to syslog with a "error" level and with a QString as input parameter
	 * @param message
	 */
	void error(const QString& message);


	/**
	 * Send a message to syslog with a "debug" level and with a QString as input parameter
	 * @param message
	 */
#undef debug
	void debug(const QString& message);


	/**
	 * Send a message to syslog with a "notice" level and with a QString as input parameter
	 * @param message
	 */
	void notice(const QString& message);


	/**
	 * Send a message to syslog with a "emerg" level and with a QString as input parameter
	 * @param message
	 */
	void emergency(const QString& message);


	/**
	 * Send a message to syslog with a "warning" level and with a QString as input parameter
	 * @param message
	 */
	void warning(const QString& message);




	/**
	 * Send a message to syslog - facility is a attribut from
	 * the ICTLogger instance
	 * @param level
	 * @param message
	 */
	void log(int level,const QString& message);

	/**
	 * Get the log level filter used by the module to log
	 * @param levelValue
	 */

	int readFacilityLevel(QString fileName);


	/**
	 * Find a facility in a predefined list of facilities (see ICTLogger.cpp)
	 * @param facility
	 *
	 */
	CODE findFacility(QString facility);


	/**
	 * Set the log level filter used by the module to log
	 * @param levelValue
	 */

	void setLevel(int levelValue);

	/**
	 * Get the log level filter used by the module to log
	 * @param levelValue
	 */

	int getLevel();




};



#endif /* ICT_LOGGER_H_ */
