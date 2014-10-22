#ifndef _NOVEL_H
#define _NOVEL_H

#include "moesen-package.h"
#include "card.h"

class WeihaoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE WeihaoCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class ZhuyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhuyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HaoqiCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HaoqiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JisuiCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE JisuiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif
