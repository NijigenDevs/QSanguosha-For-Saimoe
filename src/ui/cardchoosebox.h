
#ifndef CARDCHOOSEBOX_H
#define CARDCHOOSEBOX_H

#include "cardcontainer.h"

class CardChooseBox : public CardContainer
{
    Q_OBJECT

public:
    CardChooseBox();
    void reply();
    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool check(const QList<int> &selected, int to_select);

    public slots:
    void doCardChoose(const QList<int> &upcards, const QList<int> &downcards, const QString &reason, const QString &pattern, bool moverestricted, int min_num, int max_num);
    void clear();

    void mirrorCardChooseStart(const QString &who, const QString &reason, const QList<int> &upcards, const QList<int> &downcards, const QString &pattern, bool moverestricted, int min_num, int max_num);
    void mirrorCardChooseMove(int from, int to);

    private slots:
    void onItemReleased();
    void onItemClicked();

private:
    QList<CardItem *> upItems, downItems;
    QString reason;
    QString func;
    int downCount, min_num, up_app1 = 0, up_app2 = 0, down_app1 = 0, down_app2 = 0, width;
    bool moverestricted;
    bool buttonstate;
    bool noneoperator = false;
    void adjust();
    int itemNumberOfFirstRow(bool up) const;
    bool isOneRow(bool up) const;
    QString zhuge;
};


#endif
