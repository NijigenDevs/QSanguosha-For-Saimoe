#ifndef _CLIENT_LOG_BOX_H
#define _CLIENT_LOG_BOX_H

class ClientPlayer;

#include <QTextEdit>
#ifdef Q_OS_ANDROID
#include <QTimer>
class QPropertyAnimation;
#endif

class ClientLogBox : public QTextEdit
{
    Q_OBJECT

public:
    explicit ClientLogBox(QWidget *parent = 0);
    void appendLog(const QString &type, const QString &from_general, const QStringList &to,
        const QString card_str = QString(), const QString arg = QString(), const QString arg2 = QString());
#ifdef Q_OS_ANDROID
    ~ClientLogBox();
#endif

private:
    QString bold(const QString &str, QColor color) const;
#ifdef Q_OS_ANDROID
    QTimer timer;
#endif

public slots:
    void appendLog(const QStringList &log_str);
    void append(const QString &text);

};

#endif

