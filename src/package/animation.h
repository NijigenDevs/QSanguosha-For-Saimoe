#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "moesen-package.h"
#include "card.h"

class WuweiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE WuweiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &use) const;
};

class QuanmianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QuanmianCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class MiaolvCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MiaolvCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XiehangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XiehangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XiehangUseCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XiehangUseCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

//class BajianCard : public SkillCard
//{
//    Q_OBJECT
//
//public:
//    Q_INVOKABLE BajianCard();
//    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
//    virtual void use(Room *room, ServerPlayer *xunyu, QList<ServerPlayer *> &targets) const;
//};

class GejiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GejiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class Tengyue : public DrawCardsSkill
{
public:
    Tengyue();

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual int getDrawNum(ServerPlayer *player, int n) const;
};

#endif
