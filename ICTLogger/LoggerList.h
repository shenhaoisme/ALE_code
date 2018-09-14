/*
 * LoggerDeploy.h
 *
 *  Created on: Jun 29, 2009
 *      Author: dberg
 */

#ifndef LOGGERDEPLOY_H_
#define LOGGERDEPLOY_H_

#include <ICTLogger.h>
#include <QHash>
#include <QString>

class LoggerList {

private:
	QHash<QString,ICTLogger*> _loggerList;

	static LoggerList* _instance;
	QString _moduleName;

	ICTLogger* operator[](QString);

	/**
	 * Delete a instance of a Logger List
	 */
	void clearInstance();

	/*
	 * Constructor of LoggerList - take a QString as parameter
	 * @param module
	 */
	LoggerList(QString moduleName);

	/**
	 * Insert  a new Logger in a list of loggers
	 */
	void insert(QString key, ICTLogger* logger);

public:

	static LoggerList* getInstance(){return _instance;}

	QString getModuleName(){
		return this->_moduleName;
	}
	/**
	 * Initialize a instance of a Logger List
	 */
	static void initialize(QString moduleName);

	/**
	 * Call clearInstace() method to delete a list of loggers
	 */
	void clear();

	/**
	 * Return a logger from a existing LoggerList or return a new logger
	 *
	 * @param function
	 */
	ICTLogger* value(QString function = "");

	/*
	 * Test if an instance of logger exist in a Loggerlist
	 * The search in the LoggerList is made with a QString given as parameter
	 *
	 * @param activityName
	 */
	bool exist(QString activityName);

	bool setLevel(QString facility, int level);

	int getLevel(QString facility);
	/*
	 * Update one level filter for one facility in an existing list of loggers
	 * Test if all the level filters for all the facilities must be changed;
	 *
	 * @param list    	 QHash list of loggers
	 * @param facility	 a QString with a facility name or the reserved QString "all"
	 * @param level		 the level to be saved
	 */
	bool updateLoggerList(LoggerList* list, QString facility, int level);




	bool updateAllLoggerList(LoggerList, int level);

	QHash<QString,ICTLogger*> gethashMap();

	QString printAllLevels(LoggerList);

};
	/**
	 * Destruct a list of syslog loggers
	 */
	void clearLoggerList();


	/**
	 * Create a new logger and add it in an existing list of loggers
	 * @param name
	 */
	ICTLogger* logger(QString name = "");


	/**
	 * Create a list of syslog loggers throw a XML configuration file
	 * @param module
	 * @param modulename
	 */
	void initLoggerList(QString moduleName);




	void setLoggerList(ICTLogger* logger);



#endif /* LOGGERDEPLOY_H_ */
