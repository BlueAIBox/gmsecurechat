#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QLineEdit>
#include <QMap>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStringList>
#include <QTimer>
#include "infomanager.h"
#include "logger.h"
#include "udpmanager.h"
#include "messagehandler.h"
#include "filehandler.h"
#include "securitydashboard.h"
#include "localdappserver.h"
#include "hermesbridge.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MessageRouteAnimation;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginClicked();
    void onUserSelected(QListWidgetItem* item);
    void onSendMessageClicked();
    void onSendFileClicked();
    void onMessageReceived(const QString& fromIp, const QString& fromNick, const QString& plainText);
    void onFileReceived(const QString& fromNick, const QString& fileName, const QByteArray& fileData, bool verifyOk);
    void onLogMessage(const QString& msg, bool isNetwork);
    void onOnlineUserAdded(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress);
    void onOnlineUserAddedAck(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress);
    void onOnlineUserRemoved(const QString& ip);
    void onHeartbeat();
    void onTransferClicked();
    void onBindIdentityClicked();
    void onTamperMessageClicked();
    void onTamperFileClicked();
    void onVerifyLedgerClicked();
    void onVerifyPeerWalletClicked();
    void onDappCallback(const QString& type, const QJsonObject& payload);
    void onCreateGroupClicked();
    void onHermesAttachClicked();
    void onHermesShareClicked();
    void onHermesStatusMessage(const QString& message, bool isError);

private:
    Ui::MainWindow *ui;
    InfoManager infoMgr;
    Logger logger;
    UDPManager udp;
    MessageHandler msgHandler;
    FileHandler fileHandler;
    LocalDappServer dappServer;
    HermesBridge hermesBridge;
    QTimer heartbeatTimer;
    SecurityDashboard* securityDashboard;
    MessageRouteAnimation* routeAnimation;
    QLineEdit* walletInput;
    QLineEdit* hermesEndpointInput;
    QPushButton* transferBtn;
    QPushButton* bindIdentityBtn;
    QPushButton* tamperMessageBtn;
    QPushButton* tamperFileBtn;
    QPushButton* verifyLedgerBtn;
    QPushButton* verifyPeerWalletBtn;
    QPushButton* createGroupBtn;
    QPushButton* hermesAttachBtn;
    QPushButton* hermesShareBtn;

    QString currentTargetIp;
    QString currentTargetNick;
    QMap<QString, QStringList> chatHistories;
    QString chainAddress;
    QString chainFingerprint;
    QString sessionChainHead;
    QString auditLedgerHead;
    QString lastFileName;
    QString lastFileHash;
    QString lastMerkleRoot;
    QByteArray lastFileData;
    QMap<QString, QJsonObject> pendingHermesShares;
    QMap<QString, QJsonObject> activeHermesShares;
    QMap<QString, QJsonObject> hermesProxyContacts;
    QMap<QString, QJsonObject> pendingHermesProxyRequests;
    QMap<QString, QJsonObject> pendingWalletVerifyRequests;
    QMap<QString, QJsonObject> pendingWalletBindApprovals;
    QMap<QString, QJsonObject> verifiedWalletBindings;
    QMap<QString, QString> groupNames;
    QMap<QString, QStringList> groupMembers;
    QMap<QString, QString> conversationLogFiles;
    QString activeTraceFile;
    bool nextMessageTamperArmed;
    QString nextMessageTamperTargetIp;
    std::vector<unsigned char> chainPrivateKey;
    std::vector<unsigned char> chainPublicKey;

    void updateOnlineList();
    void updateStatusBar();
    void renderConversation(const QString& conversationIp);
    void appendChatMessage(const QString& nick, const QString& msg, bool isSelf);
    void appendChatMessage(const QString& conversationIp, const QString& nick, const QString& msg, bool isSelf);
    void appendChatEvent(const QString& conversationIp, const QString& title, const QString& detail);
    void showBottomFlow(const QString& conversationIp, const QString& title, const QStringList& stages, const QString& accentHex);
    void animateMessageRoute(const QString& conversationIp, const QString& peerNick, bool outgoing);
    void appendNetworkLog(const QString& msg);
    void appendCryptoLog(const QString& msg);
    void setupInnovationPanel();
    bool isHermesFeatureEnabled() const;
    void ensureHermesUser(bool selectAfterAttach = false);
    bool isGroupContact(const QString& targetId) const;
    QString groupIdFromWire(const QString& wireGroupId) const;
    QStringList validGroupMembers(const QString& groupId) const;
    void sendGroupMessage(const QString& groupId, const QString& plainText);
    bool handleGroupControlMessage(const QString& fromIp, const QString& fromNick, const QString& plainText);
    bool isHermesProxyContact(const QString& targetId) const;
    QString hermesProxyIdForOwner(const QString& ownerIp) const;
    QString targetDisplayName(const QString& targetId) const;
    QString newNonceHex(int byteCount = 16) const;
    QString selfFingerprint() const;
    QString pubFingerprintForIp(const QString& ip) const;
    bool sendEncryptedControlMessage(const QString& targetIp, const QJsonObject& payload);
    bool sendTamperedMessageForDemo(const QString& plainText);
    QString walletBindingStatement(const QString& subjectNick,
                                   const QString& subjectFingerprint,
                                   const QString& walletAddress,
                                   const QString& requesterFingerprint,
                                   const QString& nonce) const;
    QString verifiedWalletBadgeForIp(const QString& ip) const;
    void openDappForWalletBinding(const QJsonObject& request);
    QString ensureConversationLogFile(const QString& conversationId, const QString& displayName);
    void beginSmTrace(const QString& conversationId, const QString& displayName, const QString& direction, const QString& summary);
    void endSmTrace(const QString& result);
    void resetConversationTrace(const QString& conversationId);
    bool handleWalletBindingControlMessage(const QString& fromIp, const QString& fromNick, const QString& plainText);
    void handleWalletBindRequest(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void handleWalletBindProof(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void handleWalletBindReject(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void handleWalletBindSignatureCallback(const QJsonObject& payload);
    bool handleHermesControlMessage(const QString& fromIp, const QString& fromNick, const QString& plainText);
    void handleHermesShareChallenge(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void handleHermesShareAck(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void handleHermesShareReady(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void handleHermesShareReject(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void sendHermesProxyQuestion(const QString& proxyId, const QString& plainText);
    void handleHermesProxyRequest(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    void handleHermesProxyReply(const QString& fromIp, const QString& fromNick, const QJsonObject& payload);
    QString sm3Hex(const QByteArray& data) const;
    QString computeMerkleRoot(const QByteArray& data, int chunkSize = 1024) const;
    QString appendAuditRecord(const QString& type, const QJsonObject& payload, bool openDapp = false);
    void openDappForRecord(const QJsonObject& record);
    void openDappForTransfer(const QJsonObject& transfer);
    void recordFileMerkleEvidence(const QString& fileName, const QByteArray& fileData, const QString& direction);
    void updateSessionHashChain(const QString& direction, const QString& peerNick, const QString& plainText);
    bool createDoubleSignatureBinding(bool manualTrigger);
    bool isValidWalletAddress(const QString& address) const;
};

#endif // MAINWINDOW_H
