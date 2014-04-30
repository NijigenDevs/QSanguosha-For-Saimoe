#ifndef _GAME_H
#define _GAME_H

#include "moesen-package.h"
#include "card.h"

class HaixingCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HaixingCard();
    void HaixingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif