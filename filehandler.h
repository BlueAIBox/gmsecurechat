#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QVector>
#include "infomanager.h"
#include "logger.h"
#include "udpmanager.h"

class FileHandler : public QObject
{
    Q_OBJECT
public:
    explicit FileHandler(QObject *parent = nullptr);
    bool sendFile(const QString& filePath, const QString& targetIp,
                  InfoManager& infoMgr, UDPManager& udp, Logger& logger);
    void onReceiveFileChunk(const QByteArray& chunkData, const QString& fromIp,
                            InfoManager& infoMgr, Logger& logger, UDPManager& udp);

private:
    struct ReceivingFile {
        QString fromNick;
        QString fileName;
        qint64 fileSize;
        int totalChunks;
        QString transferId;
        QVector<QByteArray> encryptedChunks;
        QVector<QByteArray> signatures;
        QVector<bool> receivedFlags;
        std::vector<unsigned char> expectedFileHash;
    };
    QMap<QString, ReceivingFile> pendingReceives;

    bool tryAssembleFile(const QString& key, const QString& fromIp,
                         InfoManager& infoMgr, Logger& logger, UDPManager& udp);
};

#endif // FILEHANDLER_H
