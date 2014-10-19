#include "game.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"

//haixing & taozui by SE
HaixingCard::HaixingCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool HaixingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    return to_select != Self;
}

void HaixingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "haixing", QString());
    room->obtainCard(target, this, reason, false);

    int old_value = source->getMark("haixing");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "haixing", new_value);
    if (!target->hasFlag("haixing_used"))
        target->setFlags("haixing_used");
}

class HaixingViewAsSkill: public ViewAsSkill {
public:
    HaixingViewAsSkill(): ViewAsSkill("haixing") {
        response_pattern = "@@haixing";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        HaixingCard *card = new HaixingCard;
        card->addSubcards(cards);
        card->setShowSkill(objectName());
        return card;
    }
};


class Haixing: public TriggerSkill {
public:
    Haixing(): TriggerSkill("haixing") {
        events << EventPhaseEnd;
        frequency = NotFrequent;
        view_as_skill = new HaixingViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == EventPhaseEnd && player->getPhase() == Player::Draw)
            if (TriggerSkill::triggerable(player))
                return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *fuuko, QVariant &, ServerPlayer *) const{
       do{
           room->askForUseCard(fuuko, "@@haixing", "@haixing-card");
       }while(!fuuko->isKongcheng() && fuuko->askForSkillInvoke(objectName()));
       QVariant data = QVariant::fromValue(fuuko);
       foreach (ServerPlayer *p,room->getAlivePlayers()){
           if (p->hasFlag("haixing_used")){
               room->obtainCard(fuuko, room->askForCard(p, ".", "@haixing-back", data, Card::MethodNone,fuuko,false,"haixing"),CardMoveReason::S_REASON_GIVE);
               p->setFlags("-haixing_used");
           }
       }
       if (fuuko->getMark("haixing") > 1){
           ServerPlayer *t = room->askForPlayerChosen(fuuko, room->getAlivePlayers(), "haixing_recover");
           RecoverStruct recover;
           recover.who = t;
           room->recover(t, recover);
       }
       fuuko->setMark("haixing", 0);
       return false;
    }
};

