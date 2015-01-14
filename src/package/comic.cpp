#include "comic.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "strategic-advantage.h"
#include "engine.h"
#include "client.h"


//jiandao by AK
class Jiandao : public TriggerSkill {
public:
    Jiandao() : TriggerSkill("jiandao") {
        events << CardUsed << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (player->getWeapon() != NULL)
            return QStringList();
        if (triggerEvent == CardUsed) {
            if (!TriggerSkill::triggerable(player))
                return QStringList();
            if (use.card->isKindOf("Slash"))
                return QStringList(objectName());
        } else {
            if (use.card->isKindOf("Slash")) {
                foreach (ServerPlayer *p, use.to) {
                    QStringList blade_use = p->property("blade_use").toStringList();
                    if (!blade_use.contains(use.card->toString()))
                        return QStringList();

                    blade_use.removeOne(use.card->toString());
                    room->setPlayerProperty(p, "blade_use", blade_use);

                    if (blade_use.isEmpty())
                        room->removePlayerDisableShow(p, "Blade");
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : player->askForSkillInvoke(objectName(),data);
        if (invoke){
            player->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardUseStruct use = data.value<CardUseStruct>();
        bool play_animation = false;
        foreach (ServerPlayer *p, use.to) {
            if (p->getMark("Equips_of_Others_Nullified_to_You") > 0)
                continue;
            QStringList blade_use = p->property("blade_use").toStringList();
            if (blade_use.contains(use.card->toString()))
                return false;

            blade_use << use.card->toString();
            room->setPlayerProperty(p, "blade_use", blade_use);

            if (!p->hasShownAllGenerals())
                play_animation = true;

            room->setPlayerDisableShow(p, "hd", "Blade"); // this effect should always make sense.
        }

        if (play_animation)
            room->setEmotion(player, "weapon/blade");

        return false;
    }
};

class JiandaoRange : public AttackRangeSkill{
public:
    JiandaoRange() : AttackRangeSkill("#jiandao-range"){
    }

    virtual int getExtra(const Player *target, bool) const{
        if (target->hasShownSkill("jiandao")){
            if (target->getWeapon() == NULL ){
                return 2;
            }
        }
        return 0;
    }
};

class Wucun : public TriggerSkill{
public:
    Wucun() : TriggerSkill("wucun"){
        events << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player != NULL && player->getPhase() == Player::Start) {
            QList<ServerPlayer *> akaris = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *akari, akaris){
                LureTiger *luretiger = new LureTiger(Card::SuitToBeDecided , 0);
                QList<const Player *> targets;
				if ((!akari->willBeFriendWith(player)) && (!akari->isFriendWith(player)) && player->hasShownOneGeneral() && akari->getHandcardNum() <= akari->getHp() && room->alivePlayerCount() > 2 && luretiger->targetFilter(targets, akari, player) && !player->isProhibited(akari, luretiger, targets)){
					luretiger->deleteLater();
                    skill_list.insert(akari, QStringList(objectName()));
				}
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const{
        if (ask_who->askForSkillInvoke(objectName(), data)) {
            room->drawCards(ask_who, 1, objectName());
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        LureTiger *luretiger = new LureTiger(Card::SuitToBeDecided, 0);
        luretiger->setSkillName(objectName());
        QList<const Player *> targets;
        if (luretiger->targetFilter(targets, ask_who, player) && !player->isProhibited(ask_who, luretiger, targets))
            room->useCard(CardUseStruct(luretiger, player , ask_who));
        return false;
    }
};

class Kongni : public TriggerSkill {
public:
    Kongni() : TriggerSkill("kongni") {
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        if (player->getEquips().length() > 0) return QStringList();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (effect.slash->isBlack()) return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        bool invoke = player->hasShownSkill(this) ? true : player->askForSkillInvoke(objectName(), data);
        if (invoke) {
            if (player->hasShownSkill(this)) {
                SlashEffectStruct effect = data.value<SlashEffectStruct>();
                LogMessage log;
                log.type = "#SkillNullify";
                log.from = player;
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                room->sendLog(log);
            }
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const{
        return true;
    }
};

class Tianzi : public TriggerSkill {
public:
	Tianzi() : TriggerSkill("tianzi") {
		events << DiscardNCards << DrawNCards;
		frequency = NotFrequent;
	}

	virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		if (event == DrawNCards){
			return QStringList(objectName());
		}
		else{
			if (player->getHandcardNum() >= qMax(1, player->getMaxCards(MaxCardsType::Normal)))
				return QStringList(objectName());
		}
		return QStringList();
	}

	virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		if (player->askForSkillInvoke(this)){
			int x;
			if (event == DrawNCards){
				x = 1;
			}
			else{
				x = 2;
			}
			room->broadcastSkillInvoke(objectName(), x);
			return true;
		}
		return false;
	}

	virtual bool effect(TriggerEvent , Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const{
		data = data.toInt() + 1;
		return false;
	}
};

class Yuzhai : public TriggerSkill {
public:
	Yuzhai() : TriggerSkill("yuzhai") {
		events << EventPhaseEnd << CardsMoveOneTime;
		frequency = NotFrequent;
	}

	virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		if (event == CardsMoveOneTime){
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			if (player->getPhase() != Player::Discard || move.to_place != Player::DiscardPile || (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_DISCARD)
				return QStringList();
			player->setMark("@yuzhai_cards", player->getMark("@yuzhai_cards") + move.card_ids.length());
		}
		else{
			if (player->getPhase() == Player::Discard && player->getMark("@yuzhai_cards") >= player->getHp()){
				player->setMark("@yuzhai_cards", 0);
				QList<ServerPlayer *> others = room->getOtherPlayers(player);
				bool invoke = true;
				foreach(ServerPlayer * other, others){
					if (!player->canDiscard(other, "he"))
						invoke = false;
				}
				if (invoke)
					return QStringList(objectName());
			}
		}
		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		QList<ServerPlayer *> others = room->getOtherPlayers(player);
		QList<ServerPlayer *> targets;
		foreach(ServerPlayer *p, others) {
			if (player->canDiscard(p, "he"))
				targets << p;
		}
		ServerPlayer *to = room->askForPlayerChosen(player, targets, objectName(), "yuzhai-invoke", true, true);
		if (to) {
			player->tag["yuzhai_target"] = QVariant::fromValue(to);
			room->broadcastSkillInvoke(objectName(), player);
			return true;
		}
		else player->tag.remove("yuzhai_target");
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		ServerPlayer *to = player->tag["yuzhai_target"].value<ServerPlayer *>();
		player->tag.remove("yuzhai_target");
		if (to && player->canDiscard(to, "he")) {
			int card_id = room->askForCardChosen(player, to, "he", objectName(), false, Card::MethodDiscard);
			room->throwCard(card_id, to, player);
		}
		return false;
	}
};

class Mizou : public TriggerSkill{
public: 
	Mizou() : TriggerSkill("mizou"){
		events << DamageCaused << DamageInflicted;
		frequency = NotFrequent;
	}

	virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		if (player->canDiscard(player, "he"))
			return QStringList(objectName());
		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		if (room->askForCard(player, "..", "@mizou_discard", QVariant(), objectName())) {
			room->broadcastSkillInvoke(objectName());
			return true;
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
		DamageStruct damage = data.value<DamageStruct>();

		JudgeStruct judge;
		judge.good = true;
		judge.pattern = ".|red";
		judge.play_animation = false;
		judge.who = player;
		judge.reason = objectName();
		judge.time_consuming = true;

		room->judge(judge);

		if (judge.isGood()){
			if (damage.from && damage.from->isAlive())
				damage.from->drawCards(1);
			damage.damage--;
			data = QVariant::fromValue(damage);
		}

		return false;
	}
};

class Wushu : public TriggerSkill{
public:
	Wushu() : TriggerSkill("wushu"){
		events << BeforeCardsMove;
		frequency = Frequent;
	}

	virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		if (move.from != player) return QStringList();
		if (move.to_place == Player::DiscardPile && move.reason.m_reason == CardMoveReason::S_REASON_JUDGEDONE){
			QList<int> ids;
			int i = 0;
			foreach(int id, move.card_ids){
				if (Sanguosha->getCard(id)->getColor() == Card::Black && move.from_places[i] == Player::PlaceJudge)
					ids << id;
				i++;
			}
			if (!ids.isEmpty()){
				player->tag[objectName()] = IntList2VariantList(ids);
				return QStringList(objectName());
			}
		}
		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		if (player->askForSkillInvoke(this)){
			player->getRoom()->broadcastSkillInvoke(objectName());
			return true;
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer*) const{
		QList<int> ids = VariantList2IntList(player->tag[objectName()].toList());
		if (player->isAlive() && !ids.isEmpty()){
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();

			foreach(int id, ids)
				if (move.card_ids.contains(id))
					move.card_ids.removeOne(id);
			data = QVariant::fromValue(move);

			DummyCard *dummy = new DummyCard(ids);

			ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
			if (target && target->isAlive())
				room->moveCardTo(dummy, target, Player::PlaceHand, move.reason, true);

			delete dummy;
		}

		player->tag[objectName()] = QVariant();

		return false;
	}
};

//Rozen_Maiden 

class rosesuiseiseki : public TriggerSkill{
public:
	rosesuiseiseki() : TriggerSkill("rosesuiseiseki"){
		relate_to_place = "head";
		events << EventPhaseStart;
		frequency = NotFrequent;
	}

	virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
		if (!TriggerSkill::triggerable(player)) return QStringList();
		if (player->getPhase() == Player::RoundStart)
			room->removePlayerDisableShow(player, objectName());
		return QStringList();
	}
};

class meijiesuiseiseki : public MaxCardsSkill{
public:
	meijiesuiseiseki() : MaxCardsSkill("meijiesuiseiseki"){
		relate_to_place = "deputy";
	}

	virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const{
		if (target->hasShownSkill(this) && !target->getActualGeneral1Name().contains("sujiang"))
			return 1;
		return 0;
	}
};

ShuimengCard::ShuimengCard() {
	will_throw = true;
	target_fixed = false;
}

bool ShuimengCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
	return targets.isEmpty();
}

void ShuimengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
	if (targets.length() > 0){
		ServerPlayer *master = targets.first();
		if (source->hasSkill("rosesuiseiseki") && (!source->getActualGeneral2Name().contains("sujiang")) && (source->hasShownGeneral2()) && room->askForSkillInvoke(source, "rosesuiseiseki")) {
			source->hideGeneral(false);
			room->setPlayerDisableShow(source, "d", "rosesuiseiseki");
			int d_num = room->askForDiscardNum(source, "shuimeng", 2, 2, false, false, "shuimeng_discard", true);
			if (d_num == 2) 
				master->drawCards(qMin(5, master->getHandcardNum()));
		}
		else{
			int d_num = room->askForDiscardNum(source, "shuimeng", 999, 2, false, false, "shuimeng_discard", true);
			if (d_num >= 2)
				master->drawCards(d_num - 1);
		}
	}
}

class Shuimeng : public	ZeroCardViewAsSkill{
public:
	Shuimeng() : ZeroCardViewAsSkill("shuimeng"){
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		return (player->getHandcardNum() > 1 && !player->hasUsed("ShuimengCard"));
	}

	virtual const Card *viewAs() const{
		ShuimengCard *sm = new ShuimengCard;
		sm->setShowSkill(objectName());
		return sm;
	}
};

rosesuigintouCard::rosesuigintouCard() {
	will_throw = false;
	target_fixed = true;
}

void rosesuigintouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
	source->hideGeneral(false);
	room->setPlayerDisableShow(source, "d", "rosesuigintou");	
	MingmingCard *mm = new MingmingCard();
	mm->addSubcards(getSubcards());
	room->useCard(CardUseStruct(mm, source, NULL, true));
}

class rosesuigintou : public OneCardViewAsSkill{
public:
	rosesuigintou() : OneCardViewAsSkill("rosesuigintou"){
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		return player->getHandcardNum() >= 1 && (!player->hasUsed("MingmingCard")) && (!player->getActualGeneral2Name().contains("sujiang")) && player->hasShownGeneral2();
	}

	virtual bool viewFilter(const Card *card) const{
		if (card->isEquipped())
			return false;

		return card->isBlack();
	}

	virtual const Card *viewAs(const Card *card) const{
		rosesuigintouCard *rst = new rosesuigintouCard;
		rst->addSubcard(card);
		rst->setShowSkill(objectName());
		return rst;
	}
};

class rosesuigintouTrigger : public TriggerSkill{
public:
	rosesuigintouTrigger() : TriggerSkill("rosesuigintou"){
		relate_to_place = "head";
		view_as_skill = new rosesuigintou;
		events << EventPhaseStart;
	}

	virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
		if (!TriggerSkill::triggerable(player)) return QStringList();
		if (player->getPhase() == Player::RoundStart)
			room->removePlayerDisableShow(player, "rosesuigintou");
		return QStringList();
	}
};

class meijiesuigintou : public TriggerSkill{
public:
	meijiesuigintou() : TriggerSkill("meijiesuigintou"){
		frequency = Compulsory;
		relate_to_place = "deputy";
	}
};

MingmingCard::MingmingCard() {
	will_throw = true;
	target_fixed = true;
}

void MingmingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
	ArcheryAttack *aa = new ArcheryAttack(Card::NoSuit, 0);
	aa->setSkillName("mingming");
	room->useCard(CardUseStruct(aa, source, room->getOtherPlayers(source), true));
}

class Mingming : public	ViewAsSkill{
public:
	Mingming() : ViewAsSkill("mingming"){
	}

	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		if (selected.isEmpty())
			return (to_select->isBlack() && !to_select->isEquipped());
		else if (selected.length() == 1){
			return !to_select->isEquipped() && ((to_select->getSuit() == selected.first()->getSuit()) || (to_select->isBlack() && Self->hasSkill("meijiesuigintou")));
		}

