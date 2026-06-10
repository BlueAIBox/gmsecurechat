#ifndef SECURITYDASHBOARD_H
#define SECURITYDASHBOARD_H

#include <QDateTime>
#include <QMap>
#include <QWidget>
#include "infomanager.h"

class SecurityDashboard : public QWidget
{
    Q_OBJECT
public:
    explicit SecurityDashboard(QWidget *parent = nullptr);

    void setUsers(const QList<OnlineUser>& users);
    void recordNetworkEvent(const QString& message);
    void recordCryptoEvent(const QString& message);
    void recordMessageAccepted();
    void recordFileAccepted(bool verifyOk);
    void recordAuditEvent();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct CityPoint {
        QString name;
        QPointF pos;
    };

    QList<OnlineUser> onlineUsers;
    QVector<int> trafficHistory;
    QMap<QString, int> algorithmCounts;
    int cryptoEvents;
    int networkEvents;
    int keyExchangeAttempts;
    int keyExchangeSuccess;
    int verifiedFiles;
    int rejectedFiles;
    qint64 lastPulseMs;

    CityPoint cityForIp(const QString& ip) const;
    void addTrafficSample(int weight);
    void drawStatCard(QPainter& painter, const QRectF& rect,
                      const QString& title, const QString& value,
                      const QColor& accent) const;
    void drawMap(QPainter& painter, const QRectF& rect) const;
    void drawTraffic(QPainter& painter, const QRectF& rect) const;
    void drawAlgorithms(QPainter& painter, const QRectF& rect) const;
};

#endif // SECURITYDASHBOARD_H
