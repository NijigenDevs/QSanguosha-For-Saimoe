#include "animation.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"
//this file is the general skills for animation.

//mami    lieqiang, molu -SE
class Lieqiang: public TriggerSkill {
public:
    Lieqiang(): TriggerSkill("lieqiang") {
        frequency = Compulsory;
        events << TargetChosen;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card || !use.card->isKindOf("Slash") || !use.to.contains(player))
            return skill_list;

        QList<ServerPlayer *> mamis = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *mami, mamis) {
            int n = 0;
            if (mami->isKongcheng()) n++;
            if (!mami->hasEquip()) n++;
            if (mami->getJudgingArea().isEmpty()) n++;
            if (n >= 2 && mami->isFriendWith(use.from))
                skill_list.insert(mami, QStringList(objectName()));
        }
        return skill_list;
    }

     virtual bool cost(TriggerEvent , Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        bool invoke = ask_who->hasShownSkill(this) ? true : ask_who->askForSkillInvoke(objectName(), data);
        if (invoke) {
            if (ask_who->hasShownSkill(this)) {
                room->notifySkillInvoked(ask_who, objectName());
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = ask_who;
                log.arg = objectName();
                room->sendLog(log);
            }
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardUseStruct use = data.value<CardUseStruct>();
        QVariantList jink_list = use.from->tag["Jink_" + use.card->toString()].toList();

        doLieqiang(player, use, jink_list);

        use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }

private:
    static void doLieqiang(ServerPlayer *target, CardUseStruct use, QVariantList &jink_list) {
        int index = use.to.indexOf(target);
        LogMessage log;
        log.type = "#NoJink";
        log.from = target;
        target->getRoom()->sendLog(log);
        jink_list.replace(index, QVariant(0));
    }
};

class Molu: public TriggerSkill {
public:
    Molu(): TriggerSkill("molu") {
        events << AskForPeaches;
        relate_to_place = "deputy";
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const {
        DyingStruct dying = data.value<DyingStruct>();
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        if (player != dying.who || player->getActualGeneral1Name().contains("sujiang"))
            return QStringList();
        if (player->getHp() > 0)
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$MoluAnimate", 3000);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        RecoverStruct recover;
        recover.recover = player->getMaxHp() - player->getHp();
        recover.who = player;
        room->recover(player, recover);
        player->removeGeneral(true);
        return false;
    }
};

//yingqiang-Slob
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
        if (use.from && use.from->isAlive() && use.from == player && use.card && use.card->getSkillName() == "yingqiang" )
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

//madoka    cibei, renmin -SE
class Cibei: public TriggerSkill {
public:
    Cibei(): TriggerSkill("cibei") {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (player->canDiscard(damage.to, "he"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (player->askForSkillInvoke(objectName(), QVariant::fromValue(damage.to))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        DummyCard *dummy = new DummyCard;
        dummy->deleteLater();
        if (player->canDiscard(target, "h")) {
            int id = room->askForCardChosen(player, target, "h", objectName(), false, Card::MethodDiscard);
            dummy->addSubcard(id);
        }
        if (player->canDiscard(target, "e")){
            int id = room->askForCardChosen(player, target, "e", objectName(), false, Card::MethodDiscard);
            dummy->addSubcard(id);
        }
        if (!dummy->getSubcards().isEmpty())
            room->throwCard(dummy, target, player);
        return true;
    }
};

class Renmin: public TriggerSkill {
public:
    Renmin(): TriggerSkill("renmin") {
        events << CardsMoveOneTime;
    }

    virtual QStringList triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        bool can_invoke = false;
        foreach (int id, move.card_ids)
            if (room->getCardPlace(id) == Player::DiscardPile) {
                can_invoke = true;
                break;
            }
        if (!can_invoke || !move.from || move.from->isDead() || move.to_place != Player::DiscardPile ||
            ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_DISCARD))
            return QStringList();
        if (player->isFriendWith(move.from) && move.from->getMark("renmin"+player->objectName()) == 0)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName(), data)){
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *target = room->findPlayer(move.from->objectName());
        room->addPlayerMark(target, "@renmin_used");
        room->addPlayerMark(target, "renmin"+player->objectName());
        CardsMoveStruct newmove(move.card_ids, move.from, Player::PlaceHand, CardMoveReason(CardMoveReason::S_REASON_RECYCLE,
                                                                                            player->objectName(),
                                                                                            objectName(),
                                                                                            QString()));
        room->moveCardsAtomic(newmove, true);
        return false;
    }
};

//wuwei-Slob
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
        events << DamageCaused << PreCardUsed;
        view_as_skill = new WuweiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == PreCardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->getSkillName() == "wuwei" && use.from != NULL){
                room->addPlayerHistory(player, use.card->getClassName(), -1);
                use.m_addHistory = false;
                data = QVariant::fromValue(use);
            }
        }
        else {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card->getSkillName() == "wuwei") {
                return QStringList(objectName());
            }
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


