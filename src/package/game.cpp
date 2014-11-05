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

void XiayingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
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
        if (use.to.length() > 1 || use.from == player || use.from->getJudgingArea().isEmpty())
            return QStringList();
        if (!use.to.contains(player)) return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
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

//jiuzhu by SE need DUBUG~~~~~

Key::Key(Card::Suit suit, int number)
    : DelayedTrick(suit, number)
{
    setObjectName("keyCard");
}

bool Key::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    return targets.isEmpty() && !to_select->containsTrick(objectName());
}

void Key::takeEffect(ServerPlayer *target) const{
    target->clearHistory();
#ifdef QT_DEBUG
    if (!target->getAI() && target->askForSkillInvoke("userdefine:cancelkeyCard")) return;
#endif
}


class Jiuzhu : public TriggerSkill {
public:
    Jiuzhu() : TriggerSkill("jiuzhu") {
        events << Dying << PreHpLost << CardsMoveOneTime;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (event == Dying){
            DyingStruct dying = data.value<DyingStruct>();
            if (dying.who->containsTrick("keyCard")) return skill_list;
            QList<ServerPlayer *> rins = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *rin, rins){
                if (rin == player){
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
        else if (event == CardsMoveOneTime){
            QList<ServerPlayer *> rins = room->findPlayersBySkillName(objectName());
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            foreach(int id, move.card_ids){
                if (move.to_place == Player::DiscardPile){
                    foreach(ServerPlayer *rin, rins){
                        if (rin != NULL) {

                        	QVariantList key_list = room->getTag("key_list").toList();
                    	    QList<int> card_ids;
						    foreach(QVariant card_id, key_list)
						        card_ids << card_id.toInt();

                        	bool invoke = false;
                        	foreach(int list_id , card_ids){
                        		if (list_id == id){
                        			card_ids.removeOne(list_id);
                        			invoke = true;
                        		}
                        	}
                        	room->setTag("key_list", IntList2VariantList(card_ids));

                            if (invoke){
                                skill_list.insert(rin, QStringList(objectName()));
                                return skill_list;
                            }
                        }    
                    }
                    break;
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        if (event == Dying){
            if (room->askForSkillInvoke(ask_who, objectName())) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), player->objectName());
                room->broadcastSkillInvoke(objectName(), ask_who);
                return true;
            }
        }
        else if (event == PreHpLost){
        	bool invoke = ask_who->hasShownSkill(this) ? true : room->askForSkillInvoke(ask_who, objectName());
            if (invoke) {//need some other imformation passed to player? not sure
                room->broadcastSkillInvoke(objectName(), ask_who);
                return true;
            }
        }
        else if (event == CardsMoveOneTime){
        	bool invoke = ask_who->hasShownSkill(this) ? true : room->askForSkillInvoke(ask_who, "jiuzhu_getCard");
            if (invoke){
                return true;
            }
        }
        return false;
    }
    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const{
        if (event == Dying){
            DyingStruct dying = data.value<DyingStruct>();
            if (room->getDrawPile().length() == 0)
                room->swapPile();
            Card *card = Sanguosha->getCard(room->getDrawPile().at(0));

            QVariantList key_list = room->getTag("key_list").toList();
            QList<int> card_ids;
		    foreach(QVariant card_id, key_list)
		        card_ids << card_id.toInt();
		    card_ids << card->getEffectiveId();
		    room->setTag("key_list", IntList2VariantList(card_ids));

            Key* key = new Key(card->getSuit(), card->getNumber());
            key->addSubcard(card);
            key->setShowSkill(objectName());
            key->setSkillName(objectName());
            room->useCard(CardUseStruct(key, ask_who, dying.who));
            return false;
        }
        else if (event == PreHpLost){
            if (!player->containsTrick("keyCard")){
                return false;
            }
            bool have = false;
            foreach (const Card *card, player->getJudgingArea()){
                if (card->isKindOf("Key")){
                    room->throwCard(card, NULL, ask_who);
                    have = true;
                }
            }
            if (have)
                return true;
            return false;
        }
        else if (event == CardsMoveOneTime){
            ask_who->drawCards(2);
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
        return selected.isEmpty() && (to_select->isKindOf("EquipCard") || to_select->isKindOf("Peach")) && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LuoxuanCard *luoxuan = new LuoxuanCard;
        luoxuan->addSubcard(originalCard->getId());
        luoxuan->setShowSkill(objectName());
        return luoxuan;
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
    */
    General *n_rin = new General(this, "n_rin", "wu", 3, false); // Game 012
    
    n_rin->addSkill(new Pasheng);
    n_rin->addSkill(new Jiuzhu);

    General *misuzu = new General(this, "misuzu", "wu", 3, false); // Game 013
    misuzu->addSkill(new Xiaying);
    misuzu->addSkill(new Yumeng);
    skills << new YumengDraw;
    /*
    General *setsuna = new General(this, "setsuna", "wu", 3, false); // Game 014

    General *kazusa = new General(this, "kazusa", "wu", 3, false); // Game 015

    General *ayu = new General(this, "ayu", "wu", 3, false); // Game 016

    General *hayate = new General(this, "hayate", "wu", 3, false); // Game 017
    */
    General *komari = new General(this, "komari", "wu", 3, false); // Game 018
    komari->addSkill(new Luoxuan);

   addMetaObject<HaixingCard>();
   addMetaObject<TaozuiCard>();
   addMetaObject<Key>();
   addMetaObject<LuoxuanCard>();
}
