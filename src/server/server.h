

#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QSet>
#include <QStringList>

#include "protocol.h"

class Room;
class ClientSocket;
class ServerSocket;
class ServerPlayer;

class Server : public QObject
{
    Q_OBJECT

public:
    friend class BanIpDialog;

    explicit Server(QObject *parent);

    void broadcastSystemMessage(const QString &msg);

    bool listen();
    void daemonize();


    Room *createNewRoom();
    void signupPlayer(ServerPlayer *player);

private:
    void notifyClient(ClientSocket *socket, QSanProtocol::CommandType command, const QVariant &arg = QVariant());

    void processClientRequest(ClientSocket *socket, const QSanProtocol::Packet &signup);

    ServerSocket *server;
    Room *current;
    QSet<Room *> rooms;
    QHash<QString, ServerPlayer *> players;
    QStringList addresses;
    QMultiHash<QString, QString> name2objname;

    private slots:
    void processNewConnection(ClientSocket *socket);
    void processRequest(const QByteArray &request);
    void cleanup();
    void gameOver();

signals:
    void server_message(const QString &);
    void newPlayer(ServerPlayer *player);
};

#endif // SERVER_H
