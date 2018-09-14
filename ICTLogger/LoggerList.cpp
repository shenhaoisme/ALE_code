/*
 * loggerDeploy.cpp
 *
 *  Created on: Jun 29, 2009
 *      Author: dberg
 */
#include "LoggerList.h"
#include <QtCore>
#include <QFile>


LoggerList* LoggerList::_instance = NULL;

LoggerList::LoggerList(QString moduleName) :
	 _moduleName(moduleName){}

void LoggerList::initialize(QString moduleName)
{
	if (_instance == NULL)
	{
	_instance = new LoggerList(moduleName);
	}
}

void LoggerList::insert(QString key, ICTLogger* logger)
{
	if (key == "")
	{
		ICTLogger* logger = _loggerList[key];
		delete logger;
		_loggerList.remove(key);
	}
	_loggerList.insert(key, logger);

}
void LoggerList::clear()
{
	if (_instance != NULL)
	{
		_instance->clearInstance();
	}
}


void LoggerList::clearInstance()
{
	QStringList keys = LoggerList::_loggerList.keys();

	for (int i = 0; i < keys.size(); i++)
	{
		ICTLogger* logger = LoggerList::_loggerList[keys[i]];
		delete logger;
	}
	LoggerList::_loggerList.clear();
}



bool LoggerList::setLevel(QString facility, int value)
{
	ICTLogger* logger = (*_instance)[facility];
	logger->setConfiguredLevel(value);
	return true;
}

int LoggerList::getLevel(QString facility)
{
	ICTLogger* logger = (*_instance)[facility];
	return logger->getConfiguredLevel();
}


bool LoggerList::exist(QString facility)
{
	if(facility.isEmpty()) facility = _moduleName;
		ICTLogger* logger = (*_instance)[facility];
		if (logger == NULL)
		{
			return false;
		}
		else return true;
}

ICTLogger* LoggerList::value(QString functionName)
{
	if(functionName.isEmpty()){
		functionName = _moduleName;
	}
	ICTLogger* logger = (*_instance)[functionName];

	if (logger == NULL)
	{
//		qDebug() << "add a new logger in the loggerlist : " << functionName;
		logger = new ICTLogger(functionName, _moduleName);
		_instance->insert(functionName, logger);

	}
	return logger;
}
ICTLogger* LoggerList::operator[](QString functionName)
{
	if (functionName.isEmpty())
		{
		return _loggerList[_moduleName];
		}
	else return _loggerList[functionName];

}
ICTLogger* logger(QString name)
{
	return LoggerList::getInstance()->value(name);
}
void initLoggerList(QString moduleName)
{
	LoggerList::initialize(moduleName);
}

void clearLoggerList()
{
	LoggerList::getInstance()->clear();
}

QHash<QString,ICTLogger*> LoggerList::gethashMap()
{
	return _loggerList;
}

bool LoggerList::updateLoggerList(LoggerList* list, QString facility, int level)
{
	if(facility != "all")
		{
		if(list->gethashMap().contains(facility))
			{
			ICTLogger* ptr = list->value(facility);
			if (ptr == NULL)
				{
				LOGGER_ERROR("[updateLoggerList]  no such a facility : " + facility);
				return false;
				}
				// catch the actual predefined level filter
				int old_level = ptr->getConfiguredLevel();
				QString message = facility + " old level configuration : "
									+ QString::number (old_level) + " new level configuration : " + QString::number(level);
				LOGGER_DEBUG(message);

			// set the facility new filter level
			ptr->setConfiguredLevel(level);
			return true;
			}
		else
		    {
			LOGGER_DEBUG("[updateLoggerList]  The logger : " + facility + " does not exist" );
		    return false;
		    }
		 }
	else
		return updateAllLoggerList(*list, level);

}
bool LoggerList::updateAllLoggerList(LoggerList liste, int level)
	{

	QHashIterator<QString, ICTLogger*>i(liste.gethashMap());
	 int index = 0;
	 i.toFront();
	 while (i.hasNext())
		 {
		 i.next();
		 ICTLogger* ptr = i.value();
		 if(ptr != NULL)
		 {
		 ptr->setConfiguredLevel(level);
		 }
		 index++;
		 }
	printAllLevels(liste);

	return true;
	}

QString LoggerList::printAllLevels(LoggerList liste)
{
	QHashIterator<QString, ICTLogger*>i(liste.gethashMap());
	QString result ="";

	 while (i.hasNext())
		 {

		 i.next();
		 ICTLogger* ptr = i.value();

		if (ptr != NULL)
		{
		ptr->getConfiguredLevel();
		result += "level : " + QString::number(ptr->getConfiguredLevel()) + " facility : " + ptr->getLoggerfacility() + " ";
		}
		 }
        LOGGER_DEBUG(result);

return result;
}
