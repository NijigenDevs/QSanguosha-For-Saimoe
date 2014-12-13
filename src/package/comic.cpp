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

	virtual bool effect(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
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
    /*

    General *izumi = new General(this, "izumi", "shu", 3, false); // Comic 002



    General *suiseiseki = new General(this, "suiseiseki", "shu", 3, false); // Comic 004

    General *shinku = new General(this, "shinku", "shu", 3, false); // Comic 005

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

    General *suigintou = new General(this, "suigintou", "shu", 3, false); // Comic 017

    General *ika = new General(this, "ika", "shu", 3, false); // Comic 018
    */
}
