#include "game.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"

//general classes and functions

Key::Key(Card::Suit suit, int number)
    : DelayedTrick(suit, number)
{
    setObjectName("keyCard");
}

bool Key::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    // please use this to check validity when put key
    int num = 0;
    foreach(const Card *card, to_select->getJudgingArea()){
        if (card->objectName() == objectName()){
            num++;
        }
    }
    return targets.isEmpty() && (num == 0 || (to_select->hasShownSkill("huanyuan") && num < 3));
}

void Key::takeEffect(ServerPlayer *target) const{
    target->clearHistory();
#ifndef QT_NO_DEBUG
    if (!target->getAI() && target->askForSkillInvoke("userdefine:cancelkeyCard")) return;
#endif
}

void Key::onEffect(const CardEffectStruct &) const{
}

//for managing anything needed to be done with key
class keyCardGlobalManagement : public TriggerSkill {
public:
    keyCardGlobalManagement() : TriggerSkill("keyCard-global") {
        events << CardsMoveOneTime << NonTrigger;
        global = true;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (event == CardsMoveOneTime){
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            for (int i = 0; i < move.card_ids.length(); i++) {
                if (!VariantList2IntList(room->getTag("keyList").toList()).contains(move.card_ids[i]))
                    continue;
                if (move.from != NULL && move.from_places[i] != NULL && move.from_places[i] == Player::PlaceDelayedTrick) {
                    ServerPlayer *player = NULL;
                    foreach(ServerPlayer *p, room->getAlivePlayers()) {
                        if (p->objectName() == move.from->objectName()) {
                            player = p;
                            break;
                        }
                    }
                    if (player != NULL && player->isAlive() && !player->isDead()) {
                        RecoverStruct recover;
                        recover.recover = 1;
                        recover.who = player;
                        recover.card = Sanguosha->getEngineCard(move.card_ids[i]);
                        room->recover(player, recover, true);
                    }
                }
                if (move.to_place == Player::DiscardPile){
                    QList<QVariant> ql = room->getTag("keyList").toList();
                    ql.removeOne(QVariant::fromValue(move.card_ids[i]));
                    room->setTag("keyList", ql);
                }
            }
        }
        return skill_list;
    }
};

//Put an card as key for Key Skills
void putKeyFromId(Room *room, int id, ServerPlayer *from, ServerPlayer *to, QString skill_name){
    const Card *card = Sanguosha->getCard(id);
    Key* key = new Key(card->getSuit(), card->getNumber());
    key->addSubcard(card);
    key->setSkillName(skill_name);

    Card *trick = Sanguosha->cloneCard(key);
    Q_ASSERT(trick != NULL);
    WrappedCard *wrapped = Sanguosha->getWrappedCard(id);
    wrapped->takeOver(trick);
    room->broadcastUpdateCard(room->getPlayers(), wrapped->getId(), wrapped);
    wrapped->setShowSkill(card->showSkill());

    CardMoveReason reason(CardMoveReason::S_REASON_PUT, from->objectName(), to->objectName(), skill_name, QString());
    room->moveCardTo(wrapped, from, to, Player::PlaceDelayedTrick, reason, true);

    //addkey
    if (room->getTag("keyList") == NULL){
        QList<int> newList;
        room->setTag("keyList", QVariant::fromValue(newList));
    }
    QList<QVariant> ql = room->getTag("keyList").toList();
    ql.append(QVariant::fromValue(key->getEffectiveId()));
    room->setTag("keyList", ql);
}


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
        events << EventPhaseStart;
        frequency = NotFrequent;
        view_as_skill = new HaixingViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer * &) const{
        if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Start)
            if (TriggerSkill::triggerable(player) && ( player->getHandcardNum() > 0 ))
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

TaozuiCard::TaozuiCard() {
    mute = true;
}

bool TaozuiCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *Self) const{
	bool invoke = false;
	foreach(const Card *card , to_select->getJudgingArea()) {
		if ( Self->canDiscard(to_select,"j") && (card->isKindOf("Indulgence") || card->isKindOf("SupplyShortage")))
			invoke = true;
	}
    return invoke;
}

void TaozuiCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach(ServerPlayer *target, targets) {
        if (!source->canDiscard(target, "j"))
            targets.removeOne(target);
    }

    if (targets.length() > 0) {

        QVariantList target_list;
        foreach(ServerPlayer *target, targets){
            target_list << QVariant::fromValue(target);
        }

        source->tag["taozui_invoke"] = target_list;
        source->setFlags("taozui");
    }
}

class TaozuiViewAsSkill : public ViewAsSkill {
public:
    TaozuiViewAsSkill() : ViewAsSkill("taozui") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern.startsWith("@@taozui");
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *) const{
        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty()){
            TaozuiCard *taozui = new TaozuiCard;
            return taozui;
        }
        return NULL;
    }
};