//quanmian, miaolv -SE
class Quanmian: public TriggerSkill {
public:
    Quanmian(): TriggerSkill("quanmian") {
        events << CardUsed;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("EquipCard"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const  {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName() ,"@quanmian_draw", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["Quanmian"] = QVariant::fromValue(target);
            return true;
        }
        player->tag.remove("Quanmian");
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["Quanmian"].value<PlayerStar>();
        if (target)
            target->drawCards(1);
        return false;
    }
};

MiaolvCard::MiaolvCard() {
}

bool MiaolvCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasEquip();
}

void MiaolvCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *yui = targets.first();
    int cardid = room->askForCardChosen(source, yui, "e", objectName());
    yui->obtainCard(Sanguosha->getCard(cardid));
    if (yui != source)
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

// yinzhuang, xiuse -SE
class Yinzhuang: public TriggerSkill {
public:
    Yinzhuang(): TriggerSkill("yinzhuang") {
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *mio, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(mio)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == mio && move.from_places.contains(Player::PlaceEquip)
            && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_RESPONSE) {
            QStringList skill_list;
            for (int i = 0; i < move.card_ids.length(); i ++)
                if (move.from_places[i] == Player::PlaceEquip) {
                    const EquipCard *equip = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(move.card_ids[i]));
                    switch (equip->location()) {
                        case EquipCard::WeaponLocation :
                            skill_list << "#yinzhuang-weapon";
                            break;
                        case EquipCard::ArmorLocation :
                            skill_list << "#yinzhuang-armor";
                            break;
                        case EquipCard::DefensiveHorseLocation :
                        case EquipCard::OffensiveHorseLocation :
                            skill_list << "#yinzhuang-horse";
                            break;
                        default :
                            break;
                    }
                }
            return skill_list;
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const{
        return false;
    }
};

class YinzhuangWeapon: public TriggerSkill {
public:
    YinzhuangWeapon(): TriggerSkill("#yinzhuang-weapon") {
        events << NonTrigger;
        frequency = Compulsory;
    }
    
    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const{
        bool invoke = mio->hasShownSkill(this) ? true : room->askForSkillInvoke(mio, objectName());
        if (invoke) {
            room->broadcastSkillInvoke("yinzhuang");
            if (mio->hasShownSkill(this)) {
                room->notifySkillInvoked(mio, "yinzhuang");

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mio;
                log.arg = "yinzhuang";
                room->sendLog(log);
            } else
                mio->showGeneral(mio->inHeadSkills("yinzhuang"));
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer *) const{
        int cardid = room->drawCard();
        CardsMoveStruct move(cardid, NULL, Player::PlaceTable,
                             CardMoveReason(CardMoveReason::S_REASON_TURNOVER, mio->objectName(), "yinzhuang", QString()));
        room->moveCardsAtomic(move, true);
        ServerPlayer *target = room->askForPlayerChosen(mio, room->getAlivePlayers(), "yinzhuang", "@yinzhuang_give");
        CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, mio->objectName(), target->objectName(), "yinzhuang", QString());
        room->obtainCard(target, Sanguosha->getCard(cardid), reason);

        return false;
    }
};