		return false;
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		return player->getHandcardNum() >= 2 && !player->hasUsed("MingmingCard");
	}

	virtual const Card *viewAs(const QList<const Card *> &cards) const{
		if (cards.length() == 2){
			MingmingCard *mm = new MingmingCard;
			mm->setShowSkill(objectName());
			mm->addSubcards(cards);
			return mm;
		}
		return NULL;
	}
};



class roseshinku : public TriggerSkill{
public:
	roseshinku() : TriggerSkill("roseshinku"){
		relate_to_place = "head";
		events << EventPhaseStart;
		frequency = NotFrequent;
	}

	virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
		if (!TriggerSkill::triggerable(player)) return QStringList();
		if (event == EventPhaseStart){
			if (player->getPhase() == Player::RoundStart)
				room->removePlayerDisableShow(player, objectName());
		}
		return QStringList();
	}
};

class meijieshinku : public TriggerSkill{
public:
	meijieshinku() : TriggerSkill("meijieshinku"){
		frequency = Compulsory;
		relate_to_place = "deputy";
	}
};

HeliCard::HeliCard() {
	will_throw = true;
	target_fixed = false;
}

bool HeliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
	return targets.isEmpty() && to_select != Self;
}

void HeliCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
	ServerPlayer *target = targets.first();
	bool invoke = false;

	if (source->hasSkill("roseshinku") && !source->getActualGeneral2Name().contains("sujiang") && source->hasShownGeneral2() && source->askForSkillInvoke("roseshinku")){
		invoke = true;
		source->hideGeneral(false);
		room->setPlayerDisableShow(source, "d", "roseshinku");
	}


	room->damage(DamageStruct("heli", source, target, invoke ? qMin(3, subcardsLength() - 1) : qMin(3, subcardsLength() / 2), DamageStruct::Normal));
}