class Taozui: public TriggerSkill {
public:
    Taozui(): TriggerSkill("taozui") {
        events << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Start){
                 foreach (const Card *c, player->getJudgingArea()){
                    if (c->isKindOf("Indulgence"))
                        return QStringList();
                }
                foreach (ServerPlayer *p, room->getOtherPlayers(player)){
                    if (!p->getJudgingArea().isEmpty()){
                        foreach (const Card *c, p->getJudgingArea()){
                            if (c->isKindOf("Indulgence"))
                                return QStringList(objectName());
                        }
                    }
                }
                foreach(const Card *c, player->getJudgingArea()){
                    if (c->isKindOf("SupplyShortage"))
                        return QStringList(objectName());
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
       if (triggerEvent == EventPhaseStart){
            if (player->askForSkillInvoke(objectName())){
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *fuuko, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == EventPhaseStart){
            fuuko->skip(Player::Play);
            bool dis =  false, get = false;
            const Card *to_do;
            foreach(const Card *c, fuuko->getJudgingArea()){
                    if (c->isKindOf("SupplyShortage")){
                        dis = true;
                        to_do = c;
                    }
             }


            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *p, room->getOtherPlayers(fuuko)){
                if (!p->getJudgingArea().isEmpty()){
                    foreach(const Card *card, p->getJudgingArea()){
                        if (card->isKindOf("Indulgence")){
                            targets.append(p);
                            get = true;
                        }
                    }
                }
            }
            QString choice = "taozui_get";
            if (dis && get){
                choice = room->askForChoice(fuuko, objectName(), "taozui_get+taozui_discard");
            }else if(dis && !get){
                choice = "taozui_discard";
            }

            if (choice == "taozui_discard"){
                room->throwCard(to_do, CardMoveReason(CardMoveReason::S_REASON_DISCARD, fuuko->objectName(), "taozui", QString()), fuuko, fuuko);
            }else{
                if (targets.isEmpty())
                    return false;
                ServerPlayer *target = room->askForPlayerChosen(fuuko, targets, "taozui_from");
                foreach(const Card *card,target->getJudgingArea()){
                    if (card->isKindOf("Indulgence"))
                        to_do = card;
                }
                if (!to_do)
                    return false;
                room->obtainCard(fuuko, to_do,CardMoveReason(CardMoveReason::S_REASON_GOTCARD, fuuko->objectName(), "taozui", QString()));
            }
        }
        return false;
    }
};

//yuanqi by SE
class Yuanqi: public TriggerSkill {
public:
    Yuanqi(): TriggerSkill("yuanqi") {
        events << EventPhaseStart << EventPhaseChanging;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Start){
                foreach (ServerPlayer *p, room->getOtherPlayers(player)){
                    if (!p->getJudgingArea().isEmpty()){
                        foreach (const Card *c, p->getJudgingArea()){
                            if (player->getJudgingArea().isEmpty())
                                return QStringList(objectName());
                            bool good = true;
                            foreach(const Card *pc, player->getJudgingArea()){
                                if (pc->getClassName() == c->getClassName())
                                    good = false;
                            }
                            if (good)
                                return QStringList(objectName());
                        }
                    }
                }
            }
        }else if(triggerEvent == EventPhaseChanging){
            if (data.value<PhaseChangeStruct>().to == Player::Judge && player->hasFlag("yuanqi_used")){
                player->setFlags("-yuanqi_used");
                player->skip(Player::Judge);
                player->insertPhase(Player::Play);
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                change.to = Player::Play;
                data.setValue(change);
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
       if (triggerEvent == EventPhaseStart){
            if (player->askForSkillInvoke(objectName())){
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *haruka, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == EventPhaseStart){
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(haruka)){
                if (!p->getJudgingArea().isEmpty()){
                    foreach (const Card *c, p->getJudgingArea()){
                        if (haruka->getJudgingArea().isEmpty())
                            targets.append(p);
                        else{
                            bool good = true;
                            foreach(const Card *pc, haruka->getJudgingArea()){
                                if (pc->getClassName() == c->getClassName())
                                    good = false;
                            }
                            if (good)
                                if (!targets.contains(p))
                                    targets.append(p);
                        }
                    }
                }
            }
            if (targets.isEmpty())
                return false;
            ServerPlayer *target = room->askForPlayerChosen(haruka, targets, objectName());
            QList<int> cardids;
            foreach (const Card *c, target->getJudgingArea()){
                if (haruka->getJudgingArea().isEmpty())
                    cardids.append(c->getEffectiveId());
                else{
                    bool good = true;
                    foreach(const Card *pc, haruka->getJudgingArea()){
                        if (pc->getClassName() == c->getClassName())
                            good = false;
                    }
                    if (good)
                        cardids.append(c->getEffectiveId());
                }
            }
            room->fillAG(cardids, haruka);
            int cardid = room->askForAG(haruka, cardids, false, objectName());
            room->clearAG(haruka);
            if (cardid < 0)
                return false;
            room->moveCardTo(Sanguosha->getCard(cardid), target, haruka, room->getCardPlace(cardid),
                             CardMoveReason(CardMoveReason::S_REASON_TRANSFER,
                                            haruka->objectName(), objectName(), QString()));//not working good
            haruka->setFlags("yuanqi_used");
        }
        return false;
    }
};

//daihei by SE
class Daihei: public TriggerSkill {
public:
    Daihei(): TriggerSkill("daihei") {
        events << EventPhaseStart;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish){
                int length = player->getHandcardNum() + player->getEquips().length() + player->getJudgingArea().length();
                foreach (ServerPlayer *p, room->getOtherPlayers(player)){
                    if (p->getHandcardNum() + p->getEquips().length() + p->getJudgingArea().length() > length)
                        return QStringList();
                }
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
       bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke){
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *haruka, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == EventPhaseStart){
            room->drawCards(haruka,3);
            haruka->turnOver();
        }
        return false;
    }
};

//xiaying by SE
XiayingCard::XiayingCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void XiayingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = room->getCurrent();

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "xiaying", QString());
    room->obtainCard(target, this, reason, false);
	int new_value = target->getMark("xiaying") + subcards.length();
    room->setPlayerMark(target, "xiaying", new_value);
}

class XiayingViewAsSkill: public ViewAsSkill {
public:
    XiayingViewAsSkill(): ViewAsSkill("xiayingVS") {
        response_pattern = "@@xiaying";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        XiayingCard *card = new XiayingCard;
        card->addSubcards(cards);
        card->setShowSkill(objectName());
        return card;
    }
};


class Xiaying: public TriggerSkill {
public:
    Xiaying(): TriggerSkill("xiaying") {
    view_as_skill = new XiayingViewAsSkill;
    events << GeneralShown << GeneralHidden << GeneralRemoved << Death << EventPhaseStart;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player == NULL) return QStringList();
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who->hasSkill(objectName())) {
                foreach (ServerPlayer *p, room->getAllPlayers())
                    if (p->getMark("xiaying") > 0) {
                        room->detachSkillFromPlayer(p, "xiayingVS", true, true);
                    }
                return QStringList();
            }
            else {
                if (death.who->getMark("xiaying") > 0){
                    room->setPlayerMark(death.who, "xiaying", 0);
                    room->detachSkillFromPlayer(death.who, "xiayingVS", true, true);
                }
            }
        }else if(triggerEvent == EventPhaseStart){
            if (player->getPhase() == Player::Start && player->hasSkill(objectName())){
                return QStringList(objectName());
            }
            return QStringList();
        }
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if (p->getMark("xiaying") > 0) {
                room->setPlayerMark(p, "xiaying", 0);
                room->detachSkillFromPlayer(p, "xiayingVS", true, true);
            }
        }
        QList<ServerPlayer *> misuzus = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *misuzu, misuzus){
            if (misuzu->hasShownSkill(this)) {
                foreach(ServerPlayer *p, room->getOtherPlayers(misuzu)){
                    if (misuzu->isFriendWith(p)) {
                        room->setPlayerMark(p, "xiaying", 1);
                        room->attachSkillToPlayer(p, "xiayingVS");
                    }
                }
            }
        }
        return QStringList();
    }

   virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
       bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke){
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *misuzu, QVariant &, ServerPlayer *) const{
        foreach (ServerPlayer * p, room->getOtherPlayers(misuzu)){
            if (p->isFriendWith(misuzu)){
                 room->askForUseCard(p, "@@xiaying", "@xiaying-card");
            }
        }
       if (misuzu->getMark("xiaying") <= 2){
           room->loseHp(misuzu, 1);
       }
       misuzu->setMark("xiaying", 0);
       return false;
    }
};

