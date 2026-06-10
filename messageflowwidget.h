#ifndef MESSAGEFLOWWIDGET_H
#define MESSAGEFLOWWIDGET_H

#include <QTimer>
#include <QWidget>

class MessageFlowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MessageFlowWidget(QWidget *parent = nullptr);

    void startFlow(const QString& kind, const QString& peer, bool outgoing);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer timer;
    QStringList stages;
    QString title;
    QString subtitle;
    qreal progress;
    bool active;

    void setStages(const QString& kind, bool outgoing);
};

#endif // MESSAGEFLOWWIDGET_H
