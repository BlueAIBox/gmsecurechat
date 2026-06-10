#include "logger.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>

Logger::Logger(QObject *parent) : QObject(parent) {}

void Logger::logNetwork(const QString& msg)
{
    emit newLog(msg, true);
}

void Logger::logCrypto(const QString& msg)
{
    if (!detailedFilePath.isEmpty()) {
        QFile file(detailedFilePath);
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz] ")
                << msg << "\n";
        }
    }
    emit newLog(msg, false);
}

void Logger::setDetailedLogFile(const QString& filePath)
{
    detailedFilePath = filePath;
}

void Logger::clearDetailedLogFile()
{
    detailedFilePath.clear();
}

QString Logger::detailedLogFile() const
{
    return detailedFilePath;
}