class Taozui: public TriggerSkill {
public:
    Taozui(): TriggerSkill("taozui") {
        events << EventPhaseChanging;
        frequency = NotFrequent;
        view_as_skill = new TaozuiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
    	if (!TriggerSkill::triggerable(player))
    		return QStringList();
    	PhaseChangeStruct change = data.value<PhaseChangeStruct>();
    	if (change.to == Player::Judge && !player->isSkipped(Player::Judge) && !player->isSkipped(Player::Draw)){
			bool invoke = false;
			foreach(ServerPlayer *p, room->getAlivePlayers()){
				foreach(const Card *card, p->getJudgingArea()){
					if (card->isKindOf("Indulgence") || card->isKindOf("SupplyShortage")){
						invoke = true;
					}
				}
			}
			if (invoke){
				player->tag.remove("taozui_invoke");
				return QStringList(objectName());
			}
		}
        return QStringList();
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (room->askForUseCard(player, "@@taozui", "@taozui")) {
            if (player->hasFlag("taozui") && player->tag.contains("taozui_invoke")) {
                player->skip(Player::Judge);
                player->skip(Player::Draw);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *fuuko, QVariant &, ServerPlayer *) const{
    	QVariantList target_list;
    	target_list = fuuko->tag["taozui_invoke"].toList();
        fuuko->tag.remove("taozui_invoke");
        QList<ServerPlayer *> targets;

        foreach(QVariant x, target_list){
            targets << x.value<ServerPlayer *>();
        }
		if (targets.length() == 0)
			return false;
        foreach(ServerPlayer *p , targets){
        	QStringList choices;
			choices << "cancel";
        	foreach(const Card *card , p->getJudgingArea()){
        		if (card->isKindOf("Indulgence"))
        			choices << "indulgence";
        		if (card->isKindOf("SupplyShortage"))
        			choices << "supply_shortage";
        	}
        	QString choice = room->askForChoice(fuuko, objectName(), choices.join("+"));
			if (choice != "cancel"){
				foreach(const Card *disc, p->getJudgingArea()){
					if (disc->objectName() == choice) {
						CardMoveReason reason(CardMoveReason::S_REASON_PUT, fuuko->objectName());
						room->throwCard(disc, reason, NULL);
					}
				}
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

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *haruka, QVariant &, ServerPlayer *) const{
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

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer * &) const{
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

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
       bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke){
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *haruka, QVariant &, ServerPlayer *) const{
        if (triggerEvent == EventPhaseStart){
            room->drawCards(haruka,3); // Maybe drawing 2 is enough
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

void XiayingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    ServerPlayer *target = room->getCurrent();

    if (target != NULL) {
        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "xiaying", QString());
        room->obtainCard(target, this, reason, false);
	    int new_value = target->getMark("xiaying") + subcards.length();
        room->setPlayerMark(target, "xiaying", new_value);
    }
}

class XiayingViewAsSkill: public ViewAsSkill {
public:
    XiayingViewAsSkill(): ViewAsSkill("xiayingVS") {
        response_pattern = "@@xiaying";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *) const{
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
    frequency = Compulsory;
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

   virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
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

bool YumengDraw::cost(TriggerEvent , Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const{
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
                if ((misuzu->isFriendWith(player) || misuzu->willBeFriendWith(player)) && misuzu->getLostHp() > 0)
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
    virtual bool effect(TriggerEvent , Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const{
        if (ask_who->getHp() < ask_who->getMaxHp()){
            int mark = ask_who->getMaxHp()-ask_who->getHp() > 2? 2:ask_who->getMaxHp()-ask_who->getHp();
            ask_who->setMark("yumeng_use",mark);
        }else
            ask_who->setMark("yumeng_use", 0);
        return false;
    }
};

//pasheng by SE
class Pasheng : public TriggerSkill {
public:
    Pasheng() : TriggerSkill("pasheng") {
        events << TargetConfirming;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.to.length() > 1 || use.from == player || (use.from && use.from->getJudgingArea().isEmpty()))
            return QStringList();
        if (!use.to.contains(player)) return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        bool invoke = room->askForSkillInvoke(player, objectName());
        if (invoke){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        room->notifySkillInvoked(player, objectName());
        LogMessage log;
        if (use.from) {
            log.type = "$CancelTarget";
            log.from = use.from;
        }
        else {
            log.type = "$CancelTargetNoUser";
        }
        log.to = use.to;
        log.arg = use.card->objectName();
        room->sendLog(log);

        room->setEmotion(player, "cancel");

        use.to.removeOne(player);
        data = QVariant::fromValue(use);
        return false;
    }
};

//jiuzhu by SE
class Jiuzhu : public TriggerSkill {
public:
    Jiuzhu() : TriggerSkill("jiuzhu") {
        events << Dying << PreHpLost << CardsMoveOneTime << AskForPeachesDone;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (event == Dying){
            DyingStruct dying = data.value<DyingStruct>();

            if (dying.who == NULL) return skill_list;
            QList<ServerPlayer *> rins = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *rin, rins){
                Key *key = new Key(Card::NoSuit, 0);
                const QList<const Player *> empty;
                if (key->targetFilter(empty, dying.who, rin) && !rin->hasFlag("jiuzhu_used")){
                    delete key;
                    skill_list.insert(rin, QStringList(objectName()));
                }
            }
        }
        else if (event == PreHpLost){//need to add lost reason? not sure
        	if (!player->containsTrick("keyCard")) return skill_list;
            QList<ServerPlayer *> rins = room->findPlayersBySkillName(objectName());
                foreach(ServerPlayer *rin, rins){
                    if (rin != NULL)
                    	skill_list.insert(rin, QStringList(objectName()));
                }
        }
        else if (event == AskForPeachesDone){
            QList<ServerPlayer *> rins = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *rin, rins){
                if (rin->hasFlag("jiuzhu_used")){
                    rin->setFlags("-jiuzhu_used");
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        if (event == Dying){
            ask_who->setFlags("jiuzhu_used");
            if (room->askForSkillInvoke(ask_who, objectName())) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), player->objectName());
                room->broadcastSkillInvoke(objectName(), ask_who);
                return true;
            }
        }
        else if (event == PreHpLost){
            bool invoke = room->askForSkillInvoke(ask_who, objectName());
            if (invoke) {//need some other imformation passed to player? not sure
                room->broadcastSkillInvoke(objectName(), ask_who);
                return true;
            }
        }
        return false;
    }
    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        if (event == Dying){
            DyingStruct dying = data.value<DyingStruct>();
            putKeyFromId(room, room->drawCard(), ask_who, dying.who, objectName());
            return false;
        }
        else if (event == PreHpLost){
            if (!player->containsTrick("keyCard")){
                return false;
            }
            foreach (const Card *card, player->getJudgingArea()){
                if (card->isKindOf("Key")){
                    
                    CardMoveReason reason(CardMoveReason::S_REASON_PUT, ask_who->objectName(), player->objectName(), objectName(), QString());
                    room->setTag("key_in_move", QVariant::fromValue(card->getEffectiveId()));
                    room->moveCardTo(card, player, NULL, Player::DiscardPile, reason, true);
                }
            }
            ask_who->drawCards(1);
            return false;
        }
        return false;
    }
};

//luoxuan by SE
LuoxuanCard::LuoxuanCard() {
}

bool LuoxuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    return targets.isEmpty() && to_select->isWounded();
}

void LuoxuanCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
    if (effect.to->getEquips().length() > 0){
        QString choice = effect.to->getRoom()->askForChoice(effect.from, "Luoxuan_choice", "luoxuan_get+luoxuan_give_up");
        if (choice == "luoxuan_get"){
            int c = effect.to->getRoom()->askForCardChosen(effect.from, effect.to, "e", objectName());
            effect.from->obtainCard(Sanguosha->getCard(c));
        }
    }
}

class Luoxuan : public OneCardViewAsSkill {
public:
    Luoxuan() : OneCardViewAsSkill("luoxuan") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "h");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped() && (to_select->isKindOf("EquipCard") || to_select->isKindOf("Peach")) && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LuoxuanCard *luoxuan = new LuoxuanCard;
        luoxuan->addSubcard(originalCard->getId());
        luoxuan->setShowSkill(objectName());
        return luoxuan;
    }
};

//sidai
class Sidai : public TriggerSkill {
public:
    Sidai() : TriggerSkill("sidai") {
        events << Death;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (player == NULL || !player->hasSkill(objectName())) return QStringList();
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return QStringList();

        QList<ServerPlayer *> dests = room->getOtherPlayers(player);
        Key *key = new Key(Card::NoSuit, 0);
        const QList<const Player *> empty;
        foreach(ServerPlayer *p, dests){
            if (!key->targetFilter(empty, p, player))
                dests.removeOne(p);
        }
        delete key;
        if (dests.length() == 0){
            return QStringList();
        }

        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        room->notifySkillInvoked(player, objectName());

        int id = room->drawCard();
        room->showCard(player, id);
        QList<ServerPlayer *> dests = room->getOtherPlayers(player);
        Key *key = new Key(Card::NoSuit, 0);
        const QList<const Player *> empty;
        foreach(ServerPlayer *p, dests){
            if (!key->targetFilter(empty, p, player))
                dests.removeOne(p);
        }
        delete key;
        if (dests.length() == 0){
            return false;
        }

        ServerPlayer * dest = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
        if (!dest){
            return false;
        }

        putKeyFromId(room, id, player, dest, objectName());
        return false;
    }
};


class Liepo : public TriggerSkill {
public:
    Liepo() : TriggerSkill("liepo") {
        events << TargetChosen << DamageCaused ;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
    	if (event == TargetChosen){
	        CardUseStruct use = data.value<CardUseStruct>();
	        if (TriggerSkill::triggerable(use.from) && use.card != NULL && use.card->isKindOf("Slash") && use.to.contains(player)){
	            if (!player->getJudgingArea().isEmpty()){
	                ask_who = use.from;
	                return QStringList(objectName());
	            }
	        }    		
    	} else if (event == DamageCaused){
    		DamageStruct damage = data.value<DamageStruct>();
    		if (damage.from->hasFlag("liepo_select_2") && damage.to->hasFlag("liepo_change_damage_type")){
    			return QStringList(objectName());
    		}
    	}
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
    	if (event == TargetChosen){
    		if (ask_who->askForSkillInvoke(objectName(), QVariant::fromValue(player))) {
            	return true;
        	}
    	} else if (event == DamageCaused){
    		// DamageBuff Sound
    		return true;
    	}
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
    	if (event == TargetChosen) {
	        CardUseStruct use = data.value<CardUseStruct>();
	        ServerPlayer *nanoha = ask_who;

	        QStringList choices;
	        choices << "cant_use_jink";

			if (use.card->isKindOf("ThunderSlash") || use.card->isKindOf("FireSlash"))
	        	choices << "change_damage_type";

	        QString choice = room->askForChoice(nanoha,objectName(),choices.join("+"));

	        if (choice == "cant_use_jink"){
	        	QVariantList jink_list = nanoha->tag["Jink_" + use.card->toString()].toList();
	        	doLiepo(player, use, jink_list);
	        	nanoha->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
	        } else if (choice == "change_damage_type") {
	        	nanoha->setFlags("liepo_select_2");
	        	player->setFlags("liepo_change_damage_type");
	        }    		
    	} else if (event == DamageCaused) {
    		DamageStruct damage = data.value<DamageStruct>();
            LogMessage log;
            log.type = "#LiepoBuff";
            log.from = damage.from;
            log.to << damage.to;
            log.arg = QString::number(++damage.damage);
            room->sendLog(log);

            damage.from->setFlags("-liepo_select_2");
            damage.to->setFlags("-liepo_change_damage_type");

			damage.nature = DamageStruct::Normal;
            data = QVariant::fromValue(damage);
    	}
        return false;
    }

private:
    static void doLiepo(ServerPlayer *target, CardUseStruct use, QVariantList &jink_list) {
        int index = use.to.indexOf(target);
        LogMessage log;
        log.type = "#NoJink";
        log.from = target;
        target->getRoom()->sendLog(log);
        jink_list.replace(index, QVariant(0));
    }
};

Lingdan::Lingdan(Card::Suit suit, int number)
    : DelayedTrick(suit, number)
{
    setObjectName("lingdanCard");

    judge.pattern = "BasicCard|.|2~9";
    judge.good = true;
    judge.reason = objectName();
}

bool Lingdan::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->containsTrick(objectName()) && to_select != Self;
}

void Lingdan::takeEffect(ServerPlayer *target) const{
    target->clearHistory();

#ifndef QT_NO_DEBUG
    if (!target->getAI() && target->askForSkillInvoke("userdefine:cancellingdanCard")) return;
#endif

    if (target->canDiscard(target, "he"))
        target->getRoom()->askForDiscard(target, objectName(), 2, 2, false, true, "@Lingdan-discard");
}

ShenxingCard::ShenxingCard() {
	will_throw = false;
}

bool ShenxingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
	return targets.isEmpty() && !to_select->containsTrick(objectName()) && to_select != Self;
}

