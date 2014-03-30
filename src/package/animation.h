#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "moesen-package.h"
#include "card.h"

class WuweiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE WuweiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &use) const;
};

class MiaolvCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE MiaolvCard();
    bool MiaolvCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void MiaolvCard::use(Room *room, ServerPlayer *xunyu, QList<ServerPlayer *> &targets) const;
};

class MengyinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE MengyinCard();
    void MengyinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

#endif