//yumeng by SE
YumengDraw::YumengDraw(): DrawCardsSkill("yumeng-draw") {
    frequency = Compulsory;
    global = true;
}

bool YumengDraw::cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
    foreach(ServerPlayer* p, player->getRoom()->getAlivePlayers())
        if (p->getMark("yumeng_use") > 0 && p->hasShownOneGeneral() && p->isFriendWith(player))
            return true;
    return false;
}

int YumengDraw::getDrawNum(ServerPlayer *player, int n) const{
    int m = 0;
    foreach(ServerPlayer* p, player->getRoom()->getAlivePlayers())
        if (p->getMark("yumeng_use") > 0 && p->hasShownOneGeneral() && p->isFriendWith(player))
            m+=p->getMark("yumeng_use");
    if (m > 0)
        player->getRoom()->broadcastSkillInvoke("yumeng");
    return n + m;
}

class Yumeng: public TriggerSkill {
public:
    Yumeng(): TriggerSkill("yumeng") {
        frequency = Compulsory;
        events << EventPhaseStart;
    }

    virtual bool canPreshow() const{
        return true;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player != NULL && player->getPhase() == Player::Draw) {
            QList<ServerPlayer *> misuzus = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *misuzu, misuzus)
                if (misuzu->isFriendWith(player) && misuzu->getLostHp() > 0)
                    skill_list.insert(misuzu, QStringList(objectName()));
        }
        return skill_list;
    }

     virtual bool cost(TriggerEvent , Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
         bool invoke = ask_who->hasShownSkill(this) ? true : room->askForSkillInvoke(ask_who, objectName());
        if (invoke){
            return true;
        }

        return false;
    }
    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        if (ask_who->getHp() < ask_who->getMaxHp()){
            int mark = ask_who->getMaxHp()-ask_who->getHp() > 2? 2:ask_who->getMaxHp()-ask_who->getHp();
            ask_who->setMark("yumeng_use",mark);
        }else
            ask_who->setMark("yumeng_use", 0);
        return false;
    }
};

void MoesenPackage::addGameGenerals()
{
    /*General *nagisa = new General(this, "nagisa", "wu", 3, false); // Game 001
    */

    General *fuuko = new General(this, "fuuko", "wu", 3, false); // Game 002
    fuuko->addSkill(new Haixing);
    fuuko->addSkill(new Taozui);
    /*
    General *tomoyo = new General(this, "tomoyo", "wu", 3, false); // Game 003

    General *t_rin = new General(this, "t_rin", "wu", 3, false); // Game 004

    General *altria = new General(this, "altria", "wu", 3, false); // Game 005

    General *nanoha = new General(this, "nanoha", "wu", 3, false); // Game 006

    General *rika = new General(this, "rika", "wu", 3, false); // Game 007

    General *rena = new General(this, "rena", "wu", 3, false); // Game 008

    General *fate = new General(this, "fate", "wu", 3, false); // Game 009
    */
    General *haruka = new General(this, "haruka", "wu", 4, false); // Game 010
    haruka->addSkill(new Yuanqi);
    haruka->addSkill(new Daihei);

    /*
    General *miki = new General(this, "miki", "wu", 3, false); // Game 011

    General *n_rin = new General(this, "n_rin", "wu", 3, false); // Game 012
    */
    General *misuzu = new General(this, "misuzu", "wu", 3, false); // Game 013
    misuzu->addSkill(new Xiaying);
    misuzu->addSkill(new Yumeng);
    skills << new YumengDraw;
    /*
    General *setsuna = new General(this, "setsuna", "wu", 3, false); // Game 014

    General *kazusa = new General(this, "kazusa", "wu", 3, false); // Game 015

    General *ayu = new General(this, "ayu", "wu", 3, false); // Game 016

    General *hayate = new General(this, "hayate", "wu", 3, false); // Game 017

    General *komari = new General(this, "komari", "wu", 3, false); // Game 018
    */

   addMetaObject<HaixingCard>();
}