void ShenxingCard::onEffect(const CardEffectStruct &effect) const{
    Lingdan *ld = new Lingdan(effect.card->getSuit(), effect.card->getNumber());
    ld->addSubcard(effect.card);
    ld->setSkillName("shenxing");
    effect.from->getRoom()->useCard(CardUseStruct(ld, effect.from, effect.to));
}

class Shenxing : public OneCardViewAsSkill {
public:
    Shenxing() : OneCardViewAsSkill("shenxing") {
        response_or_use = true;
        filter_pattern = "BasicCard";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("ShenxingCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ShenxingCard *card = new ShenxingCard;
        card->addSubcard(originalCard);
        card->setShowSkill(objectName());
        return card;
    }
};

class LeiguangVS : public OneCardViewAsSkill {
public:
    LeiguangVS() : OneCardViewAsSkill("leiguang") {
        response_or_use = true;
        filter_pattern = "%slash";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player) ;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &) const{
        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
    	Card *acard = new ThunderSlash(originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName("leiguang");
        acard->setShowSkill("leiguang");
        return acard;
    }
};

class Leiguang : public TriggerSkill {
public:
    Leiguang() : TriggerSkill("leiguang") {
        events << CardFinished;
        view_as_skill = new LeiguangVS;
    }

    virtual bool canPreshow() const{
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();
       	CardUseStruct use = data.value<CardUseStruct>();
       	if (use.card->getSkillName() == "leiguang" && use.card->isKindOf("Slash") 
       		&& use.from == player && use.to.length() == 1 && use.to.first()->isAlive())
       		return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
    	CardUseStruct use = data.value<CardUseStruct>();
    	if (player->askForSkillInvoke(objectName(), QVariant::fromValue(use.to.first()))) {
    		//Sound
            return true;
        }        
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
    	CardUseStruct use = data.value<CardUseStruct>();
		if (use.to.first()->canDiscard(use.to.first(), "he") && room->askForDiscard(use.to.first(), objectName(), 1, 1, true, true, "@leiguang_effect")) {
    		return false;
    	} else if (use.m_addHistory){
			room->addPlayerHistory(player, use.card->getClassName(), -1);
			LogMessage log;
            log.type = "#leiguang_more_slash";
            log.from = player;
            log.to << room->getAllPlayers(true);
            room->sendLog(log);
    	}
        return false;
    }
};

class Kongwu : public DistanceSkill {
public:
    Kongwu() : DistanceSkill("kongwu") {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if (from->hasShownSkill(objectName()) && from->hasShownOneGeneral() && !to->hasShownAllGenerals())
            return -1000;
        else
            return 0;
    }
};

class Chaidao : public TriggerSkill {
public:
    Chaidao() : TriggerSkill("chaidao") {
        events << DamageCaused << DamageInflicted;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();
       	DamageStruct damage = data.value<DamageStruct>();
       	Collateral *collateral = new Collateral(Card::SuitToBeDecided, 0);
       	QList<const Player *> targets;
       	if (player->getWeapon() && 
       		((event == DamageCaused && player->canDiscard(player, player->getWeapon()->getEffectiveId()) && damage.card->isKindOf("Slash")) ||
       		(event == DamageInflicted && damage.from != player && collateral->targetFilter(targets, player,damage.from) && 
       		(!damage.from->isProhibited(player, collateral, targets))))) {
       		return QStringList(objectName());
       	}
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
    	if (event == DamageCaused && room->askForCard(player, "Weapon|.|.|equipped", "@chaidao_add_damage" , data, Card::MethodDiscard, NULL, false)){
    		//sound 1
    		return true;
    	}
    	if (event == DamageInflicted && room->askForSkillInvoke(player,objectName(),data)){
    		//sound 2
    		return true;
    	}
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
    	DamageStruct damage = data.value<DamageStruct>();
    	if (event == DamageCaused){
            LogMessage log;
            log.type = "#ChaidaoBuff";
            log.from = damage.from;
            log.to << damage.to;
            log.arg = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
    	} else {
    	    Collateral *collateral = new Collateral(Card::SuitToBeDecided, 0);
			QList<const Player *> empty_targets;
			QList<ServerPlayer *> can_slash_targets;
    	    QList<ServerPlayer *> targets;
    	    Slash *slash = new Slash(Card::SuitToBeDecided,  0);

    	    targets << player;

			foreach(ServerPlayer *p , room->getOtherPlayers(player))
				if ((slash->targetFilter(empty_targets, p,player)) && (!player->isProhibited(p , slash, empty_targets)))
					can_slash_targets << p;
    	    
    	    ServerPlayer *victim = room->askForPlayerChosen(damage.from , can_slash_targets , objectName() , "@chaidao_choose" , false , true );
    	    if (victim != NULL) 
    	    	targets << victim ;

    	    if (targets.length() != 2)
    	    	return false;

	        if (room->useCard(CardUseStruct(collateral, damage.from , targets))){
		    	LogMessage log;
		        log.type = "#Breastplate";
		        log.from = player;
		        if (damage.from)
		            log.to << damage.from;
		        log.arg = QString::number(damage.damage);
		        if (damage.nature == DamageStruct::Normal)
		            log.arg2 = "normal_nature";
		        else if (damage.nature == DamageStruct::Fire)
		            log.arg2 = "fire_nature";
		        else if (damage.nature == DamageStruct::Thunder)
		            log.arg2 = "thunder_nature";
		        room->sendLog(log);	           
		       	return true;	
	        }
    	}
        return false;
    }
};

class Canshi : public TriggerSkill {
public:
    Canshi() : TriggerSkill("canshi") {
        events << Damaged << FinishJudge;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        if (triggerEvent == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card == NULL || !damage.card->isKindOf("Slash") || damage.to->isDead())
                return skill_list;

            QList<ServerPlayer *> rins = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *rin, rins)
                if (rin->canDiscard(rin, "he"))
                    skill_list.insert(rin, QStringList(objectName()));
            return skill_list;
        }
        else {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason != objectName()) return skill_list;
            judge->pattern = QString::number(int(judge->card->getSuit()));
			if (judge->reason == objectName() && room->getCardPlace(judge->card->getEffectiveId()) == Player::PlaceJudge && judge->who->getPile("gem").length() < 10)
				judge->who->addToPile("gem", judge->card);
            return skill_list;
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *rin = ask_who;

