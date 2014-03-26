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
        return !to_select->hasFlag("using") && !to_select->isEquipped() && to_select->getSuit() != Card::NoSuit;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < 2 || cards.length() > 4)
            return NULL;

        Card *slash = new Slash(Card::SuitToBeDecided, -1);
        slash->addSubcards(cards);
        slash->setSkillName(objectName());
        slash->setShowSkill(objectName());
        return slash;
    }
};

class YingqiangSpade: public TriggerSkill {
public:
    YingqiangSpade(): TriggerSkill("yingqiang_spade") {
        events << PreDamageDone << Damage;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (triggerEvent == PreDamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->getSkillName() == "yingqiang" && damage.from)
                damage.from->tag["InvokeYingqiangSpade"] = (damage.from->distanceTo(player) <= 1);
        } else if(triggerEvent == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from->isAlive() && damage.card && damage.card->getSkillName() == "yingqiang")
                foreach (int cardid, damage.card->getSubcards())
                    if (Sanguosha->getCard(cardid)->getSuit() == Card::Spade && player->isWounded()) {
                        QStringList skill_list;
                        for (int i = 0; i < damage.damage; i++)
                            skill_list << objectName();
                        return skill_list;
                    }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->notifySkillInvoked(player, "yingqiang");
        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);

        return false;
    }
};

class YingqiangHeart: public TriggerSkill {
public:
    YingqiangHeart(): TriggerSkill("yingqiang_heart") {
        events << Damage;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from->isAlive() && damage.card && damage.card->getSkillName() == "yingqiang" && damage.to->isAlive())
            foreach (int cardid, damage.card->getSubcards())
                if (Sanguosha->getCard(cardid)->getSuit() == Card::Heart)
                    return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        room->notifySkillInvoked(player, "yingqiang");
        DamageStruct damage = data.value<DamageStruct>();
        int n = qMin(qMax(damage.to->getHp(), 0), 5);
        if (n)
            damage.to->drawCards(n);
        damage.to->turnOver();
        return false;
    }
};

class YingqiangClub: public TriggerSkill {
public:
    YingqiangClub(): TriggerSkill("yingqiang_club") {
        events << TargetChosen;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from->isAlive() && use.card && use.card->getSkillName() == "yingqiang")
            foreach (int cardid, use.card->getSubcards())
                if (Sanguosha->getCard(cardid)->getSuit() == Card::Club)
                    return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        room->notifySkillInvoked(player, "yingqiang");
        CardUseStruct use = data.value<CardUseStruct>();
        foreach (ServerPlayer *p, use.to.toSet()){
            p->addQinggangTag(use.card);
        }
        return false;
    }
};

class YingqiangDiamond: public TriggerSkill {
public:
    YingqiangDiamond(): TriggerSkill("yingqiang_diamond") {
        events << TargetChosen;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from && use.from == player && player->isAlive() && use.card && use.card->getSkillName() == "yingqiang")
            foreach (int cardid, use.card->getSubcards())
                if (Sanguosha->getCard(cardid)->getSuit() == Card::Diamond)
                    return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        room->notifySkillInvoked(player, "yingqiang");
        player->drawCards(1);
        return false;
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

//ÎÞÎ·
WuweiCard::WuweiCard() {
}

bool WuweiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Card *card = Sanguosha->getCard(getSubcards().first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->setSkillName("wuwei");
    slash->addSubcard(card);
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

const Card *WuweiCard::validate(CardUseStruct &use) const{
    ServerPlayer *sayaka = use.from;
    Room *room = sayaka->getRoom();
    room->loseHp(sayaka);
    room->broadcastSkillInvoke(objectName());
    Card *card = Sanguosha->getCard(getSubcards().first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->setSkillName("wuwei");
    slash->addSubcard(card);
    return slash;
}

class WuweiViewAsSkill: public OneCardViewAsSkill {
public:
    WuweiViewAsSkill(): OneCardViewAsSkill("wuwei") {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return !player->hasUsed("WuweiCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        WuweiCard *wuwei = new WuweiCard();
        wuwei->addSubcard(originalCard);
        wuwei->setShowSkill(objectName());
        return wuwei;
    }
};

class Wuwei: public TriggerSkill{
public:
    Wuwei(): TriggerSkill("wuwei"){
        events << DamageCaused;
        view_as_skill = new WuweiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent , Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card->getSkillName() == "wuwei") {
            return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        damage.damage ++;
        data = QVariant::fromValue(damage);
        return false;
    }
};

//´È±¯
class Cibei: public TriggerSkill {
public:
    Cibei(): TriggerSkill("cibei") {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        //if (TriggerSkill::triggerable(player).isEmpty())
        //    return QStringList();

        DamageStruct damage = data.value<DamageStruct>();
        if (player != damage.from)
            return QStringList();

        player->tag["CibeiCurrentTarget"] = QVariant::fromValue(damage.to);
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const {
        if (player->askForSkillInvoke(objectName(), player->tag.value("CibeiCurrentTarget"))) {
            room->broadcastSkillInvoke(objectName());
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

//è÷ß÷ È°Ãã-ÃîÂÉ
class Quanmian: public TriggerSkill {
public:
    Quanmian(): TriggerSkill("quanmian") {
        events << CardUsed;
		frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        //if (TriggerSkill::triggerable(player).isEmpty())
        //    return QStringList();

        CardUseStruct use = data.value<CardUseStruct>();
		if (use.card->isKindOf("EquipCard") && use.from->objectName() == player->objectName() && player->hasSkill(objectName())){
			return QStringList(objectName());
		}
        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const {
        if (player->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
		ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
		target->drawCards(1);
		return false;
    }
};

MiaolvCard::MiaolvCard() {
}

bool MiaolvCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
	return targets.isEmpty() && to_select->getEquips().length() > 0;
}

void MiaolvCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *yui = targets.first();
	int cardid = room->askForCardChosen(source, yui, "e", objectName());
	yui->obtainCard(Sanguosha->getCard(cardid));
	if (yui->objectName() == source->objectName())
		source->drawCards(1);
}


class Miaolv: public ZeroCardViewAsSkill {
public:
    Miaolv(): ZeroCardViewAsSkill("miaolv") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MiaolvCard");
    }

    virtual const Card *viewAs() const{
        MiaolvCard *card = new MiaolvCard;
        card->setShowSkill(objectName());
        return card;
    }
};

void MoesenPackage::addAnimationGenerals()
{
    //General *mami = new General(this, "mami", "wei", 3, false); // Animation 001

    General *s_kyouko = new General(this, "s_kyouko", "wei", 4, false); // Animation 002
    s_kyouko->addSkill(new Yingqiang);
    s_kyouko->addSkill(new YingqiangTargetMod);
    related_skills.insertMulti("yingqiang", "#yingqiang-target");
    skills << new YingqiangSpade
           << new YingqiangHeart
           << new YingqiangClub
           << new YingqiangDiamond;

    General *madoka = new General(this, "madoka", "wei", 3, false); // Animation 003
    madoka->addSkill(new Cibei);

    General *sayaka = new General(this, "sayaka", "wei", 4, false); // Animation 004
    sayaka->addSkill(new Wuwei);

    //General *homura = new General(this, "homura", "wei", 3, false); // Animation 005

    General *n_azusa = new General(this, "n_azusa", "wei", 3, false); // Animation 006
	n_azusa->addSkill(new Quanmian);
	n_azusa->addSkill(new Miaolv);

    /*General *mio = new General(this, "mio", "wei", 3, false); // Animation 007

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
    */

    addMetaObject<WuweiCard>();
	addMetaObject<MiaolvCard>();
}