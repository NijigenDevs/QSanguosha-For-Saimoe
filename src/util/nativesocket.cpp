/********************************************************************
    Copyright (c) 2013-2015 - Mogara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    Mogara
    *********************************************************************/

#include "nativesocket.h"
#include "settings.h"
#include "clientplayer.h"

#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QUdpSocket>

NativeServerSocket::NativeServerSocket()
{
    server = new QTcpServer(this);
    daemon = NULL;
    connect(server, &QTcpServer::newConnection, this, &NativeServerSocket::processNewConnection);
}

bool NativeServerSocket::listen()
{
    return server->listen(QHostAddress::Any, Config.ServerPort);
}

void NativeServerSocket::daemonize()
{
    daemon = new QUdpSocket(this);
    daemon->bind(Config.ServerPort, QUdpSocket::ShareAddress);
    connect(daemon, &QUdpSocket::readyRead, this, &NativeServerSocket::processNewDatagram);
}

void NativeServerSocket::processNewDatagram()
{
    while (daemon->hasPendingDatagrams()) {
        QHostAddress from;
        char ask_str[256];

        daemon->readDatagram(ask_str, sizeof(ask_str), &from);

        QByteArray data = Config.ServerName.toUtf8();
        daemon->writeDatagram(data, from, Config.DetectorPort);
        daemon->flush();
    }
}

void NativeServerSocket::processNewConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();
    NativeClientSocket *connection = new NativeClientSocket(socket);
    emit new_connection(connection);
}

// ---------------------------------

NativeClientSocket::NativeClientSocket()
    : socket(new QTcpSocket(this))
{
    init();
}

NativeClientSocket::NativeClientSocket(QTcpSocket *socket)
    : socket(socket)
{
    socket->setParent(this);
    init();
}

void NativeClientSocket::init()
{
    connect(socket, &QTcpSocket::disconnected, this, &NativeClientSocket::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NativeClientSocket::getMessage);
    connect(socket, (void (QTcpSocket::*)(QAbstractSocket::SocketError))(&QTcpSocket::error), this, &NativeClientSocket::raiseError);
    connect(socket, &QTcpSocket::connected, this, &NativeClientSocket::connected);
}

void NativeClientSocket::connectToHost()
{
    QString address = "127.0.0.1";
    ushort port = 9527u;

    if (Config.HostAddress.contains(QChar(':'))) {
        QStringList texts = Config.HostAddress.split(QChar(':'));
        address = texts.value(0);
        port = texts.value(1).toUShort();
    } else {
        address = Config.HostAddress;
        if (address == "127.0.0.1")
            port = Config.value("ServerPort", 9527u).toUInt();
    }

    socket->connectToHost(address, port);
}

void NativeClientSocket::connectToHost(const QHostAddress &address)
{
    ushort port = Config.value("ServerPort", 9527u).toUInt();
    socket->connectToHost(address, port);
}

void NativeClientSocket::connectToHost(const QHostAddress &address, ushort port)
{
    socket->connectToHost(address, port);
}

void NativeClientSocket::getMessage()
{
    while (socket->canReadLine()) {
        QByteArray msg = socket->readLine();
#ifndef QT_NO_DEBUG
        printf("recv: %s", msg.constData());
#endif
        emit message_got(msg);
    }
}

void NativeClientSocket::disconnectFromHost()
{
    socket->disconnectFromHost();
}

void NativeClientSocket::send(const QByteArray &message)
{
    if (message.isEmpty())
        return;

    socket->write(message);
    if (!message.endsWith('\n')) {
        socket->write("\n");
    }

#ifndef QT_NO_DEBUG
    printf(": %s\n", message.constData());
#endif
    socket->flush();
}

bool NativeClientSocket::isConnected() const
{
    return socket->state() == QTcpSocket::ConnectedState;
}

QString NativeClientSocket::peerName() const
{
    QString peer_name = socket->peerName();
    if (peer_name.isEmpty())
        peer_name = QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());

    return peer_name;
}

QString NativeClientSocket::peerAddress() const
{
    return socket->peerAddress().toString();
}

ushort NativeClientSocket::peerPort() const
{
    return socket->peerPort();
}

void NativeClientSocket::raiseError(QAbstractSocket::SocketError socket_error)
{
    // translate error message
    QString reason;
    switch (socket_error) {
    case QAbstractSocket::ConnectionRefusedError:
        reason = tr("Connection was refused or timeout"); break;
    case QAbstractSocket::RemoteHostClosedError:{
        if (Self && Self->hasFlag("is_kicked"))
            reason = tr("You are kicked from server");
        else
            reason = tr("Remote host close this connection");

        break;
    }
    case QAbstractSocket::HostNotFoundError:
        reason = tr("Host not found"); break;
    case QAbstractSocket::SocketAccessError:
        reason = tr("Socket access error"); break;
    case QAbstractSocket::NetworkError:
        return; // this error is ignored ...
    default: reason = tr("Unknown error"); break;
    }

    emit error_message(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}

