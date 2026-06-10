#include "securitydashboard.h"
#include <QCryptographicHash>
#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>
#include <QStringList>
#include <QtMath>

namespace {
QColor bgColor() { return QColor("#f7fbff"); }
QColor panelBorder() { return QColor("#d5e2f2"); }
QColor textColor() { return QColor("#1f2a37"); }
QColor mutedColor() { return QColor("#607085"); }

QRectF adjustedRect(const QRectF& rect, qreal left, qreal top, qreal right, qreal bottom)
{
    return QRectF(rect.left() + left, rect.top() + top,
                  rect.width() - left - right, rect.height() - top - bottom);
}
}

SecurityDashboard::SecurityDashboard(QWidget *parent)
    : QWidget(parent),
      cryptoEvents(0),
      networkEvents(0),
      keyExchangeAttempts(0),
      keyExchangeSuccess(0),
      verifiedFiles(0),
      rejectedFiles(0),
      lastPulseMs(QDateTime::currentMSecsSinceEpoch())
{
    setMinimumHeight(330);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    trafficHistory.fill(0, 36);
    algorithmCounts["SM2"] = 0;
    algorithmCounts["SM3"] = 0;
    algorithmCounts["SM4"] = 0;
    algorithmCounts["ZUC"] = 0;
}

void SecurityDashboard::setUsers(const QList<OnlineUser>& users)
{
    onlineUsers = users;
    update();
}

void SecurityDashboard::recordNetworkEvent(const QString& message)
{
    networkEvents++;
    addTrafficSample(qBound(1, message.size() / 18, 8));
    update();
}

void SecurityDashboard::recordCryptoEvent(const QString& message)
{
    cryptoEvents++;
    if (message.contains("SM2")) algorithmCounts["SM2"]++;
    if (message.contains("SM3")) algorithmCounts["SM3"]++;
    if (message.contains("SM4")) algorithmCounts["SM4"]++;
    if (message.contains("ZUC")) algorithmCounts["ZUC"]++;

    if (message.contains("密钥") && (message.contains("完成") || message.contains("成功"))) {
        keyExchangeAttempts++;
        keyExchangeSuccess++;
    } else if (message.contains("密钥") && message.contains("失败")) {
        keyExchangeAttempts++;
    }

    addTrafficSample(qBound(2, message.size() / 14, 10));
    update();
}

void SecurityDashboard::recordMessageAccepted()
{
    addTrafficSample(4);
    update();
}

void SecurityDashboard::recordFileAccepted(bool verifyOk)
{
    if (verifyOk)
        verifiedFiles++;
    else
        rejectedFiles++;
    addTrafficSample(9);
    update();
}

void SecurityDashboard::recordAuditEvent()
{
    addTrafficSample(6);
    update();
}

void SecurityDashboard::addTrafficSample(int weight)
{
    if (trafficHistory.isEmpty())
        trafficHistory.fill(0, 36);
    trafficHistory.removeFirst();
    int pulse = static_cast<int>((QDateTime::currentMSecsSinceEpoch() - lastPulseMs) % 5);
    trafficHistory.push_back(qMin(16, weight + pulse));
    lastPulseMs = QDateTime::currentMSecsSinceEpoch();
}

SecurityDashboard::CityPoint SecurityDashboard::cityForIp(const QString& ip) const
{
    QByteArray digest = QCryptographicHash::hash(ip.toUtf8(), QCryptographicHash::Sha256);
    int seedA = digest.isEmpty() ? 0 : static_cast<unsigned char>(digest.at(0));
    int seedB = digest.size() < 2 ? 0 : static_cast<unsigned char>(digest.at(1));
    qreal angle = 6.28318530718 * seedA / 255.0;
    qreal radiusX = 0.24 + 0.14 * (seedB / 255.0);
    qreal radiusY = 0.20 + 0.11 * (seedB / 255.0);
    QPointF pos(0.50 + qCos(angle) * radiusX, 0.56 + qSin(angle) * radiusY);

    if (ip.startsWith("hermes://"))
        return {QString::fromUtf8("本机Agent"), QPointF(0.50, 0.25)};

    QStringList parts = ip.split('.');
    if (parts.size() == 4) {
        bool ok0 = false, ok1 = false, ok3 = false;
        int a = parts[0].toInt(&ok0);
        int b = parts[1].toInt(&ok1);
        int tail = parts[3].toInt(&ok3);
        QString tailText = ok3 ? QString(".%1").arg(tail) : parts[3];
        if (ok0 && ok1) {
            if (a == 127)
                return {QString::fromUtf8("本机回环"), QPointF(0.50, 0.28)};
            if (a == 192 && b == 168)
                return {QString::fromUtf8("同网段") + tailText, pos};
            if (a == 10)
                return {QString::fromUtf8("10网段") + tailText, pos};
            if (a == 172 && b >= 16 && b <= 31)
                return {QString::fromUtf8("172网段") + tailText, pos};
        }
    }

    return {QString::fromUtf8("公网节点"), pos};
}

