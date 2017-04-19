#ifndef PLAYERCARDBOX_H
#define PLAYERCARDBOX_H

#include "graphicsbox.h"
#include "card.h"
#include "player.h"

class ClientPlayer;
class QGraphicsProxyWidget;
class QSanCommandProgressBar;

class PlayerCardBox : public GraphicsBox
{
    Q_OBJECT

public:
    explicit PlayerCardBox();

    void chooseCard(const QString &reason, const ClientPlayer *player,
        const QString &flags = "hej", bool handcardVisible = false,
        Card::HandlingMethod method = Card::MethodNone,
        const QList<int> &disabledIds = QList<int>(), const QList<int> &handcards = QList<int>());
    void clear();

protected:
    // GraphicsBox interface
    QRectF boundingRect() const;
    void paintLayout(QPainter *painter);

private:
    void paintArea(const QString &name, QPainter *painter);
    int getRowCount(const int &cardNumber) const;
    void updateNumbers(const int &cardNumber);
    void arrangeCards(const CardList &cards, const QPoint &topLeft);

    const ClientPlayer *player;
    QString flags;
    bool handcardVisible;
    Card::HandlingMethod method;
    QList<int> disabledIds;
    QList<CardItem *> items;

    QGraphicsProxyWidget *progressBarItem;
    QSanCommandProgressBar *progressBar;

    QList<QRect> nameRects;

    int rowCount;
    int intervalsBetweenAreas;
    int intervalsBetweenRows;
    int maxCardsInOneRow;
    QList<int> handcards;

    static const int maxCardNumberInOneRow;

    static const int verticalBlankWidth;
    static const int placeNameAreaWidth;
    static const int intervalBetweenNameAndCard;
    static const int topBlankWidth;
    static const int bottomBlankWidth;
    static const int intervalBetweenAreas;
    static const int intervalBetweenRows;
    static const int intervalBetweenCards;

    public slots:
    void reply();
};

#endif // PLAYERCARDBOX_H