class Heli : public	ViewAsSkill{
public:
	Heli() : ViewAsSkill("heli"){
	}

	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		return !to_select->isEquipped() && (selected.length() + 1 <= Self->aliveCount()) && (to_select->isRed() || (Self->hasSkill("meijieshinku") && !Self->getActualGeneral1Name().contains("sujiang")));
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		return player->getCardCount(true) >=2 && !player->hasUsed("HeliCard");
	}

	virtual const Card *viewAs(const QList<const Card *> &cards) const{
		if (cards.length() >= 2){
			HeliCard *hl = new HeliCard;
			hl->setShowSkill(objectName());
			hl->addSubcards(cards);
			return hl;
		}
		return NULL;
	}
};

//ziwo by official hegemony
class Ziwo : public TriggerSkill {
public:
	Ziwo() : TriggerSkill("ziwo") {
		events << GeneralShown << GeneralHidden << GeneralRemoved << EventPhaseStart << Death << EventAcquireSkill << EventLoseSkill;
	}

	void doZiwo(Room *room, ServerPlayer *kyouko, bool set) const{
		if (set && !kyouko->tag["ziwo"].toBool()) {
			foreach(ServerPlayer *p, room->getOtherPlayers(kyouko))
				room->setPlayerDisableShow(p, "hd", "ziwo");

			kyouko->tag["ziwo"] = true;
		}
		else if (!set && kyouko->tag["ziwo"].toBool()) {
			foreach(ServerPlayer *p, room->getOtherPlayers(kyouko))
				room->removePlayerDisableShow(p, "ziwo");

			kyouko->tag["ziwo"] = false;
		}
	}

	virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
		QMap<ServerPlayer *, QStringList> r;
		if (player == NULL)
			return r;
		if (triggerEvent != Death && !player->isAlive())
			return r;
		ServerPlayer *c = room->getCurrent();
		if (c == NULL || (triggerEvent != EventPhaseStart && c->getPhase() == Player::NotActive) || c != player)
			return r;

		if ((triggerEvent == GeneralShown || triggerEvent == EventPhaseStart || triggerEvent == EventAcquireSkill) && !player->hasShownSkill(this))
			return r;
		if ((triggerEvent == GeneralShown || triggerEvent == GeneralHidden) && (!player->ownSkill(this) || player->inHeadSkills(this) != data.toBool()))
			return r;
		if (triggerEvent == GeneralRemoved && data.toString() != "zoushi")
			return r;
		if (triggerEvent == EventPhaseStart && !(player->getPhase() == Player::RoundStart || player->getPhase() == Player::NotActive))
			return r;
		if (triggerEvent == Death && (data.value<DeathStruct>().who != player || !player->hasShownSkill(this)))
			return r;
		if ((triggerEvent == EventAcquireSkill || triggerEvent == EventLoseSkill) && data.toString() != objectName())
			return r;

		bool set = false;
		if (triggerEvent == GeneralShown || triggerEvent == EventAcquireSkill || (triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart))
			set = true;

		doZiwo(room, player, set);

		return r;
	}
};

#include <roomthread.h>
//baozou by hmqgg

BaozouCard::BaozouCard() {
	mute = true;
}

bool BaozouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
	Slash *slash = new Slash(NoSuit, 0);
	slash->setSkillName("baozou");
	slash->deleteLater();
	return slash->targetFilter(targets, to_select, Self);
}

void BaozouCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
	foreach(ServerPlayer *target, targets) {
		if (!source->canSlash(target, NULL, true))
			targets.removeOne(target);
	}

	if (targets.length() > 0) {

		QVariantList target_list;
		foreach(ServerPlayer *target, targets){
			target_list << QVariant::fromValue(target);
		}

		source->tag["baozou_invoke"] = target_list;
	}
}

class BaozouVS : public ViewAsSkill {
public:
	BaozouVS() : ViewAsSkill("baozou") {
	}

	virtual bool viewFilter(const QList<const Card *> &, const Card *) const{
		return false;
	}

	virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
		return pattern == "@@baozou";
	}

	virtual bool isEnabledAtPlay(const Player *) const{
		return false;
	}

	virtual const Card *viewAs(const QList<const Card *> &) const{
		BaozouCard *bz = new BaozouCard;
		bz->setShowSkill("baozou");
		return bz;
	}
};

class Baozou : public TriggerSkill{
public:
	Baozou() : TriggerSkill("baozou"){
		events << Damage;
		view_as_skill = new BaozouVS;
	}

	virtual QStringList triggerable(TriggerEvent , Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		DamageStruct damage = data.value<DamageStruct>();
		if (damage.card && damage.card->isKindOf("Slash") && damage.to && damage.to->isAlive() && !(damage.transfer || damage.chain || damage.prevented || damage.damage < 1))
			return QStringList(objectName()); 
		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		return player->askForSkillInvoke(this); //TODO: different sounds in effect
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		int id = room->drawCard();
		if (id != -1){
			CardsMoveStruct move;
			move.card_ids << id;
			move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), objectName(), QString());
			move.to_place = Player::PlaceTable;
			room->moveCardsAtomic(move, true);
			room->getThread()->delay();

			if (Sanguosha->getEngineCard(id)->isKindOf("Slash")){
				if (room->askForUseCard(player, "@@baozou", "@baozou", -1, Card::MethodUse, false)){
					QVariantList target_list;

					target_list = player->tag["baozou_invoke"].toList();
					player->tag.remove("baozou_invoke");

					Slash *slash = new Slash(Card::SuitToBeDecided, -1);
					slash->addSubcard(id);
					slash->setSkillName("_baozou");
					QList<ServerPlayer *> targets;
					foreach(QVariant x, target_list){
						targets << x.value<ServerPlayer *>();
					}

					room->useCard(CardUseStruct(slash, player, targets));
				}
				else{
					CardsMoveStruct new_move;
					new_move.card_ids << id;
					new_move.reason = CardMoveReason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
					new_move.from_place = Player::PlaceTable;
					new_move.to_place = Player::DiscardPile;
					room->moveCardsAtomic(new_move, true);
				}
			}
			else {
				CardsMoveStruct new_move;
				new_move.card_ids << id;
				new_move.from_place = Player::PlaceTable;
				
				if (player->canDiscard(player, "h") && room->askForDiscard(player, objectName(), 1, 1, true, false, "@baozou_discard", true)){
					new_move.to = player;
					new_move.to_place = Player::PlaceHand;
					new_move.reason = CardMoveReason(CardMoveReason::S_REASON_GOTCARD, player->objectName(), objectName(), QString());
				}
				else{
					new_move.to_place = Player::DiscardPile;
					new_move.reason = CardMoveReason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
				}
				room->moveCardsAtomic(new_move, true);
			}
		}
		return false;
	}
};

//maoshi by official Hegemony
class Maoshi : public TriggerSkill {
public:
	Maoshi() : TriggerSkill("maoshi") {
		events << CardsMoveOneTime;
		frequency = Frequent;
	}

	virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *sunshangxiang, QVariant &data, ServerPlayer * &) const{
		if (!TriggerSkill::triggerable(sunshangxiang)) return QStringList();
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		if (move.from == sunshangxiang && move.from_places.contains(Player::PlaceEquip)) {
			return QStringList(objectName());
		}
		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const{
		if (sunshangxiang->askForSkillInvoke(this)){
			room->broadcastSkillInvoke(objectName(), sunshangxiang);
			return true;
		}

		return false;
	}

	virtual bool effect(TriggerEvent, Room *, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const{
		sunshangxiang->drawCards(2);

		return false;
	}
};

ZhiyuCard::ZhiyuCard() {
}

bool ZhiyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
	if (!targets.isEmpty())
		return false;

	return (Self->isFriendWith(to_select) || Self->willBeFriendWith(to_select)) && to_select->isWounded() && to_select != Self;
}

void ZhiyuCard::onEffect(const CardEffectStruct &effect) const{
	Room *room = effect.from->getRoom();
	RecoverStruct recover;
	recover.card = this;
	recover.who = effect.from;

	room->recover(effect.from, recover, true);
	room->recover(effect.to, recover, true);
}

class Zhiyu : public ViewAsSkill {
public:
	Zhiyu() : ViewAsSkill("zhiyu") {
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		return player->getHandcardNum() >= 2 && !player->hasUsed("ZhiyuCard");
	}

	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		if (selected.length() > 1 || Self->isJilei(to_select))
			return false;

		return !to_select->isEquipped();
	}

	virtual const Card *viewAs(const QList<const Card *> &cards) const{
		if (cards.length() != 2)
			return NULL;

		ZhiyuCard *zhiyuCard = new ZhiyuCard();
		zhiyuCard->addSubcards(cards);
		zhiyuCard->setShowSkill(objectName());
		return zhiyuCard;
	}
};

class Aolin : public TriggerSkill{
public:
	Aolin() : TriggerSkill("aolin"){
		events << Damage;
	}

	virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		DamageStruct damage = data.value<DamageStruct>();
		if (player->hasShownSkill(this) && damage.card && damage.card->isKindOf("Slash") && !(damage.transfer || damage.prevented || damage.chain || damage.damage < 1 || player->getActualGeneral2Name().contains("sujiang")))
			return QStringList(objectName());
		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		if (player->askForSkillInvoke(this)){
			room->broadcastSkillInvoke(objectName());
			return true;
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		QString str = "";
		if (player->getHeadSkillList().contains(this))
			str = str + "head";
		if (player->getDeputySkillList().contains(this))
			str = str + "deputy";

		if (str == "headdeputy")
			player->hideGeneral(room->askForChoice(player, objectName(), "GeneralA+GeneralB") == "GeneralA" ? true : false);
		else{
			if (str.contains("head")) player->hideGeneral(true);
			if (str.contains("deputy")) player->hideGeneral(false);
		}

		player->drawCards(1);
		return false;
	}
};

class TsukkomiVS : public ZeroCardViewAsSkill {
public:
	TsukkomiVS() : ZeroCardViewAsSkill("tsukkomi") {
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		Slash *slash = new Slash(Card::NoSuit, 0);
		slash->deleteLater();
		return slash->isAvailable(player) && !player->hasShownSkill(objectName()) && !player->hasFlag("tsukkomi_used");
	}
	
	virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
		return  !player->hasFlag("tsukkomi_used") && (pattern == "Slash" || pattern == "nullification") && !player->hasShownSkill("tsukkomi");
	}

	virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
		return !player->hasFlag("tsukkomi_used") && !player->hasShownSkill("tsukkomi");
	}

	virtual bool viewFilter(const QList<const Card *> &, const Card *) const{
		return false;
	}

	virtual const Card *viewAs() const{
		CardUseStruct::CardUseReason reason = Sanguosha->getCurrentCardUseReason();
		if (reason == CardUseStruct::CARD_USE_REASON_PLAY){
			Slash *slash = new Slash(Card::NoSuit, 0);
			slash->setSkillName(objectName());
			slash->setShowSkill(objectName());
			return slash;
		}
		else{
			if (reason == CardUseStruct::CARD_USE_REASON_RESPONSE_USE){
				Nullification *nulli = new Nullification(Card::NoSuit, 0);
				nulli->setSkillName(objectName());
				nulli->setShowSkill(objectName());
				return nulli;
			}
		}
		return NULL;
	}
};

class Tsukkomi : public TriggerSkill{
public:
	Tsukkomi() : TriggerSkill("tsukkomi"){
		events << CardUsed;
		view_as_skill = new TsukkomiVS;
	}

	virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		CardUseStruct use = data.value<CardUseStruct>();
		if (use.card->getSkillName().contains(objectName())){
			player->setFlags("tsukkomi_used");
		}
		return QStringList();
	}
};

class Xipin : public TriggerSkill{
public :
	Xipin() : TriggerSkill("xipin"){
		events << CardAsked;
	}

	virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();

		if (data.toStringList().first() == "jink")
			return QStringList(objectName());

		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		if (player->askForSkillInvoke(this)) {
			room->broadcastSkillInvoke(objectName());
			return true;
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		bool has_given = false;
		foreach(ServerPlayer *p, room->getOtherPlayers(player)){
			if (p->getHandcardNum() > 0){
				const Card *card = room->askForExchange(p, objectName(), 1, false, "@xipin_give", true);
				if (card){
					CardsMoveStruct move;
					move.card_ids << card->getEffectiveId();
					move.from = p;
					move.from_place = Player::PlaceHand;
					move.to = player;
					move.to_place = Player::PlaceHand;
					move.reason = CardMoveReason(CardMoveReason::S_REASON_GIVE, p->objectName(), player->objectName(), objectName(), QString());

					room->moveCardsAtomic(move, false);

					has_given = true;
				}
			}
		}
		if (!has_given){
			Jink *jink = new Jink(Card::NoSuit, 0);
			jink->setSkillName(objectName());
			room->provide(jink);

			return true;
		}
		
		return false;
	}
};