void SecurityDashboard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bgColor());

    QRectF area = adjustedRect(rect(), 10, 8, 10, 10);
    QRectF top(area.left(), area.top(), area.width(), 50);
    qreal gap = 8;
    qreal cardW = (top.width() - gap * 2) / 3;

    double successRate = keyExchangeAttempts == 0 ? 100.0 :
                         (100.0 * keyExchangeSuccess / keyExchangeAttempts);
    drawStatCard(painter, QRectF(top.left(), top.top(), cardW, top.height()),
                 "在线节点", QString::number(onlineUsers.size()), QColor("#2563eb"));
    drawStatCard(painter, QRectF(top.left() + cardW + gap, top.top(), cardW, top.height()),
                 "密钥成功率", QString::number(successRate, 'f', 0) + "%", QColor("#0f9f6e"));
    drawStatCard(painter, QRectF(top.left() + (cardW + gap) * 2, top.top(), cardW, top.height()),
                 "链路事件", QString::number(cryptoEvents + networkEvents), QColor("#d97706"));

    QRectF body(area.left(), top.bottom() + 9, area.width(), area.height() - top.height() - 9);
    QRectF mapRect(body.left(), body.top(), body.width() * 0.48, body.height() * 0.52);
    QRectF trafficRect(mapRect.right() + 8, body.top(), body.right() - mapRect.right() - 8, mapRect.height());
    QRectF algoRect(body.left(), mapRect.bottom() + 8, body.width(), body.bottom() - mapRect.bottom() - 8);

    drawMap(painter, mapRect);
    drawTraffic(painter, trafficRect);
    drawAlgorithms(painter, algoRect);
}

void SecurityDashboard::drawStatCard(QPainter& painter, const QRectF& rect,
                                     const QString& title, const QString& value,
                                     const QColor& accent) const
{
    painter.setPen(QPen(panelBorder(), 1));
    painter.setBrush(QColor("#ffffff"));
    painter.drawRoundedRect(rect, 8, 8);

    painter.setPen(accent);
    painter.setBrush(accent);
    painter.drawRoundedRect(QRectF(rect.left(), rect.top(), 4, rect.height()), 2, 2);

    painter.setPen(mutedColor());
    QFont small = painter.font();
    small.setPointSize(7);
    painter.setFont(small);
    QFontMetrics smallMetrics(small);
    QString compactTitle = smallMetrics.elidedText(title, Qt::ElideRight, static_cast<int>(rect.width() - 18));
    painter.drawText(adjustedRect(rect, 9, 5, 6, 0), Qt::AlignLeft | Qt::AlignTop, compactTitle);

    painter.setPen(textColor());
    QFont valueFont = painter.font();
    valueFont.setPointSize(10);
    valueFont.setBold(true);
    painter.setFont(valueFont);
    painter.drawText(adjustedRect(rect, 9, 21, 6, 3), Qt::AlignLeft | Qt::AlignVCenter, value);
}

void SecurityDashboard::drawMap(QPainter& painter, const QRectF& rect) const
{
    painter.setPen(QPen(panelBorder(), 1));
    painter.setBrush(QColor("#ffffff"));
    painter.drawRoundedRect(rect, 8, 8);

    painter.setPen(textColor());
    QFont title = painter.font();
    title.setPointSize(8);
    title.setBold(true);
    painter.setFont(title);
    painter.drawText(adjustedRect(rect, 8, 5, 8, 0), Qt::AlignLeft | Qt::AlignTop, "局域网拓扑定位");

    QRectF mapArea = adjustedRect(rect, 9, 24, 9, 10);
    QPointF local(mapArea.left() + mapArea.width() * 0.50, mapArea.top() + mapArea.height() * 0.56);
    QRectF outer(local.x() - mapArea.width() * 0.40, local.y() - mapArea.height() * 0.36,
                 mapArea.width() * 0.80, mapArea.height() * 0.72);
    QRectF inner(local.x() - mapArea.width() * 0.24, local.y() - mapArea.height() * 0.22,
                 mapArea.width() * 0.48, mapArea.height() * 0.44);

    painter.setPen(QPen(QColor("#d7e6f8"), 1));
    painter.setBrush(QColor("#f4f9ff"));
    painter.drawEllipse(outer);
    painter.setBrush(QColor("#eaf4ff"));
    painter.drawEllipse(inner);
    painter.setPen(QPen(QColor("#c6d8ec"), 1, Qt::DashLine));
    painter.drawLine(QPointF(mapArea.left() + 8, local.y()), QPointF(mapArea.right() - 8, local.y()));
    painter.drawLine(QPointF(local.x(), mapArea.top() + 5), QPointF(local.x(), mapArea.bottom() - 5));

    painter.setPen(QPen(QColor("#22a6b3"), 1));
    painter.setBrush(QColor("#22a6b3"));
    painter.drawEllipse(local, 4, 4);
    painter.setPen(QColor("#0f766e"));
    QFontMetrics centerMetrics(painter.font());
    painter.drawText(QPointF(local.x() - centerMetrics.horizontalAdvance("本机") / 2.0, local.y() + 16), "本机");

    QFont labelFont = painter.font();
    labelFont.setPointSize(7);
    painter.setFont(labelFont);

    int drawn = 0;
    for (const OnlineUser& user : onlineUsers) {
        CityPoint city = cityForIp(user.ip);
        QPointF p(mapArea.left() + mapArea.width() * city.pos.x(),
                  mapArea.top() + mapArea.height() * city.pos.y());
        painter.setPen(QPen(QColor("#60a5fa"), 1));
        painter.drawLine(local, p);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#2563eb"));
        painter.drawEllipse(p, 5, 5);
        painter.setPen(QColor("#334155"));
        QString label = QFontMetrics(labelFont).elidedText(user.nick + " " + city.name, Qt::ElideRight, 58);
        painter.drawText(QPointF(p.x() + 6, p.y() - 3), label);
        if (++drawn >= 5)
            break;
    }

    if (onlineUsers.isEmpty()) {
        painter.setPen(mutedColor());
        painter.drawText(mapArea, Qt::AlignCenter, "等待上线用户");
    }
}

void SecurityDashboard::drawTraffic(QPainter& painter, const QRectF& rect) const
{
    painter.setPen(QPen(panelBorder(), 1));
    painter.setBrush(QColor("#ffffff"));
    painter.drawRoundedRect(rect, 8, 8);

    painter.setPen(textColor());
    QFont title = painter.font();
    title.setPointSize(8);
    title.setBold(true);
    painter.setFont(title);
    painter.drawText(adjustedRect(rect, 8, 5, 8, 0), Qt::AlignLeft | Qt::AlignTop, "实时加密流量");

    QRectF chart = adjustedRect(rect, 9, 25, 9, 14);
    painter.setPen(QPen(QColor("#e2e8f0"), 1));
    for (int i = 1; i < 4; i++) {
        qreal y = chart.top() + chart.height() * i / 4.0;
        painter.drawLine(QPointF(chart.left(), y), QPointF(chart.right(), y));
    }

    QPainterPath path;
    for (int i = 0; i < trafficHistory.size(); i++) {
        qreal x = chart.left() + chart.width() * i / qMax(1, trafficHistory.size() - 1);
        qreal y = chart.bottom() - chart.height() * qMin(16, trafficHistory[i]) / 16.0;
        if (i == 0)
            path.moveTo(x, y);
        else
            path.lineTo(x, y);
    }

    painter.setPen(QPen(QColor("#0f9f6e"), 2));
    painter.drawPath(path);
    painter.setPen(mutedColor());
    QFont small = painter.font();
    small.setPointSize(7);
    painter.setFont(small);
    painter.drawText(adjustedRect(rect, 8, rect.height() - 17, 8, 2),
                     Qt::AlignLeft | Qt::AlignBottom,
                     "消息/文件/验签事件动态采样");
}

void SecurityDashboard::drawAlgorithms(QPainter& painter, const QRectF& rect) const
{
    painter.setPen(QPen(panelBorder(), 1));
    painter.setBrush(QColor("#ffffff"));
    painter.drawRoundedRect(rect, 8, 8);

    painter.setPen(textColor());
    QFont title = painter.font();
    title.setPointSize(8);
    title.setBold(true);
    painter.setFont(title);
    painter.drawText(adjustedRect(rect, 8, 5, 8, 0), Qt::AlignLeft | Qt::AlignTop, "算法调用频率");

    QRectF bars = adjustedRect(rect, 10, 28, 10, 10);
    QStringList names = {"SM2", "SM3", "SM4", "ZUC"};
    QColor colors[] = {QColor("#2563eb"), QColor("#0f9f6e"), QColor("#7c3aed"), QColor("#f59e0b")};
    int maxVal = 1;
    for (const QString& name : names)
        maxVal = qMax(maxVal, algorithmCounts.value(name));

    qreal rowH = qMax<qreal>(18, bars.height() / names.size());
    QFont label = painter.font();
    label.setPointSize(8);
    painter.setFont(label);

    for (int i = 0; i < names.size(); i++) {
        QString name = names[i];
        int value = algorithmCounts.value(name);
        qreal y = bars.top() + i * rowH + 2;
        QRectF labelRect(bars.left(), y, 34, qMax<qreal>(12, rowH - 4));
        QRectF track(bars.left() + 38, y + 4, bars.width() - 76, qMax<qreal>(8, rowH - 10));
        QRectF fill(track.left(), track.top(), track.width() * value / maxVal, track.height());

        painter.setPen(textColor());
        painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, name);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#edf2f7"));
        painter.drawRoundedRect(track, 4, 4);
        painter.setBrush(colors[i]);
        painter.drawRoundedRect(fill, 4, 4);
        painter.setPen(mutedColor());
        painter.drawText(QRectF(track.right() + 5, y, 28, rowH - 4),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(value));
    }
}
