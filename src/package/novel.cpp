#include "novel.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"

WeihaoCard::WeihaoCard() {
    target_fixed = true;
}

void WeihaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
     //broadcast
    source->drawCards(1);
    if (source->getMark("@zhenhao") == 0)
        source->gainMark("@weihao", 1);
    else
        source->loseMark("@zhenhao");
}

class Weihao: public ZeroCardViewAsSkill {
public:
    Weihao(): ZeroCardViewAsSkill("weihao") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMaxCards() > 0;
    }

    virtual const Card *viewAs() const{
        WeihaoCard *card = new WeihaoCard;
        card->setShowSkill(objectName());
        return card;
    }
};

ZhuyiCard::ZhuyiCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool ZhuyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ZhuyiCard::onEffect(const CardEffectStruct &effect) const{
     //broadcast
    effect.from->getRoom()->moveCardTo(this, effect.from, effect.to, Player::PlaceEquip,
                                   CardMoveReason(CardMoveReason::S_REASON_PUT,
                                                  effect.from->objectName(), "zhuyi", QString()));
    if (effect.from->getMark("@weihao") == 0)
        effect.from->gainMark("@zhenhao", 1);
    else
        effect.from->loseMark("@weihao");
}

class Zhuyi: public OneCardViewAsSkill {
public:
    Zhuyi(): OneCardViewAsSkill("zhuyi") {
        filter_pattern = "EquipCard|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        ZhuyiCard *zhuyi = new ZhuyiCard();
        zhuyi->addSubcard(originalCard);
        zhuyi->setShowSkill(objectName());
        return zhuyi;
    }
};

class AzusaMaxCards: public MaxCardsSkill {
public:
    AzusaMaxCards(): MaxCardsSkill("#azusa-maxcard") {
    }

    virtual int getExtra(const Player *target) const{
        return target->getMark("@zhenhao") - target->getMark("@weihao");
    }
};

class AzusaTrigger: public TriggerSkill {
public:
    AzusaTrigger(): TriggerSkill("#azusa-cardHandle") {
        events << EventPhaseEnd;
    }

    virtual QStringList triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (player->getPhase() == Player::Discard){
            room->setPlayerMark(player, "@weihao", 0);
            room->setPlayerMark(player, "@zhenhao", 0);
        }
        return QStringList();
    }
};

void MoesenPackage::addNovelGenerals()
{
    /*General *shana = new General(this, "shana", "qun", 3, false); // Novel 001

    General *louise = new General(this, "louise", "qun", 3, false); // Novel 002

    General *ruri = new General(this, "ruri", "qun", 3, false); // Novel 003

    General *tsukiko = new General(this, "tsukiko", "qun", 3, false); // Novel 004
    */
    General *a_azusa = new General(this, "a_azusa", "qun", 3, false); // Novel 005
    a_azusa->addSkill(new Weihao);
    a_azusa->addSkill(new Zhuyi);
    a_azusa->addSkill(new AzusaMaxCards);
    a_azusa->addSkill(new AzusaTrigger);

    /*
    General *yuki = new General(this, "yuki", "qun", 3, false); // Novel 006

    General *haruhi = new General(this, "haruhi", "qun", 3, false); // Novel 007

    General *watashi = new General(this, "watashi", "qun", 3, false); // Novel 008

    General *taiga = new General(this, "taiga", "qun", 3, false); // Novel 009

    General *aria = new General(this, "aria", "qun", 3, false); // Novel 010

    General *ruiko = new General(this, "ruiko", "qun", 3, false); // Novel 011

    General *mikoto = new General(this, "mikoto", "qun", 3, false); // Novel 012

    General *eru = new General(this, "eru", "qun", 3, false); // Novel 013

    General *asuna = new General(this, "asuna", "qun", 3, false); // Novel 014

    General *sena = new General(this, "sena", "qun", 3, false); // Novel 015

    General *hitagi = new General(this, "hitagi", "qun", 3, false); // Novel 016

    General *rikka = new General(this, "rikka", "qun", 3, false); // Novel 017

    General *holo = new General(this, "holo", "qun", 3, false); // Novel 018
    */
    addMetaObject<WeihaoCard>();
    addMetaObject<ZhuyiCard>();
}