class Zhaihun : public TriggerSkill{
public:
	Zhaihun() : TriggerSkill("zhaihun"){
		events << CardsMoveOneTime << EventPhaseEnd;
	}

	virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (event == CardsMoveOneTime) {
			if (!player || !player->isAlive() || !player->hasSkill("zhaihun")) return QStringList();
			ServerPlayer *current = room->getCurrent();
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			if (player != current)
				return QStringList();

			if (move.reason.m_skillName == objectName() && move.reason.m_playerId == player->objectName() && Sanguosha->getEngineCard(move.card_ids.first()) && Sanguosha->getEngineCard(move.card_ids.first())->isRed() && player->isWounded())
				return QStringList(objectName());

			if (current->getPhase() == Player::Discard) {
				QVariantList zhaihuncards = player->tag["ZhaihunCards"].toList();

				if ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
					foreach(int card_id, move.card_ids)
						if (!zhaihuncards.contains(card_id))
							zhaihuncards << card_id;
				}

				player->tag["ZhaihunCards"] = zhaihuncards;
			}

			return QStringList();
		}
		else{
			if (!TriggerSkill::triggerable(player)) return QStringList();
			if ((player->getPhase() != Player::Discard)) return QStringList();

			QVariantList zhaihuncards = player->tag["ZhaihunCards"].toList();

			QList<int> card_ids;

			foreach(QVariant card_data, zhaihuncards) {
				int card_id = card_data.toInt();
				if (room->getCardPlace(card_id) == Player::DiscardPile)
					card_ids << card_id;
			}

			if (card_ids.isEmpty()){
				player->tag.remove("ZhaihunCards");
				return QStringList();
			}

			int i = 1;
			QVariantList targets_list;
			foreach(ServerPlayer *p, room->getOtherPlayers(player)){
				if (player->inMyAttackRange(p) && i <= card_ids.length()){
					targets_list << QVariant::fromValue(p);
					i++;
				}
			}

			if (!targets_list.isEmpty()) return QStringList(objectName());
			else player->tag.remove("ZhaihunCards");

		}
		return QStringList();
	}

	virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		if (event == CardsMoveOneTime){
			room->broadcastSkillInvoke(objectName(), 2);
			room->notifySkillInvoked(player, objectName());
			return true;
		}
		else{
			return player->hasShownSkill(this) ? true : player->askForSkillInvoke(this);
		}
		return false;
	}

	virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		if (event == EventPhaseEnd){
			QVariantList zhaihuncards = player->tag["ZhaihunCards"].toList();

			QList<int> card_ids;

			foreach(QVariant card_data, zhaihuncards) {
				int card_id = card_data.toInt();
				if (room->getCardPlace(card_id) == Player::DiscardPile)
					card_ids << card_id;
			}

			if (card_ids.isEmpty()){
				player->tag.remove("ZhaihunCards");
				return false;
			}

			foreach(ServerPlayer *p, room->getOtherPlayers(player)){
				if (player->inMyAttackRange(p) && !card_ids.isEmpty()){
					room->broadcastSkillInvoke(objectName(), p);
					room->fillAG(card_ids, p);

					int id = room->askForAG(p, card_ids, true, objectName());
					if (id != -1){
						room->obtainCard(p, Sanguosha->getCard(id), CardMoveReason(CardMoveReason::S_REASON_GOTBACK, player->objectName(), p->objectName(), objectName(), QString()), true);
						if (card_ids.contains(id))
							card_ids.removeOne(id);
					}

					room->clearAG(p);
				}
			}

			player->tag.remove("ZhaihunCards");

		}
		else{
			RecoverStruct recover;
			recover.who = player;
			recover.recover = 1;
			room->recover(player, recover);
		}
		return false;
	}
};

class Qinlve : public TriggerSkill{
public:
	Qinlve() : TriggerSkill("qinlve"){
		events << EventPhaseChanging;
	}

	virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		PhaseChangeStruct change = data.value<PhaseChangeStruct>();
		if (change.to == Player::Play && !player->hasFlag("qinlve_failed") && player->getHandcardNum() > 0 && !player->isSkipped(change.to)){
			bool invoke = false;
			foreach(ServerPlayer *p, room->getOtherPlayers(player)){
				if (p->getHandcardNum() > 0)
					invoke = true;
			}
			if (invoke)
				return QStringList(objectName());
		}
		return QStringList();
	}
	
	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const{
		QList<ServerPlayer *> targets;
		foreach(ServerPlayer *p, room->getOtherPlayers(player))
			if (p->getHandcardNum() > 0)
				targets << p;

		if (!targets.isEmpty()){
			ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@qinlve_target", true);
			if (target != NULL){
				PindianStruct *pindian = player->pindianSelect(target, objectName());

				if (pindian != NULL){
					player->tag["qinlve_pindian"] = QVariant::fromValue(pindian);
					return true;
				}
			}
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer*) const{
		if (!player->isSkipped(Player::Play))
			player->skip(Player::Play, true);
		PindianStruct *pindian = player->tag["qinlve_pindian"].value<PindianStruct *>();
		player->tag.remove("qinlve_pindian");
		ServerPlayer *target = pindian->to;
		if (!target || !target->isAlive())
			return false;
		bool success = player->pindian(pindian);
		if (success){
			Slash *slash = new Slash(Card::NoSuit, 0);
			slash->setSkillName("_qinlve");

			if (player->canSlash(target, false, 0, QList<const Player*>()))
				room->useCard(CardUseStruct(slash, player, target));

			ServerPlayer *nil = NULL;
			if (cost(EventPhaseChanging, room, player, data, nil))
				effect(EventPhaseChanging, room, player, data, nil);
		}
		return false;
	}
};

BaibianCard::BaibianCard() {
	target_fixed = true;
}

void BaibianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
	if (source->isAlive())
		room->drawCards(source, subcards.length());
}

