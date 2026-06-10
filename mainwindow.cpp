#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sm2.h"
#include "sm3.h"
#include "utils.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>
#include <QTime>
#include <QDateTime>
#include <QShortcut>
#include <QFileInfo>
#include <QFile>
#include <QBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QSplitter>
#include <QStandardPaths>
#include <QStyle>
#include <QAbstractItemView>
#include <QDir>
#include <QSet>
#include <QTextStream>
#include <array>
#include <algorithm>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

static const QString STYLESHEET = R"(
    QMainWindow {
        background-color: #f3f7fb;
    }
    QWidget {
        font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
        font-size: 13px;
        color: #1f2a37;
    }
    QGroupBox {
        font-weight: bold;
        font-size: 13px;
        color: #1d4ed8;
        border: 1px solid #cbd9ea;
        border-radius: 8px;
        margin-top: 10px;
        padding: 12px 6px 6px 6px;
        background-color: #ffffff;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        subcontrol-position: top left;
        left: 12px;
        padding: 0 6px;
        color: #1d4ed8;
        background-color: #ffffff;
    }
    QLineEdit, QTextEdit, QListWidget {
        background-color: #fbfdff;
        color: #1f2937;
        border: 1px solid #cbd9ea;
        border-radius: 6px;
        padding: 4px 6px;
        selection-background-color: #bfdbfe;
        selection-color: #0f172a;
    }
    QLineEdit:focus, QTextEdit:focus, QListWidget:focus {
        border-color: #2563eb;
    }
    QListWidget::item {
        padding: 6px 8px;
        border-bottom: 1px solid #e5edf7;
    }
    QListWidget::item:selected {
        background-color: #dbeafe;
        color: #1d4ed8;
    }
    QListWidget::item:hover {
        background-color: #eef6ff;
    }
    QPushButton {
        background-color: #2563eb;
        color: #ffffff;
        border: 1px solid #1d4ed8;
        border-radius: 6px;
        padding: 6px 12px;
        font-weight: bold;
        min-height: 20px;
    }
    QPushButton:hover {
        background-color: #1d4ed8;
        border-color: #1e40af;
    }
    QPushButton:pressed {
        background-color: #1e3a8a;
    }
    QPushButton:disabled {
        background-color: #e5e7eb;
        color: #9ca3af;
        border-color: #d1d5db;
    }
    QPushButton#loginBtn {
        background-color: #16a34a;
        border-color: #15803d;
    }
    QPushButton#loginBtn:hover {
        background-color: #15803d;
    }
    QPushButton#loginBtn:disabled {
        background-color: #dcfce7;
        color: #86a88f;
        border-color: #bbf7d0;
    }
    QPushButton#sendMsgBtn {
        background-color: #2563eb;
        border-color: #1d4ed8;
    }
    QPushButton#sendMsgBtn:hover {
        background-color: #1d4ed8;
    }
    QPushButton#sendFileBtn {
        background-color: #7c3aed;
        border-color: #6d28d9;
    }
    QPushButton#sendFileBtn:hover {
        background-color: #6d28d9;
    }
    QPushButton#transferBtn {
        background-color: #0f766e;
        border-color: #0f5f59;
    }
    QPushButton#transferBtn:hover {
        background-color: #115e59;
    }
    QGroupBox#inputGroup {
        padding: 10px 8px 8px 8px;
        background-color: #ffffff;
    }
    QWidget#composerBar {
        background-color: #ffffff;
        border: 1px solid #d5dfed;
        border-radius: 8px;
    }
    QTextEdit#msgInput {
        background-color: #ffffff;
        border: none;
        border-radius: 0px;
        padding: 7px 8px;
        color: #111827;
        font-size: 13px;
    }
    QPushButton#sendFileBtn, QPushButton#transferBtn {
        background-color: transparent;
        border: none;
        color: #4b5563;
        min-width: 32px;
        max-width: 32px;
        min-height: 30px;
        max-height: 30px;
        padding: 0px;
        font-size: 18px;
        font-weight: 600;
    }
    QPushButton#sendFileBtn:hover, QPushButton#transferBtn:hover {
        background-color: #edf2f7;
        border-radius: 6px;
    }
    QPushButton#sendMsgBtn {
        background-color: #e5e7eb;
        border: 1px solid #e5e7eb;
        border-radius: 8px;
        color: #6b7280;
        min-width: 76px;
        max-width: 76px;
        min-height: 32px;
        max-height: 32px;
        padding: 0px 12px;
    }
    QPushButton#sendMsgBtn:hover {
        background-color: #dbeafe;
        border-color: #bfdbfe;
        color: #1d4ed8;
    }
    QTextEdit#chatDisplay {
        background-color: #ffffff;
        color: #1f2937;
        font-family: "Cascadia Code", "Consolas", "Microsoft YaHei", monospace;
        font-size: 13px;
    }
    QTextEdit#netLog {
        background-color: #f8fbff;
        color: #1e3a5f;
        font-family: "Cascadia Code", "Consolas", monospace;
        font-size: 11px;
    }
    QTextEdit#cryptoLog {
        background-color: #fffdf5;
        color: #365314;
        font-family: "Cascadia Code", "Consolas", monospace;
        font-size: 11px;
    }
    QStatusBar {
        background-color: #eaf1f8;
        color: #475569;
        border-top: 1px solid #cbd9ea;
        font-size: 11px;
    }
    QMenuBar {
        background-color: #eaf1f8;
        color: #475569;
        border-bottom: 1px solid #cbd9ea;
    }
    QMenuBar::item:selected {
        background-color: #dbeafe;
    }
    QScrollBar:vertical {
        background: #eef2f7;
        width: 8px;
        border-radius: 4px;
    }
    QScrollBar::handle:vertical {
        background: #b8c7d9;
        border-radius: 4px;
        min-height: 30px;
    }
    QScrollBar::handle:vertical:hover {
        background: #94a3b8;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0px;
    }
    QSplitter::handle {
        background-color: #d8e2ef;
    }
    QSplitter::handle:hover {
        background-color: #93c5fd;
    }
    QSplitter::handle:horizontal {
        width: 6px;
    }
    QSplitter::handle:vertical {
        height: 6px;
    }
)";

static std::vector<unsigned char> qbytesToVector(const QByteArray& data)
{
    return std::vector<unsigned char>(data.begin(), data.end());
}

static QString normalizedEthAddress(const QString& address)
{
    QString trimmed = address.trimmed();
    if (!trimmed.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
        return {};
    QString hex = trimmed.mid(2);
    if (!QRegularExpression(QStringLiteral("^[0-9a-fA-F]{40}$")).match(hex).hasMatch())
        return {};
    return QStringLiteral("0x") + hex.toLower();
}

static QString normalizedBindingStatement(QString statement)
{
    statement.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    statement.replace(QChar('\r'), QChar('\n'));
    return statement.trimmed();
}

static QByteArray cleanHexBytes(const QString& value)
{
    QString hex = value.trimmed();
    if (hex.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
        hex = hex.mid(2);
    if (hex.size() % 2 != 0)
        hex.prepend('0');
    if (!QRegularExpression(QStringLiteral("^[0-9a-fA-F]*$")).match(hex).hasMatch())
        return {};
    return QByteArray::fromHex(hex.toLatin1());
}

static quint64 rotl64(quint64 value, int shift)
{
    if (shift == 0)
        return value;
    return (value << shift) | (value >> (64 - shift));
}

static QByteArray keccak256(const QByteArray& input)
{
    static const quint64 roundConstants[24] = {
        0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
        0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
        0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
        0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
        0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
        0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
        0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
        0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
    };
    static const int rotation[25] = {
        0, 1, 62, 28, 27,
        36, 44, 6, 55, 20,
        3, 10, 43, 25, 39,
        41, 45, 15, 21, 8,
        18, 2, 61, 56, 14
    };

    std::array<quint64, 25> state{};
    QByteArray bytes = input;
    const int rate = 136;
    bytes.append(char(0x01));
    while ((bytes.size() % rate) != rate - 1)
        bytes.append(char(0x00));
    bytes.append(char(0x80));

    auto permute = [&]() {
        for (quint64 rc : roundConstants) {
            quint64 c[5];
            quint64 d[5];
            for (int x = 0; x < 5; ++x)
                c[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^ state[x + 15] ^ state[x + 20];
            for (int x = 0; x < 5; ++x)
                d[x] = c[(x + 4) % 5] ^ rotl64(c[(x + 1) % 5], 1);
            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y)
                    state[x + 5 * y] ^= d[x];
            }

            std::array<quint64, 25> b{};
            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y)
                    b[y + 5 * ((2 * x + 3 * y) % 5)] = rotl64(state[x + 5 * y], rotation[x + 5 * y]);
            }
            for (int x = 0; x < 5; ++x) {
                for (int y = 0; y < 5; ++y)
                    state[x + 5 * y] = b[x + 5 * y] ^ ((~b[((x + 1) % 5) + 5 * y]) & b[((x + 2) % 5) + 5 * y]);
            }
            state[0] ^= rc;
        }
    };

    for (int block = 0; block < bytes.size(); block += rate) {
        for (int i = 0; i < rate; ++i) {
            int lane = i / 8;
            int shift = (i % 8) * 8;
            state[lane] ^= (quint64(static_cast<unsigned char>(bytes[block + i])) << shift);
        }
        permute();
    }

    QByteArray out;
    out.resize(32);
    for (int i = 0; i < 32; ++i) {
        int lane = i / 8;
        int shift = (i % 8) * 8;
        out[i] = char((state[lane] >> shift) & 0xff);
    }
    return out;
}

static QByteArray ethereumPersonalSignDigest(const QString& message)
{
    QByteArray msg = message.toUtf8();
    QByteArray prefixed;
    prefixed.append(char(0x19));
    prefixed.append("Ethereum Signed Message:\n");
    prefixed.append(QByteArray::number(msg.size()));
    prefixed.append(msg);
    return keccak256(prefixed);
}

static QString recoverEthereumPersonalSignAddress(const QString& message, const QString& signatureHex, QString* errorText = nullptr)
{
    auto setError = [errorText](const QString& text) {
        if (errorText)
            *errorText = text;
    };

    QByteArray signature = cleanHexBytes(signatureHex);
    if (signature.size() != 65) {
        setError(QString::fromUtf8("MetaMask 签名长度不是 65 字节"));
        return {};
    }

    int recId = static_cast<unsigned char>(signature[64]);
    if (recId >= 27)
        recId -= 27;
    if (recId < 0 || recId > 3) {
        setError(QString::fromUtf8("MetaMask 签名恢复位 v 无效"));
        return {};
    }

    QByteArray digest = ethereumPersonalSignDigest(message);
    BN_CTX* ctx = BN_CTX_new();
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    if (!ctx || !group) {
        setError(QString::fromUtf8("OpenSSL secp256k1 初始化失败"));
        if (group) EC_GROUP_free(group);
        if (ctx) BN_CTX_free(ctx);
        return {};
    }

    BN_CTX_start(ctx);
    BIGNUM* order = BN_CTX_get(ctx);
    BIGNUM* field = BN_CTX_get(ctx);
    BIGNUM* r = BN_CTX_get(ctx);
    BIGNUM* s = BN_CTX_get(ctx);
    BIGNUM* x = BN_CTX_get(ctx);
    BIGNUM* e = BN_CTX_get(ctx);
    BIGNUM* eMod = BN_CTX_get(ctx);
    BIGNUM* eNeg = BN_CTX_get(ctx);
    BIGNUM* rInv = BN_CTX_get(ctx);
    BIGNUM* rr = BN_CTX_get(ctx);
    if (!rr) {
        setError(QString::fromUtf8("OpenSSL BN_CTX 空间不足"));
        BN_CTX_end(ctx);
        EC_GROUP_free(group);
        BN_CTX_free(ctx);
        return {};
    }

    bool ok = EC_GROUP_get_order(group, order, ctx) &&
              EC_GROUP_get_curve_GFp(group, field, nullptr, nullptr, ctx) &&
              BN_bin2bn(reinterpret_cast<const unsigned char*>(signature.constData()), 32, r) &&
              BN_bin2bn(reinterpret_cast<const unsigned char*>(signature.constData() + 32), 32, s) &&
              BN_bin2bn(reinterpret_cast<const unsigned char*>(digest.constData()), digest.size(), e);
    if (!ok || BN_is_zero(r) || BN_is_zero(s) || BN_cmp(r, order) >= 0 || BN_cmp(s, order) >= 0) {
        setError(QString::fromUtf8("MetaMask 签名 r/s 参数无效"));
        BN_CTX_end(ctx);
        EC_GROUP_free(group);
        BN_CTX_free(ctx);
        return {};
    }

    BN_copy(x, r);
    if (recId >= 2)
        BN_add(x, x, order);
    if (BN_cmp(x, field) >= 0) {
        setError(QString::fromUtf8("MetaMask 签名恢复点超出 secp256k1 曲线范围"));
        BN_CTX_end(ctx);
        EC_GROUP_free(group);
        BN_CTX_free(ctx);
        return {};
    }

    EC_POINT* R = EC_POINT_new(group);
    EC_POINT* nR = EC_POINT_new(group);
    EC_POINT* temp = EC_POINT_new(group);
    EC_POINT* Q = EC_POINT_new(group);
    if (!R || !nR || !temp || !Q ||
        !EC_POINT_set_compressed_coordinates_GFp(group, R, x, recId & 1, ctx) ||
        !EC_POINT_mul(group, nR, nullptr, R, order, ctx) ||
        !EC_POINT_is_at_infinity(group, nR)) {
        setError(QString::fromUtf8("MetaMask 签名恢复点无效"));
        if (R) EC_POINT_free(R);
        if (nR) EC_POINT_free(nR);
        if (temp) EC_POINT_free(temp);
        if (Q) EC_POINT_free(Q);
        BN_CTX_end(ctx);
        EC_GROUP_free(group);
        BN_CTX_free(ctx);
        return {};
    }

    BN_mod(eMod, e, order, ctx);
    BN_mod_sub(eNeg, order, eMod, order, ctx);
    rInv = BN_mod_inverse(rInv, r, order, ctx);
    if (!rInv ||
        !EC_POINT_mul(group, temp, eNeg, R, s, ctx) ||
        !EC_POINT_mul(group, Q, nullptr, temp, rInv, ctx)) {
        setError(QString::fromUtf8("MetaMask 公钥恢复计算失败"));
        EC_POINT_free(R);
        EC_POINT_free(nR);
        EC_POINT_free(temp);
        EC_POINT_free(Q);
        BN_CTX_end(ctx);
        EC_GROUP_free(group);
        BN_CTX_free(ctx);
        return {};
    }

    size_t pubLen = EC_POINT_point2oct(group, Q, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, ctx);
    QByteArray pub;
    pub.resize(static_cast<int>(pubLen));
    if (pubLen != 65 ||
        EC_POINT_point2oct(group, Q, POINT_CONVERSION_UNCOMPRESSED,
                           reinterpret_cast<unsigned char*>(pub.data()), pubLen, ctx) != pubLen) {
        setError(QString::fromUtf8("MetaMask 公钥序列化失败"));
        EC_POINT_free(R);
        EC_POINT_free(nR);
        EC_POINT_free(temp);
        EC_POINT_free(Q);
        BN_CTX_end(ctx);
        EC_GROUP_free(group);
        BN_CTX_free(ctx);
        return {};
    }

    QByteArray hash = keccak256(pub.mid(1));
    QString address = QStringLiteral("0x") + QString::fromLatin1(hash.right(20).toHex());

    EC_POINT_free(R);
    EC_POINT_free(nR);
    EC_POINT_free(temp);
    EC_POINT_free(Q);
    BN_CTX_end(ctx);
    EC_GROUP_free(group);
    BN_CTX_free(ctx);
    return normalizedEthAddress(address);
}

static QSplitter* replaceLayoutWithSplitter(QBoxLayout* layout, QWidget* parent,
                                            Qt::Orientation orientation, const QList<int>& sizes)
{
    QList<QWidget*> widgets;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widgets.append(widget);
        } else if (QLayout* nested = item->layout()) {
            while (QLayoutItem* nestedItem = nested->takeAt(0)) {
                if (QWidget* nestedWidget = nestedItem->widget())
                    widgets.append(nestedWidget);
                delete nestedItem;
            }
        }
        delete item;
    }

    QSplitter* splitter = new QSplitter(orientation, parent);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(6);
    for (QWidget* widget : widgets) {
        widget->setParent(splitter);
        splitter->addWidget(widget);
    }
    if (!sizes.isEmpty())
        splitter->setSizes(sizes);
    layout->addWidget(splitter);
    return splitter;
}

class MessageRouteAnimation : public QWidget
{
public:
    explicit MessageRouteAnimation(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(48);
        setMaximumHeight(52);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(&timer, &QTimer::timeout, this, [this]() {
            frame++;
            if (frame > totalFrames + 38) {
                timer.stop();
                active = false;
                title = QString::fromUtf8("安全链路就绪");
                currentText = QString::fromUtf8("等待消息、文件或转账事件");
            }
            update();
        });
        title = QString::fromUtf8("安全链路就绪");
        currentText = QString::fromUtf8("等待消息、文件或转账事件");
    }

    void startFlow(const QString& flowTitle, const QStringList& flowStages, const QColor& flowAccent)
    {
        title = flowTitle;
        stages = flowStages;
        accent = flowAccent;
        active = true;
        frame = 0;
        totalFrames = qMax(80, stages.size() * 22);
        currentText = stages.isEmpty() ? QString::fromUtf8("处理中") : stages.first();
        timer.start(28);
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF panel = rect().adjusted(8, 5, -8, -6);
        QLinearGradient bg(panel.topLeft(), panel.bottomRight());
        bg.setColorAt(0, QColor("#ffffff"));
        bg.setColorAt(1, active ? QColor("#f1f7ff") : QColor("#f8fafc"));
        painter.setPen(QPen(active ? accent.lighter(145) : QColor("#d7e1ee"), 1));
        painter.setBrush(bg);
        painter.drawRoundedRect(panel, 8, 8);

        double progress = active ? qMin(1.0, frame / static_cast<double>(totalFrames)) : 0.0;
        int stepCount = qMax(1, stages.size());
        int activeStep = stages.isEmpty() ? -1 : qMin(stages.size() - 1, static_cast<int>(progress * stages.size()));
        if (active && activeStep >= 0)
            currentText = stages.at(activeStep);

        QFont titleFont = painter.font();
        titleFont.setBold(true);
        titleFont.setPointSize(7);
        painter.setFont(titleFont);
        painter.setPen(active ? accent.darker(130) : QColor("#64748b"));
        QString compactTitle = QFontMetrics(titleFont).elidedText(title, Qt::ElideRight, 105);
        painter.drawText(QRectF(panel.left() + 10, panel.top() + 5, 110, 14), Qt::AlignLeft | Qt::AlignVCenter, compactTitle);

        QFont textFont = painter.font();
        textFont.setBold(false);
        textFont.setPointSize(7);
        painter.setFont(textFont);
        painter.setPen(QColor("#475569"));
        QString compactStep = QFontMetrics(textFont).elidedText(currentText, Qt::ElideRight, qMax(80, width() / 4));
        painter.drawText(QRectF(panel.left() + 122, panel.top() + 5, qMax<qreal>(80, panel.width() * 0.28), 14),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         compactStep);

        int left = panel.left() + 10;
        int right = panel.right() - 10;
        int y = panel.bottom() - 12;
        if (right <= left + 120)
            return;

        QRectF track(left, y - 3, right - left, 6);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#e7edf5"));
        painter.drawRoundedRect(track, 3, 3);

        QRectF fill(track.left(), track.top(), track.width() * progress, track.height());
        painter.setBrush(active ? accent : QColor("#94a3b8"));
        painter.drawRoundedRect(fill, 3, 3);

        for (int i = 0; i < stepCount; ++i) {
            double ratio = stepCount == 1 ? 0.0 : i / static_cast<double>(stepCount - 1);
            int x = left + static_cast<int>((right - left) * ratio);
            bool done = active && ratio <= progress + 0.001;

            painter.setPen(QPen(done ? accent : QColor("#cbd5e1"), 1));
            painter.setBrush(done ? accent : QColor("#ffffff"));
            painter.drawEllipse(QPointF(x, y), 4, 4);
        }

        if (active) {
            int progressX = left + static_cast<int>((right - left) * progress);
            QColor glow = accent;
            glow.setAlpha(45);
            painter.setBrush(glow);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(progressX, y), 10, 10);
            painter.setBrush(accent);
            painter.drawEllipse(QPointF(progressX, y), 3, 3);
        }
    }

private:
    QTimer timer;
    QString title;
    QString currentText;
    QStringList stages;
    QColor accent = QColor("#2563eb");
    int frame = 0;
    int totalFrames = 110;
    bool active = false;
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , hermesBridge(this)
    , securityDashboard(nullptr)
    , routeAnimation(nullptr)
    , walletInput(nullptr)
    , hermesEndpointInput(nullptr)
    , transferBtn(nullptr)
    , bindIdentityBtn(nullptr)
    , tamperMessageBtn(nullptr)
    , tamperFileBtn(nullptr)
    , verifyLedgerBtn(nullptr)
    , verifyPeerWalletBtn(nullptr)
    , createGroupBtn(nullptr)
    , hermesAttachBtn(nullptr)
    , hermesShareBtn(nullptr)
    , nextMessageTamperArmed(false)
{
    ui->setupUi(this);
    setStyleSheet(STYLESHEET);
    setMinimumSize(1120, 680);
    ui->userList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->leftPanel->setMinimumWidth(170);
    ui->leftPanel->setMaximumWidth(QWIDGETSIZE_MAX);
    ui->centerPanel->setMinimumWidth(360);
    ui->rightPanel->setMinimumWidth(280);
    ui->rightPanel->setMaximumWidth(QWIDGETSIZE_MAX);

    walletInput = new QLineEdit(ui->loginGroup);
    walletInput->setObjectName("walletInput");
    walletInput->setPlaceholderText(QString::fromUtf8("Sepolia 钱包地址（用于局域网转账，可选）"));
    walletInput->setText("0x7bD441c4C131901dA79a04B0a8137e029754c9DE");
    walletInput->setToolTip(QString::fromUtf8("本机会广播这个 MetaMask Sepolia 地址；另一位同学应改成自己的钱包地址"));
    ui->loginGroupLayout->insertWidget(2, walletInput);

    transferBtn = new QPushButton(QString::fromUtf8("¥"), ui->inputGroup);
    transferBtn->setObjectName("transferBtn");
    transferBtn->setFixedSize(QSize(32, 30));
    transferBtn->setCursor(Qt::PointingHandCursor);
    transferBtn->setToolTip(QString::fromUtf8("向当前选中的局域网用户发起 MetaMask Sepolia 测试币转账"));

    ui->inputGroup->setTitle(QString::fromUtf8("安全输入"));
    ui->inputGroup->setMinimumHeight(122);
    ui->inputGroup->setMaximumHeight(QWIDGETSIZE_MAX);
    ui->msgInput->setMinimumHeight(52);
    ui->msgInput->setMaximumHeight(QWIDGETSIZE_MAX);
    ui->msgInput->setPlaceholderText(QString::fromUtf8("输入消息，按发送按钮发送..."));
    ui->sendMsgBtn->setText(QString::fromUtf8("发送"));
    ui->sendMsgBtn->setFixedSize(QSize(76, 32));
    ui->sendMsgBtn->setCursor(Qt::PointingHandCursor);
    ui->sendFileBtn->setText("");
    ui->sendFileBtn->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->sendFileBtn->setIconSize(QSize(20, 20));
    ui->sendFileBtn->setFixedSize(QSize(32, 30));
    ui->sendFileBtn->setToolTip(QString::fromUtf8("发送文件"));

    while (QLayoutItem* item = ui->inputGroupLayout->takeAt(0)) {
        if (item->widget())
            item->widget()->setParent(ui->inputGroup);
        if (QLayout* nested = item->layout()) {
            while (QLayoutItem* nestedItem = nested->takeAt(0)) {
                if (nestedItem->widget())
                    nestedItem->widget()->setParent(ui->inputGroup);
                delete nestedItem;
            }
        }
        delete item;
    }

    QWidget* composerBar = new QWidget(ui->inputGroup);
    composerBar->setObjectName("composerBar");
    QVBoxLayout* composerLayout = new QVBoxLayout(composerBar);
    composerLayout->setContentsMargins(8, 6, 8, 8);
    composerLayout->setSpacing(4);
    composerLayout->addWidget(ui->msgInput);

    QHBoxLayout* toolLayout = new QHBoxLayout();
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->setSpacing(8);
    toolLayout->addWidget(ui->sendFileBtn);
    toolLayout->addWidget(transferBtn);
    toolLayout->addStretch();
    toolLayout->addWidget(ui->sendMsgBtn);
    composerLayout->addLayout(toolLayout);
    ui->inputGroupLayout->setContentsMargins(2, 2, 2, 2);
    ui->inputGroupLayout->addWidget(composerBar);

    QGroupBox* dashboardGroup = new QGroupBox("安全态势感知", ui->rightPanel);
    dashboardGroup->setMinimumHeight(220);
    QVBoxLayout* dashboardLayout = new QVBoxLayout(dashboardGroup);
    dashboardLayout->setContentsMargins(6, 8, 6, 6);
    securityDashboard = new SecurityDashboard(dashboardGroup);
    dashboardLayout->addWidget(securityDashboard);
    ui->rightLayout->insertWidget(0, dashboardGroup);
    setupInnovationPanel();

    routeAnimation = new MessageRouteAnimation(ui->chatGroup);
    ui->chatGroupLayout->addWidget(routeAnimation);

    QSplitter* leftSplitter = replaceLayoutWithSplitter(ui->leftLayout, ui->leftPanel, Qt::Vertical, {145, 130, 390});
    leftSplitter->setObjectName("leftSplitter");
    QSplitter* centerSplitter = replaceLayoutWithSplitter(ui->centerLayout, ui->centerPanel, Qt::Vertical, {500, 125});
    centerSplitter->setObjectName("centerSplitter");
    QSplitter* rightSplitter = replaceLayoutWithSplitter(ui->rightLayout, ui->rightPanel, Qt::Vertical, {270, 190, 190});
    rightSplitter->setObjectName("rightSplitter");
    QSplitter* mainSplitter = replaceLayoutWithSplitter(ui->horizontalLayout, ui->centralwidget, Qt::Horizontal, {220, 560, 340});
    mainSplitter->setObjectName("mainSplitter");
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setStretchFactor(2, 0);

    statusBar()->showMessage("未连接 | 端口: 8888 | 等待登录...");

    connect(ui->loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(ui->userList, &QListWidget::itemClicked, this, &MainWindow::onUserSelected);
    connect(ui->sendMsgBtn, &QPushButton::clicked, this, &MainWindow::onSendMessageClicked);
    connect(ui->sendFileBtn, &QPushButton::clicked, this, &MainWindow::onSendFileClicked);
    connect(transferBtn, &QPushButton::clicked, this, &MainWindow::onTransferClicked);
    connect(bindIdentityBtn, &QPushButton::clicked, this, &MainWindow::onBindIdentityClicked);
    connect(tamperMessageBtn, &QPushButton::clicked, this, &MainWindow::onTamperMessageClicked);
    connect(tamperFileBtn, &QPushButton::clicked, this, &MainWindow::onTamperFileClicked);
    connect(verifyLedgerBtn, &QPushButton::clicked, this, &MainWindow::onVerifyLedgerClicked);
    connect(verifyPeerWalletBtn, &QPushButton::clicked, this, &MainWindow::onVerifyPeerWalletClicked);
    connect(createGroupBtn, &QPushButton::clicked, this, &MainWindow::onCreateGroupClicked);
    connect(hermesAttachBtn, &QPushButton::clicked, this, &MainWindow::onHermesAttachClicked);
    connect(hermesShareBtn, &QPushButton::clicked, this, &MainWindow::onHermesShareClicked);

    // Ctrl+Enter sends message
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this, SLOT(onSendMessageClicked()));

    connect(&udp, &UDPManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(&udp, &UDPManager::fileReceived, this, &MainWindow::onFileReceived);
    // "login" broadcast → add user AND respond with login_ack so they discover us too
    connect(&udp, &UDPManager::loginBroadcastReceived, this, &MainWindow::onOnlineUserAdded);
    // "login_ack" unicast response → just add user, don't respond again
    connect(&udp, &UDPManager::loginAckReceived, this, &MainWindow::onOnlineUserAddedAck);
    connect(&udp, &UDPManager::logoutBroadcastReceived, this, &MainWindow::onOnlineUserRemoved);
    // Route raw incoming message/file data through crypto handlers for decryption
    connect(&udp, &UDPManager::rawMessageReceived, this, [this](const QString& fromIp, const QByteArray& data) {
        beginSmTrace(fromIp,
                     targetDisplayName(fromIp),
                     QString::fromUtf8("解密接收 / 聊天消息"),
                     QString::fromUtf8("UDP密文包 %1 字节").arg(data.size()));
        msgHandler.onReceiveRawMessage(data, fromIp, infoMgr, logger, udp);
        endSmTrace(QString::fromUtf8("接收处理完成，详见 SM3/SM2 校验结果"));
    });
    connect(&udp, &UDPManager::rawFileChunkReceived, this, [this](const QString& fromIp, const QByteArray& chunk) {
        beginSmTrace(fromIp,
                     targetDisplayName(fromIp),
                     QString::fromUtf8("解密接收 / 文件分片"),
                     QString::fromUtf8("UDP文件分片 %1 字节").arg(chunk.size()));
        fileHandler.onReceiveFileChunk(chunk, fromIp, infoMgr, logger, udp);
        endSmTrace(QString::fromUtf8("文件分片处理完成"));
    });
    connect(&infoMgr, &InfoManager::userListChanged, this, &MainWindow::updateOnlineList);
    connect(&logger, &Logger::newLog, this, &MainWindow::onLogMessage);
    connect(&dappServer, &LocalDappServer::callbackReceived, this, &MainWindow::onDappCallback);
    connect(&hermesBridge, &HermesBridge::encryptedReplyReady, this, [this](const QByteArray& packet) {
        beginSmTrace(HermesBridge::peerId(),
                     HermesBridge::displayName(),
                     QString::fromUtf8("解密接收 / 本机Hermes回复"),
                     QString::fromUtf8("Hermes国密回包 %1 字节").arg(packet.size()));
        msgHandler.onReceiveRawMessage(packet, HermesBridge::peerId(), infoMgr, logger, udp);
        endSmTrace(QString::fromUtf8("Hermes 回复接收处理完成"));
    });
    connect(&hermesBridge, &HermesBridge::plainReplyReady, this, [this](const QString& requestId, const QString& replyText, const QString& errorText) {
        QJsonObject req = pendingHermesProxyRequests.take(requestId);
        if (req.isEmpty())
            return;

        QString requesterIp = req["requesterIp"].toString();
        QString requesterNick = req["requesterNick"].toString();
        QString finalText = errorText.isEmpty()
                                ? replyText
                                : QString::fromUtf8("Hermes 代理调用失败：%1").arg(errorText);

        QJsonObject payload;
        payload["gmControl"] = "hermes_proxy";
        payload["kind"] = "proxy_reply";
        payload["version"] = 1;
        payload["requestId"] = requestId;
        payload["ownerNick"] = infoMgr.getSelfNick();
        payload["ownerFingerprint"] = selfFingerprint();
        payload["ok"] = errorText.isEmpty();
        payload["reply"] = finalText;
        payload["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

        beginSmTrace(requesterIp,
                     requesterNick,
                     QString::fromUtf8("加密发送 / 共享Hermes回包"),
                     finalText.left(160));
        bool sent = sendEncryptedControlMessage(requesterIp, payload);
        endSmTrace(sent ? QString::fromUtf8("共享 Hermes 回包已发送") : QString::fromUtf8("共享 Hermes 回包发送失败"));
        if (sent) {
            appendChatEvent(requesterIp,
                            QString::fromUtf8("Hermes 代理回复"),
                            QString::fromUtf8("已将本机 Hermes 回复加密返回给 %1。").arg(requesterNick));
            showBottomFlow(requesterIp,
                           QString::fromUtf8("共享 Hermes 回包"),
                           QStringList() << QString::fromUtf8("Hermes 生成回复")
                                         << QString::fromUtf8("SM3 摘要")
                                         << QString::fromUtf8("SM2 签名")
                                         << QString::fromUtf8("SM4 加密")
                                         << QString::fromUtf8("回传授权用户"),
                           "#0f766e");
        }
    });
    connect(&hermesBridge, &HermesBridge::statusMessage, this, &MainWindow::onHermesStatusMessage);

    // Heartbeat: re-broadcast every 15s so late joiners discover us
    connect(&heartbeatTimer, &QTimer::timeout, this, &MainWindow::onHeartbeat);
}

MainWindow::~MainWindow()
{
    heartbeatTimer.stop();
    if (ui->loginBtn->isEnabled() == false) {
        udp.sendLogoutBroadcast(infoMgr.getSelfNick());
    }
    delete ui;
}

void MainWindow::setupInnovationPanel()
{
    QGroupBox* labGroup = new QGroupBox("创新实验台", ui->leftPanel);
    QVBoxLayout* labLayout = new QVBoxLayout(labGroup);
    labLayout->setSpacing(6);
    labLayout->setContentsMargins(6, 8, 6, 6);

    bindIdentityBtn = new QPushButton("双签名绑定", labGroup);
    tamperMessageBtn = new QPushButton("消息篡改", labGroup);
    tamperFileBtn = new QPushButton("文件篡改", labGroup);
    verifyLedgerBtn = new QPushButton("核验存证", labGroup);
    verifyPeerWalletBtn = new QPushButton(QString::fromUtf8("验证对方钱包"), labGroup);
    createGroupBtn = new QPushButton(QString::fromUtf8("创建群组"), labGroup);
    hermesEndpointInput = new QLineEdit(labGroup);
    hermesEndpointInput->setPlaceholderText(QString::fromUtf8("Hermes one-shot 命令"));
    hermesEndpointInput->setText(hermesBridge.endpoint());
    hermesEndpointInput->setToolTip(QString::fromUtf8("推荐使用官方非交互命令 hermes -z；也可写成 wsl:hermes -z。若命令中写 %1，会用消息替换它。"));
    hermesAttachBtn = new QPushButton(QString::fromUtf8("接入 Hermes"), labGroup);
    hermesShareBtn = new QPushButton(QString::fromUtf8("共享 Hermes"), labGroup);

    bindIdentityBtn->setToolTip("用通信SM2身份和模拟链上SM2身份同时签名绑定声明");
    tamperMessageBtn->setToolTip("模拟密文/摘要被改动后，SM3和SM2如何拦截");
    tamperFileBtn->setToolTip("模拟文件被改动后，SM3文件哈希和Merkle根如何发现异常");
    verifyLedgerBtn->setToolTip("核验本地存证链是否连续");
    verifyPeerWalletBtn->setToolTip(QString::fromUtf8("要求当前选中的真实用户同时用通信 SM2 私钥和 MetaMask 钱包签名同一份绑定声明，验证“用户=钱包”"));
    createGroupBtn->setToolTip(QString::fromUtf8("先在在线列表中多选真实用户，再创建本地国密群组；群消息会分别用每个成员的 SM2 公钥加密发送"));
    hermesAttachBtn->setToolTip(QString::fromUtf8("把本机 WSL 中的 Hermes Agent 添加为可选 AI 协同节点"));
    hermesShareBtn->setToolTip(QString::fromUtf8("把本机 Hermes 授权共享给当前选中的局域网用户，认证依据是 SM2 公钥指纹而不是昵称"));

    labLayout->addWidget(bindIdentityBtn);
    labLayout->addWidget(tamperMessageBtn);
    labLayout->addWidget(tamperFileBtn);
    labLayout->addWidget(verifyLedgerBtn);
    labLayout->addWidget(verifyPeerWalletBtn);
    labLayout->addWidget(createGroupBtn);
    labLayout->addSpacing(4);
    labLayout->addWidget(hermesEndpointInput);
    labLayout->addWidget(hermesAttachBtn);
    labLayout->addWidget(hermesShareBtn);
    bool hermesVisible = isHermesFeatureEnabled();
    hermesEndpointInput->setVisible(hermesVisible);
    hermesAttachBtn->setVisible(hermesVisible);
    hermesShareBtn->setVisible(hermesVisible);
    ui->leftLayout->insertWidget(1, labGroup);
}

bool MainWindow::isHermesFeatureEnabled() const
{
    QByteArray env = qgetenv("GMSECURECHAT_ENABLE_HERMES").trimmed().toLower();
    if (env == "1" || env == "true" || env == "yes" || env == "on")
        return true;

    QString appFlag = QCoreApplication::applicationDirPath() + "/enable_hermes.local";
    if (QFile::exists(appFlag))
        return true;

    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!configDir.isEmpty() && QFile::exists(configDir + "/enable_hermes.local"))
        return true;

    return false;
}

void MainWindow::ensureHermesUser(bool selectAfterAttach)
{
    if (!isHermesFeatureEnabled())
        return;
    if (hermesEndpointInput)
        hermesBridge.setEndpoint(hermesEndpointInput->text());
    if (!hermesBridge.ensureIdentity()) {
        appendCryptoLog(QString::fromUtf8("[HermesBridge] Hermes 本地 SM2 身份生成失败"));
        return;
    }

    if (!infoMgr.addOrUpdateUser(HermesBridge::peerId(),
                                 HermesBridge::displayName(),
                                 hermesBridge.publicKeyHex(),
                                 QString())) {
        appendCryptoLog(QString::fromUtf8("[HermesBridge] 添加虚拟用户失败"));
        return;
    }

    appendNetworkLog(QString::fromUtf8("Hermes Agent 已接入本地会话: %1").arg(hermesBridge.endpoint()));
    appendCryptoLog(QString::fromUtf8("[HermesBridge] 虚拟用户 SM2 公钥指纹: %1").arg(hermesBridge.fingerprint()));

    if (selectAfterAttach) {
        currentTargetIp = HermesBridge::peerId();
        currentTargetNick = HermesBridge::displayName();
        ui->chatGroup->setTitle(QString::fromUtf8("安全会话 — Hermes Agent"));
        renderConversation(currentTargetIp);
        appendChatEvent(currentTargetIp,
                        QString::fromUtf8("Hermes 已接入"),
                        QString::fromUtf8("发送文字消息时会先走 SM3/SM2/SM4 本地安全桥接，再调用 Hermes 官方 one-shot CLI。"));
    }
}

bool MainWindow::isGroupContact(const QString& targetId) const
{
    return groupMembers.contains(targetId);
}

QString MainWindow::groupIdFromWire(const QString& wireGroupId) const
{
    if (wireGroupId.startsWith(QStringLiteral("group://")))
        return wireGroupId;
    return QStringLiteral("group://") + wireGroupId;
}

QStringList MainWindow::validGroupMembers(const QString& groupId) const
{
    QStringList result;
    for (const QString& ip : groupMembers.value(groupId)) {
        if (ip != HermesBridge::peerId() &&
            !isHermesProxyContact(ip) &&
            !isGroupContact(ip) &&
            !infoMgr.getPubKeyByIp(ip).empty() &&
            !result.contains(ip)) {
            result << ip;
        }
    }
    return result;
}

QString MainWindow::ensureConversationLogFile(const QString& conversationId, const QString& displayName)
{
    if (conversationId.isEmpty())
        return {};
    if (conversationLogFiles.contains(conversationId))
        return conversationLogFiles.value(conversationId);

    QString dirPath = QCoreApplication::applicationDirPath() + QStringLiteral("/sm_trace_logs");
    QDir().mkpath(dirPath);

    QString safeSelf = infoMgr.getSelfNick().trimmed();
    if (safeSelf.isEmpty())
        safeSelf = QStringLiteral("local");
    QString safePeer = displayName.trimmed();
    if (safePeer.isEmpty())
        safePeer = conversationId;
    QRegularExpression unsafe(QStringLiteral("[\\\\/:*?\"<>|\\s]+"));
    safeSelf.replace(unsafe, QStringLiteral("_"));
    safePeer.replace(unsafe, QStringLiteral("_"));
    if (safePeer.size() > 40)
        safePeer = safePeer.left(40);

    QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    QString filePath = QStringLiteral("%1/%2_%3_%4_SM算法追踪.log")
                           .arg(dirPath, stamp, safeSelf, safePeer);
    conversationLogFiles[conversationId] = filePath;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "GM Secure Chat SM Algorithm Trace Log\n";
        out << "CreatedAt: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "\n";
        out << "Conversation: " << displayName << "\n";
        out << "ConversationId: " << conversationId << "\n";
        out << "LocalNick: " << infoMgr.getSelfNick() << "\n";
        out << "LocalSM2Fingerprint: " << selfFingerprint() << "\n";
        out << "Note: 本文件按会话首次消息时间命名；同一在线会话内持续追加 SM2/SM3/SM4/ZUC 中间值、校验结果和失败原因。\n\n";
    }
    appendNetworkLog(QString::fromUtf8("SM算法追踪日志已创建: %1").arg(filePath));
    return filePath;
}

