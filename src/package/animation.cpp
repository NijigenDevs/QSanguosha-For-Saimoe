#include "animation.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"

//巴麻美 烈枪，殁颅 -SE
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
        foreach (ServerPlayer *mami, mamis)
            if (mami->getKingdom() == use.from->getKingdom() && ((mami->isKongcheng() && mami->getEquips().length() == 0) ||
                (mami->isKongcheng() && mami->getJudgingArea().length() == 0) ||(mami->getEquips().length() == 0 && mami->getEquips().length() == 0)))
                skill_list.insert(mami, QStringList(objectName()));
        return skill_list;
    }

     virtual bool cost(TriggerEvent , Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        bool invoke = ask_who->hasShownSkill(this) ? true : room->askForSkillInvoke(ask_who, objectName(), data);
        if (invoke){
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
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
        frequency = Limited;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const {

        DyingStruct dying = data.value<DyingStruct>();
        if (player != dying.who || !player->hasSkill(objectName()) || player->getActualGeneral1Name().contains("sujiang"))
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$MoluAnimate", 3000);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {

        RecoverStruct recover;
        recover.recover = player->getLostHp();
        recover.who = player;
        room->recover(player, recover);
        player->removeGeneral(true);
        return false;
    }
};



//缨枪-Slob
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

//大圆脸 慈悲，仁悯 -SE
class Cibei: public TriggerSkill {
public:
    Cibei(): TriggerSkill("cibei") {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        //if (TriggerSkill::triggerable(player).isEmpty())
        //    return QStringList();

        DamageStruct damage = data.value<DamageStruct>();
        if (player != damage.from || !player->hasSkill(objectName()))
            return QStringList();

        player->tag["CibeiCurrentTarget"] = QVariant::fromValue(damage.to);
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName(), ask_who->tag.value("CibeiCurrentTarget"))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if (!target->isKongcheng()){
            int id = room->askForCardChosen(ask_who, target, "h", objectName());
            room->throwCard(id, target, ask_who);
        }
        if (target->getEquips().length() > 0){
            int id = room->askForCardChosen(ask_who, target, "e", objectName());
            room->throwCard(id, target, ask_who);
        }
        ask_who->tag.remove("CibeiCurrentTarget");
        return true;
    }
};

class Renmin: public TriggerSkill {
public:
    Renmin(): TriggerSkill("renmin") {
        events << CardsMoveOneTime;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from != player || move.card_ids.length() == 0 || move.from->isDead() ||move.to_place != Player::DiscardPile||
            ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_DISCARD)|| move.reason.m_playerId != move.from->objectName())//最后一句为歧义：该角色弃置的牌 我这里写的意思是该角色自行弃置的牌。
            return skill_list;

        QList<ServerPlayer *> madokas = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *madoka, madokas)
            if (madoka->getKingdom() == move.from->getKingdom() && move.from->getMark("@renmin_used") == 0)
                skill_list.insert(madoka, QStringList(objectName()));
        return skill_list;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *madoka = ask_who;

        if (madoka != NULL){
            if (madoka->askForSkillInvoke(objectName(), data)){
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *madoka = ask_who;
        if (madoka == NULL) return false;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        player->gainMark("@renmin_used");//
        CardsMoveStruct newmove = CardsMoveStruct();
        newmove.card_ids = move.card_ids;
        newmove.to = move.from;
        newmove.to_place = Player::PlaceHand;
        newmove.reason = CardMoveReason(CardMoveReason::S_REASON_RECYCLE, madoka->objectName(),move.from->objectName(), objectName(), objectName());
        room->moveCardsAtomic(newmove, true);
        return false;
    }
};

//无畏-Slob
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


//梓喵 劝勉，妙律 -SE
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

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const  {
        if (ask_who->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        ServerPlayer *target = room->askForPlayerChosen(ask_who, room->getAlivePlayers(), objectName() ,"@quanmian_draw");
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
    if (yui->objectName() != source->objectName())
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

// 秋山澪 音装 羞涩 -SE
class Yinzhuang: public TriggerSkill {
public:
    Yinzhuang(): TriggerSkill("yinzhuang") {
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *mio, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(mio)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == mio && move.from_places.contains(Player::PlaceEquip) && move.reason.m_reason != CardMoveReason::S_REASON_RESPONSE) {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const{
        bool invoke = mio->hasShownSkill(this) ? true : room->askForSkillInvoke(mio, objectName());
        if (invoke){
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer *) const{
        if (!mio) return false;
         CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
         foreach(int cardid, move.card_ids){
             Card *card = Sanguosha->getCard(cardid);
             if (card->isKindOf("Weapon")){
                 if (room->getDrawPile().length() == 0)
                     room->swapPile();
                 int cardid = room->getDrawPile().at(0);
                 room->showCard(mio, cardid);//BUG not showing card
                 room->obtainCard(room->askForPlayerChosen(mio, room->getAlivePlayers(), objectName(), "@yinzhuang_give"), cardid);
             }else if(card->isKindOf("Armor")){
                 Slash *slash = new Slash(Card::NoSuit, 0);
                 slash->setSkillName(objectName());
                 room->useCard(CardUseStruct(slash, mio, room->askForPlayerChosen(mio, room->getOtherPlayers(mio), objectName(), "@yinzhuang_slash")));
             }else if(card->isKindOf("Horse")){
                 mio->drawCards(1);
             }
         }
        return false;
    }
};

class Xiuse: public TriggerSkill {
public:
    Xiuse(): TriggerSkill("xiuse") {
        events << CardsMoveOneTime;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *mio, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(mio)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to == mio && move.to_place == Player::PlaceHand && mio->hasSkill(objectName()) && !mio->hasFlag("xiuse_used")) {//“获得”的定义需要斟酌
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const{
        if (room->askForSkillInvoke(mio, objectName())){
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer *) const{
        if (!mio) return false;
        mio->setFlags("xiuse_used");
        mio->drawCards(1);
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
    mio->addSkill(new Xiuse);

    /*General *yui = new General(this, "yui", "wei", 3, false); // Animation 008

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