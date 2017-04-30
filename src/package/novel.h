#ifndef _NOVEL_H
#define _NOVEL_H

#include "moesen-package.h"
#include "card.h"

class TongheCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TongheCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TiaotingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TiaotingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class BaoyanCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE BaoyanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XianliCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XianliCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WeihaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE WeihaoCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class ZhuyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhuyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HaoqiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HaoqiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JisuiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JisuiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JingdiCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE JingdiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BianchiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BianchiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhuanyuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhuanyuCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class XianqunCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XianqunCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class XieyuSummon : public ArraySummonCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XieyuSummon();
};
#endif

