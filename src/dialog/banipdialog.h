#ifndef BANIPDIALOG_H
#define BANIPDIALOG_H

#include "flatdialog.h"

class Server;
class QListWidget;
class ServerPlayer;

class BanIpDialog : public FlatDialog
{
    Q_OBJECT

public:
    BanIpDialog(QWidget *parent, Server *server);

private:
    QListWidget *left;
    QListWidget *right;

    Server *server;
    QList<ServerPlayer *> sp_list;

    void loadIPList();
    void loadBannedList();

private slots:
    void addPlayer(ServerPlayer *player);
    void removePlayer();

    void insertClicked();
    void removeClicked();
    void kickClicked();

    void save();

};

#endif // BANIPDIALOG_H
