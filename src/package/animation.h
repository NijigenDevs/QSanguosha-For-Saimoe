#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "moesen-package.h"
#include "card.h"

class WuweiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE WuweiCard();
	virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif