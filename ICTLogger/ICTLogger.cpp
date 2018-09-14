//============================================================================
// Name        : ICTLogger.cpp
// Author      : Bergzoll
// Version     :
//Documentation reference :   ICTouch R100 - Application Framework
//  						  3ak_29000_0155_dtzza
//============================================================================


#include "ICTConstants.h"
#include "ICTLogger.h"
#include <QDebug>
#include <QStringList>

#define MESSAGE_MAX_SIZE 1000

#define ERROR_MSG "Facility configuration file corrupted :"

const int EMERG=0;
const int ERROR=3;
const int WARNING=4;
const int NOTICE=5;
const int INFO=6;
const int DEBUG=7;

extern bool moduleIsDead;

// syslog-ng platform configuration path
const QString ICTLogger::configurationSyslogRootPath= QString(SYSLOG_ROOT_PATH);
// syslog-ng application configuration path
const QString ICTLogger::configurationSyslogBranchPath= QString(SYSLOG_BRANCH_PATH);
// LoggerModule's directory for syslog-ng save and temporary configuration file
const QString ICTLogger::syslogWorkingDirectory= QString(SYSLOG_WORKING_DIR);
// LoggerModule's directory for facilities configuration files
const QString ICTLogger::facilityConfigFilesPath= QString(SYSLOG_FACILITY_PATH);
// syslog-ng exec path
const QString ICTLogger::syslogExecutablePath= QString(SYSLOG_EXECUTABLE_PATH);


#ifdef ICT_HOST
#else

//  ICTouch executable applications
// ph: don't know why this is target only
const QString ICTLogger::commandFilesPath 			 = QString("/usr/lib/ICTApplication");

#endif



QString ICTLogger::getLoggerfacility()
{
	return _facility;
}

void ICTLogger::setConfiguredLevel(int level)
{
	_configuredLevel = level;
}

int ICTLogger::getConfiguredLevel()
{
	return _configuredLevel;
}

CODE ICTLogger::findFacility(QString fileName)
{
	QString msg = ERROR_MSG;

	QStringList liste;
	bool ok = false;
	QString line;
	CODE result;
	QFile file(facilityConfigFilesPath + fileName + ".conf");

    msg += "[ICTLogger] -- Bad facility value in : " + fileName + ".conf";


	QTextStream stream(&file);

    if (! file.exists())
        {
    	msg += " --error type : facility configuration file not exixts";
        result.c_val = 23<<3;//LOG_LOCAL7;
    	syslog(LOG_LOCAL7|LOG_ERR, "%s",msg.toUtf8().data());
    	return result;
        }
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	// The facility config file could not be opened - we must log with LOG_LOCAL7 facility
	// Numerical value : 23 -- facility name : "no_facility"
    result.c_val = 23<<3; //LOG_LOCAL7 facility -- see syslog.h file;
    msg += " --error type : facility configuration file is not readable";
	syslog(LOG_LOCAL7|LOG_ERR, "%s",msg.toUtf8().data());
	return result;
	}
	// The access to the file is OK
	else
	{
	// read all the facility configuration file
		while (!stream.atEnd())
			{
			line = stream.readLine();
			liste.append(line);
			}
		file.close();
		// Test if the facility configuration file size is OK
		// - size must be 4
		// - last field of the file must contain the QString : "facility signature field"
		if ((liste.size() == 4) && (liste[3] == "facility signature field"))
		{
		// Catch the facility value
		int facility = liste[1].toInt(&ok,10);
		// Test if the call of "toInt" method return ok
		if (! ok)
			{
			msg += " --error type : facility value is not an int";
			// Facility value is not a number - log in LOG_LOCAL7 facility
			result.c_val = 23<<3; //LOG_LOCAL7 facility
			syslog(LOG_LOCAL7|LOG_ERR, "%s",msg.toUtf8().data());
			return result;
			}
		// Shift the facility value -- must be done for syslog() method
		result.c_val = facility << 3;
		// the file is readable we return the saved facility without testing
		return result;
		}
		else
		    {
			msg += " --error type : facility configuration file corrupted";
			result.c_val = 23<<3; //LOG_LOCAL7 facility
			syslog(LOG_LOCAL7|LOG_ERR, "%s",msg.toUtf8().data());
			return result;
		    }
		result.c_val = 23<<3; //LOG_LOCAL7 facility
		syslog(LOG_LOCAL7|LOG_ERR, "%s",msg.toUtf8().data());
		return result;
	}


}


int ICTLogger::readFacilityLevel(QString fileName)
{
	QString msg = ERROR_MSG;
	QFile file(facilityConfigFilesPath + fileName + ".conf");
	QString line;
	QStringList liste;
	QTextStream stream(&file);
	bool ok =  false;
	int level;

	msg += "[ICTLogger] -- Bad level value in : "+ fileName + ".conf";


    if (! file.exists())
        {
    	// We must log with error level if the facility config file
    	// does not exists
    	msg += " -- error type : facility configuration file not exists";
    	syslog(LOG_LOCAL7|LOG_ERR, "%s",msg.toUtf8().data());
    	return ERROR;
        }
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	// We must log with error level if the facility config file is
	// not readable
		msg += " --error type : file not readable";
	syslog(LOG_LOCAL7|LOG_ERR,"%s",msg.toUtf8().data());
	return ERROR;
	}
	// The access to the file is OK
	else
	{
		while (!stream.atEnd())
			{
			line = stream.readLine();
			liste.append(line);
			}
		   file.close();
		// Test if facility configuration file has a correct size
		// - size must be 4
		// - last field of the file must contain the QString : "facility signature field"

		if ((liste.size() == 4) && (liste[3] == "facility signature field"))
			{
			// read only the first block in the file
			level = liste[0].toInt(&ok,10);
			if (!ok)
				{
				msg += " --error type : level value not an int" + QString::number(level);
				syslog(LOG_LOCAL7|LOG_ERR,"%s",msg.toUtf8().data());
				return ERROR;
				}


			// Test if the level value is OK
			if ((level == 0) || ((level >= 3) && (level < 8)))
			   {
			   return level;
			   }
			else
			    {
				msg += " --error type : level value not exists";
				syslog(LOG_LOCAL7|LOG_ERR, "%s", msg.toUtf8().data());
				return ERROR;
			    }
			}
		// the size of the config file is not correct, the file is corrupted
		// we return ERROR default level
		else
			{
			msg += " -- error type : facility config file corrupted";
			syslog(LOG_LOCAL7|LOG_ERR, "%s", msg.toUtf8().data());
			return ERROR;
			}
	syslog(LOG_LOCAL7|LOG_ERR, "%s", msg.toUtf8().data());
	return ERROR;
	}


}


/*
 *
 * function         : ICTLogger(QString)
 * input parameter  : QString message
 * output parameter : none
 * function_name	:  our own QString
 */

ICTLogger::ICTLogger(QString facility,QString moduleName) : _facility(facility), _moduleName(moduleName)

{
	CODE code ;
	code =  findFacility(facility);
//	if (code.c_val != -1)
//		{
		_facilityValue   = code.c_val;
		_configuredLevel = readFacilityLevel(facility);
//		}

/*		else
		{
		code = findFacility(_moduleName);
		_facilityValue = code.c_val;
		_configuredLevel = readFacilityLevel(_moduleName);
		}*/

    // store the value of isatty
	// test letter if sdoutOk is a console.
    // stderrOk = (isatty(STDERR_FILENO) == 1);

    // In fact isatty forbids redirecting to a file,
    // so it's a bit hard to work with it.
    // New realization: try to write a newline and if it fails,
    // the output is probably closed.
    // More than this, qdebug writes on stderr and not stdout!
    stderrOk = (write(STDERR_FILENO, "\n", 1) != -1);

}


ICTLogger::~ICTLogger(void) {
	// Do not delete logger while it's logging. This solution is not bullet proof.
	mutex.lock()  ; // wait mutex is unlocked, to ensure no log is ongoing
	mutex.unlock(); // unlock mutex before destruction (avoid warning)
	return;
};

void ICTLogger::setLevel(int levelValue)
{
	_configuredLevel = levelValue;

}

int ICTLogger::getLevel()
{
	return DEBUG;
}


/*
 * function         : void info(QString)
 * input parameter  : QString message
 * output parameter : none
 * message 			:  our own QString
 */

void ICTLogger::info(const QString& message) {

    if (moduleIsDead) return;

	#ifdef ICT_TARGET
	if (LOG_INFO > _configuredLevel) return;
	#endif

	log(LOG_INFO,QString(" [") + _facility + "|INFO]" + message);

	#ifdef ICT_TARGET
	if (stderrOk)	qDebug() << QString("[") + _facility + "|INFO]" + message;
	#else
	if (stderrOk) fprintf(stderr, "[%s|INFO]%s\n", _facility.toUtf8().data(), message.toUtf8().data());
	#endif
}

/*
 * function         : void notice(QString)
 * input parameter  : QString message
 * output parameter : none
 * message 			:  our own QString
 */

void ICTLogger::notice(const QString& message) {

    if (moduleIsDead) return;

	#ifdef ICT_TARGET
	if (LOG_NOTICE > _configuredLevel) return;
	#endif

	log(LOG_NOTICE,QString(" [") + _facility + "|NOTICE]" + message);

	#ifdef ICT_TARGET
	if (stderrOk)	qDebug() << QString("[") + _facility + "|NOTICE]" + message;
	#else
	if (stderrOk) fprintf(stderr, "[%s|NOTICE]%s\n", _facility.toUtf8().data(), message.toUtf8().data());
	#endif
}




