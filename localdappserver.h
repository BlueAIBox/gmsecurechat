#ifndef LOCALDAPPSERVER_H
#define LOCALDAPPSERVER_H

#include <QObject>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>

class LocalDappServer : public QObject
{
    Q_OBJECT
public:
    explicit LocalDappServer(QObject *parent = nullptr);

    bool start();
    quint16 port() const;
    QString baseUrl() const;

signals:
    void callbackReceived(const QString& type, const QJsonObject& payload);

private slots:
    void onNewConnection();

private:
    QTcpServer server;
    QString rootPath;

    QString resolveRootPath() const;
    QByteArray contentTypeForPath(const QString& path) const;
    void handleRequest(QTcpSocket* socket);
};

#endif // LOCALDAPPSERVER_H
