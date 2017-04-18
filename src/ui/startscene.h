

#ifndef _START_SCENE_H
#define _START_SCENE_H

#include "qsanselectableitem.h"
#include "server.h"

#include <QGraphicsScene>
#include <QAction>
#include <QTextEdit>

class Tile;

class StartScene : public QGraphicsScene
{
    Q_OBJECT

public:
    StartScene(QObject *parent = 0);

    void addButton(QAction *action);
    void setServerLogBackground();
    void switchToServer(Server *server);

    void showOrganization();

    private slots:
    void onSceneRectChanged(const QRectF &rect);

private:
    void printServerInfo();

    QSanSelectableItem *logo;
    QTextEdit *serverLog;
    QList<Tile *> buttons;
    bool shouldMourn;
};

#endif