/*
 * function         : void warning(QString)
 * input parameter  : QString message
 * output parameter : none
 * message 			: our own QString
 */

void ICTLogger::warning(const QString& message) {

    if (moduleIsDead) return;

	#ifdef ICT_TARGET
	if (LOG_WARNING > _configuredLevel) return;
	#endif

	log(LOG_WARNING,QString(" [") + _facility + "|WARNING]" + message);

	#ifdef ICT_TARGET
	if (stderrOk)	qDebug() << QString("[") + _facility + "|WARNING]" + message;
	#else
	if (stderrOk) fprintf(stderr, "[%s|WARNING]%s\n", _facility.toUtf8().data(), message.toUtf8().data());
	#endif
}

/*
 * function         : void error(QString)
 * input parameter  : QString message
 * output parameter : none
 * message 			: our own QString
 */

void ICTLogger::error(const QString& message) {

    if (moduleIsDead) return;

	log(LOG_ERR,QString(" [") + _facility + "|ERROR]" + message);

	#ifdef ICT_TARGET
	if (stderrOk)	qDebug() << QString("[") + _facility + "|ERROR]" + message;
	#else
	if (stderrOk) fprintf(stderr, "[%s|ERROR]%s\n", _facility.toUtf8().data(), message.toUtf8().data());
	#endif
}

/*
 * function         : void emergency(QString)
 * input parameter  : QString message
 * output parameter : none
 * message 			: our own QString
 */

void ICTLogger::emergency(const QString& message) {

    if (moduleIsDead) return;

	log(LOG_EMERG,QString(" [") + _facility + "|EMERGENCY]" + message);

	#ifdef ICT_TARGET
	if (stderrOk)	qDebug() << QString("[") + _facility + "|EMERGENCY]" + message;
	#else
	if (stderrOk) fprintf(stderr, "[%s|EMERGENCY]%s\n", _facility.toUtf8().data(), message.toUtf8().data());
	#endif
}



/*
 * function         : void debug(QString)
 * @param  message
 * message 			: our own QString
 */

void ICTLogger::debug(const QString& message) {

    if (moduleIsDead) return;

	#ifdef ICT_TARGET
	if (LOG_DEBUG > _configuredLevel) return;
	#endif

	log(LOG_DEBUG,QString(" [") + _facility + "|DEBUG]" + message);

	#ifdef ICT_TARGET
	if (stderrOk)	qDebug() << QString("[") + _facility + "|DEBUG]" + message;
	#else
	if (stderrOk) fprintf(stderr, "[%s|DEBUG]%s\n", _facility.toUtf8().data(), message.toUtf8().data());
	#endif
}


/*
 * log a message through syslog() method with a check of the length.
 * Max length of the message : MESSAGE_MAX_SIZE.
 * @param level
 * @param message
 *
 */
void ICTLogger::log(int level,const QString& message) {

	// Catch the real time to log with syslog messages
	QTime timer  = QTime::currentTime();
	QString time = timer.toString("hh:mm:ss.zzz");

	// All other entries are protected, warning, debug, normal, etc.
	// But this one may still be called directly, so protect it also.
	// This makes a real difference when noemmi terminates on host for example, according to valgrind.
	if (moduleIsDead) return;

	mutex.lock();


	// open a new log only if the facility passing
	// to the ICTLogger constructor is a new one != _inusefacility
	if (! _inuseFacility.contains(_facility))
	{
		// we cannot store the name in the qba because the logger
		// may be used after than the QT finalizers have destroyed
		// the qba at the end of the program.
		// So replace it with a static char array.
		QByteArray qba = _moduleName.toUtf8();
		static char name[100];
		strncpy(name, qba.data(), sizeof(name)); name[sizeof(name)-1] = '\0';
		closelog();
		openlog(name, LOG_ODELAY,_facilityValue);
		_inuseFacility = _facility;
	}
	int length = message.length();

	// use openlog method for choosing /dev/log socket
	//openlog(NULL,LOG_NDELAY, _facilityValue);

	// all this size things are really approximate
    // MESSAGE_MAX_SIZE is in characters
    // and after, we output more bytes due to utf8 conversion


    if (length > MESSAGE_MAX_SIZE )
      {
       QString formattedMessage = message.left(MESSAGE_MAX_SIZE) ;
       syslog(_facilityValue|level, "%s ...\n",(time + formattedMessage).toUtf8().data());
      }
      else
      {
      syslog(_facilityValue|level, "%s", (time + message).toUtf8().data());
      }
	//closelog();
	mutex.unlock();

}








