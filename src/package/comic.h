#ifndef _COMIC_H
#define _COMIC_H

#include "moesen-package.h"
#include "card.h"

class MizouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MizouCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShuimengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShuimengCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class MingmingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MingmingCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class rosesuigintouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE rosesuigintouCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class HeliCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HeliCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhiyuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhiyuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BaozouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BaozouCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LingshangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LingshangCard();
};

class KaihuaCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE KaihuaCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SuanlvCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SuanlvCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LaoyueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LaoyueCard();
    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

class TianziCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TianziCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

#endif