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
		if (room->askForCard(player, ".", "@mizou_discard", QVariant(), objectName())) {
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
		if (move.to_place == Player::Discard && move.reason.m_reason == CardMoveReason::S_REASON_JUDGEDONE){
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
		return (selected.length() + 1 <= Self->aliveCount()) && (to_select->isRed() || (Self->hasSkill("meijieshinku") && !Self->getActualGeneral1Name().contains("sujiang")));
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


void MoesenPackage::addComicGenerals(){

    General *hinagiku = new General(this, "hinagiku", "shu", 5, false); // Comic 001  (should change No.)
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

    /*




    General *t_kyouko = new General(this, "t_kyouko", "shu", 3, false); // Comic 006

    General *konata = new General(this, "konata", "shu", 3, false); // Comic 008

    General *kagami = new General(this, "kagami", "shu", 3, false); // Comic 009

    General *tsukasa = new General(this, "tsukasa", "shu", 3, false); // Comic 010

    General *sakura = new General(this, "sakura", "shu", 3, false); // Comic 011

    General *toki = new General(this, "toki", "shu", 3, false); // Comic 012

    General *saki = new General(this, "saki", "shu", 3, false); // Comic 013

    General *nodoka = new General(this, "nodoka", "shu", 3, false); // Comic 014

    General *koromo = new General(this, "koromo", "shu", 3, false); // Comic 015

    General *shizuno = new General(this, "shizuno", "shu", 3, false); // Comic 016


    General *ika = new General(this, "ika", "shu", 3, false); // Comic 018
    */

	addMetaObject<ShuimengCard>();
	addMetaObject<rosesuigintouCard>();
	addMetaObject<MingmingCard>();
	addMetaObject<HeliCard>();
}