class YinzhuangArmor: public TriggerSkill {
public:
    YinzhuangArmor(): TriggerSkill("#yinzhuang-armor") {
        events << NonTrigger;
        frequency = Compulsory;
    }
    
    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const{
        bool invoke = mio->hasShownSkill(this) ? true : room->askForSkillInvoke(mio, objectName());
        if (invoke) {
            room->broadcastSkillInvoke("yinzhuang");
            if (mio->hasShownSkill(this)) {
                room->notifySkillInvoked(mio, "yinzhuang");

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mio;
                log.arg = "yinzhuang";
                room->sendLog(log);
            } else
                mio->showGeneral(mio->inHeadSkills("yinzhuang"));
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer *) const{
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_"+objectName());
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (mio->canSlash(p, slash, false))
                targets << p;
        if (!targets.isEmpty()) {
            ServerPlayer *target = room->askForPlayerChosen(mio, targets, objectName(), "@yinzhuang_slash");
            room->useCard(CardUseStruct(slash, mio, target), false);
        }

        return false;
    }
};

class YinzhuangHorse: public TriggerSkill {
public:
    YinzhuangHorse(): TriggerSkill("#yinzhuang-horse") {
        events << NonTrigger;
        frequency = Compulsory;
    }
    
    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const{
        bool invoke = mio->hasShownSkill(this) ? true : room->askForSkillInvoke(mio, objectName());
        if (invoke) {
            room->broadcastSkillInvoke("yinzhuang");
            if (mio->hasShownSkill(this)) {
                room->notifySkillInvoked(mio, "yinzhuang");

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mio;
                log.arg = "yinzhuang";
                room->sendLog(log);
            } else
                mio->showGeneral(mio->inHeadSkills("yinzhuang"));
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer *) const{
        mio->drawCards(1);
        return false;
    }
};

class Xiuse: public TriggerSkill {
public:
    Xiuse(): TriggerSkill("xiuse") {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(mio)) return QStringList();
        ServerPlayer *current = room->getCurrent();
        if (!current || current->isDead() || current->getPhase() == Player::NotActive) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to == mio && move.to_place == Player::PlaceHand && !current->hasFlag("xiuse_used")) {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const{
        if (mio->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer *) const{
        room->setPlayerFlag(room->getCurrent(), "xiuse_used");
        mio->drawCards(1);
        return false;
    }
};

//yinren by Fsu0413
class Yinren: public TriggerSkill{
public:
    Yinren(): TriggerSkill("yinren"){
        events << EventPhaseStart << Damage;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const{
        if (triggerEvent == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && (player->getPhase() == Player::Start || player->getPhase() == Player::Finish) && !player->isKongcheng()){
                return QStringList(objectName());
            }
        }
        else if (triggerEvent == Damage){
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->getSkillName() == "yinren" && damage.from && !damage.from->isKongcheng() && damage.from->isAlive()){
                ask_who = damage.to;
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        if (triggerEvent == EventPhaseStart){
            //broadcastskillinvoke
            return player->askForSkillInvoke(objectName());
        }
        else {
            if (ask_who){
                return ask_who->askForSkillInvoke(objectName(), "gainacard");
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        if (triggerEvent == EventPhaseStart){
            room->showAllCards(player);
            QList<Card::Suit> suit_list;
            bool duplicate = false;
            foreach (const Card *c, player->getHandcards()){
                Card::Suit s = c->getSuit();
                if (!suit_list.contains(s))
                    suit_list << s;
                else {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate){
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName("yinren");
                QList<ServerPlayer *> can_slashers;
                foreach (ServerPlayer *p, room->getOtherPlayers(player)){
                    if (player->canSlash(p, slash, false)){
                        can_slashers << p;
                    }
                }
                if (can_slashers.isEmpty()){
                    delete slash;
                    //log
                    return false;
                }
                ServerPlayer *slasher = room->askForPlayerChosen(player, can_slashers, objectName(), "@yinren-slash", false, true);
                room->useCard(CardUseStruct(slash, player, slasher));
            }
        }
        else {
            if (ask_who){
                DamageStruct damage = data.value<DamageStruct>();
                int id = room->askForCardChosen(ask_who, damage.from, "h", objectName());
                ask_who->obtainCard(Sanguosha->getCard(id), false);
            }
        }
        return false;
    }
};

class Tongxin: public TriggerSkill{
public:
    Tongxin(): TriggerSkill("tongxin"){
        events << Death << GameStart;
        frequency = Limited;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (triggerEvent == GameStart){
            if (player->ownSkill(objectName())){
                room->setPlayerProperty(player, "tongxin", 1);
            }
        }
        else if ((triggerEvent == Death) && (player != NULL && player->isDead())){
            foreach (ServerPlayer *p, room->getAllPlayers(true)){
                if (p->isDead()){
                    bool ok = false;
                    int tongxin = p->property("tongxin").toInt(&ok);
                    if (ok && tongxin > 0){
                        DeathStruct death = data.value<DeathStruct>();
                        if (death.who->isFriendWith(p) && death.who != p){
                            ask_who = p;
                            return QStringList(objectName());
                        }
                    }
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        if (ask_who->askForSkillInvoke(objectName(), data)){
            //animate && broadcastskillinvoke
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        room->setPlayerProperty(ask_who, "hp", player->getHp() + 2); //don't use the normal recover method
        room->revivePlayer(ask_who);
        room->handleAcquireDetachSkills(ask_who, "-yinren");
        ask_who->drawCards(2);

        room->setPlayerProperty(ask_who, "tongxin", 0);

        return false;
    }
};

void MoesenPackage::addAnimationGenerals()
{
    General *mami = new General(this, "mami", "wei", 4, false); // Animation 001
    mami->addSkill(new Lieqiang);
    mami->addSkill(new Molu);

    General *s_kyouko = new General(this, "s_kyouko", "wei", 4, false); // Animation 002
    s_kyouko->addCompanion("sayaka");
    s_kyouko->addSkill(new Yingqiang);
    s_kyouko->addSkill(new YingqiangTargetMod);
    related_skills.insertMulti("yingqiang", "#yingqiang-target");
    skills << new YingqiangSpade
           << new YingqiangHeart
           << new YingqiangClub
           << new YingqiangDiamond;

    General *madoka = new General(this, "madoka", "wei", 4, false); // Animation 003
    madoka->addSkill(new Cibei);
    madoka->addSkill(new Renmin);

    General *sayaka = new General(this, "sayaka", "wei", 4, false); // Animation 004
    sayaka->addSkill(new Wuwei);

    //General *homura = new General(this, "homura", "wei", 3, false); // Animation 005

    General *n_azusa = new General(this, "n_azusa", "wei", 3, false); // Animation 006
    n_azusa->addSkill(new Quanmian);
    n_azusa->addSkill(new Miaolv);

    General *mio = new General(this, "mio", "wei", 3, false); // Animation 007
    mio->addSkill(new Yinzhuang);
    mio->addSkill(new YinzhuangWeapon);
    mio->addSkill(new YinzhuangArmor);
    mio->addSkill(new YinzhuangHorse);
    related_skills.insertMulti("yinzhuang", "#yinzhuang-weapon");
    related_skills.insertMulti("yinzhuang", "#yinzhuang-armor");
    related_skills.insertMulti("yinzhuang", "#yinzhuang-horse");
    mio->addSkill(new Xiuse);

    //General *yui = new General(this, "yui", "wei", 3, false); // Animation 008

    General *kanade = new General(this, "kanade", "wei", 3, false); // Animation 009
    kanade->addSkill(new Yinren);
    kanade->addSkill(new Tongxin);

    /*
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