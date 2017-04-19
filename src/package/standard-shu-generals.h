#ifndef _STANDARD_SHU_H
#define _STANDARD_SHU_H

#include "standard-package.h"
#include "card.h"
#include "skill.h"

class Mashu : public DistanceSkill
{
public:
    explicit Mashu(const QString &);

    virtual int getCorrect(const Player *from, const Player *) const;
};

class RendeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE RendeCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
};

class SavageAssaultAvoid : public TriggerSkill
{
public:
    explicit SavageAssaultAvoid(const QString &);

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const;
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
private:
    QString avoid_skill;
};

class FangquanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FangquanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif

