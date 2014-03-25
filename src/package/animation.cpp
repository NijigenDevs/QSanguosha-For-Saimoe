#include "animation.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"

//Ó§Ç¹
class Yingqiang: public ViewAsSkill {
public:
    Yingqiang(): ViewAsSkill("yingqiang") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        foreach (const Card *card, selected){
            if (to_select->getSuit() == card->getSuit())
                return false;
        }
        return !to_select->isEquipped() && to_select->getSuit() != Card::NoSuit;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < 2 || cards.length() >4)
            return NULL;

        Card *slash = new Slash(cards.first()->getSuit(), cards.first()->getNumber());
        foreach (const Card *card, cards){
            slash->addSubcard(card->getId());
        }
        slash->setSkillName(objectName());
        slash->setShowSkill(objectName());
        return slash;
    }
};


class YingqiangTargetMod: public TargetModSkill {
public:
    YingqiangTargetMod(): TargetModSkill("#yingqiang-target") {
    }
    virtual int getDistanceLimit(const Player *from, const Card *card) const{
        if (card->getSkillName() == "yingqiang")
            return 1000;
        else
            return 0;
    }
};

class YingqiangEffect: public TriggerSkill {//TODO
public:
    YingqiangEffect(): TriggerSkill("#yingqiang-effect") {
        events << TargetConfirmed<<PreDamageDone << Damage<< CardFinished;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
       if (triggerEvent == TargetConfirmed){
            if (player != NULL){
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.card->getSkillName() != "yingqiang" || use.card==NULL)
                    return QStringList();
                QList<int> cardids = use.card->getSubcards();
                foreach (int cardid, cardids){
                    if (Sanguosha->getCard(cardid)->getSuit() == Card::Club || Sanguosha->getCard(cardid)->getSuit() == Card::Diamond){
                        return QStringList(objectName());
                    }
                }
            }
        }else if(triggerEvent == PreDamageDone){
            if (player != NULL){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *kyouko = damage.from;
            if (kyouko)
                kyouko->tag["InvokeKuanggu"] = (kyouko->distanceTo(damage.to) <= 1);
            }
            return QStringList();
       }else if(triggerEvent == Damage){
            if (player != NULL){
                DamageStruct damage = data.value<DamageStruct>();
                if (damage.card->getSkillName() != "yingqiang")
                    return QStringList();
                QList<int> cardids = damage.card->getSubcards();
                foreach (int cardid, cardids){
                    if (Sanguosha->getCard(cardid)->getSuit() == Card::Spade || Sanguosha->getCard(cardid)->getSuit() == Card::Heart){
                        return QStringList(objectName());
                    }
                }
            }else if (triggerEvent == CardFinished){
                CardUseStruct use = data.value<CardUseStruct>();
                ServerPlayer *kyouko = use.from;
                if (kyouko){
                    if (kyouko->hasFlag("yingqiangQinggang")){
                        room->setPlayerMark(kyouko, "Armor_Nullified", 0) ;
                    }
                    kyouko->tag["InvokeKuanggu"] = false;
                }
            }
       }
        return QStringList();
    }
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (triggerEvent == TargetConfirmed){
            room->broadcastSkillInvoke(objectName());
        }
        player->drawCards(1);//
        return true;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed){
            if (player != NULL){
                CardUseStruct use = data.value<CardUseStruct>();
                ServerPlayer *kyouko = use.from;
                if (use.card->getSkillName() != "yingqiang")
                    return false;
                QList<int> cardids = use.card->getSubcards();
                use.from->drawCards(1);//
                foreach (int cardid, cardids){
                    switch (Sanguosha->getCard(cardid)->getSuit()){
                        case Card::Club:
                            kyouko->setFlags("yingqiangQinggang");
                            room->setPlayerMark(kyouko, "Armor_Nullified", 1) ;
                            break;
                        case Card::Diamond:
                            use.from->drawCards(1);//
                            kyouko->drawCards(1);
                            break;
                        default:;
                    }
                }
            }
        }else if(triggerEvent == Damage){
            if (player != NULL){
                DamageStruct damage = data.value<DamageStruct>();
                ServerPlayer *kyouko = damage.from;
                if (damage.card->getSkillName() != "yingqiang")
                    return false;
                QList<int> cardids = damage.card->getSubcards();
                foreach (int cardid, cardids){
                    switch (Sanguosha->getCard(cardid)->getSuit()){
                        case Card::Spade:
                            if (kyouko->tag.value("InvokeKuanggu") == 1){
                                RecoverStruct recover;
                                recover.who = kyouko;
                                recover.recover = damage.damage;
                                room->recover(kyouko, recover);
                            }
                            break;
                        case Card::Heart:
                            damage.to->drawCards(damage.to->getHp() > 5? 5: damage.to->getHp());
                            damage.to->turnOver();
                            break;
                        default:;
                    }
                }
            }
       }

        return false;
    }
};

//ÎÞÎ·
WuweiCard::WuweiCard() {//todo
    will_throw = true;
    target_fixed = false;
}

bool WuweiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isCardLimited(Sanguosha->cloneCard("Slash"), MethodUse) && !to_select->isProhibited(Self, Sanguosha->cloneCard("Slash"), Self->getSiblings()) && Self->distanceTo(to_select) <= Self->getAttackRange();
}

void WuweiCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *sayaka = effect.from;
    sayaka->getRoom()->loseHp(sayaka);
    sayaka->getRoom()->broadcastSkillInvoke(objectName());
    int id = getSubcards().first();
    Slash *slash = new Slash(Sanguosha->getCard(id)->getSuit(), Sanguosha->getCard(id)->getNumber());
    slash->setSkillName("wuwei");
    slash->addSubcard(id);
    sayaka->getRoom()->useCard(CardUseStruct(slash, sayaka, effect.to), false);
}


class Wuwei: public OneCardViewAsSkill {
public:
    Wuwei(): OneCardViewAsSkill("wuwei") {

    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("WuweiCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        WuweiCard *wuwei = new WuweiCard();
        wuwei->addSubcard(originalCard);
        wuwei->setShowSkill(objectName());
        return wuwei;
    }
};

class WuweiEffect: public TriggerSkill{
public:
    WuweiEffect(): TriggerSkill("#wuwei-effect"){
        events <<DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent , Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (player != NULL){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *sayaka = damage.from;
            if (damage.card->getSkillName() == "wuwei" && sayaka){
                damage.damage = damage.damage + 1;
                data = QVariant::fromValue(damage);
            }
        }
        

        return QStringList();
    }
};
//´È±¯
class Cibei: public TriggerSkill {
public:
    Cibei(): TriggerSkill("cibei") {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (TriggerSkill::triggerable(player).isEmpty())
            return QStringList();

        DamageStruct damage = data.value<DamageStruct>();
        if (player != damage.from)
            return QStringList();

        player->tag["CibeiCurrentTarget"] = QVariant::fromValue(damage.to);
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const {
        if (player->askForSkillInvoke(objectName(), player->tag.value("CibeiCurrentTarget"))) {
            room->broadcastSkillInvoke(objectName(), 1);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if (!target->isKongcheng()){
            int id = room->askForCardChosen(player, target, "h", objectName());
            room->throwCard(id, target, player);
        }
        if (target->getEquips().length() > 0){
            int id = room->askForCardChosen(player, target, "e", objectName());
            room->throwCard(id, target, player);
        }
        player->tag.remove("CibeiCurrentTarget");
        return true;
    }
};


void MoesenPackage::addAnimationGenerals()
{
    General *mami = new General(this, "mami", "wei", 3, false); // Animation 001

    General *s_kyouko = new General(this, "s_kyouko", "wei", 4, false); // Animation 002

    s_kyouko->addSkill(new Yingqiang);
    s_kyouko->addSkill(new YingqiangTargetMod);
    s_kyouko->addSkill(new YingqiangEffect);
    related_skills.insertMulti("yingqiang", "#yingqiang-target");
    related_skills.insertMulti("yingqiang", "#yingqiang-effect");

    General *madoka = new General(this, "madoka", "wei", 3, false); // Animation 003

    madoka->addSkill(new Cibei);

    General *sayaka = new General(this, "sayaka", "wei", 4, false); // Animation 004

    sayaka->addSkill(new Wuwei);
    sayaka->addSkill(new WuweiEffect);
    related_skills.insertMulti("wuwei", "#wuwei-effect");

    General *homura = new General(this, "homura", "wei", 3, false); // Animation 005

    General *n_azusa = new General(this, "n_azusa", "wei", 3, false); // Animation 006

    General *mio = new General(this, "mio", "wei", 3, false); // Animation 007

    General *yui = new General(this, "yui", "wei", 3, false); // Animation 008

    General *kanade = new General(this, "kanade", "wei", 3, false); // Animation 009

    General *rei = new General(this, "rei", "wei", 3, false); // Animation 010

    General *asuka = new General(this, "asuka", "wei", 3, false); // Animation 011

    General *inori = new General(this, "inori", "wei", 3, false); // Animation 012

    General *nico = new General(this, "nico", "wei", 3, false); // Animation 013

    General *mayu = new General(this, "mayu", "wei", 3, false); // Animation 014

    General *lacus = new General(this, "lacus", "wei", 3, false); // Animation 015
    
    General *ayu = new General(this, "sawa", "wei", 3, false); // Animation 016

    General *sawa = new General(this, "astarotte", "wei", 3, false); // Animation 017

    General *astarotte = new General(this, "miho", "wei", 3, false); // Animation 018


    addMetaObject<WuweiCard>();

}