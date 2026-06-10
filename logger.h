#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
    void logNetwork(const QString& msg);
    void logCrypto(const QString& msg);
    void setDetailedLogFile(const QString& filePath);
    void clearDetailedLogFile();
    QString detailedLogFile() const;

signals:
    void newLog(const QString& msg, bool isNetwork);

private:
    QString detailedFilePath;
};

#endif // LOGGER_H
