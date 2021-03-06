#ifndef _H_MOMENTUM_H
#define _H_MOMENTUM_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

//class CunsiCard : public SkillCard
//{
//    Q_OBJECT
//
//public:
//    Q_INVOKABLE CunsiCard();
//
//    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
//    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
//    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
//    virtual void onEffect(const CardEffectStruct &effect) const;
//};
//
//class DuanxieCard : public SkillCard
//{
//    Q_OBJECT
//
//public:
//    Q_INVOKABLE DuanxieCard();
//
//    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
//    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
//    virtual void onEffect(const CardEffectStruct &effect) const;
//};
//
//class FengshiSummon : public ArraySummonCard
//{
//    Q_OBJECT
//
//public:
//    Q_INVOKABLE FengshiSummon();
//};
//
//class WendaoCard : public SkillCard
//{
//    Q_OBJECT
//
//public:
//    Q_INVOKABLE WendaoCard();
//
//    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
//};

class MomentumPackage : public Package
{
    Q_OBJECT

public:
    MomentumPackage();
};

class PeaceSpell : public Armor
{
    Q_OBJECT

public:
    Q_INVOKABLE PeaceSpell(Card::Suit suit = Heart, int number = 3);
    virtual void onUninstall(ServerPlayer *player) const;
};

class MomentumEquipPackage : public Package
{
    Q_OBJECT

public:
    MomentumEquipPackage();
};

#endif
