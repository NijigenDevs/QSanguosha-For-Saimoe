#ifndef UDPDETECTORDIALOG_H
#define UDPDETECTORDIALOG_H

#include "flatdialog.h"

class UdpDetector;
class QListWidget;
class QListWidgetItem;

class UdpDetectorDialog : public FlatDialog
{
    Q_OBJECT

public:
    UdpDetectorDialog(QDialog *parent);

private:
    QListWidget *list;
    UdpDetector *detector;
    QPushButton *detect_button;
    QPushButton *cancel_button;

private slots:
    void startDetection();
    void stopDetection();
    void chooseAddress(QListWidgetItem *item);
    void addServerAddress(const QString &server_name, const QString &address);

signals:
    void address_chosen(const QString &address);
};

#endif // UDPDETECTORDIALOG_H
