#ifndef _GAME_H
#define _GAME_H

#include "moesen-package.h"
#include "card.h"

class HaixingCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HaixingCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TaozuiCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TaozuiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XiayingCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XiayingCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YumengDraw: public DrawCardsSkill {
public:
    YumengDraw();

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual int getDrawNum(ServerPlayer *player, int n) const;
};

class Key : public DelayedTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Key(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

class ShenxingCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShenxingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LuoxuanCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE LuoxuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Lingdan : public DelayedTrick {
    Q_OBJECT

public:
    Q_INVOKABLE Lingdan(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};

#endif