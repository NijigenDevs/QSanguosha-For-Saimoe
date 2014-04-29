#ifndef _NOVEL_H
#define _NOVEL_H

#include "moesen-package.h"
#include "card.h"

class WeihaoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE WeihaoCard();
    void WeihaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class ZhuyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhuyiCard();
    bool ZhuyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void ZhuyiCard::onEffect(const CardEffectStruct &effect) const;
};

#endif