void MainWindow::beginSmTrace(const QString& conversationId, const QString& displayName, const QString& direction, const QString& summary)
{
    activeTraceFile = ensureConversationLogFile(conversationId, displayName);
    if (activeTraceFile.isEmpty())
        return;

    QFile file(activeTraceFile);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "\n========== " << direction << " ==========\n";
        out << "Time: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "\n";
        out << "Conversation: " << displayName << "\n";
        out << "Summary: " << summary.left(300).replace('\n', "\\n") << "\n";
        out << "----------------------------------------\n";
    }
    logger.setDetailedLogFile(activeTraceFile);
}

void MainWindow::endSmTrace(const QString& result)
{
    QString filePath = activeTraceFile;
    logger.clearDetailedLogFile();
    activeTraceFile.clear();
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "Result: " << result << "\n";
        out << "========== END ==========\n";
    }
}

void MainWindow::resetConversationTrace(const QString& conversationId)
{
    conversationLogFiles.remove(conversationId);
}

bool MainWindow::handleGroupControlMessage(const QString& fromIp, const QString& fromNick, const QString& plainText)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(plainText.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    QJsonObject payload = doc.object();
    if (payload.value(QStringLiteral("gmControl")).toString() != QStringLiteral("group_chat"))
        return false;
    if (payload.value(QStringLiteral("kind")).toString() != QStringLiteral("group_message"))
        return true;

    QString groupId = groupIdFromWire(payload.value(QStringLiteral("groupId")).toString());
    QString groupName = payload.value(QStringLiteral("groupName")).toString(QString::fromUtf8("未命名群组"));
    QString textValue = payload.value(QStringLiteral("text")).toString();
    QJsonArray members = payload.value(QStringLiteral("members")).toArray();

    QStringList ips = groupMembers.value(groupId);
    if (!ips.contains(fromIp))
        ips << fromIp;
    for (const QJsonValue& value : members) {
        QJsonObject member = value.toObject();
        QString ip = member.value(QStringLiteral("ip")).toString();
        QString fp = member.value(QStringLiteral("fingerprint")).toString();
        if (!ip.isEmpty() && fp == pubFingerprintForIp(ip) && !ips.contains(ip))
            ips << ip;
    }

    groupNames[groupId] = groupName;
    groupMembers[groupId] = ips;
    appendChatMessage(groupId, fromNick, textValue, false);
    showBottomFlow(groupId,
                   QString::fromUtf8("群消息安全接收"),
                   QStringList() << QString::fromUtf8("收到成员密文")
                                 << QString::fromUtf8("SM2 解封")
                                 << QString::fromUtf8("SM4 解密")
                                 << QString::fromUtf8("SM3 校验")
                                 << QString::fromUtf8("SM2 验签"),
                   "#2563eb");
    updateSessionHashChain("receive_group", groupName, textValue);
    updateOnlineList();
    return true;
}

void MainWindow::sendGroupMessage(const QString& groupId, const QString& plainText)
{
    QStringList members = validGroupMembers(groupId);
    if (members.isEmpty()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("群组不可用"),
                             QString::fromUtf8("群组里没有当前在线且拥有有效 SM2 公钥的成员。"));
        return;
    }

    QString groupName = groupNames.value(groupId, QString::fromUtf8("未命名群组"));
    QJsonArray memberArray;
    QSet<QString> allMemberIps;
    for (const QString& ip : members)
        allMemberIps.insert(ip);
    for (const QString& ip : allMemberIps) {
        QJsonObject member;
        member["ip"] = ip;
        member["nick"] = infoMgr.getNickByIp(ip);
        member["fingerprint"] = pubFingerprintForIp(ip);
        memberArray.append(member);
    }

    QJsonObject selfMember;
    selfMember["ip"] = QStringLiteral("self");
    selfMember["nick"] = infoMgr.getSelfNick();
    selfMember["fingerprint"] = selfFingerprint();
    memberArray.append(selfMember);

    QJsonObject payload;
    payload["gmControl"] = "group_chat";
    payload["kind"] = "group_message";
    payload["version"] = 1;
    payload["groupId"] = groupId;
    payload["groupName"] = groupName;
    payload["senderNick"] = infoMgr.getSelfNick();
    payload["senderFingerprint"] = selfFingerprint();
    payload["text"] = plainText;
    payload["members"] = memberArray;
    payload["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

    beginSmTrace(groupId, groupName, QString::fromUtf8("加密发送 / 群组消息"), plainText);
    int okCount = 0;
    for (const QString& ip : members) {
        logger.logCrypto(QString::fromUtf8("[GROUP] 群发目标: %1 (%2) | SM2指纹=%3")
                         .arg(infoMgr.getNickByIp(ip), ip, pubFingerprintForIp(ip)));
        if (sendEncryptedControlMessage(ip, payload))
            okCount++;
    }
    endSmTrace(QString::fromUtf8("群发完成: %1/%2 个成员已发送").arg(okCount).arg(members.size()));

    appendChatMessage(groupId, infoMgr.getSelfNick(), plainText, true);
    showBottomFlow(groupId,
                   QString::fromUtf8("群消息安全发送"),
                   QStringList() << QString::fromUtf8("群成员展开")
                                 << QString::fromUtf8("逐成员SM2取公钥")
                                 << QString::fromUtf8("SM3/SM2/SM4封包")
                                 << QString::fromUtf8("UDP 分发")
                                 << QString::fromUtf8("成员验签接收"),
                   "#2563eb");
    updateSessionHashChain("send_group", groupName, plainText);
    ui->msgInput->clear();
    ui->msgInput->setFocus();
}

bool MainWindow::isHermesProxyContact(const QString& targetId) const
{
    return hermesProxyContacts.contains(targetId);
}

QString MainWindow::hermesProxyIdForOwner(const QString& ownerIp) const
{
    return QStringLiteral("hermes-proxy://") + ownerIp;
}

QString MainWindow::targetDisplayName(const QString& targetId) const
{
    if (targetId == HermesBridge::peerId())
        return HermesBridge::displayName();
    if (groupNames.contains(targetId))
        return groupNames.value(targetId);
    if (hermesProxyContacts.contains(targetId))
        return hermesProxyContacts.value(targetId).value(QStringLiteral("displayName")).toString();
    return infoMgr.getNickByIp(targetId);
}

QString MainWindow::newNonceHex(int byteCount) const
{
    std::vector<unsigned char> bytes(qMax(8, byteCount));
    if (!Utils::randomBytes(bytes))
        return QString::number(QDateTime::currentMSecsSinceEpoch(), 16);
    return Utils::bytesToHex(bytes);
}

QString MainWindow::selfFingerprint() const
{
    return Utils::shortFingerprint(infoMgr.getSelfPublicKey());
}

QString MainWindow::pubFingerprintForIp(const QString& ip) const
{
    return Utils::shortFingerprint(infoMgr.getPubKeyByIp(ip));
}

bool MainWindow::sendEncryptedControlMessage(const QString& targetIp, const QJsonObject& payload)
{
    return msgHandler.sendEncryptedMessage(QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)),
                                           targetIp,
                                           infoMgr,
                                           udp,
                                           logger);
}

QString MainWindow::walletBindingStatement(const QString& subjectNick,
                                           const QString& subjectFingerprint,
                                           const QString& walletAddress,
                                           const QString& requesterFingerprint,
                                           const QString& nonce) const
{
    return QString::fromUtf8(
               "GM Secure Chat Wallet Binding v1\n"
               "network=Sepolia\n"
               "subjectNick=%1\n"
               "subjectSM2Fingerprint=%2\n"
               "walletAddress=%3\n"
               "requesterSM2Fingerprint=%4\n"
               "nonce=%5\n"
               "purpose=prove_lan_user_controls_wallet")
        .arg(subjectNick,
             subjectFingerprint,
             normalizedEthAddress(walletAddress),
             requesterFingerprint,
             nonce);
}

QString MainWindow::verifiedWalletBadgeForIp(const QString& ip) const
{
    QString wallet = normalizedEthAddress(infoMgr.getWalletByIp(ip));
    if (wallet.isEmpty())
        return QString::fromUtf8(" [未广播钱包]");

    QJsonObject proof = verifiedWalletBindings.value(ip);
    bool ok = normalizedEthAddress(proof.value(QStringLiteral("walletAddress")).toString()) == wallet &&
              proof.value(QStringLiteral("subjectFingerprint")).toString() == pubFingerprintForIp(ip) &&
              proof.value(QStringLiteral("walletVerified")).toBool(false) &&
              proof.value(QStringLiteral("sm2Verified")).toBool(false);
    return ok ? QString::fromUtf8(" [钱包已验证]") : QString::fromUtf8(" [钱包待验证]");
}

void MainWindow::openDappForWalletBinding(const QJsonObject& request)
{
    if (!dappServer.start()) {
        appendNetworkLog(QString::fromUtf8("[MetaMask] 本地 DApp 服务启动失败，请检查端口 5173-5179 是否被占用"));
        return;
    }

    QUrl url(dappServer.baseUrl() + "/index.html");
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("mode"), QStringLiteral("wallet_bind"));
    query.addQueryItem(QStringLiteral("nonce"), request.value(QStringLiteral("nonce")).toString());
    query.addQueryItem(QStringLiteral("wallet"), request.value(QStringLiteral("walletAddress")).toString());
    query.addQueryItem(QStringLiteral("message"), request.value(QStringLiteral("statement")).toString());
    query.addQueryItem(QStringLiteral("requester"), request.value(QStringLiteral("requesterNick")).toString());
    query.addQueryItem(QStringLiteral("subject"), infoMgr.getSelfNick());
    url.setQuery(query);

    appendNetworkLog(QString::fromUtf8("[MetaMask] 已打开钱包归属验证 DApp: ") + url.toString(QUrl::RemoveQuery));
    QDesktopServices::openUrl(url);
}

bool MainWindow::handleWalletBindingControlMessage(const QString& fromIp, const QString& fromNick, const QString& plainText)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(plainText.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    QJsonObject payload = doc.object();
    if (payload.value(QStringLiteral("gmControl")).toString() != QStringLiteral("wallet_binding"))
        return false;

    QString kind = payload.value(QStringLiteral("kind")).toString();
    if (kind == QStringLiteral("bind_request")) {
        handleWalletBindRequest(fromIp, fromNick, payload);
    } else if (kind == QStringLiteral("bind_proof")) {
        handleWalletBindProof(fromIp, fromNick, payload);
    } else if (kind == QStringLiteral("bind_reject")) {
        handleWalletBindReject(fromIp, fromNick, payload);
    } else {
        appendNetworkLog(QString::fromUtf8("[钱包验证] 未知控制消息: %1").arg(kind));
    }
    return true;
}

void MainWindow::handleWalletBindRequest(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString nonce = payload.value(QStringLiteral("nonce")).toString();
    QString requesterFp = payload.value(QStringLiteral("requesterFingerprint")).toString();
    QString targetFp = payload.value(QStringLiteral("targetFingerprint")).toString();
    QString wallet = normalizedEthAddress(payload.value(QStringLiteral("walletAddress")).toString());
    QString selfWallet = normalizedEthAddress(infoMgr.getSelfWalletAddress());
    QString statement = payload.value(QStringLiteral("statement")).toString();

    auto rejectWith = [&](const QString& reason) {
        QJsonObject reject;
        reject["gmControl"] = "wallet_binding";
        reject["kind"] = "bind_reject";
        reject["version"] = 1;
        reject["nonce"] = nonce;
        reject["reason"] = reason;
        reject["targetFingerprint"] = selfFingerprint();
        reject["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
        sendEncryptedControlMessage(fromIp, reject);
        appendCryptoLog(QString::fromUtf8("[钱包验证] 已拒绝 %1 的验证请求: %2").arg(fromNick, reason));
    };

    if (nonce.isEmpty() ||
        requesterFp != pubFingerprintForIp(fromIp) ||
        targetFp != selfFingerprint() ||
        wallet.isEmpty() ||
        selfWallet.isEmpty() ||
        wallet != selfWallet) {
        rejectWith(QString::fromUtf8("SM2 指纹或钱包地址不匹配"));
        return;
    }

    QString expectedStatement = walletBindingStatement(infoMgr.getSelfNick(), selfFingerprint(), selfWallet, requesterFp, nonce);
    if (statement != expectedStatement) {
        rejectWith(QString::fromUtf8("绑定声明内容不一致"));
        return;
    }

    QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        QString::fromUtf8("钱包归属验证"),
        QString::fromUtf8("%1 请求验证：当前局域网身份是否控制钱包 %2。\n\n同意后会打开本地 DApp，请在 MetaMask 中对绑定声明签名。")
            .arg(fromNick, selfWallet),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    if (choice != QMessageBox::Yes) {
        rejectWith(QString::fromUtf8("用户拒绝打开 MetaMask 签名"));
        return;
    }

    std::vector<unsigned char> bindingHash = SM3::hash(qbytesToVector(statement.toUtf8()));
    std::vector<unsigned char> sm2Sig = SM2::sign(bindingHash, infoMgr.getSelfPrivateKey());
    bool sm2SelfOk = SM2::verify(bindingHash, sm2Sig, infoMgr.getSelfPublicKey());
    if (!sm2SelfOk) {
        rejectWith(QString::fromUtf8("本机 SM2 绑定声明签名失败"));
        return;
    }

    QJsonObject approval;
    approval["nonce"] = nonce;
    approval["requesterIp"] = fromIp;
    approval["requesterNick"] = fromNick;
    approval["requesterFingerprint"] = requesterFp;
    approval["targetFingerprint"] = selfFingerprint();
    approval["walletAddress"] = selfWallet;
    approval["statement"] = statement;
    approval["bindingSM3"] = Utils::bytesToHex(bindingHash);
    approval["communicationSM2Signature"] = Utils::bytesToBase64(sm2Sig);
    pendingWalletBindApprovals[nonce] = approval;

    appendChatEvent(fromIp,
                    QString::fromUtf8("钱包验证请求"),
                    QString::fromUtf8("已确认 SM2 指纹与钱包地址，等待 MetaMask 签名。"));
    appendCryptoLog(QString::fromUtf8("========== 对方钱包归属验证 / B端签名准备 =========="));
    appendCryptoLog(QString::fromUtf8("[1/4] 请求方: %1 | SM2指纹=%2").arg(fromNick, requesterFp));
    appendCryptoLog(QString::fromUtf8("[2/4] 本机钱包: %1").arg(selfWallet));
    appendCryptoLog(QString::fromUtf8("[3/4] 绑定声明 SM3: %1").arg(Utils::bytesToHex(bindingHash)));
    appendCryptoLog(QString::fromUtf8("[4/4] 本机通信 SM2 签名: 通过，等待 MetaMask personal_sign"));
    openDappForWalletBinding(approval);
}

void MainWindow::handleWalletBindSignatureCallback(const QJsonObject& payload)
{
    QString nonce = payload.value(QStringLiteral("nonce")).toString();
    QJsonObject pending = pendingWalletBindApprovals.take(nonce);
    if (pending.isEmpty()) {
        appendCryptoLog(QString::fromUtf8("[钱包验证] 收到未知 nonce 的 MetaMask 回调: %1").arg(nonce));
        return;
    }

    QString wallet = normalizedEthAddress(payload.value(QStringLiteral("wallet")).toString());
    QString account = normalizedEthAddress(payload.value(QStringLiteral("account")).toString());
    QString signature = payload.value(QStringLiteral("walletSignature")).toString();
    QString callbackStatement = payload.value(QStringLiteral("message")).toString();
    QString providerRecovered = normalizedEthAddress(payload.value(QStringLiteral("walletRecoveredAddress")).toString());
    QString expectedWallet = normalizedEthAddress(pending.value(QStringLiteral("walletAddress")).toString());
    QString pendingStatement = pending.value(QStringLiteral("statement")).toString();
    QString formDecodedStatement = callbackStatement;
    formDecodedStatement.replace(QChar('+'), QChar(' '));
    bool statementOk = normalizedBindingStatement(callbackStatement) == normalizedBindingStatement(pendingStatement) ||
                       normalizedBindingStatement(formDecodedStatement) == normalizedBindingStatement(pendingStatement);
    QString statement = pendingStatement;

    QString recoverError;
    QString recovered = recoverEthereumPersonalSignAddress(statement, signature, &recoverError);
    QString effectiveRecovered = (recovered == expectedWallet) ? recovered : providerRecovered;
    bool walletSignatureOk = effectiveRecovered == expectedWallet;
    if (wallet != expectedWallet || account != expectedWallet || !walletSignatureOk) {
        QString reason = QString::fromUtf8("MetaMask 签名回调校验失败 wallet=%1 account=%2 recovered=%3 providerRecovered=%4 statementOk=%5 error=%6")
                             .arg(wallet,
                                  account,
                                  recovered,
                                  providerRecovered,
                                  statementOk ? QStringLiteral("true") : QStringLiteral("false"),
                                  recoverError);
        appendCryptoLog(QString::fromUtf8("[钱包验证] ") + reason);
        QMessageBox::warning(this, QString::fromUtf8("钱包验证失败"), reason);
        QJsonObject reject;
        reject["gmControl"] = "wallet_binding";
        reject["kind"] = "bind_reject";
        reject["version"] = 1;
        reject["nonce"] = nonce;
        reject["reason"] = reason;
        reject["targetFingerprint"] = selfFingerprint();
        reject["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
        sendEncryptedControlMessage(pending.value(QStringLiteral("requesterIp")).toString(), reject);
        return;
    }

    QJsonObject proof;
    proof["gmControl"] = "wallet_binding";
    proof["kind"] = "bind_proof";
    proof["version"] = 1;
    proof["nonce"] = nonce;
    proof["targetNick"] = infoMgr.getSelfNick();
    proof["targetFingerprint"] = selfFingerprint();
    proof["requesterFingerprint"] = pending.value(QStringLiteral("requesterFingerprint")).toString();
    proof["walletAddress"] = expectedWallet;
    proof["statement"] = statement;
    proof["bindingSM3"] = pending.value(QStringLiteral("bindingSM3")).toString();
    proof["communicationSM2Signature"] = pending.value(QStringLiteral("communicationSM2Signature")).toString();
    proof["walletSignature"] = signature;
    proof["walletRecoveredAddress"] = effectiveRecovered;
    proof["walletRecoveredByOpenSsl"] = recovered;
    proof["walletRecoveredByMetaMask"] = providerRecovered;
    proof["callbackStatementMatched"] = statementOk;
    proof["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    QString requesterIp = pending.value(QStringLiteral("requesterIp")).toString();
    QString requesterNick = pending.value(QStringLiteral("requesterNick")).toString();
    if (sendEncryptedControlMessage(requesterIp, proof)) {
        appendChatEvent(requesterIp,
                        QString::fromUtf8("钱包签名已回传"),
                        QString::fromUtf8("MetaMask 已证明钱包 %1 由当前通信身份控制，证明已加密回传。").arg(expectedWallet));
        appendCryptoLog(QString::fromUtf8("[钱包验证] 已向 %1 回传 SM2 + MetaMask 双签名证明 nonce=%2")
                        .arg(requesterNick, nonce.left(12)));
    }
}

void MainWindow::handleWalletBindProof(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString nonce = payload.value(QStringLiteral("nonce")).toString();
    QJsonObject pending = pendingWalletVerifyRequests.take(nonce);
    QString expectedWallet = normalizedEthAddress(pending.value(QStringLiteral("walletAddress")).toString());
    QString expectedTargetFp = pending.value(QStringLiteral("targetFingerprint")).toString();
    QString statement = payload.value(QStringLiteral("statement")).toString();
    QString signature = payload.value(QStringLiteral("walletSignature")).toString();
    QString sm2Signature = payload.value(QStringLiteral("communicationSM2Signature")).toString();
    QString claimedWallet = normalizedEthAddress(payload.value(QStringLiteral("walletAddress")).toString());
    QString claimedTargetFp = payload.value(QStringLiteral("targetFingerprint")).toString();

    bool contextOk = !pending.isEmpty() &&
                     pending.value(QStringLiteral("targetIp")).toString() == fromIp &&
                     expectedWallet == claimedWallet &&
                     expectedTargetFp == claimedTargetFp &&
                     claimedTargetFp == pubFingerprintForIp(fromIp) &&
                     payload.value(QStringLiteral("requesterFingerprint")).toString() == selfFingerprint() &&
                     normalizedBindingStatement(statement) ==
                         normalizedBindingStatement(pending.value(QStringLiteral("statement")).toString());

    std::vector<unsigned char> bindingHash = SM3::hash(qbytesToVector(statement.toUtf8()));
    bool sm2Ok = contextOk && SM2::verify(bindingHash,
                                          Utils::base64ToBytes(sm2Signature),
                                          infoMgr.getPubKeyByIp(fromIp));
    QString recoverError;
    QString recovered = recoverEthereumPersonalSignAddress(statement, signature, &recoverError);
    QString providerRecovered = normalizedEthAddress(payload.value(QStringLiteral("walletRecoveredAddress")).toString());
    QString effectiveRecovered = (recovered == expectedWallet) ? recovered : providerRecovered;
    bool walletOk = contextOk && !effectiveRecovered.isEmpty() && effectiveRecovered == expectedWallet;

    QJsonObject record;
    record["subjectNick"] = fromNick;
    record["subjectIp"] = fromIp;
    record["subjectFingerprint"] = claimedTargetFp;
    record["walletAddress"] = claimedWallet;
    record["requesterNick"] = infoMgr.getSelfNick();
    record["requesterFingerprint"] = selfFingerprint();
    record["statement"] = statement;
    record["bindingSM3"] = Utils::bytesToHex(bindingHash);
    record["communicationSM2Signature"] = sm2Signature;
    record["walletSignature"] = signature;
    record["walletRecoveredAddress"] = effectiveRecovered;
    record["walletRecoveredByOpenSsl"] = recovered;
    record["walletRecoveredByMetaMask"] = providerRecovered;
    record["sm2Verified"] = sm2Ok;
    record["walletVerified"] = walletOk;
    record["contextVerified"] = contextOk;
    record["recoverError"] = recoverError;
    QString recordHash = appendAuditRecord("peer_wallet_identity_binding", record);
    record["recordHash"] = recordHash;

    appendCryptoLog(QString::fromUtf8("========== 对方钱包归属验证 / A端核验 =========="));
    appendCryptoLog(QString::fromUtf8("[1/5] 对方: %1 (%2) | SM2指纹=%3").arg(fromNick, fromIp, claimedTargetFp));
    appendCryptoLog(QString::fromUtf8("[2/5] 声明钱包: %1 | 签名恢复地址: %2").arg(claimedWallet, effectiveRecovered));
    appendCryptoLog(QString::fromUtf8("[3/5] 绑定声明 SM3: %1").arg(Utils::bytesToHex(bindingHash)));
    appendCryptoLog(QString::fromUtf8("[4/5] 通信 SM2 验签: %1 | MetaMask 地址校验: %2")
                    .arg(sm2Ok ? QString::fromUtf8("通过") : QString::fromUtf8("失败"),
                         walletOk ? QString::fromUtf8("通过") : QString::fromUtf8("失败")));
    appendCryptoLog(QString::fromUtf8("[5/5] 本地存证 Record: %1...").arg(recordHash.left(24)));

    if (sm2Ok && walletOk) {
        verifiedWalletBindings[fromIp] = record;
        appendChatEvent(fromIp,
                        QString::fromUtf8("钱包归属验证通过"),
                        QString::fromUtf8("%1 的 SM2 通信身份已证明控制钱包 %2。").arg(fromNick, claimedWallet));
        showBottomFlow(fromIp,
                       QString::fromUtf8("对方钱包验证通过"),
                       QStringList() << QString::fromUtf8("收到证明")
                                     << QString::fromUtf8("SM3 复算")
                                     << QString::fromUtf8("SM2 验签")
                                     << QString::fromUtf8("恢复钱包地址")
                                     << QString::fromUtf8("绑定成立"),
                       "#0f9f6e");
    } else {
        appendChatEvent(fromIp,
                        QString::fromUtf8("钱包归属验证失败"),
                        QString::fromUtf8("SM2=%1，MetaMask=%2，详情见加密日志。")
                            .arg(sm2Ok ? QString::fromUtf8("通过") : QString::fromUtf8("失败"),
                                 walletOk ? QString::fromUtf8("通过") : QString::fromUtf8("失败")));
    }
    updateOnlineList();
}

void MainWindow::handleWalletBindReject(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString nonce = payload.value(QStringLiteral("nonce")).toString();
    pendingWalletVerifyRequests.remove(nonce);
    QString reason = payload.value(QStringLiteral("reason")).toString(QString::fromUtf8("对方拒绝验证"));
    appendChatEvent(fromIp,
                    QString::fromUtf8("钱包归属验证被拒绝"),
                    QString::fromUtf8("%1：%2").arg(fromNick, reason));
    appendCryptoLog(QString::fromUtf8("[钱包验证] %1 拒绝钱包归属验证 nonce=%2 reason=%3")
                    .arg(fromNick, nonce.left(12), reason));
}

bool MainWindow::handleHermesControlMessage(const QString& fromIp, const QString& fromNick, const QString& plainText)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(plainText.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    QJsonObject payload = doc.object();
    if (payload.value(QStringLiteral("gmControl")).toString() != QStringLiteral("hermes_proxy"))
        return false;

    QString kind = payload.value(QStringLiteral("kind")).toString();
    if (kind == QStringLiteral("share_challenge")) {
        handleHermesShareChallenge(fromIp, fromNick, payload);
    } else if (kind == QStringLiteral("share_ack")) {
        handleHermesShareAck(fromIp, fromNick, payload);
    } else if (kind == QStringLiteral("share_ready")) {
        handleHermesShareReady(fromIp, fromNick, payload);
    } else if (kind == QStringLiteral("share_reject")) {
        handleHermesShareReject(fromIp, fromNick, payload);
    } else if (kind == QStringLiteral("proxy_request")) {
        handleHermesProxyRequest(fromIp, fromNick, payload);
    } else if (kind == QStringLiteral("proxy_reply")) {
        handleHermesProxyReply(fromIp, fromNick, payload);
    } else {
        appendNetworkLog(QString::fromUtf8("[HermesProxy] 未知控制消息: %1").arg(kind));
    }
    return true;
}

void MainWindow::handleHermesShareChallenge(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString nonce = payload.value(QStringLiteral("nonce")).toString();
    QString targetFp = payload.value(QStringLiteral("targetFingerprint")).toString();
    QString ownerFp = payload.value(QStringLiteral("ownerFingerprint")).toString();
    QString actualOwnerFp = pubFingerprintForIp(fromIp);

    if (nonce.isEmpty() || targetFp != selfFingerprint() || ownerFp != actualOwnerFp) {
        QJsonObject reject;
        reject["gmControl"] = "hermes_proxy";
        reject["kind"] = "share_reject";
        reject["version"] = 1;
        reject["nonce"] = nonce;
        reject["reason"] = QString::fromUtf8("SM2 指纹不匹配，拒绝 Hermes 共享授权");
        reject["targetFingerprint"] = selfFingerprint();
        reject["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();
        sendEncryptedControlMessage(fromIp, reject);
        appendCryptoLog(QString::fromUtf8("[HermesProxy] 共享挑战被拒绝: 指纹不匹配 owner=%1 target=%2")
                        .arg(ownerFp, targetFp));
        return;
    }

    QString proxyId = hermesProxyIdForOwner(fromIp);
    QJsonObject contact;
    contact["ownerIp"] = fromIp;
    contact["ownerNick"] = fromNick;
    contact["ownerFingerprint"] = ownerFp;
    contact["displayName"] = QString::fromUtf8("%1 的 Hermes Agent").arg(fromNick);
    contact["authorized"] = false;
    hermesProxyContacts[proxyId] = contact;

    QJsonObject ack;
    ack["gmControl"] = "hermes_proxy";
    ack["kind"] = "share_ack";
    ack["version"] = 1;
    ack["nonce"] = nonce;
    ack["targetNick"] = infoMgr.getSelfNick();
    ack["targetFingerprint"] = selfFingerprint();
    ack["ownerFingerprint"] = ownerFp;
    ack["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

    sendEncryptedControlMessage(fromIp, ack);
    appendChatEvent(proxyId,
                    QString::fromUtf8("Hermes 授权认证"),
                    QString::fromUtf8("收到 %1 的共享挑战，SM2 指纹匹配，已签名确认。").arg(fromNick));
    appendCryptoLog(QString::fromUtf8("[HermesProxy] 已用本机 SM2 身份确认 Hermes 共享挑战 nonce=%1").arg(nonce.left(12)));
    updateOnlineList();
}

void MainWindow::handleHermesShareAck(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString nonce = payload.value(QStringLiteral("nonce")).toString();
    QJsonObject pending = pendingHermesShares.take(nonce);
    QString targetFp = payload.value(QStringLiteral("targetFingerprint")).toString();
    QString actualFp = pubFingerprintForIp(fromIp);

    if (pending.isEmpty() ||
        pending.value(QStringLiteral("targetIp")).toString() != fromIp ||
        pending.value(QStringLiteral("targetFingerprint")).toString() != targetFp ||
        targetFp != actualFp) {
        appendCryptoLog(QString::fromUtf8("[HermesProxy] 共享确认被拒绝: nonce 或 SM2 指纹不匹配"));
        return;
    }

    QJsonObject grant;
    grant["targetIp"] = fromIp;
    grant["targetNick"] = fromNick;
    grant["targetFingerprint"] = targetFp;
    grant["nonce"] = nonce;
    grant["authorizedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    activeHermesShares[fromIp] = grant;

    QJsonObject ready;
    ready["gmControl"] = "hermes_proxy";
    ready["kind"] = "share_ready";
    ready["version"] = 1;
    ready["nonce"] = nonce;
    ready["ownerNick"] = infoMgr.getSelfNick();
    ready["ownerFingerprint"] = selfFingerprint();
    ready["targetFingerprint"] = targetFp;
    ready["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();
    sendEncryptedControlMessage(fromIp, ready);

    appendChatEvent(fromIp,
                    QString::fromUtf8("Hermes 共享成功"),
                    QString::fromUtf8("已授权 %1 使用本机 Hermes。认证对象是 SM2 指纹 %2。")
                        .arg(fromNick, targetFp));
    appendCryptoLog(QString::fromUtf8("[HermesProxy] Hermes 共享授权完成: %1 (%2)").arg(fromNick, targetFp));
}

void MainWindow::handleHermesShareReady(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString ownerFp = payload.value(QStringLiteral("ownerFingerprint")).toString();
    if (ownerFp != pubFingerprintForIp(fromIp)) {
        appendCryptoLog(QString::fromUtf8("[HermesProxy] share_ready 指纹不匹配，已忽略"));
        return;
    }

    QString proxyId = hermesProxyIdForOwner(fromIp);
    QJsonObject contact = hermesProxyContacts.value(proxyId);
    contact["ownerIp"] = fromIp;
    contact["ownerNick"] = fromNick;
    contact["ownerFingerprint"] = ownerFp;
    contact["displayName"] = QString::fromUtf8("%1 的 Hermes Agent").arg(fromNick);
    contact["authorized"] = true;
    hermesProxyContacts[proxyId] = contact;
    appendChatEvent(proxyId,
                    QString::fromUtf8("Hermes 已共享"),
                    QString::fromUtf8("已通过 SM2 指纹认证，可向 %1 的本机 Hermes Agent 发送问题。").arg(fromNick));
    updateOnlineList();
}

void MainWindow::handleHermesShareReject(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString nonce = payload.value(QStringLiteral("nonce")).toString();
    QString reason = payload.value(QStringLiteral("reason")).toString();
    pendingHermesShares.remove(nonce);
    appendChatEvent(fromIp,
                    QString::fromUtf8("Hermes 共享被拒绝"),
                    reason.isEmpty() ? QString::fromUtf8("%1 拒绝了共享认证。").arg(fromNick) : reason);
}

void MainWindow::sendHermesProxyQuestion(const QString& proxyId, const QString& plainText)
{
    QJsonObject contact = hermesProxyContacts.value(proxyId);
    QString ownerIp = contact.value(QStringLiteral("ownerIp")).toString();
    QString ownerNick = contact.value(QStringLiteral("ownerNick")).toString();
    QString ownerFp = contact.value(QStringLiteral("ownerFingerprint")).toString();
    if (ownerIp.isEmpty() || ownerFp != pubFingerprintForIp(ownerIp)) {
        QMessageBox::warning(this,
                             QString::fromUtf8("Hermes 代理不可用"),
                             QString::fromUtf8("共享 Hermes 的拥有者不在线，或 SM2 指纹已经变化。"));
        return;
    }

    QString requestId = newNonceHex(16);
    QJsonObject payload;
    payload["gmControl"] = "hermes_proxy";
    payload["kind"] = "proxy_request";
    payload["version"] = 1;
    payload["requestId"] = requestId;
    payload["ownerFingerprint"] = ownerFp;
    payload["requesterNick"] = infoMgr.getSelfNick();
    payload["requesterFingerprint"] = selfFingerprint();
    payload["prompt"] = plainText;
    payload["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

    beginSmTrace(proxyId,
                 targetDisplayName(proxyId),
                 QString::fromUtf8("加密发送 / 共享Hermes请求"),
                 plainText);
    bool sent = sendEncryptedControlMessage(ownerIp, payload);
    endSmTrace(sent ? QString::fromUtf8("共享 Hermes 请求已发送") : QString::fromUtf8("共享 Hermes 请求发送失败"));
    if (sent) {
        appendChatMessage(proxyId, infoMgr.getSelfNick(), plainText, true);
        showBottomFlow(proxyId,
                       QString::fromUtf8("共享 Hermes 请求"),
                       QStringList() << QString::fromUtf8("问题入队")
                                     << QString::fromUtf8("SM3 摘要")
                                     << QString::fromUtf8("SM2 签名")
                                     << QString::fromUtf8("SM4 加密")
                                     << QString::fromUtf8("发往授权主机"),
                       "#7c3aed");
        updateSessionHashChain("send_hermes_proxy", targetDisplayName(proxyId), plainText);
        ui->msgInput->clear();
        ui->msgInput->setFocus();
        appendNetworkLog(QString::fromUtf8("已向 %1 的 Hermes Agent 发送代理请求 request=%2")
                         .arg(ownerNick, requestId.left(12)));
    }
}

void MainWindow::handleHermesProxyRequest(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QJsonObject grant = activeHermesShares.value(fromIp);
    QString requesterFp = payload.value(QStringLiteral("requesterFingerprint")).toString();
    QString actualFp = pubFingerprintForIp(fromIp);
    if (grant.isEmpty() ||
        grant.value(QStringLiteral("targetFingerprint")).toString() != requesterFp ||
        requesterFp != actualFp) {
        appendCryptoLog(QString::fromUtf8("[HermesProxy] 未授权或指纹不匹配，拒绝来自 %1 的 Hermes 请求").arg(fromNick));
        return;
    }

    QString requestId = payload.value(QStringLiteral("requestId")).toString();
    QString prompt = payload.value(QStringLiteral("prompt")).toString();
    if (requestId.isEmpty() || prompt.trimmed().isEmpty())
        return;

    QJsonObject req;
    req["requesterIp"] = fromIp;
    req["requesterNick"] = fromNick;
    req["requesterFingerprint"] = requesterFp;
    req["prompt"] = prompt;
    pendingHermesProxyRequests[requestId] = req;

    appendChatEvent(fromIp,
                    QString::fromUtf8("Hermes 代理请求"),
                    QString::fromUtf8("%1 已通过 SM2 授权校验，正在调用本机 Hermes。").arg(fromNick));
    if (hermesEndpointInput)
        hermesBridge.setEndpoint(hermesEndpointInput->text());
    hermesBridge.requestPlainReply(requestId, prompt, logger);
}

void MainWindow::handleHermesProxyReply(const QString& fromIp, const QString& fromNick, const QJsonObject& payload)
{
    QString proxyId = hermesProxyIdForOwner(fromIp);
    QJsonObject contact = hermesProxyContacts.value(proxyId);
    QString ownerFp = payload.value(QStringLiteral("ownerFingerprint")).toString();
    if (contact.isEmpty() || ownerFp != pubFingerprintForIp(fromIp)) {
        appendCryptoLog(QString::fromUtf8("[HermesProxy] 收到未认证的 Hermes 代理回复，已忽略"));
        return;
    }

    QString reply = payload.value(QStringLiteral("reply")).toString();
    bool ok = payload.value(QStringLiteral("ok")).toBool(true);
    appendChatMessage(proxyId, targetDisplayName(proxyId), reply, false);
    showBottomFlow(proxyId,
                   ok ? QString::fromUtf8("共享 Hermes 回复") : QString::fromUtf8("共享 Hermes 异常"),
                   QStringList() << QString::fromUtf8("收到密文")
                                 << QString::fromUtf8("SM2 解封")
                                 << QString::fromUtf8("SM4 解密")
                                 << QString::fromUtf8("SM3 校验")
                                 << QString::fromUtf8("验签放行"),
                   ok ? "#0f766e" : "#ef4444");
    updateSessionHashChain("receive_hermes_proxy", targetDisplayName(proxyId), reply);
}

void MainWindow::onHermesAttachClicked()
{
    if (!isHermesFeatureEnabled()) {
        QMessageBox::information(this,
                                 QString::fromUtf8("Hermes 未启用"),
                                 QString::fromUtf8("Hermes 是可选 AI Agent 协同能力。当前环境没有启用标记，因此仅保留基础 A/B 国密安全通信功能。"));
        return;
    }
    if (ui->loginBtn->isEnabled()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("需要登录"),
                             QString::fromUtf8("请先输入昵称并登录上线，再把 Hermes Agent 接入聊天列表。"));
        return;
    }
    ensureHermesUser(true);
}

void MainWindow::onHermesShareClicked()
{
    if (!isHermesFeatureEnabled()) {
        QMessageBox::information(this,
                                 QString::fromUtf8("Hermes 未启用"),
                                 QString::fromUtf8("当前环境没有启用本机 Hermes，因此不能作为共享方。"));
        return;
    }
    if (ui->loginBtn->isEnabled()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("需要登录"),
                             QString::fromUtf8("请先登录上线，再把 Hermes 授权共享给指定同学。"));
        return;
    }
    if (currentTargetIp.isEmpty() ||
        currentTargetIp == HermesBridge::peerId() ||
        isHermesProxyContact(currentTargetIp)) {
        QMessageBox::warning(this,
                             QString::fromUtf8("请选择授权对象"),
                             QString::fromUtf8("请先在左侧选择一个真实局域网用户。授权依据是该用户的 SM2 公钥指纹，不是昵称。"));
        return;
    }

    std::vector<unsigned char> targetPub = infoMgr.getPubKeyByIp(currentTargetIp);
    QString targetFp = Utils::shortFingerprint(targetPub);
    if (targetPub.empty() || targetFp.isEmpty()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("缺少 SM2 身份"),
                             QString::fromUtf8("当前用户没有可用 SM2 公钥，不能共享 Hermes。"));
        return;
    }

    if (hermesEndpointInput)
        hermesBridge.setEndpoint(hermesEndpointInput->text());
    if (!hermesBridge.ensureIdentity()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("Hermes 身份失败"),
                             QString::fromUtf8("无法生成本机 Hermes SM2 身份。"));
        return;
    }

    QString detail = QString::fromUtf8("将本机 Hermes 共享给：\n\n昵称：%1\n地址：%2\nSM2 指纹：%3\n\n即使局域网内有同名用户，也只会授权这个指纹。")
                         .arg(currentTargetNick, currentTargetIp, targetFp);
    if (QMessageBox::question(this,
                              QString::fromUtf8("确认共享 Hermes"),
                              detail,
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes) != QMessageBox::Yes) {
        return;
    }

    QString nonce = newNonceHex(16);
    QJsonObject pending;
    pending["targetIp"] = currentTargetIp;
    pending["targetNick"] = currentTargetNick;
    pending["targetFingerprint"] = targetFp;
    pending["createdAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    pendingHermesShares[nonce] = pending;

    QJsonObject payload;
    payload["gmControl"] = "hermes_proxy";
    payload["kind"] = "share_challenge";
    payload["version"] = 1;
    payload["nonce"] = nonce;
    payload["ownerNick"] = infoMgr.getSelfNick();
    payload["ownerFingerprint"] = selfFingerprint();
    payload["targetNick"] = currentTargetNick;
    payload["targetFingerprint"] = targetFp;
    payload["hermesFingerprint"] = hermesBridge.fingerprint();
    payload["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

    if (sendEncryptedControlMessage(currentTargetIp, payload)) {
        appendChatEvent(currentTargetIp,
                        QString::fromUtf8("Hermes 共享认证已发起"),
                        QString::fromUtf8("目标 %1 | SM2 指纹 %2 | nonce %3")
                            .arg(currentTargetNick, targetFp, nonce.left(12)));
        showBottomFlow(currentTargetIp,
                       QString::fromUtf8("Hermes 共享握手"),
                       QStringList() << QString::fromUtf8("选择指定用户")
                                     << QString::fromUtf8("绑定 SM2 指纹")
                                     << QString::fromUtf8("发送随机挑战")
                                     << QString::fromUtf8("等待对方签名确认"),
                       "#0f766e");
    } else {
        pendingHermesShares.remove(nonce);
        QMessageBox::warning(this,
                             QString::fromUtf8("共享失败"),
                             QString::fromUtf8("共享认证挑战没有通过加密通道发出。"));
    }
}

void MainWindow::onCreateGroupClicked()
{
    QList<QListWidgetItem*> selected = ui->userList->selectedItems();
    QStringList members;
    QStringList labels;
    for (QListWidgetItem* item : selected) {
        QString id = item->data(Qt::UserRole).toString();
        if (id.isEmpty() ||
            id == HermesBridge::peerId() ||
            isHermesProxyContact(id) ||
            isGroupContact(id) ||
            infoMgr.getPubKeyByIp(id).empty()) {
            continue;
        }
        if (!members.contains(id)) {
            members << id;
            labels << QString::fromUtf8("%1(%2)").arg(infoMgr.getNickByIp(id), pubFingerprintForIp(id));
        }
    }

    if (members.size() < 2) {
        QMessageBox::information(this,
                                 QString::fromUtf8("创建群组"),
                                 QString::fromUtf8("请在在线用户列表里至少多选两个真实局域网用户。可按住 Ctrl 或 Shift 多选。"));
        return;
    }

    bool ok = false;
    QString defaultName = QString::fromUtf8("%1 的国密群组").arg(infoMgr.getSelfNick());
    QString groupName = QInputDialog::getText(this,
                                              QString::fromUtf8("创建群组"),
                                              QString::fromUtf8("群名称："),
                                              QLineEdit::Normal,
                                              defaultName,
                                              &ok).trimmed();
    if (!ok || groupName.isEmpty())
        return;

    QString groupId = QStringLiteral("group://") + newNonceHex(12);
    groupNames[groupId] = groupName;
    groupMembers[groupId] = members;
    updateOnlineList();

    currentTargetIp = groupId;
    currentTargetNick = groupName;
    ui->chatGroup->setTitle(QString::fromUtf8("安全群聊 — %1").arg(groupName));
    renderConversation(groupId);
    appendChatEvent(groupId,
                    QString::fromUtf8("群组已创建"),
                    QString::fromUtf8("成员：%1。后续群消息会对每个成员分别执行 SM3摘要、SM2签名、SM4加密和SM2密钥封装。")
                        .arg(labels.join(QString::fromUtf8("，"))));
    appendNetworkLog(QString::fromUtf8("创建群组: %1 | 成员数 %2").arg(groupName).arg(members.size()));
}

void MainWindow::onHermesStatusMessage(const QString& message, bool isError)
{
    if (isError) {
        appendNetworkLog(message);
        appendChatEvent(HermesBridge::peerId(),
                        QString::fromUtf8("Hermes 连接提示"),
                        message);
    } else {
        appendNetworkLog(message);
    }
}

QString MainWindow::sm3Hex(const QByteArray& data) const
{
    return Utils::bytesToHex(SM3::hash(qbytesToVector(data)));
}

bool MainWindow::isValidWalletAddress(const QString& address) const
{
    static const QRegularExpression re("^0x[0-9a-fA-F]{40}$");
    return re.match(address.trimmed()).hasMatch();
}

QString MainWindow::computeMerkleRoot(const QByteArray& data, int chunkSize) const
{
    QVector<std::vector<unsigned char>> level;
    int safeChunk = qMax(1, chunkSize);
    if (data.isEmpty()) {
        level.append(SM3::hash({}));
    } else {
        for (int offset = 0; offset < data.size(); offset += safeChunk) {
            level.append(SM3::hash(qbytesToVector(data.mid(offset, safeChunk))));
        }
    }

    while (level.size() > 1) {
        QVector<std::vector<unsigned char>> next;
        for (int i = 0; i < level.size(); i += 2) {
            std::vector<unsigned char> combined = level[i];
            const std::vector<unsigned char>& right = (i + 1 < level.size()) ? level[i + 1] : level[i];
            combined.insert(combined.end(), right.begin(), right.end());
            next.append(SM3::hash(combined));
        }
        level = next;
    }

    return Utils::bytesToHex(level.first());
}

QString MainWindow::appendAuditRecord(const QString& type, const QJsonObject& payload, bool openDapp)
{
    QJsonObject record;
    record["type"] = type;
    record["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    record["prev"] = auditLedgerHead;
    record["payload"] = payload;

    QByteArray body = QJsonDocument(record).toJson(QJsonDocument::Compact);
    QString recordHash = sm3Hex(auditLedgerHead.toUtf8() + body);
    record["recordHash"] = recordHash;
    auditLedgerHead = recordHash;

    QString path = QCoreApplication::applicationDirPath() + "/gm_audit_ledger.jsonl";
    QFile ledger(path);
    if (ledger.open(QIODevice::Append | QIODevice::Text)) {
        ledger.write(QJsonDocument(record).toJson(QJsonDocument::Compact));
        ledger.write("\n");
        ledger.close();
    }

    if (securityDashboard)
        securityDashboard->recordAuditEvent();

    if (openDapp)
        openDappForRecord(record);

    return recordHash;
}

void MainWindow::openDappForRecord(const QJsonObject& record)
{
    if (!dappServer.start()) {
        appendNetworkLog("[MetaMask] 本地 DApp 服务启动失败，请检查端口 5173-5179 是否被占用");
        return;
    }

    QUrl url(dappServer.baseUrl() + "/index.html");
    QUrlQuery query;
    query.addQueryItem("auto", "1");
    query.addQueryItem("record", QString::fromUtf8(QJsonDocument(record).toJson(QJsonDocument::Compact)));
    QString wallet = infoMgr.getSelfWalletAddress();
    if (wallet.isEmpty() && walletInput)
        wallet = walletInput->text().trimmed();
    if (!wallet.isEmpty())
        query.addQueryItem("wallet", wallet);
    url.setQuery(query);

    appendNetworkLog("[MetaMask] 已自动打开本地DApp: " + url.toString(QUrl::RemoveQuery));
    QDesktopServices::openUrl(url);
}

void MainWindow::openDappForTransfer(const QJsonObject& transfer)
{
    if (!dappServer.start()) {
        appendNetworkLog(QString::fromUtf8("[MetaMask] 本地 DApp 服务启动失败，请检查端口 5173-5179 是否被占用"));
        return;
    }

    QUrl url(dappServer.baseUrl() + "/index.html");
    QUrlQuery query;
    query.addQueryItem("mode", "transfer");
    query.addQueryItem("transfer", QString::fromUtf8(QJsonDocument(transfer).toJson(QJsonDocument::Compact)));
    query.addQueryItem("wallet", transfer["fromWallet"].toString());
    url.setQuery(query);

    appendNetworkLog(QString::fromUtf8("[MetaMask] 已打开局域网转账 DApp: ") + url.toString(QUrl::RemoveQuery));
    QDesktopServices::openUrl(url);
}

void MainWindow::recordFileMerkleEvidence(const QString& fileName, const QByteArray& fileData, const QString& direction)
{
    QString safeFileName = QFileInfo(fileName).fileName();
    if (safeFileName.isEmpty())
        safeFileName = "unnamed";

    lastFileName = safeFileName;
    lastFileData = fileData;
    lastFileHash = sm3Hex(fileData);
    lastMerkleRoot = computeMerkleRoot(fileData);

    QJsonObject payload;
    payload["direction"] = direction;
    payload["fileName"] = safeFileName;
    payload["fileSize"] = QString::number(fileData.size());
    payload["fileHashSM3"] = lastFileHash;
    payload["merkleRootSM3"] = lastMerkleRoot;
    payload["chunkSize"] = 1024;
    payload["ownerNick"] = infoMgr.getSelfNick();
    payload["ownerFingerprint"] = Utils::shortFingerprint(infoMgr.getSelfPublicKey());

    QString recordHash = appendAuditRecord("file_merkle_evidence", payload);
    appendCryptoLog(QString("[Merkle存证] %1 | SM3=%2... | Root=%3... | Record=%4...")
                    .arg(safeFileName,
                         lastFileHash.left(16),
                         lastMerkleRoot.left(16),
                         recordHash.left(16)));
}

void MainWindow::updateSessionHashChain(const QString& direction, const QString& peerNick, const QString& plainText)
{
    if (sessionChainHead.isEmpty())
        sessionChainHead = sm3Hex("GMSECURECHAT_SESSION_V1");

    QByteArray msgDigest = sm3Hex(plainText.toUtf8()).toLatin1();
    QByteArray material;
    material.append(sessionChainHead.toUtf8());
    material.append("|");
    material.append(direction.toUtf8());
    material.append("|");
    material.append(peerNick.toUtf8());
    material.append("|");
    material.append(msgDigest);
    material.append("|");
    material.append(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toUtf8());

    sessionChainHead = sm3Hex(material);

    QJsonObject payload;
    payload["direction"] = direction;
    payload["peer"] = peerNick;
    payload["messageSM3"] = QString::fromLatin1(msgDigest);
    payload["chainHead"] = sessionChainHead;
    appendAuditRecord("session_hash_chain", payload);
    appendCryptoLog("[会话哈希链] 当前链头: " + sessionChainHead.left(32) + "...");
}

bool MainWindow::createDoubleSignatureBinding(bool manualTrigger)
{
    if (infoMgr.getSelfPrivateKey().empty() || infoMgr.getSelfPublicKey().empty()) {
        if (manualTrigger)
            QMessageBox::warning(this, "需要登录", "请先输入昵称并登录上线，再生成双签名身份绑定");
        return false;
    }

    if (chainPrivateKey.empty() || chainPublicKey.empty()) {
        if (!SM2::generateKeyPair(chainPrivateKey, chainPublicKey)) {
            appendCryptoLog("[双签名绑定] 模拟链上身份密钥生成失败");
            return false;
        }
    }

    chainFingerprint = Utils::shortFingerprint(chainPublicKey);
    chainAddress = "0x" + Utils::bytesToHex(SM3::hash(chainPublicKey)).left(40);
    QString commFingerprint = Utils::shortFingerprint(infoMgr.getSelfPublicKey());
    QString bindingText = QString("GM-BIND|%1|%2|%3|%4")
                              .arg(infoMgr.getSelfNick(), commFingerprint, chainAddress, chainFingerprint);
    std::vector<unsigned char> bindingHash = SM3::hash(qbytesToVector(bindingText.toUtf8()));
    std::vector<unsigned char> commSig = SM2::sign(bindingHash, infoMgr.getSelfPrivateKey());
    std::vector<unsigned char> chainSig = SM2::sign(bindingHash, chainPrivateKey);

    bool commOk = SM2::verify(bindingHash, commSig, infoMgr.getSelfPublicKey());
    bool chainOk = SM2::verify(bindingHash, chainSig, chainPublicKey);

    QJsonObject payload;
    payload["nick"] = infoMgr.getSelfNick();
    payload["communicationFingerprint"] = commFingerprint;
    payload["chainAddress"] = chainAddress;
    payload["chainFingerprint"] = chainFingerprint;
    payload["bindingSM3"] = Utils::bytesToHex(bindingHash);
    payload["communicationSM2Signature"] = Utils::bytesToBase64(commSig);
    payload["chainSM2Signature"] = Utils::bytesToBase64(chainSig);
    payload["verifyCommunicationSignature"] = commOk;
    payload["verifyChainSignature"] = chainOk;
    QString recordHash = appendAuditRecord("double_signature_identity_binding", payload, manualTrigger);

    appendCryptoLog("========== 双签名身份绑定 ==========");
    appendCryptoLog("[1/4] 通信身份 SM2 指纹: " + commFingerprint);
    appendCryptoLog("[2/4] 模拟链上地址: " + chainAddress + " | 链上身份指纹: " + chainFingerprint);
    appendCryptoLog("[3/4] 通信身份签名: " + QString(commOk ? "通过" : "失败") +
                    " | 链上身份签名: " + QString(chainOk ? "通过" : "失败"));
    appendCryptoLog("[4/4] 身份绑定存证 Record: " + recordHash.left(32) + "...");
    return commOk && chainOk;
}

void MainWindow::onLoginClicked()
{
    QString nick = ui->nickInput->text().trimmed();
    if (nick.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入昵称");
        return;
    }

    // Check if manual IP is provided as fallback for discovery
    QString manualIp = ui->manualIpInput->text().trimmed();
    QString walletAddress = walletInput ? walletInput->text().trimmed() : QString();
    if (!walletAddress.isEmpty() && !isValidWalletAddress(walletAddress)) {
        QMessageBox::warning(this,
                             QString::fromUtf8("钱包地址格式不正确"),
                             QString::fromUtf8("Sepolia 钱包地址应为 0x 开头的 40 字节十六进制地址。"));
        return;
    }

    infoMgr.setSelfNick(nick);
    infoMgr.setSelfWalletAddress(walletAddress);
    conversationLogFiles.clear();
    pendingWalletVerifyRequests.clear();
    pendingWalletBindApprovals.clear();
    verifiedWalletBindings.clear();
    activeTraceFile.clear();
    logger.clearDetailedLogFile();
    if (!infoMgr.generateSelfKeys()) {
        QMessageBox::critical(this, "错误", "生成密钥对失败");
        return;
    }
    chainPrivateKey.clear();
    chainPublicKey.clear();
    SM2::generateKeyPair(chainPrivateKey, chainPublicKey);
    QByteArray sessionSeed = "GMSECURECHAT_SESSION_V1|";
    sessionSeed.append(nick.toUtf8());
    sessionChainHead = sm3Hex(sessionSeed);

    if (!udp.start(8888)) {
        QMessageBox::critical(this, "错误", "UDP 端口 8888 绑定失败，请检查是否已有实例占用该端口");
        appendNetworkLog("UDP 端口 8888 绑定失败");
        return;
    }
    udp.sendLoginBroadcast(nick, infoMgr.getSelfPublicKeyHex(), infoMgr.getSelfWalletAddress());

    // Start heartbeat: re-broadcast every 15 seconds
    heartbeatTimer.start(15000);

    appendCryptoLog("算法链路: SM2 身份/密钥封装 | SM3 摘要/指纹 | SM4-CBC 消息 | ZUC 文件流加密");
    appendCryptoLog("SM2 密钥对已生成 | 公钥指纹: " +
                    Utils::shortFingerprint(infoMgr.getSelfPublicKey()));
    createDoubleSignatureBinding(false);
    if (isHermesFeatureEnabled())
        ensureHermesUser(false);
    appendNetworkLog("已发送登录广播 -> 端口 8888");

    // Show local IPs
    QStringList ips = udp.getLocalIPs();
    appendNetworkLog("本机 IP: " + (ips.isEmpty() ? "无" : ips.join(", ")));

    // If manual IP provided, send unicast login directly to that IP
    if (!manualIp.isEmpty()) {
        udp.sendLoginResponse(manualIp, nick, infoMgr.getSelfPublicKeyHex(), infoMgr.getSelfWalletAddress());
        appendNetworkLog("已向手动指定 IP 发送单播: " + manualIp);
    }

    ui->loginBtn->setEnabled(false);
    ui->nickInput->setEnabled(false);
    ui->manualIpInput->setEnabled(false);
    if (walletInput)
        walletInput->setEnabled(false);
    if (currentTargetIp.isEmpty())
        ui->chatDisplay->clear();
    updateStatusBar();
}

void MainWindow::onHeartbeat()
{
    if (ui->loginBtn->isEnabled() == false) {
        udp.sendLoginBroadcast(infoMgr.getSelfNick(), infoMgr.getSelfPublicKeyHex(), infoMgr.getSelfWalletAddress());
    }
}

void MainWindow::onTransferClicked()
{
    if (ui->loginBtn->isEnabled()) {
        QMessageBox::warning(this, QString::fromUtf8("需要登录"), QString::fromUtf8("请先填写昵称和钱包地址并登录上线。"));
        return;
    }
    if (currentTargetIp.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("请选择用户"), QString::fromUtf8("请先在左侧在线用户列表选择收款方。"));
        return;
    }
    if (currentTargetIp == HermesBridge::peerId() || isHermesProxyContact(currentTargetIp) || isGroupContact(currentTargetIp)) {
        QMessageBox::information(this,
                                 QString::fromUtf8("当前会话不支持转账"),
                                 QString::fromUtf8("Hermes/群组不是单一 Sepolia 收款方；转账功能请面向单个局域网同学使用。"));
        return;
    }

    QString fromWallet = infoMgr.getSelfWalletAddress().trimmed();
    if (!isValidWalletAddress(fromWallet)) {
        QMessageBox::warning(this,
                             QString::fromUtf8("缺少本机钱包"),
                             QString::fromUtf8("请在登录前填写自己的 MetaMask Sepolia 钱包地址，然后重新登录。"));
        return;
    }

    QString toWallet = infoMgr.getWalletByIp(currentTargetIp).trimmed();
    if (!isValidWalletAddress(toWallet)) {
        bool ok = false;
        toWallet = QInputDialog::getText(this,
                                         QString::fromUtf8("输入收款钱包"),
                                         QString::fromUtf8("对方没有广播有效钱包地址，请手动输入 %1 的 Sepolia 地址：").arg(currentTargetNick),
                                         QLineEdit::Normal,
                                         toWallet,
                                         &ok).trimmed();
        if (!ok)
            return;
        if (!isValidWalletAddress(toWallet)) {
            QMessageBox::warning(this, QString::fromUtf8("收款地址无效"), QString::fromUtf8("收款地址应为 0x 开头的 40 字节十六进制地址。"));
            return;
        }
    }

    QJsonObject walletProof = verifiedWalletBindings.value(currentTargetIp);
    bool peerWalletVerified =
        normalizedEthAddress(walletProof.value(QStringLiteral("walletAddress")).toString()) == normalizedEthAddress(toWallet) &&
        walletProof.value(QStringLiteral("subjectFingerprint")).toString() == pubFingerprintForIp(currentTargetIp) &&
        walletProof.value(QStringLiteral("sm2Verified")).toBool(false) &&
        walletProof.value(QStringLiteral("walletVerified")).toBool(false);
    if (!peerWalletVerified) {
        QMessageBox::StandardButton choice = QMessageBox::question(
            this,
            QString::fromUtf8("对方钱包未验证"),
            QString::fromUtf8("当前尚未确认“%1 的 SM2 通信身份”是否控制钱包 %2。\n\n建议先点击“验证对方钱包”。仍要继续未验证转账吗？")
                .arg(currentTargetNick, toWallet),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (choice != QMessageBox::Yes)
            return;
    }

    bool amountOk = false;
    QString amountEth = QInputDialog::getText(this,
                                             QString::fromUtf8("Sepolia 测试币转账"),
                                             QString::fromUtf8("向 %1 转账金额（单位 SepoliaETH）：").arg(currentTargetNick),
                                             QLineEdit::Normal,
                                             "0.001",
                                             &amountOk).trimmed();
    if (!amountOk)
        return;
    static const QRegularExpression amountRe("^(0|[1-9][0-9]*)(\\.[0-9]{1,18})?$");
    if (!amountRe.match(amountEth).hasMatch()) {
        QMessageBox::warning(this, QString::fromUtf8("金额格式不正确"), QString::fromUtf8("请输入十进制金额，最多 18 位小数，例如 0.001。"));
        return;
    }

    QString senderFingerprint = Utils::shortFingerprint(infoMgr.getSelfPublicKey());
    QString recipientFingerprint = Utils::shortFingerprint(infoMgr.getPubKeyByIp(currentTargetIp));
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    QJsonObject transfer;
    transfer["fromNick"] = infoMgr.getSelfNick();
    transfer["toNick"] = currentTargetNick;
    transfer["fromWallet"] = fromWallet;
    transfer["toWallet"] = toWallet;
    transfer["amountEth"] = amountEth;
    transfer["recipientIp"] = currentTargetIp;
    transfer["senderFingerprint"] = senderFingerprint;
    transfer["recipientFingerprint"] = recipientFingerprint;
    transfer["sessionHead"] = sessionChainHead;
    transfer["timestamp"] = timestamp;
    transfer["network"] = "Sepolia";
    transfer["algorithm"] = "SM2 signature + SM3 transfer digest";

    QByteArray canonical = QJsonDocument(transfer).toJson(QJsonDocument::Compact);
    QString transferSM3 = sm3Hex(canonical);
    std::vector<unsigned char> transferHash = SM3::hash(qbytesToVector(canonical));
    std::vector<unsigned char> signature = SM2::sign(transferHash, infoMgr.getSelfPrivateKey());
    bool verifyOk = SM2::verify(transferHash, signature, infoMgr.getSelfPublicKey());

    transfer["transferSM3"] = transferSM3;
    transfer["senderSM2Signature"] = Utils::bytesToBase64(signature);
    transfer["verifySenderSM2Signature"] = verifyOk;

    QString recordHash = appendAuditRecord("lan_wallet_transfer_prepare", transfer);
    transfer["localRecordHash"] = recordHash;

    appendChatEvent(currentTargetIp,
                    QString::fromUtf8("Sepolia 转账待确认"),
                    QString::fromUtf8("收款方 %1，金额 %2 SepoliaETH，转账意图 SM3 %3...，等待 MetaMask 确认。")
                        .arg(currentTargetNick, amountEth, transferSM3.left(16)));
    showBottomFlow(currentTargetIp,
                   QString::fromUtf8("转账安全准备"),
                   QStringList() << QString::fromUtf8("身份绑定")
                                 << QString::fromUtf8("SM3 意图摘要")
                                 << QString::fromUtf8("SM2 签名")
                                 << QString::fromUtf8("打开 DApp")
                                 << QString::fromUtf8("等待 MetaMask"),
                   "#0f766e");

    appendCryptoLog(QString::fromUtf8("========== 国密身份 Sepolia 转账预校验 =========="));
    appendCryptoLog(QString::fromUtf8("[1/4] 收款方: %1 | %2").arg(currentTargetNick, toWallet));
    appendCryptoLog(QString::fromUtf8("[2/4] 金额: %1 SepoliaETH | 会话链头: %2...")
                    .arg(amountEth, sessionChainHead.left(16)));
    appendCryptoLog(QString::fromUtf8("[3/4] 转账意图 SM3: %1... | SM2签名: %2")
                    .arg(transferSM3.left(24), verifyOk ? QString::fromUtf8("通过") : QString::fromUtf8("失败")));
    appendCryptoLog(QString::fromUtf8("[4/4] 本地预存证 Record: %1...，等待 MetaMask 确认真实转账")
                    .arg(recordHash.left(24)));

    openDappForTransfer(transfer);
}

void MainWindow::onBindIdentityClicked()
{
    createDoubleSignatureBinding(true);
}

void MainWindow::onVerifyPeerWalletClicked()
{
    if (ui->loginBtn->isEnabled()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("需要登录"),
                             QString::fromUtf8("请先填写昵称和自己的钱包地址并登录上线。"));
        return;
    }
    if (currentTargetIp.isEmpty() ||
        currentTargetIp == HermesBridge::peerId() ||
        isHermesProxyContact(currentTargetIp) ||
        isGroupContact(currentTargetIp)) {
        QMessageBox::warning(this,
                             QString::fromUtf8("请选择真实用户"),
                             QString::fromUtf8("请先在左侧在线用户列表选择一个真实局域网用户。"));
        return;
    }

    QString wallet = normalizedEthAddress(infoMgr.getWalletByIp(currentTargetIp));
    if (wallet.isEmpty()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("对方未广播钱包"),
                             QString::fromUtf8("对方需要在登录前填写自己的 MetaMask Sepolia 钱包地址，并重新上线。"));
        return;
    }

    QString targetFp = pubFingerprintForIp(currentTargetIp);
    if (targetFp.isEmpty()) {
        QMessageBox::warning(this,
                             QString::fromUtf8("缺少 SM2 身份"),
                             QString::fromUtf8("当前用户没有有效 SM2 公钥，无法验证钱包归属。"));
        return;
    }

    QString nonce = newNonceHex(16);
    QString statement = walletBindingStatement(currentTargetNick, targetFp, wallet, selfFingerprint(), nonce);
    QJsonObject payload;
    payload["gmControl"] = "wallet_binding";
    payload["kind"] = "bind_request";
    payload["version"] = 1;
    payload["nonce"] = nonce;
    payload["requesterNick"] = infoMgr.getSelfNick();
    payload["requesterFingerprint"] = selfFingerprint();
    payload["targetNick"] = currentTargetNick;
    payload["targetFingerprint"] = targetFp;
    payload["walletAddress"] = wallet;
    payload["statement"] = statement;
    payload["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    QJsonObject pending;
    pending["targetIp"] = currentTargetIp;
    pending["targetNick"] = currentTargetNick;
    pending["targetFingerprint"] = targetFp;
    pending["walletAddress"] = wallet;
    pending["statement"] = statement;
    pending["requesterFingerprint"] = selfFingerprint();
    pendingWalletVerifyRequests[nonce] = pending;

    if (sendEncryptedControlMessage(currentTargetIp, payload)) {
        appendChatEvent(currentTargetIp,
                        QString::fromUtf8("已发起钱包归属验证"),
                        QString::fromUtf8("等待 %1 用通信 SM2 身份和 MetaMask 钱包同时签名。").arg(currentTargetNick));
        showBottomFlow(currentTargetIp,
                       QString::fromUtf8("请求对方钱包验证"),
                       QStringList() << QString::fromUtf8("生成 nonce")
                                     << QString::fromUtf8("绑定 SM2 指纹")
                                     << QString::fromUtf8("绑定钱包地址")
                                     << QString::fromUtf8("加密发送请求")
                                     << QString::fromUtf8("等待双签名"),
                       "#2563eb");
        appendCryptoLog(QString::fromUtf8("========== 发起对方钱包归属验证 =========="));
        appendCryptoLog(QString::fromUtf8("[1/4] 对方: %1 (%2)").arg(currentTargetNick, currentTargetIp));
        appendCryptoLog(QString::fromUtf8("[2/4] 对方 SM2 指纹: %1").arg(targetFp));
        appendCryptoLog(QString::fromUtf8("[3/4] 待验证钱包: %1").arg(wallet));
        appendCryptoLog(QString::fromUtf8("[4/4] 绑定声明 nonce=%1 已通过国密通道发出").arg(nonce.left(16)));
    } else {
        pendingWalletVerifyRequests.remove(nonce);
        QMessageBox::warning(this,
                             QString::fromUtf8("发送失败"),
                             QString::fromUtf8("钱包归属验证请求发送失败，请检查对方是否在线。"));
    }
}

bool MainWindow::sendTamperedMessageForDemo(const QString& plainText)
{
    std::vector<unsigned char> remotePub = infoMgr.getPubKeyByIp(currentTargetIp);
    if (remotePub.empty()) {
        appendCryptoLog(QString::fromUtf8("[篡改攻击] 未找到接收方 SM2 公钥，无法生成真实加密包。"));
        return false;
    }

    beginSmTrace(currentTargetIp,
                 currentTargetNick,
                 QString::fromUtf8("加密发送 / 真实消息篡改攻击"),
                 plainText);

    QByteArray originalPacket = msgHandler.createEncryptedPayload(plainText,
                                                                  infoMgr.getSelfPrivateKey(),
                                                                  remotePub,
                                                                  infoMgr.getSelfNick(),
                                                                  logger);
    if (originalPacket.isEmpty()) {
        endSmTrace(QString::fromUtf8("篡改攻击失败：原始国密包生成失败"));
        return false;
    }

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(originalPacket, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        endSmTrace(QString::fromUtf8("篡改攻击失败：原始国密包不是有效 JSON"));
        return false;
    }

    QJsonObject packet = doc.object();
    std::vector<unsigned char> originalHash = Utils::base64ToBytes(packet.value(QStringLiteral("msgHash")).toString());
    std::vector<unsigned char> tamperedHash = originalHash;
    if (tamperedHash.empty())
        tamperedHash.push_back(0x5A);
    else
        tamperedHash[0] ^= 0x5A;

    packet[QStringLiteral("msgHash")] = Utils::bytesToBase64(tamperedHash);
    packet[QStringLiteral("tamperLab")] = true;
    packet[QStringLiteral("tamperKind")] = QStringLiteral("message_sm3_modified_after_signature");
    packet[QStringLiteral("originalMsgHashHex")] = Utils::bytesToHex(originalHash);
    packet[QStringLiteral("tamperedMsgHashHex")] = Utils::bytesToHex(tamperedHash);
    packet[QStringLiteral("tamperedAt")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    QByteArray tamperedPacket = QJsonDocument(packet).toJson(QJsonDocument::Compact);
    udp.sendEncryptedMessage(currentTargetIp, tamperedPacket);

    appendCryptoLog(QString::fromUtf8("========== 真实链路消息篡改：已发出 =========="));
    appendCryptoLog(QString::fromUtf8("[1/5] 原始明文: \"%1\"").arg(plainText.left(120).replace('\n', "\\n")));
    appendCryptoLog(QString::fromUtf8("[2/5] 原始 SM3: %1").arg(Utils::bytesToHex(originalHash)));
    appendCryptoLog(QString::fromUtf8("[3/5] 攻击者改写 SM3 字段: %1").arg(Utils::bytesToHex(tamperedHash)));
    appendCryptoLog(QString::fromUtf8("[4/5] SM2 签名保持原值，接收方会重算 SM3 并发现摘要不一致。"));
    appendCryptoLog(QString::fromUtf8("[5/5] 篡改 UDP 包已发往 %1 (%2)，预期对方聊天框不显示该消息。")
                    .arg(currentTargetNick, currentTargetIp));

    QJsonObject payload;
    payload[QStringLiteral("attack")] = QStringLiteral("live_message_sm3_tamper");
    payload[QStringLiteral("targetNick")] = currentTargetNick;
    payload[QStringLiteral("targetIp")] = currentTargetIp;
    payload[QStringLiteral("plainPreview")] = plainText.left(160);
    payload[QStringLiteral("originalSM3")] = Utils::bytesToHex(originalHash);
    payload[QStringLiteral("tamperedSM3")] = Utils::bytesToHex(tamperedHash);
    payload[QStringLiteral("packetBytesBefore")] = originalPacket.size();
    payload[QStringLiteral("packetBytesAfter")] = tamperedPacket.size();
    payload[QStringLiteral("sentOverUdp")] = true;
    payload[QStringLiteral("expectedReceiverResult")] = QStringLiteral("SM3 digest mismatch; message rejected before chat display");
    QString recordHash = appendAuditRecord(QStringLiteral("tamper_live_message"), payload, true);
    appendCryptoLog(QString::fromUtf8("[存证] 本次真实篡改攻击记录: %1...，已送入 DApp 展示/可选上链。")
                    .arg(recordHash.left(16)));

    appendChatEvent(currentTargetIp,
                    QString::fromUtf8("篡改攻击包已发出"),
                    QString::fromUtf8("原文“%1”对应的国密包已被改写 SM3 字段。请看对方加密追踪日志：应出现 SM3 摘要不一致并拒绝进入聊天框。").arg(plainText.left(60)));
    showBottomFlow(currentTargetIp,
                   QString::fromUtf8("消息篡改被接收端拦截"),
                   QStringList() << QString::fromUtf8("生成原始密文包")
                                 << QString::fromUtf8("改写 SM3 摘要")
                                 << QString::fromUtf8("UDP 发出")
                                 << QString::fromUtf8("接收端重算 SM3")
                                 << QString::fromUtf8("校验失败拒收"),
                   "#ef4444");
    updateSessionHashChain(QStringLiteral("tamper_send"), currentTargetNick, plainText);
    endSmTrace(QString::fromUtf8("真实篡改包已发送；预期接收端 SM3 校验失败并拦截"));
    return true;
}

void MainWindow::onTamperMessageClicked()
{
    if (infoMgr.getSelfPrivateKey().empty()) {
        QMessageBox::warning(this, QString::fromUtf8("需要登录"),
                             QString::fromUtf8("请先登录，再启动消息篡改攻击演示。"));
        return;
    }
    if (currentTargetIp.isEmpty() ||
        currentTargetIp == HermesBridge::peerId() ||
        isHermesProxyContact(currentTargetIp) ||
        isGroupContact(currentTargetIp)) {
        QMessageBox::warning(this, QString::fromUtf8("请选择单聊用户"),
                             QString::fromUtf8("请先在左侧选择 A/B/C 这种真实局域网用户，再点击消息篡改。"));
        return;
    }

    nextMessageTamperArmed = true;
    nextMessageTamperTargetIp = currentTargetIp;
    appendChatEvent(currentTargetIp,
                    QString::fromUtf8("消息篡改攻击已就绪"),
                    QString::fromUtf8("下一条发给 %1 的文字消息会先完成 SM3/SM2/SM4 加密签名，再在 UDP 发出前被故意改动摘要字段；对方应被 SM3 校验拦截，聊天框不显示该消息。").arg(currentTargetNick));
    showBottomFlow(currentTargetIp,
                   QString::fromUtf8("篡改攻击待触发"),
                   QStringList() << QString::fromUtf8("等待下一条消息")
                                 << QString::fromUtf8("正常国密加密")
                                 << QString::fromUtf8("篡改 SM3 字段")
                                 << QString::fromUtf8("UDP 发出攻击包")
                                 << QString::fromUtf8("接收端拦截"),
                   "#ef4444");
    appendCryptoLog(QString::fromUtf8("========== 真实链路消息篡改：已待命 =========="));
    appendCryptoLog(QString::fromUtf8("[待触发] 目标: %1 (%2)，下一条文字消息将作为篡改攻击包发送。")
                    .arg(currentTargetNick, currentTargetIp));
    return;

    if (infoMgr.getSelfPrivateKey().empty()) {
        QMessageBox::warning(this, "需要登录", "请先登录，再运行篡改攻击实验台");
        return;
    }

    QString sample = "GM tamper lab message";
    std::vector<unsigned char> originalHash = SM3::hash(qbytesToVector(sample.toUtf8()));
    std::vector<unsigned char> signature = SM2::sign(originalHash, infoMgr.getSelfPrivateKey());
    std::vector<unsigned char> tamperedHash = originalHash;
    if (!tamperedHash.empty())
        tamperedHash[0] ^= 0x5A;

    bool originalOk = SM2::verify(originalHash, signature, infoMgr.getSelfPublicKey());
    bool tamperedOk = SM2::verify(tamperedHash, signature, infoMgr.getSelfPublicKey());

    QJsonObject payload;
    payload["attack"] = "message_digest_tamper";
    payload["originalSM3"] = Utils::bytesToHex(originalHash);
    payload["tamperedSM3"] = Utils::bytesToHex(tamperedHash);
    payload["originalVerify"] = originalOk;
    payload["tamperedVerify"] = tamperedOk;
    QString recordHash = appendAuditRecord("tamper_lab_message", payload, true);

    QString logConversation = currentTargetIp.isEmpty() ? QStringLiteral("tamper_lab") : currentTargetIp;
    QString logName = currentTargetIp.isEmpty() ? QString::fromUtf8("篡改攻击实验台") : currentTargetNick;
    beginSmTrace(logConversation, logName, QString::fromUtf8("篡改验证 / 消息摘要"), sample);
    appendCryptoLog("========== 篡改攻击实验台：消息 ==========");
    appendCryptoLog("[1/4] 原始消息 SM3: " + Utils::bytesToHex(originalHash));
    appendCryptoLog("[2/4] 攻击者篡改摘要首字节: " + Utils::bytesToHex(tamperedHash));
    appendCryptoLog("[3/4] 原始验签: " + QString(originalOk ? "通过" : "失败") +
                    " | 篡改后验签: " + QString(tamperedOk ? "通过" : "失败"));
    appendCryptoLog("[4/4] 结论: " + QString(tamperedOk ? "异常，需要检查" : "SM2/SM3 已拦截篡改") +
                    " | Record " + recordHash.left(16) + "...");
    endSmTrace(tamperedOk ? QString::fromUtf8("异常：篡改后仍通过") : QString::fromUtf8("成功：SM2/SM3 拦截篡改"));
}

void MainWindow::onTamperFileClicked()
{
    QByteArray original = lastFileData;
    QString fileName = lastFileName;
    if (original.isEmpty()) {
        original = "GM Secure Chat file tamper lab data";
        fileName = "demo_tamper_sample.bin";
    }

    QString originalHash = sm3Hex(original);
    QString originalRoot = computeMerkleRoot(original);
    QByteArray tampered = original;
    if (tampered.isEmpty())
        tampered.append('x');
    else
        tampered[0] = static_cast<char>(tampered[0] ^ 0x33);
    QString tamperedHash = sm3Hex(tampered);
    QString tamperedRoot = computeMerkleRoot(tampered);

    QJsonObject payload;
    payload["attack"] = "file_byte_tamper";
    payload["fileName"] = fileName;
    payload["originalFileSM3"] = originalHash;
    payload["tamperedFileSM3"] = tamperedHash;
    payload["originalMerkleRoot"] = originalRoot;
    payload["tamperedMerkleRoot"] = tamperedRoot;
    payload["detected"] = (originalHash != tamperedHash && originalRoot != tamperedRoot);
    QString recordHash = appendAuditRecord("tamper_lab_file", payload, true);

    QString logConversation = currentTargetIp.isEmpty() ? QStringLiteral("tamper_lab") : currentTargetIp;
    QString logName = currentTargetIp.isEmpty() ? QString::fromUtf8("篡改攻击实验台") : currentTargetNick;
    beginSmTrace(logConversation, logName, QString::fromUtf8("篡改验证 / 文件Merkle"), fileName);
    appendCryptoLog("========== 篡改攻击实验台：文件 ==========");
    appendCryptoLog("[1/4] 文件样本: " + fileName);
    appendCryptoLog("[2/4] 原始 SM3 / Merkle: " + originalHash.left(16) + "... / " + originalRoot.left(16) + "...");
    appendCryptoLog("[3/4] 篡改 SM3 / Merkle: " + tamperedHash.left(16) + "... / " + tamperedRoot.left(16) + "...");
    appendCryptoLog("[4/4] 结论: " + QString(originalRoot == tamperedRoot ? "未发现异常" : "Merkle 树存证已发现篡改") +
                    " | Record " + recordHash.left(16) + "...");
    endSmTrace(originalRoot == tamperedRoot ? QString::fromUtf8("异常：Merkle 未发现差异") : QString::fromUtf8("成功：Merkle/SM3 发现篡改"));
}

void MainWindow::onVerifyLedgerClicked()
{
    QString path = QCoreApplication::applicationDirPath() + "/gm_audit_ledger.jsonl";
    QFile ledger(path);
    if (!ledger.open(QIODevice::ReadOnly | QIODevice::Text)) {
        appendNetworkLog("暂无本地存证文件: " + path);
        return;
    }

    QString runningHead;
    int count = 0;
    bool ok = true;
    while (!ledger.atEnd()) {
        QByteArray line = ledger.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            ok = false;
            break;
        }

        QJsonObject record = doc.object();
        QString expectedHash = record["recordHash"].toString();
        QString prev = record["prev"].toString();
        record.remove("recordHash");
        QString actualHash = sm3Hex(runningHead.toUtf8() + QJsonDocument(record).toJson(QJsonDocument::Compact));
        if (prev != runningHead || expectedHash != actualHash) {
            ok = false;
            break;
        }
        runningHead = expectedHash;
        count++;
    }
    ledger.close();

    auditLedgerHead = runningHead;
    QJsonObject payload;
    payload["recordCount"] = count;
    payload["ledgerHead"] = runningHead;
    payload["verifyOk"] = ok;
    payload["ledgerPath"] = path;
    QString recordHash = appendAuditRecord("ledger_verification", payload, true);

    appendCryptoLog("========== 存证链核验 ==========");
    appendCryptoLog("[1/2] 记录数: " + QString::number(count) + " | 文件: " + path);
    appendCryptoLog("[2/2] 链式 SM3 校验: " + QString(ok ? "通过" : "失败") +
                    " | Head " + runningHead.left(32) + "... | Record " + recordHash.left(16) + "...");
    if (securityDashboard)
        securityDashboard->recordAuditEvent();
}

void MainWindow::onUserSelected(QListWidgetItem *item)
{
    currentTargetIp = item->data(Qt::UserRole).toString();
    currentTargetNick = targetDisplayName(currentTargetIp);
    if (currentTargetIp.isEmpty()) {
        appendNetworkLog("错误：无法获取目标IP");
        return;
    }
    appendNetworkLog("选中会话对象: " + currentTargetNick + " (" + currentTargetIp + ")");
    ui->chatGroup->setTitle((isGroupContact(currentTargetIp) ? QString::fromUtf8("安全群聊 — ") : QString::fromUtf8("安全会话 — ")) + currentTargetNick);
    renderConversation(currentTargetIp);
}

void MainWindow::onSendMessageClicked()
{
    if (currentTargetIp.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在左侧选择一个在线用户");
        return;
    }
    QString plain = ui->msgInput->toPlainText().trimmed();
    if (plain.isEmpty()) return;

    if (isGroupContact(currentTargetIp)) {
        sendGroupMessage(currentTargetIp, plain);
        return;
    }

    if (isHermesProxyContact(currentTargetIp)) {
        sendHermesProxyQuestion(currentTargetIp, plain);
        return;
    }

    if (currentTargetIp == HermesBridge::peerId()) {
        if (!isHermesFeatureEnabled()) {
            QMessageBox::information(this,
                                     QString::fromUtf8("Hermes 未启用"),
                                     QString::fromUtf8("当前环境未启用 Hermes AI Agent 协同能力。"));
            return;
        }
        if (hermesEndpointInput)
            hermesBridge.setEndpoint(hermesEndpointInput->text());
        beginSmTrace(currentTargetIp,
                     currentTargetNick,
                     QString::fromUtf8("加密发送 / 本机Hermes桥接"),
                     plain);
        bool hermesSent = hermesBridge.sendPrompt(plain, infoMgr, msgHandler, logger);
        endSmTrace(hermesSent ? QString::fromUtf8("Hermes 请求已进入本地桥接") : QString::fromUtf8("Hermes 请求桥接失败"));
        if (hermesSent) {
            appendChatMessage(currentTargetIp, infoMgr.getSelfNick(), plain, true);
            showBottomFlow(currentTargetIp,
                           QString::fromUtf8("Hermes 本地安全桥接"),
                           QStringList() << QString::fromUtf8("明文入队")
                                         << QString::fromUtf8("SM3 摘要")
                                         << QString::fromUtf8("SM2 签名")
                                         << QString::fromUtf8("SM4 加密")
                                         << QString::fromUtf8("WSL Agent"),
                           "#0f766e");
            updateSessionHashChain("send_hermes", currentTargetNick, plain);
            ui->msgInput->clear();
            ui->msgInput->setFocus();
        } else {
            QMessageBox::warning(this, QString::fromUtf8("Hermes 发送失败"), QString::fromUtf8("消息未通过本地 HermesBridge 国密流程。"));
        }
        return;
    }

    if (nextMessageTamperArmed) {
        if (currentTargetIp != nextMessageTamperTargetIp) {
            QMessageBox::warning(this,
                                 QString::fromUtf8("篡改目标不一致"),
                                 QString::fromUtf8("消息篡改已为另一个会话待命。请切回原目标发送，或重新点击“消息篡改”。"));
            return;
        }
        bool attacked = sendTamperedMessageForDemo(plain);
        nextMessageTamperArmed = false;
        nextMessageTamperTargetIp.clear();
        if (attacked) {
            ui->msgInput->clear();
            ui->msgInput->setFocus();
        } else {
            QMessageBox::warning(this,
                                 QString::fromUtf8("篡改攻击失败"),
                                 QString::fromUtf8("未能生成或发送篡改国密包，请查看加密追踪日志。"));
        }
        return;
    }

    beginSmTrace(currentTargetIp,
                 currentTargetNick,
                 QString::fromUtf8("加密发送 / 单聊消息"),
                 plain);
    bool sent = msgHandler.sendEncryptedMessage(plain, currentTargetIp, infoMgr, udp, logger);
    endSmTrace(sent ? QString::fromUtf8("发送成功") : QString::fromUtf8("发送失败"));
    if (sent) {
        appendChatMessage(currentTargetIp, infoMgr.getSelfNick(), plain, true);
        animateMessageRoute(currentTargetIp, currentTargetNick, true);
        updateSessionHashChain("send", currentTargetNick, plain);
        ui->msgInput->clear();
        ui->msgInput->setFocus();
    } else {
        QMessageBox::warning(this, "发送失败", "消息未通过本地加密流程，已取消发送");
    }
}

void MainWindow::onSendFileClicked()
{
    if (currentTargetIp.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在左侧选择一个在线用户");
        return;
    }
    if (currentTargetIp == HermesBridge::peerId() || isHermesProxyContact(currentTargetIp) || isGroupContact(currentTargetIp)) {
        QMessageBox::information(this,
                                 QString::fromUtf8("当前会话不支持文件"),
                                 QString::fromUtf8("当前版本群组/Hermes 会话支持文字安全对话；文件请继续发送给单个局域网同学。"));
        return;
    }
    QString filePath = QFileDialog::getOpenFileName(this, "选择要发送的文件");
    if (filePath.isEmpty()) return;

    beginSmTrace(currentTargetIp,
                 currentTargetNick,
                 QString::fromUtf8("加密发送 / 文件传输"),
                 QFileInfo(filePath).fileName());
    bool fileSent = fileHandler.sendFile(filePath, currentTargetIp, infoMgr, udp, logger);
    endSmTrace(fileSent ? QString::fromUtf8("文件发送成功") : QString::fromUtf8("文件发送失败"));
    if (!fileSent) {
        QMessageBox::warning(this, "发送失败", "文件未通过本地加密或签名流程，已取消发送");
    } else {
        QFile sent(filePath);
        if (sent.open(QIODevice::ReadOnly)) {
            QString sentName = QFileInfo(filePath).fileName();
            recordFileMerkleEvidence(sentName, sent.readAll(), "send");
            appendChatEvent(currentTargetIp,
                            QString::fromUtf8("文件发送"),
                            QString::fromUtf8("%1 已通过 ZUC 分块加密与 SM2 签名发送。").arg(sentName));
            showBottomFlow(currentTargetIp,
                           QString::fromUtf8("文件安全发送"),
                           QStringList() << QString::fromUtf8("读取文件")
                                         << QString::fromUtf8("ZUC 分块加密")
                                         << QString::fromUtf8("SM2 分块签名")
                                         << QString::fromUtf8("UDP 分片发送")
                                         << QString::fromUtf8("Merkle 存证"),
                           "#7c3aed");
            sent.close();
        }
    }
}

void MainWindow::onMessageReceived(const QString &fromIp, const QString &fromNick, const QString &plainText)
{
    if (handleWalletBindingControlMessage(fromIp, fromNick, plainText))
        return;
    if (handleHermesControlMessage(fromIp, fromNick, plainText))
        return;
    if (handleGroupControlMessage(fromIp, fromNick, plainText))
        return;

    if (securityDashboard)
        securityDashboard->recordMessageAccepted();
    appendChatMessage(fromIp, fromNick, plainText, false);
    animateMessageRoute(fromIp, fromNick, false);
    updateSessionHashChain("receive", fromNick, plainText);
}

void MainWindow::onFileReceived(const QString &fromNick, const QString &fileName,
                                const QByteArray &fileData, bool verifyOk)
{
    recordFileMerkleEvidence(fileName, fileData, verifyOk ? "receive_verified" : "receive_failed");
    QString conversationIp = infoMgr.getIpByNick(fromNick);
    appendChatEvent(conversationIp,
                    QString::fromUtf8("文件接收"),
                    QString::fromUtf8("%1 | 完整性校验: %2").arg(fileName, verifyOk ? QString::fromUtf8("通过") : QString::fromUtf8("失败")));
    showBottomFlow(conversationIp,
                   QString::fromUtf8("文件安全接收"),
                   QStringList() << QString::fromUtf8("收到分片")
                                 << QString::fromUtf8("ZUC 解密")
                                 << QString::fromUtf8("SM2 验签")
                                 << QString::fromUtf8("SM3 复算")
                                 << QString::fromUtf8("Merkle 核验"),
                   verifyOk ? "#0f9f6e" : "#ef4444");
    QString safeFileName = QFileInfo(fileName).fileName();
    if (safeFileName.isEmpty())
        safeFileName = "received.dat";
    QString safeNick = fromNick;
    for (QChar ch : QString("\\/:*?\"<>|")) {
        safeNick.replace(ch, '_');
        safeFileName.replace(ch, '_');
    }
    QString defaultName = "received_" + safeNick + "_" +
                          QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") +
                          "_" + safeFileName;
    QString savePath = QFileDialog::getSaveFileName(this, "保存接收的文件",
                                                    QDir::home().filePath(defaultName));
    if (!savePath.isEmpty()) {
        QFile f(savePath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(fileData);
            f.close();
            appendNetworkLog(QString("文件保存: %1 | 完整性校验: %2")
                                 .arg(QFileInfo(savePath).fileName())
                                 .arg(verifyOk ? "通过" : "失败"));
            if (securityDashboard)
                securityDashboard->recordFileAccepted(verifyOk);
        } else {
            appendNetworkLog("保存文件失败: " + savePath);
        }
    }
}

void MainWindow::onLogMessage(const QString &msg, bool isNetwork)
{
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    if (isNetwork) {
        if (securityDashboard)
            securityDashboard->recordNetworkEvent(msg);
        ui->netLog->append(timestamp + msg);
    } else {
        if (securityDashboard)
            securityDashboard->recordCryptoEvent(msg);
        ui->cryptoLog->append(timestamp + msg);
    }
}

void MainWindow::onOnlineUserAdded(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress)
{
    // Don't add ourselves
    QStringList localIps = udp.getLocalIPs();
    if (localIps.contains(ip)) return;

    if (!infoMgr.addOrUpdateUser(ip, nick, pubKeyHex, walletAddress)) {
        appendCryptoLog(QString("[警告] 忽略来自 %1 的无效 SM2 公钥").arg(ip));
        return;
    }
    QJsonObject verified = verifiedWalletBindings.value(ip);
    if (!verified.isEmpty() &&
        (normalizedEthAddress(verified.value(QStringLiteral("walletAddress")).toString()) != normalizedEthAddress(walletAddress) ||
         verified.value(QStringLiteral("subjectFingerprint")).toString() != pubFingerprintForIp(ip))) {
        verifiedWalletBindings.remove(ip);
        appendCryptoLog(QString::fromUtf8("[钱包验证] %1 的钱包或 SM2 指纹已变化，旧验证状态已清除").arg(nick));
    }
    appendNetworkLog(QString("用户上线: %1 (%2)").arg(nick, ip));
    appendCryptoLog(QString("获取 %1 公钥指纹: %2")
                    .arg(nick, Utils::shortFingerprint(infoMgr.getPubKeyByIp(ip))));

    // Respond with our own info so the new user discovers us too
    if (ui->loginBtn->isEnabled() == false) {
        udp.sendLoginResponse(ip, infoMgr.getSelfNick(), infoMgr.getSelfPublicKeyHex(), infoMgr.getSelfWalletAddress());
        appendNetworkLog("已向 " + nick + " 回复本机信息");
    }

    updateStatusBar();
}

void MainWindow::onOnlineUserAddedAck(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress)
{
    // Same as onOnlineUserAdded but without sending a response back (avoid infinite loop)
    QStringList localIps = udp.getLocalIPs();
    if (localIps.contains(ip)) return;

    if (!infoMgr.addOrUpdateUser(ip, nick, pubKeyHex, walletAddress)) {
        appendCryptoLog(QString("[警告] 忽略来自 %1 的无效 SM2 公钥").arg(ip));
        return;
    }
    QJsonObject verified = verifiedWalletBindings.value(ip);
    if (!verified.isEmpty() &&
        (normalizedEthAddress(verified.value(QStringLiteral("walletAddress")).toString()) != normalizedEthAddress(walletAddress) ||
         verified.value(QStringLiteral("subjectFingerprint")).toString() != pubFingerprintForIp(ip))) {
        verifiedWalletBindings.remove(ip);
        appendCryptoLog(QString::fromUtf8("[钱包验证] %1 的钱包或 SM2 指纹已变化，旧验证状态已清除").arg(nick));
    }
    appendNetworkLog(QString("用户发现: %1 (%2) [单播响应]").arg(nick, ip));
    appendCryptoLog(QString("获取 %1 公钥指纹: %2")
                    .arg(nick, Utils::shortFingerprint(infoMgr.getPubKeyByIp(ip))));
    updateStatusBar();
}

void MainWindow::onOnlineUserRemoved(const QString& ip)
{
    QString nick = infoMgr.getNickByIp(ip);
    infoMgr.removeUser(ip);
    hermesProxyContacts.remove(hermesProxyIdForOwner(ip));
    activeHermesShares.remove(ip);
    verifiedWalletBindings.remove(ip);
    resetConversationTrace(ip);
    for (auto it = groupMembers.begin(); it != groupMembers.end(); ++it) {
        if (it.value().contains(ip))
            resetConversationTrace(it.key());
    }
    appendNetworkLog(QString("用户下线: %1 (%2)").arg(nick, ip));
    if (currentTargetIp == ip || currentTargetIp == hermesProxyIdForOwner(ip)) {
        currentTargetIp.clear();
        currentTargetNick.clear();
        ui->chatGroup->setTitle("安全会话");
    }
    updateOnlineList();
    updateStatusBar();
}

void MainWindow::updateOnlineList()
{
    ui->userList->clear();
    for (const auto& user : infoMgr.getOnlineUsers()) {
        QString fingerprint = Utils::shortFingerprint(user.pubKey);
        if (user.ip == HermesBridge::peerId()) {
            if (!isHermesFeatureEnabled())
                continue;
            QListWidgetItem* item = new QListWidgetItem(user.nick + "  [WSL one-shot]\nSM3 FP " + fingerprint +
                                                        "\n" + QString::fromUtf8("Local HermesBridge"));
            item->setData(Qt::UserRole, user.ip);
            item->setToolTip(QString::fromUtf8("可选 AI Agent 协同节点：消息先走 SM2/SM3/SM4，再桥接到 WSL Hermes CLI\nCommand: ") +
                             hermesBridge.endpoint());
            ui->userList->addItem(item);
            continue;
        }
        QString walletLine = user.walletAddress.isEmpty()
                                 ? QString("Wallet not broadcast")
                                 : QString("Wallet ") + user.walletAddress.left(8) + "..." + user.walletAddress.right(6) +
                                       verifiedWalletBadgeForIp(user.ip);
        QListWidgetItem* item = new QListWidgetItem(user.nick + "  [" + user.ip + "]\nSM3 FP " + fingerprint + "\n" + walletLine);
        item->setData(Qt::UserRole, user.ip);
        item->setToolTip("SM2 公钥 SM3 指纹: " + fingerprint);
        item->setToolTip("SM2 public key SM3 fingerprint: " + fingerprint +
                         (user.walletAddress.isEmpty() ? QString() : "\nSepolia wallet: " + user.walletAddress));
        ui->userList->addItem(item);
    }
    for (auto it = hermesProxyContacts.begin(); it != hermesProxyContacts.end(); ++it) {
        const QJsonObject contact = it.value();
        QString ownerIp = contact.value(QStringLiteral("ownerIp")).toString();
        QString displayName = contact.value(QStringLiteral("displayName")).toString();
        QString ownerFp = contact.value(QStringLiteral("ownerFingerprint")).toString();
        bool authorized = contact.value(QStringLiteral("authorized")).toBool(false);
        QListWidgetItem* item = new QListWidgetItem(displayName +
                                                    QString::fromUtf8("  [共享代理]\nOwner %1\nSM2 FP %2")
                                                        .arg(ownerIp, ownerFp));
        item->setData(Qt::UserRole, it.key());
        item->setToolTip(QString::fromUtf8("由 %1 授权共享的本机 Hermes Agent。\n消息会先用 SM2/SM3/SM4 加密发给拥有者，再由其本机调用 Hermes。\n授权状态: %2")
                         .arg(ownerIp, authorized ? QString::fromUtf8("已确认") : QString::fromUtf8("等待确认")));
        ui->userList->addItem(item);
    }
    for (auto it = groupNames.begin(); it != groupNames.end(); ++it) {
        QString groupId = it.key();
        QStringList members = groupMembers.value(groupId);
        QStringList names;
        for (const QString& ip : members) {
            QString nick = infoMgr.getNickByIp(ip);
            if (nick.isEmpty())
                nick = ip;
            names << nick;
        }
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8("%1  [群组]\n成员 %2 人\n%3")
                                                    .arg(it.value())
                                                    .arg(members.size())
                                                    .arg(names.join(QString::fromUtf8("、"))));
        item->setData(Qt::UserRole, groupId);
        item->setToolTip(QString::fromUtf8("本地国密群组：发送时会对每个成员分别执行 SM3 摘要、SM2 签名、SM4 加密和 SM2 密钥封装。"));
        ui->userList->addItem(item);
    }
    if (securityDashboard)
        securityDashboard->setUsers(infoMgr.getOnlineUsers());
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    QStringList ips = udp.getLocalIPs();
    QString ipStr = ips.isEmpty() ? "无" : ips.join(", ");
    QString msg = "已登录 | " + infoMgr.getSelfNick() + " | 本机IP: " + ipStr +
                  " | 在线: " + QString::number(infoMgr.getOnlineUsers().size()) + " 人";
    if (ui->loginBtn->isEnabled()) {
        msg = "未连接 | 端口: 8888 | 本机IP: " + ipStr + " | 等待登录...";
    }
    statusBar()->showMessage(msg);
}

void MainWindow::appendChatMessage(const QString &nick, const QString &msg, bool isSelf)
{
    QString timeStr = QTime::currentTime().toString("hh:mm:ss");
    QString color = isSelf ? "#2563eb" : "#0f9f6e";
    QString name = isSelf ? "我" : nick;
    QString html = QString(
        "<div style='margin:4px 0;'>"
        "<span style='color:%1; font-weight:bold;'>[%2] %3</span>"
        "<br/>"
        "<span style='color:#1f2937; margin-left:8px;'>%4</span>"
        "</div>"
    ).arg(color, timeStr, name, msg.toHtmlEscaped());
    ui->chatDisplay->append(html);
}

void MainWindow::renderConversation(const QString& conversationIp)
{
    ui->chatDisplay->clear();
    const QStringList entries = chatHistories.value(conversationIp);
    for (const QString& entry : entries)
        ui->chatDisplay->append(entry);
}

void MainWindow::appendChatMessage(const QString& conversationIp, const QString &nick, const QString &msg, bool isSelf)
{
    if (conversationIp.isEmpty())
        return;

    QString timeStr = QTime::currentTime().toString("hh:mm:ss");
    QString color = isSelf ? "#2563eb" : "#0f9f6e";
    QString bubble = isSelf ? "#eef5ff" : "#f0fdf4";
    QString border = isSelf ? "#bfdbfe" : "#bbf7d0";
    QString name = isSelf ? QString::fromUtf8("我") : nick.toHtmlEscaped();
    QString html = QString(
        "<div style='margin:6px 0;'>"
        "<div style='display:inline-block; max-width:78%; padding:8px 10px; border-radius:8px; background:%1; border:1px solid %2;'>"
        "<div style='color:%3; font-weight:bold; font-size:12px;'>[%4] %5</div>"
        "<div style='color:#111827; margin-top:4px;'>%6</div>"
        "</div>"
        "</div>"
    ).arg(bubble, border, color, timeStr, name, msg.toHtmlEscaped());

    chatHistories[conversationIp].append(html);
    if (currentTargetIp == conversationIp)
        ui->chatDisplay->append(html);
}

void MainWindow::appendChatEvent(const QString& conversationIp, const QString& title, const QString& detail)
{
    if (conversationIp.isEmpty())
        return;

    QString timeStr = QTime::currentTime().toString("hh:mm:ss");
    QString html = QString(
        "<div style='margin:6px 0; color:#475569;'>"
        "<span style='background:#f8fafc; border:1px solid #dbe4f0; border-radius:8px; padding:6px 8px;'>"
        "[%1] <b>%2</b> | %3"
        "</span>"
        "</div>"
    ).arg(timeStr, title.toHtmlEscaped(), detail.toHtmlEscaped());

    chatHistories[conversationIp].append(html);
    if (currentTargetIp == conversationIp)
        ui->chatDisplay->append(html);
}

void MainWindow::showBottomFlow(const QString& conversationIp, const QString& title,
                                const QStringList& stages, const QString& accentHex)
{
    if (!routeAnimation || conversationIp.isEmpty() || conversationIp != currentTargetIp)
        return;
    routeAnimation->startFlow(title, stages, QColor(accentHex));
}

void MainWindow::animateMessageRoute(const QString& conversationIp, const QString& peerNick, bool outgoing)
{
    QStringList stages;
    if (outgoing) {
        stages << QString::fromUtf8("明文")
               << QString::fromUtf8("SM3 摘要")
               << QString::fromUtf8("SM2 签名")
               << QString::fromUtf8("SM4 加密")
               << QString::fromUtf8("UDP 到达 %1").arg(peerNick);
        showBottomFlow(conversationIp, QString::fromUtf8("消息安全发送"), stages, "#2563eb");
    } else {
        stages << QString::fromUtf8("收到密文")
               << QString::fromUtf8("SM2 解封")
               << QString::fromUtf8("SM4 解密")
               << QString::fromUtf8("SM3 校验")
               << QString::fromUtf8("验签放行");
        showBottomFlow(conversationIp, QString::fromUtf8("消息安全接收"), stages, "#0f9f6e");
    }
}

void MainWindow::onDappCallback(const QString& type, const QJsonObject& payload)
{
    if (type == "wallet_bind_signature") {
        handleWalletBindSignatureCallback(payload);
        return;
    }

    if (type != "transfer_success" && type != "transfer_audit")
        return;

    QString conversationIp = payload["recipientIp"].toString();
    QString txHash = payload["transferTxHash"].toString();
    QString auditTxHash = payload["auditTxHash"].toString();
    QString amountEth = payload["amountEth"].toString();
    QString toWallet = payload["toWallet"].toString();
    QString toNick = payload["toNick"].toString(currentTargetNick);

    if (type == "transfer_success") {
        QString detail = QString::fromUtf8("%1 SepoliaETH -> %2，tx=%3...")
                             .arg(amountEth, toNick.isEmpty() ? toWallet : toNick, txHash.left(18));
        appendChatEvent(conversationIp, QString::fromUtf8("Sepolia 转账成功"), detail);
        showBottomFlow(conversationIp,
                       QString::fromUtf8("链上转账完成"),
                       QStringList() << QString::fromUtf8("MetaMask 签名")
                                     << QString::fromUtf8("Sepolia 广播")
                                     << QString::fromUtf8("交易确认")
                                     << QString::fromUtf8("回写会话")
                                     << QString::fromUtf8("准备存证"),
                       "#0f766e");

        QString notice = QString::fromUtf8("【Sepolia 转账成功】金额 %1 SepoliaETH，交易哈希 %2")
                             .arg(amountEth, txHash);
        if (!conversationIp.isEmpty() &&
            msgHandler.sendEncryptedMessage(notice, conversationIp, infoMgr, udp, logger)) {
            appendChatMessage(conversationIp, infoMgr.getSelfNick(), notice, true);
            animateMessageRoute(conversationIp, toNick, true);
        }
    } else if (type == "transfer_audit") {
        appendChatEvent(conversationIp,
                        QString::fromUtf8("转账存证已上链"),
                        QString::fromUtf8("auditTx=%1...，合约账本已记录这次转账证据。").arg(auditTxHash.left(18)));
        showBottomFlow(conversationIp,
                       QString::fromUtf8("转账证据上链"),
                       QStringList() << QString::fromUtf8("整理证据")
                                     << QString::fromUtf8("提交合约")
                                     << QString::fromUtf8("记录 txHash")
                                     << QString::fromUtf8("写入 metadata")
                                     << QString::fromUtf8("存证完成"),
                       "#f59e0b");
    }
}

void MainWindow::appendNetworkLog(const QString &msg)
{
    logger.logNetwork(msg);
}

void MainWindow::appendCryptoLog(const QString &msg)
{
    logger.logCrypto(msg);
}
