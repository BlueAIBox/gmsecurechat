#include "localdappserver.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

LocalDappServer::LocalDappServer(QObject *parent)
    : QObject(parent),
      rootPath(resolveRootPath())
{
    connect(&server, &QTcpServer::newConnection, this, &LocalDappServer::onNewConnection);
}

bool LocalDappServer::start()
{
    if (server.isListening())
        return true;

    for (quint16 p = 5173; p < 5180; ++p) {
        if (server.listen(QHostAddress::LocalHost, p))
            return true;
    }
    return false;
}

quint16 LocalDappServer::port() const
{
    return server.serverPort();
}

QString LocalDappServer::baseUrl() const
{
    return QString("http://127.0.0.1:%1").arg(port());
}

void LocalDappServer::onNewConnection()
{
    while (server.hasPendingConnections()) {
        QTcpSocket* socket = server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            handleRequest(socket);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

QString LocalDappServer::resolveRootPath() const
{
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        QDir(appDir).filePath("web-dapp"),
        QDir(appDir).filePath("../web-dapp"),
        QDir::current().filePath("web-dapp")
    };

    for (const QString& candidate : candidates) {
        if (QFileInfo::exists(QDir(candidate).filePath("index.html")))
            return QDir(candidate).absolutePath();
    }
    return QDir(appDir).filePath("../web-dapp");
}

QByteArray LocalDappServer::contentTypeForPath(const QString& path) const
{
    if (path.endsWith(".html")) return "text/html; charset=utf-8";
    if (path.endsWith(".css")) return "text/css; charset=utf-8";
    if (path.endsWith(".js")) return "application/javascript; charset=utf-8";
    if (path.endsWith(".json")) return "application/json; charset=utf-8";
    return "text/plain; charset=utf-8";
}

void LocalDappServer::handleRequest(QTcpSocket* socket)
{
    QByteArray request = socket->readAll();
    QList<QByteArray> lines = request.split('\n');
    if (lines.isEmpty()) {
        socket->disconnectFromHost();
        return;
    }

    QList<QByteArray> parts = lines.first().trimmed().split(' ');
    if (parts.size() < 2 || parts.first() != "GET") {
        socket->write("HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n");
        socket->disconnectFromHost();
        return;
    }

    QUrl url(QString::fromLatin1(parts.at(1)));
    QString path = url.path();
    if (path == "/" || path.isEmpty())
        path = "/index.html";
    path = QDir::cleanPath(path);
    if (path.startsWith("..") || path.contains("/../")) {
        socket->write("HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n");
        socket->disconnectFromHost();
        return;
    }

    if (path == "/callback") {
        QUrlQuery query(url);
        QJsonObject payload;
        for (const auto& item : query.queryItems()) {
            payload[item.first] = item.second;
        }
        QString type = payload["type"].toString();
        emit callbackReceived(type, payload);

        QByteArray body = QJsonDocument(QJsonObject{{"ok", true}}).toJson(QJsonDocument::Compact);
        socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " +
                      QByteArray::number(body.size()) + "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n" + body);
        socket->disconnectFromHost();
        return;
    }

    QString filePath = QDir(rootPath).filePath(path.mid(1));
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QByteArray body = "Not found";
        socket->write("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: " +
                      QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
        socket->disconnectFromHost();
        return;
    }

    QByteArray body = file.readAll();
    QByteArray header = "HTTP/1.1 200 OK\r\nContent-Type: " + contentTypeForPath(filePath) +
                        "\r\nContent-Length: " + QByteArray::number(body.size()) +
                        "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
    socket->write(header);
    socket->write(body);
    socket->disconnectFromHost();
}