class Baibian : public ViewAsSkill {
public:
	Baibian() : ViewAsSkill("baibian") {
	}

	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		int x = 0;
		foreach(const Card * card, selected)
			x = x + card->getNumber();
		return (x + to_select->getNumber() <= 52) && !Self->isJilei(to_select) ;
	}

	virtual const Card *viewAs(const QList<const Card *> &cards) const{
		if (cards.isEmpty())
			return NULL;

		BaibianCard *bbcard = new BaibianCard;
		bbcard->addSubcards(cards);
		bbcard->setSkillName(objectName());
		bbcard->setShowSkill(objectName());
		return bbcard;
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		return player->canDiscard(player, "he") && !player->hasUsed("BaibianCard");
	}
};

class Yujian : public TriggerSkill{
public:
	Yujian() : TriggerSkill("yujian"){
		events << TargetChosen << Damage << CardFinished;
		frequency = Frequent;
	}

	virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (event == TargetChosen){
			CardUseStruct use = data.value<CardUseStruct>();
			if (!TriggerSkill::triggerable(use.from)) return QStringList();
			if (use.to.length() > 0 && use.from == player && use.card->isKindOf("Slash"))
				return QStringList(objectName());
		}
		else if (event == Damage){
			if (player->hasFlag("forecastWillDamage")){
				player->setFlags("-forecastWillDamage");
				player->setFlags("forecastTrue");
			}
			if (player->hasFlag("forecastWontDamage")){
				player->setFlags("-forecastWontDamage");
			}
		}
		else if (event == CardFinished){
			if (player->hasFlag("forecastTrue") || player->hasFlag("forecastWontDamage")){
				player->setFlags("-forecastTrue");
				player->setFlags("-forecastWontDamage");
				player->drawCards(1);
			}
		}
		return QStringList();
	}

	virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		if (event == TargetChosen){
			if (room->askForSkillInvoke(player, objectName())){
				room->broadcastSkillInvoke(objectName());
				return true;
			}
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		QString choice = room->askForChoice(player, objectName(), "forecastWillDamage+forecastWontDamage");
		if (choice != ".")
			player->setFlags(choice);

		return false;
	}
};

class Tiruo : public TriggerSkill{
public:
	Tiruo() : TriggerSkill("tiruo"){
		events << TargetConfirmed;
		frequency = NotFrequent;
	}

	virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		CardUseStruct use = data.value<CardUseStruct>();
		if (use.from && use.from->isAlive() && use.to.contains(player) && use.card->isKindOf("Slash") && player->canDiscard(player, "he"))
			return QStringList(objectName());
		return QStringList();
	}

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
		if (room->askForCard(player, "..", QString("@tiruo_discard:" + data.value<CardUseStruct>().from->objectName()), QVariant(), objectName())){
			room->broadcastSkillInvoke(objectName());
			return true;
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
		CardUseStruct use = data.value<CardUseStruct>();
		room->notifySkillInvoked(player, objectName());

		room->cancelTarget(use, player);

		use.from->drawCards(1);

		data = QVariant::fromValue(use);
		return false;
	}
};

LingshangCard::LingshangCard() {
	mute = true;
	will_throw = true;
	target_fixed = true;
}

class LingshangVS : public ViewAsSkill {
public:
	LingshangVS() : ViewAsSkill("lingshang") {
	}

	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		if (selected.length() >= 4) return false;
		if (!selected.isEmpty())
			return to_select->getSuit() == selected.first()->getSuit() && !to_select->isEquipped();
		else
			return !to_select->isEquipped();
		return false;
	}

	virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
		return pattern == "@@lingshang";
	}

	virtual bool isEnabledAtPlay(const Player *) const{
		return false;
	}

	virtual const Card *viewAs(const QList<const Card *> &cards) const{
		int heart = 0;
		int diamond = 0;
		int spade = 0;
		int club = 0;
		if (!Self->getEquips().isEmpty()){
			foreach(const Card *equip, Self->getEquips()){
				switch (equip->getSuit()) {
				case Card::Heart :
					heart++;
					break;
				case Card::Diamond :
					diamond++;
					break;
				case Card::Spade :
					spade++;
					break;
				case Card::Club :
					club++;
					break;
				default:
					break;
				}
			}
		}

		if (!cards.isEmpty()){
			foreach(const Card *card, cards){
				switch (card->getSuit()) {
				case Card::Heart:
					heart++;
					break;
				case Card::Diamond:
					diamond++;
					break;
				case Card::Spade:
					spade++;
					break;
				case Card::Club:
					club++;
					break;
				default:
					break;
				}
			}
		}

		if (qMax(qMax(qMax(heart, diamond), spade), club) == 4 && !cards.isEmpty()){
			LingshangCard *ls = new LingshangCard;
			ls->setSkillName("lingshang");
			ls->setShowSkill("lingshang");
			ls->addSubcards(cards);
			return ls;
		}
		return NULL;
	}
};

class Lingshang : public TriggerSkill{
public:
	Lingshang() : TriggerSkill("lingshang"){
		events << CardsMoveOneTime;
		view_as_skill = new LingshangVS;
	}

	virtual bool canPreshow() const{
		return true;
	}

	virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
		if (!TriggerSkill::triggerable(player)) return QStringList();
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		if (move.from && move.from == player && move.from_places.contains(Player::PlaceEquip)) return QStringList();
		if (move.to == player && move.to_place == Player::PlaceHand && room->getCurrent() && room->getCurrent() == player && player->canDiscard(player, "h")){
			return QStringList(objectName());
		}
		return QStringList(); 
	}

	virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		if (room->askForUseCard(player, "@@lingshang", "@lingshang")){
			room->broadcastSkillInvoke(objectName());
			return true;
		}
		return false;
	}

	virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
		room->addPlayerMark(player, "@gang", 1);
		room->drawCards(player, 1);
		return false;
	}
};

KaihuaCard::KaihuaCard() {
	target_fixed = true;
	mute = true;
}

void KaihuaCard::onUse(Room *room, const CardUseStruct &card_use) const{
	room->removePlayerMark(card_use.from, "@gang", qMax(card_use.from->getMark("kaihuaTimes"), 1));
	card_use.from->setMark("kaihuaTimes", card_use.from->getMark("kaihuaTimes") + 1);
	room->broadcastSkillInvoke("kaihua", card_use.from);

	CardUseStruct new_use = card_use;
	new_use.to << room->getOtherPlayers(card_use.from);

	Card::onUse(room, new_use);
}

void KaihuaCard::onEffect(const CardEffectStruct &effect) const{
	Room *room = effect.to->getRoom();

	const Card *equip = room->askForCard(effect.to, "EquipCard", QString("@kaihua_pass:" + effect.from->objectName()), QVariant(), Card::MethodNone, 0, false, objectName());
	if (equip)
		room->obtainCard(effect.from, equip, CardMoveReason(CardMoveReason::S_REASON_GIVE, effect.to->objectName(), effect.from->objectName(), "kaihua", ""), true);
	else{
		QString choice = "kaihuaDamage";
		if (effect.from->isWounded())
			choice = choice + "+kaihuaRecover";
		choice = room->askForChoice(effect.to, "kaihua", choice);
		if (choice == "kaihuaDamage")
			room->damage(DamageStruct("kaihua", effect.from, effect.to));
		else{
			RecoverStruct recover;
			recover.recover = 1;
			recover.who = effect.from;
			recover.card = NULL;
			room->recover(effect.from, recover);
		}
	}
}

class Kaihua : public ZeroCardViewAsSkill {
public:
	Kaihua() : ZeroCardViewAsSkill("kaihua") {
	}

	virtual bool isEnabledAtPlay(const Player *player) const{
		return player->getMark("@gang") >= qMax(player->getMark("kaihuaTimes"), 1) && !player->hasUsed("KaihuaCard");
	}

	virtual const Card *viewAs() const{
		KaihuaCard *kh = new KaihuaCard;
		kh->setShowSkill(objectName());
		return kh;
	}
};

void MoesenPackage::addComicGenerals(){

    General *hinagiku = new General(this, "hinagiku", "shu", 5, false); // Comic 001  (@todo:should change No.)
    hinagiku->addSkill(new Jiandao);
    hinagiku->addSkill(new JiandaoRange);
    insertRelatedSkills("jiandao", "#jiandao-range");

    General *akari = new General(this, "akari", "shu", 3, false); // Comic 007
    akari->addSkill(new Wucun);
    akari->addSkill(new Kongni);

    General *nagi = new General(this, "nagi", "shu", 3, false); // Comic 003
	nagi->addSkill(new Tianzi);
	nagi->addSkill(new Yuzhai);
	
	General *izumi = new General(this, "izumi", "shu", 3, false); // Comic 002
	izumi->addSkill(new Mizou);
	izumi->addSkill(new Wushu);

    General *suiseiseki = new General(this, "suiseiseki", "shu", 3, false); // Comic 004
	suiseiseki->addSkill(new Shuimeng);
	suiseiseki->addSkill(new rosesuiseiseki);
	suiseiseki->addSkill(new meijiesuiseiseki);

    General *suigintou = new General(this, "suigintou", "shu", 3, false); // Comic 017
	suigintou->addSkill(new Mingming);
	suigintou->addSkill(new rosesuigintouTrigger);
	suigintou->addSkill(new meijiesuigintou);
	
	General *shinku = new General(this, "shinku", "shu", 3, false); // Comic 005
	shinku->addSkill(new Heli); 
	shinku->addSkill(new roseshinku);
	shinku->addSkill(new meijieshinku);

    General *t_kyouko = new General(this, "t_kyouko", "shu", 3, false); // Comic 006
	t_kyouko->addSkill(new Ziwo);
	t_kyouko->addSkill(new Baozou);

    General *tsukasa = new General(this, "tsukasa", "shu", 3, false); // Comic 010
	tsukasa->addSkill(new Zhiyu);
	tsukasa->addSkill(new Maoshi);

    General *kagami = new General(this, "kagami", "shu", 3, false); // Comic 009
	kagami->addSkill(new Tsukkomi);
	kagami->addSkill(new Aolin);

    General *konata = new General(this, "konata", "shu", 3, false); // Comic 008
	konata->addSkill(new Xipin);
	konata->addSkill(new Zhaihun);

    General *ika = new General(this, "ika", "shu", 4, false); // Comic 018
	ika->addSkill(new Qinlve);

    General *sakura = new General(this, "sakura", "shu", 4, false); // Comic 011
	sakura->addSkill(new Baibian);

    General *toki = new General(this, "toki", "shu", 3, false); // Comic 012
	toki->addSkill(new Tiruo);
	toki->addSkill(new Yujian);

    General *saki = new General(this, "saki", "shu", 4, false); // Comic 013
	saki->addSkill(new Lingshang);
	saki->addSkill(new Kaihua);
    /*

    General *nodoka = new General(this, "nodoka", "shu", 3, false); // Comic 014

    General *koromo = new General(this, "koromo", "shu", 3, false); // Comic 015

    General *shizuno = new General(this, "shizuno", "shu", 3, false); // Comic 016

    */

	addMetaObject<ShuimengCard>();
	addMetaObject<rosesuigintouCard>();
	addMetaObject<MingmingCard>();
	addMetaObject<HeliCard>();
	addMetaObject<ZhiyuCard>();
	addMetaObject<BaozouCard>();
	addMetaObject<BaibianCard>();
	addMetaObject<LingshangCard>();
	addMetaObject<KaihuaCard>();
	addMetaObject<SuanlvCard>();
}