        if (rin != NULL){
            rin->tag["canshi_data"] = data;
            bool invoke = room->askForDiscard(rin, objectName(), 1, 1, true, true, "@canshi", true);
            rin->tag.remove("canshi_data");

            if (invoke){
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, rin->objectName(), data.value<DamageStruct>().to->objectName());
                room->broadcastSkillInvoke(objectName(), rin);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        ServerPlayer *rin = ask_who;
        if (rin == NULL) return false;
        DamageStruct damage = data.value<DamageStruct>();

        JudgeStruct judge;
        judge.good = true;
        judge.play_animation = false;
        judge.who = rin;
        judge.reason = objectName();

        room->judge(judge);

        Card::Suit suit = (Card::Suit)(judge.pattern.toInt());
        switch (suit) {
        case Card::Heart: {
            RecoverStruct recover;
            recover.who = rin;
            room->recover(player, recover);

            break;
        }
        case Card::Diamond: {
            player->drawCards(2);
            break;
        }
        case Card::Club: {
            if (damage.from && damage.from->isAlive())
                room->askForDiscard(damage.from, "canshi_discard", 2, 2, false, true);

            break;
        }
        case Card::Spade: {
            if (damage.from && damage.from->isAlive())
                damage.from->turnOver();

            break;
        }
        default:
            break;
        }

        return false;
    }
};

