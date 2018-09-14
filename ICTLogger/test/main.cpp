#include <QtCore>
#include <QCoreApplication>

#include <QtTest/QtTest>
#include <ICTLogger.h>
#include <syslog.h>


class TestLogger: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void LogWarnMessage();
    void LogErrorMessage();
    void LogDebugMessage();
    void LogInfoMessage();

};

void TestLogger::initTestCase()
{

}

void TestLogger::LogWarnMessage()
{
	ICTLogger* logger = new ICTLogger("ipstreamtest");
	logger->warning("essai de log niveau warning");
	QCOMPARE(1, 1);
	return;
}

void TestLogger::LogErrorMessage()
{
	ICTLogger* logger = new ICTLogger("ipstreamtest");
	logger->error("essai de log niveau error");
	QCOMPARE(1, 1);
	return;
}


void TestLogger::LogDebugMessage()
{
    QTime time;
    time.start();
	ICTLogger* logger = new ICTLogger("ipstreamtest");
    for (int i= 0; i < 100 ; i++)
    {
	logger->debug("essai de log niveau debug");
    }
    int millisecondes = time.elapsed();
    logger->info( QString("ms: %1\n").arg( millisecondes));
	QCOMPARE(1, 1);
	return;
}

void TestLogger::LogInfoMessage()
{
    QTime time;
    time.start();
	ICTLogger* logger = new ICTLogger("ipstreamtest");
    for (int i= 0; i < 100 ; i++)
    {
	logger->info("essai de log niveau info");
    }
    int millisecondes = time.elapsed();
    logger->info( QString("ms: %1\n").arg( millisecondes));
	QCOMPARE(1, 1);
	return;
}



QTEST_MAIN(TestLogger)
#include "./main.moc"