class Modan : public ViewAsSkill {
public:
    Modan() : ViewAsSkill("modan") {
        expand_pile = "gem";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("gem").isEmpty() ;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		ExpPattern pattern(".|.|.|gem");
		if (!pattern.match(Self, to_select))
			return false;

	    foreach (const Card *gem, selected)
	        if (to_select->getSuit() == gem->getSuit())
	            return false;

    	if (selected.length() >= 4)
	        return false;
	    return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() == 4) {
            ArcheryAttack *aa = new ArcheryAttack(Card::SuitToBeDecided, 0);
            aa->addSubcards(cards);
            aa->setSkillName("modan");
            aa->setShowSkill("modan");
            return aa;
        }
        else
            return NULL;
    }
};

//huanyuan
class Huanyuan : public TriggerSkill {
public:
    Huanyuan() : TriggerSkill("huanyuan") {
        events << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual bool canPreshow() const {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer * &) const{
        if (player->getPhase() == Player::Start){
            if (TriggerSkill::triggerable(player)){
                int num = 0;
                foreach(const Card *card, player->getJudgingArea()){
                    if (card->isKindOf("Key")){
                        num++;
                    }
                }
                if (num < 3)
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(this)){
            const Card *c = room->askForCard(player, ".|heart", "@huanyuan_discard", NULL, objectName());
            if (!c){
                room->loseHp(player);
            }
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *ayu, QVariant &, ServerPlayer *) const{
        if (!ayu->isAlive()) return false;
        int id = room->drawCard();
        room->showCard(ayu, id);
        putKeyFromId(room, id, ayu, ayu, objectName());

        return false;
    }
};

//Mengxian
class Mengxian : public TriggerSkill {
public:
    Mengxian() : TriggerSkill("mengxian") {
        events << AskForPeaches;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *target, QVariant &data, ServerPlayer * &) const {
        if (TriggerSkill::triggerable(target)){
            DyingStruct dying_data = data.value<DyingStruct>();

            if (target->getHp() > 0)
                return QStringList();

            if (dying_data.who != target)
                return QStringList();

            int num = 0;
            foreach(const Card *card, target->getJudgingArea()){
                if (card->isKindOf("Key")){
                    num++;
                }
            }
            if (num < 2)
                return QStringList();
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *ayu, QVariant &, ServerPlayer *) const{
        QList<int> keys;
        foreach(const Card *card, ayu->getJudgingArea()){
            if (card->isKindOf("Key")){
                keys.append(card->getEffectiveId());
            }
        }
        room->fillAG(keys, ayu);
        int id = room->askForAG(ayu, keys, true, objectName());
        room->clearAG(ayu);
        if (id == -1){
            return false;
        }
        ayu->tag["mengxian_id"] = id;
        CardMoveReason reason(CardMoveReason::S_REASON_PUT, ayu->objectName(), ayu->objectName(), objectName(), QString());
        room->moveCardTo(Sanguosha->getEngineCard(id), ayu, NULL, Player::DiscardPile, reason, true);
        room->broadcastSkillInvoke(objectName(), ayu);
        //room->doSuperLightbox("mengxian", objectName());
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *ayu, QVariant &, ServerPlayer *) const{
        int num = 0;
        foreach(const Card *card, ayu->getJudgingArea()){
            if (card->isKindOf("Key")){
                num++;
            }
        }

        ServerPlayer *y1 = room->askForPlayerChosen(ayu, room->getOtherPlayers(ayu), objectName());

        if (!y1){
            return false;
        }

        QList<ServerPlayer *> drawers;
        drawers << ayu << y1;

        room->drawCards(drawers, num, objectName());
        return false;
    }
};

class FengwangFilter : public FilterSkill{
public:
    FengwangFilter() : FilterSkill("#fengwang-filter"){
    }

    virtual bool viewFilter(const Card *to_select) const{
        Room *room = Sanguosha->currentRoom();
        foreach(ServerPlayer *p, room->getPlayers()) {
            if (p->ownSkill("fengwang") && p->hasShownSkill("fengwang") && p->getMark("@excalibur") > 0)
                return to_select->isKindOf("Weapon") && room->getCardPlace(to_select->getEffectiveId()) == Player::PlaceHand;
        }
        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->setSkillName(objectName());
        WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
        card->takeOver(slash);
        return card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

FengwangCard::FengwangCard() {
    will_throw = false;
    target_fixed = true;
    handling_method = Card::MethodNone;
}

void FengwangCard::onUse(Room *room, const CardUseStruct &card_use) const{
    // it's a weird problem that setShowSkill() can't make general shown, so I put it into onUse().
    QString skill_name = card_use.card->showSkill();
    if (!skill_name.isNull() && card_use.from->ownSkill(skill_name) && !card_use.from->hasShownSkill(skill_name))
        card_use.from->showGeneral(card_use.from->inHeadSkills(skill_name));
    ServerPlayer *source = card_use.from;
    if (source != NULL && subcards.length() > 0) {
        bool toPile = false;
        foreach(int id, subcards) {
            if (room->getCardPlace(id) == Player::PlaceHand) {
                toPile = true;
                break;
            }
        }
        DummyCard dummy(subcards);
        toPile ? source->addToPile("jian", subcards, false) : source->obtainCard(&dummy, false);
    }
}

class FengwangVS : public OneCardViewAsSkill{
public:
    FengwangVS() : OneCardViewAsSkill("fengwang"){
        filter_pattern = ".|.|.|jian,hand";
        expand_pile = "jian";
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return !(player->getPile("jian").isEmpty() && player->getHandcardNum() == 0);
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        if (Self->getPile("jian").length() > 0 ? Self->getPile("jian").first() == originalCard->getId() : Self->getPile("jian").first() != originalCard->getId()) {
            FengwangCard *fw = new FengwangCard;
            fw->addSubcard(originalCard);
            fw->setSkillName(objectName());
            fw->setShowSkill(objectName());
            return fw;
        }
        return NULL;
    }
};

class FengwangTMS : public TargetModSkill{
public:
    FengwangTMS() : TargetModSkill("#fengwang-target"){
        pattern = "Slash";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill("fengwang") && from->getPile("jian").length() > 0)
            return 1000;
        else
            return 0;
    }
};

#include "json.h"
using namespace QSanProtocol;

class Fengwang : public TriggerSkill{
public:
    Fengwang() : TriggerSkill("fengwang"){
        events << TargetChosen << FinishRetrial << SlashEffected;
        view_as_skill = new FengwangVS;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const {
        if (event == TargetChosen) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(use.from) && use.card != NULL && use.card->isKindOf("Slash") && use.from->getPile("jian").length() > 0) {
                if (player == use.from){
                    ask_who = player;
                    player->setFlags("fengwang_user");
                    return QStringList(objectName());
                } 
                if (use.to.contains(player)) {
                    ask_who = use.from;
                    return QStringList(objectName());
                }
            }
        }
        else if (event == FinishRetrial) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (TriggerSkill::triggerable(player) && !player->hasShownSkill(this) && player->getMark("@excalibur") > 0
                && judge->who == player && judge->card->isKindOf("Weapon"))
                return QStringList(objectName());
        }
        else if (event == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.slash->hasFlag("fengwang_nullified"))
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (event == TargetChosen) {
            if (!player->hasFlag("fengwang_user")) {
                if (player->askForSkillInvoke(this)) {
                    room->broadcastSkillInvoke("fengwang", 1);
                    return true;
                }
            }
            else if (player->hasShownSkill(this) ? true : player->askForSkillInvoke(this)) {
                room->broadcastSkillInvoke("fengwang", 2);
                return true;
            }
            player->setFlags("-fengwang_user");
        }
        else if (event == SlashEffected) {
            return true;
        }
        else if (event == FinishRetrial) {
            return player->askForSkillInvoke(this, data);
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const {
        if (event == TargetChosen) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (player->hasFlag("fengwang_user")) {
                foreach(ServerPlayer *p, use.to.toSet())
                    p->addQinggangTag(use.card);
                LogMessage log;
                log.from = player;
                log.arg = objectName();
                log.type = "#TriggerSkill";
                room->sendLog(log);
            }
            else {
                int id = ask_who->getPile("jian").first();
                room->notifyMoveFocus(player);
                JsonArray show_arg;
                show_arg << player->objectName();
                show_arg << id;

                WrappedCard *card = Sanguosha->getWrappedCard(id);
                bool modified = card->isModified();
                QList<ServerPlayer *>players;
                players << player << ask_who;
                if (modified)
                    room->notifyUpdateCard(player, id, card);
                else
                    room->notifyResetCard(player, id);
                room->doBroadcastNotify(players, S_COMMAND_SHOW_CARD, show_arg);

                const Card *jian = Sanguosha->getEngineCard(id);
                if (jian != NULL && jian->isKindOf("Weapon") && qobject_cast<const Weapon *>(jian->getRealCard())->getRange() >= ask_who->distanceTo(player)) {
                    int x = use.to.indexOf(player);
                    QVariantList jink_list = ask_who->tag["Jink_" + use.card->toString()].toList();
                    if (jink_list.at(x).toInt() == 1)
                        jink_list[x] = 2;

                    ask_who->tag["Jink_" + use.card->toString()] = jink_list;
                    LogMessage log;
                    log.from = ask_who;
                    log.arg = objectName();
                    log.type = "#TriggerSkill";
                    room->sendLog(log);
                }
                else {
                    use.card->setFlags("fengwang_nullified");
                }
            }
            player->setFlags("-fengwang_user");
        }
        else if (event == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            LogMessage log;
            log.type = "#HalberdNullified";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            room->sendLog(log);
            return true;
        }
        else if (event == FinishRetrial) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            QList<const Card *> cards;
            cards << judge->card;
            room->filterCards(player, cards, true);
            judge->updateResult();
        }
        return false;
    }
};

class Duangang : public TriggerSkill{
public:
    Duangang() : TriggerSkill("duangang"){
        events << EventPhaseStart << Death << DamageCaused;
        frequency = Limited;
        limit_mark = "@excalibur";
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (event == EventPhaseStart) {
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Play && player->getMark("@excalibur") > 0)
                return QStringList(objectName());
        }
        else if (event == DamageCaused) {
            if (player != NULL && player->hasFlag("excalibur") && player->isAlive())
                return QStringList(objectName());
        }
        else if (event == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (player != NULL && player->hasFlag("excalibur") && death.who != player)
                player->setFlags("-excalibur");
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (event == EventPhaseStart) {
            if (player->askForSkillInvoke(this, data)) {
                room->broadcastSkillInvoke(objectName(), 1);
                room->removePlayerMark(player, "@excalibur");
                room->doSuperLightbox("altria", objectName());
                return true;
            }
        }
        else if (event == DamageCaused) {
            room->broadcastSkillInvoke(objectName(), 2);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (event == EventPhaseStart) {
            room->setPlayerFlag(player, "excalibur");
        }
        else {
            DamageStruct damage = data.value<DamageStruct>();
            LogMessage log;
            log.type = "#DuangangBuff";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            damage.damage += player->tag["excalibur_count"].toInt();
            if (damage.damage == log.arg.toInt())
                return false;
            log.arg2 = QString::number(damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class DuangangRecord : public TriggerSkill{
public:
    DuangangRecord() : TriggerSkill("#duangang-record"){
        events << Damaged;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (player != NULL) {
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *saber = room->getCurrent();
            if (damage.from != NULL && saber != NULL && saber->hasFlag("excalibur")
                && saber == damage.from && saber->getPhase() == Player::Play)
                saber->tag["excalibur_count"] = saber->tag["excalibur_count"].toInt() + damage.damage;
        }
        return QStringList();
    }
};

YonglanCard::YonglanCard() {
    will_throw = false;
}

bool YonglanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    Indulgence *indu = new Indulgence(Card::NoSuit, 0);
    indu->deleteLater();
    if (Self->isProhibited(to_select, indu))
        return false;
    return targets.isEmpty() && !to_select->containsTrick("indulgence") && (Self->isFriendWith(to_select) || Self->willBeFriendWith(to_select));
}

void YonglanCard::onEffect(const CardEffectStruct &effect) const{
    if (subcards.length() == 1) {
        Indulgence *indu = new Indulgence(Card::SuitToBeDecided, -1);
        indu->addSubcards(subcards);
        indu->setSkillName("yonglan");
        Room *room = effect.from->getRoom();
        if (room->useCard(CardUseStruct(indu, effect.from, effect.to, true), true)) {
            room->askForUseCard(effect.from, "@@yonglan", "@yonglan");
        }
    }
}

#include "roomthread.h"

YonglanPindianCard::YonglanPindianCard()
{
}

bool YonglanPindianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.length() < 2 && to_select != Self && !to_select->isKongcheng();
}

bool YonglanPindianCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
    return targets.length() == 2;
}

void YonglanPindianCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    ServerPlayer *miki = card_use.from;

    LogMessage log;
    log.from = miki;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, miki, data);
    room->broadcastSkillInvoke("yonglan", miki);

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, miki->objectName(), QString(), "yonglan", QString());
    room->moveCardTo(this, miki, NULL, Player::PlaceTable, reason, true);

    if (miki->ownSkill("yonglan") && !miki->hasShownSkill("yonglan"))
        miki->showGeneral(miki->inHeadSkills("yonglan"));

    QList<int> table_ids = room->getCardIdsOnTable(this);
    if (!table_ids.isEmpty()) {
        DummyCard dummy(table_ids);
        room->moveCardTo(&dummy, miki, NULL, Player::DiscardPile, reason, true);
    }

    thread->trigger(CardUsed, room, miki, data);
    thread->trigger(CardFinished, room, miki, data);
}

void YonglanPindianCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *user = targets.at(0);
    ServerPlayer *victim = targets.at(1);
    ServerPlayer *winner;
    ServerPlayer *loser;

    PindianStruct *pd = user->pindianSelect(victim, "yonglan");
    if (pd != NULL) {
        bool success = user->pindian(pd);
        if (success) {
            winner = pd->from;
            loser = pd->to;
        }
        else if (pd->to_number > pd->from_number) {
            winner = pd->to;
            loser = pd->from;
        }
        pd = NULL;
        if (winner != NULL && loser != NULL) {
            Duel *duel = new Duel(Card::NoSuit, 0);
            duel->setSkillName(QString("_%1").arg(getSkillName()));
            if (!winner->isCardLimited(duel, Card::MethodUse) && !winner->isProhibited(loser, duel))
                room->useCard(CardUseStruct(duel, winner, loser));
            else {
                LogMessage log;
                log.type = "#YonglanNullified";
                log.from = winner;
                log.arg = winner->objectName();
                delete duel;
            }
        }
    }
}

class Yonglan : public ViewAsSkill{
public:
    Yonglan() : ViewAsSkill("yonglan"){
    }
    
    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const {
        return !to_select->isEquipped() && Sanguosha->currentRoomState()->getCurrentCardUsePattern() == "@@yonglan" ? false : selected.length() == 0;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const {
        return pattern == "@@yonglan";
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return player->getHandcardNum() + player->getPile("wooden_ox").length() > 0;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern() == "@@yonglan" && cards.length() == 0) {
            YonglanPindianCard *pd = new YonglanPindianCard;
            pd->setShowSkill(objectName());
            return pd;
        }
        else if (cards.length() == 1) {
            YonglanCard *yl = new YonglanCard;
            yl->addSubcards(cards);
            yl->setShowSkill(objectName());
            return yl;
        }
        return NULL;
    }
};

class ZhiyanMiki : public TriggerSkill{
public:
    ZhiyanMiki() : TriggerSkill("zhiyanmiki"){
        events << EventPhaseSkipping << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
        if (event == EventPhaseStart) {
            if (TriggerSkill::triggerable(player) && player->hasFlag("zhiyanmiki_skip") && player->getPhase() == Player::Finish) {
                return QStringList(objectName());
            }
        }
        else {
            if (player->ownSkill(this) && player->isAlive()) {
                player->setFlags("zhiyanmiki_skip");
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        return player->hasShownSkill(this) ? true : player->askForSkillInvoke(this);
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QString choice = "draw";
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getAlivePlayers())
            if (player->canDiscard(p, "hej"))
                targets << p;
        if (!targets.isEmpty())
            choice += "+discard";
        if (room->askForChoice(player, objectName(), choice) == "draw") {
            player->drawCards(1, objectName());
        }
        else {
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
            if (target != NULL) {
                room->throwCard(room->askForCardChosen(player, target, "hej", objectName(), false, Card::MethodDiscard), target, player, objectName());
            }
        }
        return false;
    }
};

//夜天: 每当一名角色使用的非延时锦囊牌因结算完毕而置入弃牌堆后，若你于此牌结算时对之使用了【无懈可击】，你可以获得之。锁定技，准备阶段开始时，若你没有手牌，你失去1点体力。
class Yetian : public TriggerSkill
{
public:
    Yetian() : TriggerSkill("yetian")
    {
        events << CardFinished << CardResponded << CardsMoveOneTime << EventPhaseStart;
        frequency = NotFrequent;
    }
    
    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (player == NULL && !player->isAlive())
            return QStringList();
        if (event == CardResponded)
        {
            CardResponseStruct resp = data.value<CardResponseStruct>();
            if (resp.m_card->isKindOf("Nullification") && resp.m_isUse && TriggerSkill::triggerable(player))
                player->setFlags("yetian_usenull");
        }
        else if (event == CardFinished)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from != NULL && use.card != NULL && use.card->isKindOf("TrickCard"))
            {
                
            }
        }
    }
}

void MoesenPackage::addGameGenerals()
{
    skills << new keyCardGlobalManagement;
    
    General *nanoha = new General(this, "nanoha", "wu", 4, false); // G001
    nanoha->addSkill(new Liepo);
    nanoha->addSkill(new Shenxing);
    
    General *fate = new General(this, "fate", "wu", 4, false); // G002
    fate->addSkill(new Leiguang);
    fate->addSkill(new Kongwu);

    //General *hayate = new General(this, "hayate", "wu", 3, false); // G003
    
    General *altria = new General(this, "altria", "wu", 4, false); // G004
    altria->addSkill(new Fengwang);
    altria->addSkill(new FengwangFilter);
    altria->addSkill(new FengwangTMS);
    altria->addSkill(new Duangang);
    altria->addSkill(new DuangangRecord);
    insertRelatedSkills("fengwang", "#fengwang-target");
    insertRelatedSkills("fengwang", "#fengwang-filter");
    insertRelatedSkills("duangang", "#duangang-record");
    
    General *t_rin = new General(this, "t_rin", "wu", 3, false); // G005
    t_rin->addSkill(new Canshi);
    t_rin->addSkill(new Modan);
    
    //General *rika = new General(this, "rika", "wu", 3, false); // G006
    
    General *rena = new General(this, "rena", "wu", 4, false); // G007
    rena->addSkill(new Chaidao);

    General *haruka = new General(this, "haruka", "wu", 4, false); // G008
    haruka->addSkill(new Yuanqi);
    haruka->addSkill(new Daihei);

    General *miki = new General(this, "miki", "wu", 3, false); // G009
    miki->addSkill(new Yonglan);
    miki->addSkill(new ZhiyanMiki);

    //General *setsuna = new General(this, "setsuna", "wu", 3, false); // G010

    //General *kazusa = new General(this, "kazusa", "wu", 3, false); // G011

    //General *nagisa = new General(this, "nagisa", "wu", 3, false); // G012
    
    //General *tomoyo = new General(this, "tomoyo", "wu", 3, false); // G013
    
    General *fuuko = new General(this, "fuuko", "wu", 3, false); // G014
    fuuko->addSkill(new Haixing);
    fuuko->addSkill(new Taozui);
    
    General *misuzu = new General(this, "misuzu", "wu", 3, false); // G015
    misuzu->addSkill(new Xiaying);
    misuzu->addSkill(new Yumeng);
    skills << new YumengDraw;
    
    General *ayu = new General(this, "ayu", "wu", 3, false); // G016
    ayu->addSkill(new Huanyuan);
    ayu->addSkill(new Mengxian);
    
    General *n_rin = new General(this, "n_rin", "wu", 3, false); // G017
    n_rin->addSkill(new Pasheng);
    n_rin->addSkill(new Jiuzhu);

    General *komari = new General(this, "komari", "wu", 3, false); // G018
    komari->addSkill(new Luoxuan);
    komari->addSkill(new Sidai);

    addMetaObject<HaixingCard>();
    addMetaObject<TaozuiCard>();
    addMetaObject<ShenxingCard>();
    addMetaObject<Key>();
    addMetaObject<Lingdan>();
    addMetaObject<LuoxuanCard>();
    addMetaObject<FengwangCard>();
    addMetaObject<YonglanCard>();
    addMetaObject<YonglanPindianCard>();
}