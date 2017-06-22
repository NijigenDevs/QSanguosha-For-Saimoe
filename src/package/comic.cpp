#include "comic.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "strategic-advantage.h"
#include "engine.h"
#include "client.h"

class Wucun : public TriggerSkill
{
public:
    Wucun() : TriggerSkill("wucun")
    {
        events << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player != NULL && player->getPhase() == Player::Start)
        {
            QList<ServerPlayer *> akaris = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *akari, akaris)
            {
                LureTiger *luretiger = new LureTiger(Card::SuitToBeDecided, 0);
                QList<const Player *> targets;
                if ((!akari->willBeFriendWith(player)) && (!akari->isFriendWith(player)) && player->hasShownOneGeneral() && akari->getHandcardNum() <= akari->getHp() && room->alivePlayerCount() > 2 && luretiger->targetFilter(targets, akari, player) && !player->isProhibited(akari, luretiger, targets))
                {
                    luretiger->deleteLater();
                    skill_list.insert(akari, QStringList(objectName()));
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        if (ask_who->askForSkillInvoke(objectName(), data))
        {
            room->drawCards(ask_who, 1, objectName());
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const
    {
        LureTiger *luretiger = new LureTiger(Card::SuitToBeDecided, 0);
        luretiger->setSkillName(objectName());
        QList<const Player *> targets;
        if (luretiger->targetFilter(targets, ask_who, player) && !player->isProhibited(ask_who, luretiger, targets))
            room->useCard(CardUseStruct(luretiger, player, ask_who));
        return false;
    }
};

class Kongni : public TriggerSkill
{
public:
    Kongni() : TriggerSkill("kongni")
    {
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        if (player->getEquips().length() > 0) return QStringList();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (effect.slash->isBlack()) return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        bool invoke = player->hasShownSkill(this) ? true : player->askForSkillInvoke(objectName(), data);
        if (invoke)
        {
            if (player->hasShownSkill(this))
            {
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

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const
    {
        return true;
    }
};

class TianziVS : public ZeroCardViewAsSkill
{
public:
    TianziVS() : ZeroCardViewAsSkill("tianzi")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("TianziCard");
    }

    virtual const Card *viewAs() const
    {
        TianziCard *card = new TianziCard;
        card->setShowSkill(objectName());
        return card;
    }
};

class Tianzi : public TriggerSkill
{
public:
    Tianzi() : TriggerSkill("tianzi")
    {
        events << DrawNCards << EventPhaseEnd << CardsMoveOneTime << EventPhaseChanging;
        view_as_skill = new TianziVS;
        frequency = NotFrequent;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseChanging)
        {
            auto change = data.value<PhaseChangeStruct>();
            if (player != NULL && change.to == Player::NotActive)
            {
                if (player->getMark("@tianzi_draw") > 0)
                    room->setPlayerMark(player, "@tianzi_draw", 0);

                if (player->getMark("tianzidiscards") > 0)
                    room->setPlayerMark(player, "tianzidiscards", 0);
            }
        }
        else if (event == CardsMoveOneTime)
        {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != NULL && player == move.from && player->getMark("@tianzi_draw") > 0 && player->getPhase() == Player::Discard
                && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD && move.to_place == Player::PlaceTable)
            {
                foreach(int id, move.card_ids)
                    if (Sanguosha->getEngineCard(id)->isKindOf("TrickCard"))
                        room->setPlayerMark(player, "tianzidiscards", player->getMark("tianzidiscards") + 1);
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        if (event == DrawNCards)
        {
            return QStringList(objectName());
        }
        else if (event == EventPhaseEnd && player->getMark("tianzidiscards") > 0 && player->getPhase() == Player::Discard)
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (event == DrawNCards)
        {
            if (player->askForSkillInvoke(this))
            {
                room->broadcastSkillInvoke(objectName(), 1);
                return true;
            }
        }
        else if (event == EventPhaseEnd)
        {
            room->broadcastSkillInvoke(objectName(), 2);
            return true;
        }
        room->setPlayerMark(player, "tianzidiscards", 0);
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (event == DrawNCards)
        {
            data = data.toInt() + 1;
        }
        else
        {
            int length = player->getMark("tianzidiscards");
            if (length > 0)
            {
                room->drawCards(player, length, objectName());
                room->setPlayerMark(player, "tianzidiscards", 0);
            }
        }
        return false;
    }
};

TianziCard::TianziCard()
{
    will_throw = true;
    target_fixed = true;
}

void TianziCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->setPlayerMark(source, "@tianzi_draw", 1);
}

class TianziMaxCards : public MaxCardsSkill
{
public:
    TianziMaxCards() : MaxCardsSkill("#tianzi-maxcard")
    {
    }

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const
    {
        if (target->hasShownSkill("tianzi"))
            return -target->getMark("@tianzi_draw");
        return 0;
    }
};

class Yuzhai : public TriggerSkill
{
public:
    Yuzhai() : TriggerSkill("yuzhai")
    {
        events << EventPhaseStart << CardsMoveOneTime << EventPhaseChanging;
        frequency = NotFrequent;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == CardsMoveOneTime)
        {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != NULL && move.from == player && player->isAlive() && player->ownSkill(this) && player->getPhase() != Player::NotActive
                && move.to_place == Player::PlaceTable && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
            {
                room->setPlayerMark(player, "yuzhai_cards", player->getMark("yuzhai_cards") + move.card_ids.length());
                if (player->hasShownSkill(this))
                {
                    room->setPlayerMark(player, "@yuzhai_cards", player->getMark("yuzhai_cards"));
                }
            }
        }
        else if (event == EventPhaseChanging)
        {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
            {
                room->setPlayerMark(player, "yuzhai_cards", 0);
                room->setPlayerMark(player, "@yuzhai_cards", 0);
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        if (event == EventPhaseStart)
        {
            if (player->getPhase() == Player::Finish && player->getMark("yuzhai_cards") > player->getHp())
            {
                QList<ServerPlayer *> others = room->getOtherPlayers(player);
                bool invoke = false;
                foreach (ServerPlayer * other, others)
                {
                    if (player->canDiscard(other, "h"))
                    {
                        invoke = true;
                        break;
                    }
                }
                if (invoke)
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->setPlayerMark(player, "yuzhai_cards", 0);
        room->setPlayerMark(player, "@yuzhai_cards", 0);
        QList<ServerPlayer *> others = room->getOtherPlayers(player);
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, others)
        {
            if (player->canDiscard(p, "h"))
                targets << p;
        }
        ServerPlayer *to = room->askForPlayerChosen(player, targets, objectName(), "yuzhai-invoke", true, true);
        if (to)
        {
            player->tag["yuzhai_target"] = QVariant::fromValue(to);
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        else
            player->tag.remove("yuzhai_target");
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        ServerPlayer *to = player->tag["yuzhai_target"].value<ServerPlayer *>();
        player->tag.remove("yuzhai_target");
        if (to && player->canDiscard(to, "he"))
        {
            int card_id = room->askForCardChosen(player, to, "h", objectName(), false, Card::MethodDiscard);
            if (card_id != -1)
                room->throwCard(card_id, to, player, objectName());
        }
        return false;
    }
};

//class Mizou : public TriggerSkill
//{
//public:
//    Mizou() : TriggerSkill("mizou")
//    {
//        events << DamageCaused << DamageInflicted;
//        frequency = NotFrequent;
//    }
//
//    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const
//    {
//        if (!TriggerSkill::triggerable(player)) return QStringList();
//        if (player->canDiscard(player, "he"))
//            return QStringList(objectName());
//        return QStringList();
//    }
//
//    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        if (room->askForCard(player, ".", "@mizou_discard", data, objectName()))
//        {
//            room->broadcastSkillInvoke(objectName());
//            return true;
//        }
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        DamageStruct damage = data.value<DamageStruct>();
//
//        JudgeStruct judge;
//        judge.good = true;
//        judge.pattern = ".|red";
//        judge.play_animation = false;
//        judge.who = player;
//        judge.reason = objectName();
//        judge.time_consuming = true;
//
//        room->judge(judge);
//
//        if (judge.isGood())
//        {
//            if (damage.from && damage.from->isAlive())
//                damage.from->drawCards(1);
//            damage.damage--;
//            data = QVariant::fromValue(damage);
//        }
//
//        return false;
//    }
//};
//
//class Wushu : public TriggerSkill
//{
//public:
//    Wushu() : TriggerSkill("wushu")
//    {
//        events << BeforeCardsMove;
//        frequency = Frequent;
//    }
//
//    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
//    {
//        if (!TriggerSkill::triggerable(player)) return QStringList();
//        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
//        if (move.from != player) return QStringList();
//        if (move.to_place == Player::DiscardPile && move.reason.m_reason == CardMoveReason::S_REASON_JUDGEDONE)
//        {
//            QList<int> ids;
//            int i = 0;
//            foreach (int id, move.card_ids)
//            {
//                if (Sanguosha->getCard(id)->getColor() == Card::Black && move.from_places[i] == Player::PlaceJudge)
//                    ids << id;
//                i++;
//            }
//            if (!ids.isEmpty())
//            {
//                player->tag[objectName()] = IntList2VariantList(ids);
//                return QStringList(objectName());
//            }
//        }
//        return QStringList();
//    }
//
//    virtual bool cost(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer*) const
//    {
//        if (player->askForSkillInvoke(this))
//        {
//            player->getRoom()->broadcastSkillInvoke(objectName());
//            return true;
//        }
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer*) const
//    {
//        QList<int> ids = VariantList2IntList(player->tag[objectName()].toList());
//        if (player->isAlive() && !ids.isEmpty())
//        {
//            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
//
//            foreach (int id, ids)
//                if (move.card_ids.contains(id))
//                    move.card_ids.removeOne(id);
//            data = QVariant::fromValue(move);
//
//            DummyCard *dummy = new DummyCard(ids);
//
//            ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
//            if (target && target->isAlive())
//                room->moveCardTo(dummy, target, Player::PlaceHand, move.reason, true);
//
//            delete dummy;
//        }
//
//        player->tag[objectName()] = QVariant();
//
//        return false;
//    }
//};

class Duhun : public TriggerSkill
{
public:
    Duhun() : TriggerSkill("duhun")
    {
        events << BuryVictim;
        frequency = Frequent;
    }

    virtual TriggerList triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        TriggerList list;
        DeathStruct death = data.value<DeathStruct>();
        foreach (auto izumi, room->findPlayersBySkillName(objectName()))
        {
            if (TriggerSkill::triggerable(izumi) && death.who != NULL && izumi != death.who && (izumi->isFriendWith(death.who) || izumi->willBeFriendWith(death.who)))
            {
                list.insert(izumi, QStringList(objectName()));
            }
        }
        return list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        if (room->askForSkillInvoke(ask_who, objectName(), qVariantFromValue(data.value<DeathStruct>().who)))
        {
            room->drawCards(ask_who, 1, objectName());
            room->broadcastSkillInvoke(objectName(), ask_who);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const
    {
        DeathStruct death = data.value<DeathStruct>();
        auto from = death.damage->from;
        if (from == NULL || from->isDead())
            return false;

        room->setPlayerFlag(from, "Duhun_flag");
        return false;
    }
};

MizouCard::MizouCard()
{
    will_throw = true;
    target_fixed = true;
}

void MizouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    QList<ServerPlayer *> candidates;
    foreach (auto p, room->getAlivePlayers())
    {
        if (source->isFriendWith(p) || source->willBeFriendWith(p))
        {
            candidates << p;
        }
    }

    if (candidates.length() > 0)
    {
        if (candidates.length() == 1)
        {
            room->drawCards(source, 2, "mizou");
        }
        else
        {
            room->drawCards(candidates, 1, "mizou");
        }
    }
}

class Mizou : public ViewAsSkill
{
public: 
    Mizou() : ViewAsSkill("mizou")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return !to_select->isEquipped();
    }
    
    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() == 0 || cards.length() < Self->getHandcardNum())
            return NULL;

        auto mz = new MizouCard;
        mz->addSubcards(cards);
        mz->setShowSkill("mizou");
        return mz;
    }
    
    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->isKongcheng() && !player->hasUsed("MizouCard");
    }
};

//Rozen_Maiden 

class rosesuiseiseki : public TriggerSkill
{
public:
    rosesuiseiseki() : TriggerSkill("rosesuiseiseki")
    {
        relate_to_place = "head";
        events << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual void record(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getPhase() == Player::RoundStart)
            room->removePlayerDisableShow(player, objectName());
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }
};

class meijiesuiseiseki : public MaxCardsSkill
{
public:
    meijiesuiseiseki() : MaxCardsSkill("meijiesuiseiseki")
    {
        relate_to_place = "deputy";
    }

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const
    {
        if (target->hasShownSkill(this) && !target->getActualGeneral1Name().contains("sujiang"))
            return 1;
        return 0;
    }
};

ShuimengCard::ShuimengCard()
{
    will_throw = true;
    target_fixed = false;
}

bool ShuimengCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const
{
    return targets.isEmpty();
}

void ShuimengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    if (targets.length() > 0)
    {
        ServerPlayer *master = targets.first();
        if (source->hasSkill("rosesuiseiseki") && (!source->getActualGeneral2Name().contains("sujiang")) && (source->hasShownGeneral2()) && room->askForSkillInvoke(source, "rosesuiseiseki"))
        {
            source->hideGeneral(false);
            room->setPlayerDisableShow(source, "d", "rosesuiseiseki");
            bool d_num = room->askForDiscard(source, "shuimeng", 2, 2, false, false, "shuimeng_discard", true);
            if (d_num)
                master->drawCards(qMin(5, master->getHandcardNum()));
        }
        else
        {
            auto d_num = room->askForExchange(source, "shuimeng", 999, 2, "shuimeng_discard_any", "", ".|.|.|hand");
            DummyCard dummy(d_num);
            room->throwCard(&dummy, source, source, "shuimeng");
            if (d_num.length() >= 2)
                master->drawCards(d_num.length());
        }
    }
}

class Shuimeng : public	ZeroCardViewAsSkill
{
public:
    Shuimeng() : ZeroCardViewAsSkill("shuimeng")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return (player->getHandcardNum() > 1 && !player->hasUsed("ShuimengCard"));
    }

    virtual const Card *viewAs() const
    {
        ShuimengCard *sm = new ShuimengCard;
        sm->setShowSkill(objectName());
        return sm;
    }
};

class rosesuigintou : public TriggerSkill
{
public:
    rosesuigintou() : TriggerSkill("rosesuigintou")
    {
        relate_to_place = "head";
        events << EventPhaseStart;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual void record(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getPhase() == Player::RoundStart)
            room->removePlayerDisableShow(player, objectName());
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }
};

class meijiesuigintou : public TriggerSkill
{
public:
    meijiesuigintou() : TriggerSkill("meijiesuigintou")
    {
        frequency = Compulsory;
        relate_to_place = "deputy";
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }
};

MingmingCard::MingmingCard()
{
    will_throw = true;
    target_fixed = true;
}

void MingmingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    if (subcardsLength() == 1 && source->ownSkill("rosesuigintou") && source->hasShownGeneral2() && !source->getActualGeneral2Name().contains("sujiang"))
    {
        source->hideGeneral(false);
        room->setPlayerDisableShow(source, "d", "rosesuigintou");
    }

    ArcheryAttack *aa = new ArcheryAttack(Card::NoSuit, 0);
    aa->setSkillName("mingming");

    QList<ServerPlayer *> targets;
    QList<const Player *> selected;
    foreach (auto target, room->getOtherPlayers(source))
    {
        if (!source->isProhibited(target, aa, selected))
        {
            targets << target;
            selected << target;
        }
    }

    if (targets.length() > 0)
    {
        room->useCard(CardUseStruct(aa, source, targets, true));
    }
}

class Mingming : public	ViewAsSkill
{
public:
    Mingming() : ViewAsSkill("mingming")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (to_select->isEquipped())
            return false;
        if (selected.isEmpty())
            return (to_select->isBlack());
        else if (selected.length() == 1)
        {
            return (to_select->getSuit() == selected.first()->getSuit()) || (to_select->isBlack() && Self->hasShownSkill("meijiesuigintou"));
        }
        return false;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getHandcardNum() >= 1 && !player->hasUsed("MingmingCard");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() == 2 || (cards.length() == 1 && Self->ownSkill("rosesuigintou") && Self->hasShownGeneral2() && !Self->getActualGeneral2Name().contains("sujiang")))
        {
            MingmingCard *mm = new MingmingCard;
            mm->setShowSkill(objectName());
            mm->addSubcards(cards);
            return mm;
        }
        return NULL;
    }
};

class roseshinku : public TriggerSkill
{
public:
    roseshinku() : TriggerSkill("roseshinku")
    {
        relate_to_place = "head";
        events << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual void record(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getPhase() == Player::RoundStart)
            room->removePlayerDisableShow(player, objectName());
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }
};

class meijieshinku : public TriggerSkill
{
public:
    meijieshinku() : TriggerSkill("meijieshinku")
    {
        frequency = Compulsory;
        relate_to_place = "deputy";
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }
};

HeliCard::HeliCard()
{
    will_throw = true;
    target_fixed = false;
}

bool HeliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (subcardsLength() == 1 && Self->ownSkill("roseshinku") && Self->hasShownGeneral2() && !Self->getActualGeneral2Name().contains("sujiang"))
    {
        return targets.isEmpty() && to_select != Self;
    }

    return targets.isEmpty() && to_select != Self && qMax(1, to_select->getHp()) == subcardsLength();
}

void HeliCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.first();

    if (source->ownSkill("roseshinku") && source->hasShownGeneral2() && !source->getActualGeneral2Name().contains("sujiang"))
    {
        if (subcardsLength() != qMax(1, target->getHp()) || (target->getHp() == 1 && room->askForSkillInvoke(source, "roseshinku")))
        {
            source->hideGeneral(false);
            room->setPlayerDisableShow(source, "d", "roseshinku");
        }
    }

    auto damage = DamageStruct("heli", source, target, 1, DamageStruct::Normal);
    room->damage(damage);
}

class HeliVS : public ViewAsSkill
{
public:
    HeliVS() : ViewAsSkill("heli")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (selected.length() + 1 > Self->aliveCount())
            return false;

        return !to_select->isEquipped() && (to_select->isRed() || (Self->hasShownSkill("meijieshinku") && !Self->getActualGeneral1Name().contains("sujiang")));
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasFlag("helidying") && player->getHandcardNum() > 0;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() >= 1)
        {
            HeliCard *hl = new HeliCard;
            hl->setShowSkill(objectName());
            hl->addSubcards(cards);
            return hl;
        }
        return NULL;
    }
};

class Heli : public TriggerSkill
{
public:
    Heli() : TriggerSkill("heli")
    {
        view_as_skill = new HeliVS;
        events << QuitDying;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual void record(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage != NULL && dying.damage->from != NULL && dying.damage->getReason() == "heli"
            && !dying.damage->transfer && !dying.damage->chain)
        {
            room->setPlayerFlag(dying.damage->from, "helidying");
        }
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }
};

//ziwo by official hegemony
class Ziwo : public TriggerSkill
{
public:
    Ziwo() : TriggerSkill("ziwo")
    {
        events << GeneralShown << GeneralHidden << GeneralRemoved << EventPhaseStart << Death << EventAcquireSkill << EventLoseSkill;
        frequency = Compulsory;
    }

    void doZiwo(Room *room, ServerPlayer *kyouko, bool set) const
    {
        if (set && !kyouko->tag["ziwo"].toBool())
        {
            foreach (ServerPlayer *p, room->getOtherPlayers(kyouko))
                room->setPlayerDisableShow(p, "hd", "ziwo");

            kyouko->tag["ziwo"] = true;
        }
        else if (!set && kyouko->tag["ziwo"].toBool())
        {
            foreach (ServerPlayer *p, room->getOtherPlayers(kyouko))
                room->removePlayerDisableShow(p, "ziwo");

            kyouko->tag["ziwo"] = false;
        }
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
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
        if ((triggerEvent == GeneralShown || triggerEvent == GeneralHidden))
        {
            auto handle = data.value<GeneralHandleStruct>();
            if (!player->ownSkill(this) || player->inHeadSkills(this) != handle.isHead)
            {
                return r;
            }
        }
        if (triggerEvent == GeneralRemoved)
        {
            auto handle = data.value<GeneralHandleStruct>();
            if (handle.generalName != "t_kyouko")
            {
                return r;
            }
        }
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

BaozouCard::BaozouCard()
{
    mute = true;
}

bool BaozouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    Slash *slash = new Slash(NoSuit, 0);
    slash->setSkillName("baozou");
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

void BaozouCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    foreach (ServerPlayer *target, targets)
    {
        if (!source->canSlash(target, NULL, true))
            targets.removeOne(target);
    }

    if (targets.length() > 0)
    {

        QVariantList target_list;
        foreach (ServerPlayer *target, targets)
        {
            target_list << QVariant::fromValue(target);
        }

        source->tag["baozou_invoke"] = target_list;
    }
}

class BaozouVS : public ViewAsSkill
{
public:
    BaozouVS() : ViewAsSkill("baozou")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *) const
    {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@baozou";
    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &) const
    {
        BaozouCard *bz = new BaozouCard;
        bz->setShowSkill("baozou");
        return bz;
    }
};

class Baozou : public TriggerSkill
{
public:
    Baozou() : TriggerSkill("baozou")
    {
        events << Damage;
        view_as_skill = new BaozouVS;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash") && damage.to && damage.to->isAlive() && !(damage.transfer || damage.chain || damage.prevented || damage.damage < 1))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        if (player->askForSkillInvoke(this))
        {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        int id = room->drawCard();
        if (id != -1)
        {
            CardsMoveStruct move;
            move.card_ids << id;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), objectName(), QString());
            move.to_place = Player::PlaceTable;
            room->moveCardsAtomic(move, true);
            room->getThread()->delay(500);

            if (Sanguosha->getEngineCard(id)->isKindOf("Slash"))
            {
                if (room->askForUseCard(player, "@@baozou", "@baozou", -1, Card::MethodUse))
                {
                    QVariantList target_list;

                    target_list = player->tag["baozou_invoke"].toList();
                    player->tag.remove("baozou_invoke");

                    Slash *slash = new Slash(Card::SuitToBeDecided, -1);
                    slash->addSubcard(id);
                    slash->setSkillName("_baozou");
                    QList<ServerPlayer *> targets;
                    foreach (QVariant x, target_list)
                    {
                        targets << x.value<ServerPlayer *>();
                    }

                    room->useCard(CardUseStruct(slash, player, targets), false);
                }
                else
                {
                    CardsMoveStruct new_move;
                    new_move.card_ids << id;
                    new_move.reason = CardMoveReason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
                    new_move.from_place = Player::PlaceTable;
                    new_move.to_place = Player::DiscardPile;
                    room->moveCardsAtomic(new_move, true);
                }
            }
            else
            {
                CardsMoveStruct new_move;
                new_move.card_ids << id;
                new_move.from_place = Player::PlaceTable;

                if (player->canDiscard(player, "h") && room->askForDiscard(player, objectName(), 1, 1, true, false, "@baozou_discard", true))
                {
                    new_move.to = player;
                    new_move.to_place = Player::PlaceHand;
                    new_move.reason = CardMoveReason(CardMoveReason::S_REASON_GOTCARD, player->objectName(), objectName(), QString());
                }
                else
                {
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
class Maoshi : public TriggerSkill
{
public:
    Maoshi() : TriggerSkill("maoshi")
    {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *sunshangxiang, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(sunshangxiang)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == sunshangxiang && move.from_places.contains(Player::PlaceEquip))
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const
    {
        if (sunshangxiang->askForSkillInvoke(this))
        {
            room->broadcastSkillInvoke(objectName(), sunshangxiang);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *sunshangxiang, QVariant &, ServerPlayer *) const
    {
        sunshangxiang->drawCards(2);

        return false;
    }
};

ZhiyuCard::ZhiyuCard()
{
}

bool ZhiyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty())
        return false;

    return (Self->isFriendWith(to_select) || Self->willBeFriendWith(to_select)) && to_select->isWounded() && to_select != Self;
}

void ZhiyuCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

class Zhiyu : public ViewAsSkill
{
public:
    Zhiyu() : ViewAsSkill("zhiyu")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getHandcardNum() >= 2 && !player->hasUsed("ZhiyuCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (selected.length() > 1 || Self->isJilei(to_select))
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() != 2)
            return NULL;

        ZhiyuCard *zhiyuCard = new ZhiyuCard();
        zhiyuCard->addSubcards(cards);
        zhiyuCard->setShowSkill(objectName());
        return zhiyuCard;
    }
};

class Aolin : public TriggerSkill
{
public:
    Aolin() : TriggerSkill("aolin")
    {
        events << Damage;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (player->hasShownSkill(this) && damage.from != NULL && damage.from->hasShownAllGenerals() && damage.card != NULL 
            && damage.card->isKindOf("Slash") && !(damage.transfer || damage.prevented || damage.chain || damage.damage < 1))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        if (player->askForSkillInvoke(this))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        if (player->inHeadSkills(this))
            player->hideGeneral(true);
        else
            player->hideGeneral(false);
        player->drawCards(1);
        return false;
    }
};

class TsukkomiVS : public ZeroCardViewAsSkill
{
public:
    TsukkomiVS() : ZeroCardViewAsSkill("tsukkomi")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->deleteLater();
        return slash->isAvailable(player) && !player->hasShownSkill(objectName()) && !player->hasFlag("tsukkomi_used");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return  !player->hasFlag("tsukkomi_used") && (pattern == "Slash" || pattern == "nullification") && !player->hasShownSkill("tsukkomi");
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const
    {
        return !player->hasFlag("tsukkomi_used") && !player->isRemoved() && !player->hasShownSkill("tsukkomi");
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *) const
    {
        return false;
    }

    virtual const Card *viewAs() const
    {
        CardUseStruct::CardUseReason reason = Sanguosha->getCurrentCardUseReason();
        if (reason == CardUseStruct::CARD_USE_REASON_PLAY)
        {
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            slash->setShowSkill(objectName());
            return slash;
        }
        else
        {
            if (reason == CardUseStruct::CARD_USE_REASON_RESPONSE_USE)
            {
                Nullification *nulli = new Nullification(Card::NoSuit, 0);
                nulli->setSkillName(objectName());
                nulli->setShowSkill(objectName());
                return nulli;
            }
        }
        return NULL;
    }
};

class Tsukkomi : public TriggerSkill
{
public:
    Tsukkomi() : TriggerSkill("tsukkomi")
    {
        events << CardUsed;
        view_as_skill = new TsukkomiVS;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->getSkillName().contains(objectName()))
        {
            player->setFlags("tsukkomi_used");
        }
        return QStringList();
    }
};

class Xipin : public TriggerSkill
{
public:
    Xipin() : TriggerSkill("xipin")
    {
        events << CardAsked;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();

        if (data.toStringList().first() == "jink")
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        if (player->askForSkillInvoke(this))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        bool has_given = false;
        foreach (ServerPlayer *p, room->getOtherPlayers(player))
        {
            if (p->getHandcardNum() > 0)
            {
                auto card = room->askForExchange(p, objectName(), 1, 0, "@xipin_give", "", ".|.|.|hand");
                if (!card.isEmpty())
                {
                    CardsMoveStruct move;
                    move.card_ids << card.first();
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
        if (!has_given)
        {
            Jink *jink = new Jink(Card::NoSuit, 0);
            jink->setSkillName(objectName());
            room->provide(jink);

            return true;
        }

        return false;
    }
};

class Zhaihun : public TriggerSkill
{
public:
    Zhaihun() : TriggerSkill("zhaihun")
    {
        events << CardsMoveOneTime << EventPhaseEnd << EventPhaseChanging;
    }

    virtual void record(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data) const
    {
        if (event == CardsMoveOneTime)
        {
            if (player == NULL || !player->isAlive() || !player->ownSkill(this) || player->getPhase() != Player::Discard)
                return;

            auto move = data.value<CardsMoveOneTimeStruct>();

            if (move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
                && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
            {
                QVariantList zhaihunCards = player->tag["ZhaihunCards"].toList();
                foreach(int card_id, move.card_ids)
                {
                    if (!zhaihunCards.contains(card_id))
                    {
                        zhaihunCards << card_id;
                    }
                }
                player->tag["ZhaihunCards"] = zhaihunCards;
            }
        }
        else if (event == EventPhaseChanging)
        {
            if (data.value<PhaseChangeStruct>().from == Player::Discard || data.value<PhaseChangeStruct>().to)
            {
                player->tag["ZhaihunCards"].clear();
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        if (event == CardsMoveOneTime)
        {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();

            if (move.card_ids.length() == 0)
                return QStringList();

            // Zhaihun's last effect: if someone gets red card, konata recover 1 hp
            if (move.reason.m_skillName == objectName() && move.reason.m_playerId == player->objectName() && Sanguosha->getEngineCard(move.card_ids.first()) 
                && Sanguosha->getCard(move.card_ids.first())->isRed() && player->isWounded())
                return QStringList(objectName());

            return QStringList();
        }
        else if (event == EventPhaseEnd)
        {
            if (player->getPhase() != Player::Discard)
                return QStringList();

            QVariantList zhaihuncards = player->tag["ZhaihunCards"].toList();

            QList<int> card_ids;

            foreach (QVariant card_data, zhaihuncards)
            {
                bool ok = false;
                int card_id = card_data.toInt(&ok);
                if (!ok || card_id == -1)
                    continue;
                if (room->getCardPlace(card_id) == Player::DiscardPile)
                    card_ids << card_id;
            }

            if (card_ids.isEmpty())
            {
                player->tag.remove("ZhaihunCards");
                return QStringList();
            }

            int i = 1;
            QVariantList targets_list;
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
            {
                if (player->inMyAttackRange(p) && i <= card_ids.length())
                {
                    targets_list << QVariant::fromValue(p);
                    i++;
                }
            }

            if (!targets_list.isEmpty())
                return QStringList(objectName());
            else
                player->tag.remove("ZhaihunCards");

        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        if (event == CardsMoveOneTime)
        {
            room->broadcastSkillInvoke(objectName(), 2);
            room->notifySkillInvoked(player, objectName());
            return true;
        }
        else
        {
            return player->hasShownSkill(this) ? true : player->askForSkillInvoke(this);
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        if (event == EventPhaseEnd)
        {
            QVariantList zhaihuncards = player->tag["ZhaihunCards"].toList();

            QList<int> card_ids;

            foreach (QVariant card_data, zhaihuncards)
            {
                int card_id = card_data.toInt();
                if (room->getCardPlace(card_id) == Player::DiscardPile)
                    card_ids << card_id;
            }

            if (card_ids.isEmpty())
            {
                player->tag.remove("ZhaihunCards");
                return false;
            }

            foreach (ServerPlayer *p, room->getOtherPlayers(player))
            {
                if (player->inMyAttackRange(p) && !card_ids.isEmpty())
                {
                    room->broadcastSkillInvoke(objectName(), p);
                    room->fillAG(card_ids, p);

                    int id = room->askForAG(p, card_ids, true, objectName());
                    if (id != -1)
                    {
                        room->obtainCard(p, Sanguosha->getCard(id), CardMoveReason(CardMoveReason::S_REASON_GOTBACK, player->objectName(), p->objectName(), objectName(), QString()), true);
                        if (card_ids.contains(id))
                            card_ids.removeOne(id);
                    }

                    room->clearAG(p);
                }
            }

            player->tag.remove("ZhaihunCards");

        }
        else
        {
            RecoverStruct recover;
            recover.who = player;
            recover.recover = 1;
            room->recover(player, recover);
        }
        return false;
    }
};

class Qinlve : public TriggerSkill
{
public:
    Qinlve() : TriggerSkill("qinlve")
    {
        events << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Play && !player->hasFlag("qinlve_failed") && player->getHandcardNum() > 0 && !player->isSkipped(change.to))
        {
            bool invoke = false;
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
            {
                if (p->getHandcardNum() > 0 && !player->hasFlag("qinlve_used_" + p->objectName()))
                {
                    invoke = true;
                    break;
                }
            }
            if (invoke)
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer*) const
    {
        if (player->getHandcardNum() == 0)
            return false;

        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player))
        {
            if (p->getHandcardNum() > 0 && !player->hasFlag("qinlve_used_" + p->objectName()))
                targets << p;
        }

        if (!targets.isEmpty())
        {
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@qinlve_target", true);
            if (target != NULL)
            {
                PindianStruct *pindian = player->pindianSelect(target, objectName());

                if (pindian != NULL)
                {
                    player->tag["qinlve_pindian"] = QVariant::fromValue(pindian);
                    room->setPlayerFlag(player, "qinlve_used_" + target->objectName());
                    return true;
                }
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer*) const
    {
        if (!player->isSkipped(Player::Play))
            player->skip(Player::Play, true);
        PindianStruct *pindian = player->tag["qinlve_pindian"].value<PindianStruct *>();
        player->tag.remove("qinlve_pindian");
        ServerPlayer *target = pindian->to;
        if (!target || !target->isAlive())
            return false;
        bool success = player->pindian(pindian);
        if (success)
        {
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

class Yujian : public TriggerSkill
{
public:
    Yujian() : TriggerSkill("yujian")
    {
        events << TargetChosen << Damage << CardFinished;
        frequency = Frequent;
    }

    virtual void record(TriggerEvent event, Room *, ServerPlayer *player, QVariant &) const
    {
        if (event == Damage)
        {
            if (player->hasFlag("forecastWillDamage"))
            {
                player->setFlags("-forecastWillDamage");
                player->setFlags("forecastTrue");
            }
            if (player->hasFlag("forecastWontDamage"))
            {
                player->setFlags("-forecastWontDamage");
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (event == TargetChosen)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!TriggerSkill::triggerable(use.from)) return QStringList();
            if (use.to.length() > 0 && use.from == player && use.card != NULL && use.card->isKindOf("Slash"))
            {
                if (player->hasFlag("hadChoosedforecastWillDamage") && player->hasFlag("hadChoosedforecastWontDamage"))
                    return QStringList();

                return QStringList(objectName());
            }
        }
        else if (event == CardFinished)
        {
            if (TriggerSkill::triggerable(player) && (player->hasFlag("forecastTrue") || player->hasFlag("forecastWontDamage")))
            {
                player->setFlags("-forecastTrue");
                player->setFlags("-forecastWontDamage");
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (event == TargetChosen)
        {
            if (room->askForSkillInvoke(player, objectName()))
            {
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        }
        else if (event == CardFinished)
        {
            if (player->hasShownSkill(this) || player->askForSkillInvoke(this))
            {
                room->broadcastSkillInvoke(objectName(), 1);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (event == TargetChosen)
        {
            QStringList choices;
            if (!player->hasFlag("hadChoosedforecastWillDamage"))
            {
                choices << "forecastWillDamage";
            }
            if (!player->hasFlag("hadChoosedforecastWontDamage"))
            {
                choices << "forecastWontDamage";
            }

            if (choices.isEmpty())
                return false;

            QString choice = room->askForChoice(player, objectName(), choices.join("+"));
            if (choice != ".")
            {
                player->setFlags("hadChoosed" + choice);
                player->setFlags(choice);
            }

            LogMessage log;
            log.type = "$YujianAnnounce";
            log.from = player;
            log.arg = choice;
            room->sendLog(log);

        }
        else if (event == CardFinished)
        {
            player->drawCards(1);
        }
        return false;
    }
};

//class Tiruo : public TriggerSkill
//{
//public:
//    Tiruo() : TriggerSkill("tiruo")
//    {
//        events << TargetConfirmed;
//        frequency = NotFrequent;
//    }
//
//    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
//    {
//        if (!TriggerSkill::triggerable(player)) return QStringList();
//        CardUseStruct use = data.value<CardUseStruct>();
//        if (use.from && use.from->isAlive() && use.to.contains(player) && use.card->isKindOf("Slash") && player->canDiscard(player, "he"))
//            return QStringList(objectName());
//        return QStringList();
//    }
//
//    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        if (room->askForCard(player, "..", QString("@tiruo_discard:" + data.value<CardUseStruct>().from->objectName()), QVariant(), objectName()))
//        {
//            room->broadcastSkillInvoke(objectName());
//            return true;
//        }
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        CardUseStruct use = data.value<CardUseStruct>();
//        room->notifySkillInvoked(player, objectName());
//
//        room->cancelTarget(use, player);
//
//        use.from->drawCards(1);
//
//        data = QVariant::fromValue(use);
//        return false;
//    }
//};

class Tiruo : public TriggerSkill
{
public:
    Tiruo() : TriggerSkill("tiruo")
    {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && !damage.from->hasShownAllGenerals())
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        bool invoke = player->hasShownSkill(this) ? true : player->askForSkillInvoke(this, data);
        if (invoke) {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->notifySkillInvoked(player, objectName());

        LogMessage log;
        log.type = "#Tiruo";
        log.from = player;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(--damage.damage);
        room->sendLog(log);

        if (damage.damage < 1)
            return true;
        data = QVariant::fromValue(damage);

        return false;
    }
};

LingshangCard::LingshangCard()
{
    mute = true;
    will_throw = true;
    target_fixed = true;
}

class LingshangVS : public ViewAsSkill
{
public:
    LingshangVS() : ViewAsSkill("lingshang")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (selected.length() >= 4) return false;
        if (!selected.isEmpty())
            return to_select->getSuit() == selected.first()->getSuit() && !to_select->isEquipped();
        else
            return !to_select->isEquipped();
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@lingshang";
    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        int heart = 0;
        int diamond = 0;
        int spade = 0;
        int club = 0;
        if (!Self->getEquips().isEmpty())
        {
            foreach (const Card *equip, Self->getEquips())
            {
                switch (equip->getSuit())
                {
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

        if (!cards.isEmpty())
        {
            foreach (const Card *card, cards)
            {
                switch (card->getSuit())
                {
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

        if (qMax(qMax(qMax(heart, diamond), spade), club) == 4 && !cards.isEmpty())
        {
            LingshangCard *ls = new LingshangCard;
            ls->setSkillName("lingshang");
            ls->setShowSkill("lingshang");
            ls->addSubcards(cards);
            return ls;
        }
        return NULL;
    }
};

class Lingshang : public TriggerSkill
{
public:
    Lingshang() : TriggerSkill("lingshang")
    {
        events << CardsMoveOneTime;
        view_as_skill = new LingshangVS;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from && move.from == player && move.from_places.contains(Player::PlaceEquip)) return QStringList();
        if (move.to == player && move.to_place == Player::PlaceHand && room->getCurrent() && room->getCurrent() == player && player->canDiscard(player, "h"))
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (room->askForUseCard(player, "@@lingshang", "@lingshang"))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->addPlayerMark(player, "@gang", 1);
        room->drawCards(player, 1);
        return false;
    }
};

KaihuaCard::KaihuaCard()
{
    target_fixed = true;
    mute = true;
}

void KaihuaCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    room->removePlayerMark(card_use.from, "@gang", 1);

    CardUseStruct new_use = card_use;

    auto candidates = room->getOtherPlayers(card_use.from);
    if (candidates.length() < 1)
        return;

    new_use.to << room->askForPlayersChosen(card_use.from, candidates, "kaihua", 1, candidates.length(), "@kaihua-targetchoose");

    Card::onUse(room, new_use);
}

void KaihuaCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();

    const Card *equip = room->askForCard(effect.to, "EquipCard", QString("@kaihua_pass:" + effect.from->objectName()), QVariant(), Card::MethodNone, 0, false, objectName());
    if (equip != NULL)
        room->obtainCard(effect.from, equip, CardMoveReason(CardMoveReason::S_REASON_GIVE, effect.to->objectName(), effect.from->objectName(), "kaihua", ""), true);
    else
    {
        QString choice = "kaihuaDamage";
        if (effect.from->isWounded())
            choice = choice + "+kaihuaRecover";
        choice = room->askForChoice(effect.to, "kaihua", choice);
        if (choice == "kaihuaDamage")
            room->damage(DamageStruct("kaihua", effect.from, effect.to));
        else
        {
            RecoverStruct recover;
            recover.recover = 1;
            recover.who = effect.from;
            recover.card = NULL;
            room->recover(effect.from, recover);
        }
    }
}

class Kaihua : public ZeroCardViewAsSkill
{
public:
    Kaihua() : ZeroCardViewAsSkill("kaihua")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("@gang") >= 1 && !player->hasUsed("KaihuaCard");
    }

    virtual const Card *viewAs() const
    {
        KaihuaCard *kh = new KaihuaCard;
        kh->setShowSkill(objectName());
        return kh;
    }
};

SuanlvCard::SuanlvCard()
{
    target_fixed = true;
    will_throw = false;
}

void SuanlvCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    QList<ServerPlayer *> open_p;
    open_p << source;
    if (source->isAlive())
        source->addToPile("suan", subcards, false, open_p);
}

class SuanlvVS : public ViewAsSkill
{
public:
    SuanlvVS() : ViewAsSkill("suanlv")
    {
    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@suanlv";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return selected.length() == 0 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() > 0)
        {
            SuanlvCard *sl = new SuanlvCard;
            sl->addSubcards(cards);
            return sl;
        }
        return NULL;
    }
};

class Suanlv : public TriggerSkill
{
public:
    Suanlv() : TriggerSkill("suanlv")
    {
        events << EventPhaseStart << EventPhaseEnd << CardsMoveOneTime;
        view_as_skill = new SuanlvVS;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event != CardsMoveOneTime)
            return;

        ServerPlayer *current = room->getCurrent();

        // In order to trigger record only once
        if (current == NULL || player == NULL || current != player || current->getPhase() != Player::Discard)
            return;

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        auto nodokas = room->findPlayersBySkillName(objectName());

        foreach (auto nodoka, nodokas)
        {
            if (nodoka != NULL && nodoka->isAlive() && nodoka != current)
            {
                auto discardList = nodoka->tag["suanlvDiscardList"].toList();
                if (((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
                    && move.to_place == Player::PlaceTable)
                {
                    foreach (int id, move.card_ids)
                    {
                        if (!discardList.contains(id))
                            discardList << id;
                    }
                }
                nodoka->tag["suanlvDiscardList"] = discardList;
            }
        }
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL || player->getPhase() != Player::Discard)
            return skill_list;

        if (event == EventPhaseStart)
        {
            QList<ServerPlayer *> nodokas = room->findPlayersBySkillName(objectName());

            foreach (ServerPlayer *nodoka, nodokas)
            {
                if (nodoka->getHandcardNum() > 0 && nodoka != player)
                    skill_list.insert(nodoka, QStringList(objectName()));
            }
        }
        else if (event == EventPhaseEnd)
        {
            QList<ServerPlayer *> nodokas = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *nodoka, nodokas)
            {
                if (nodoka->getPile("suan").length() > 0 && nodoka != player)
                {
                    skill_list.insert(nodoka, QStringList(objectName()));
                }
                else
                    nodoka->tag.remove("suanlvDiscardList");
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *, QVariant &, ServerPlayer *nodoka) const
    {
        if (event == EventPhaseStart)
        {
            if (room->askForUseCard(nodoka, "@@suanlv", "@suanlv"))
            {
                room->broadcastSkillInvoke(objectName(), 1);
                return true;
            }
        }
        else if (event == EventPhaseEnd)
        {
            QList<int> suanlv = nodoka->getPile("suan");
            if (suanlv.length() > 0 && (nodoka->hasShownSkill(this) || nodoka->askForSkillInvoke(this)))
            {
                nodoka->tag["suanlvSuanList"] = IntList2VariantList(suanlv);

                DummyCard dummy(suanlv);
                CardMoveReason reason(CardMoveReason::S_REASON_DISCARD, nodoka->objectName());
                room->throwCard(&dummy, reason, NULL);
                room->broadcastSkillInvoke(objectName(), 2);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *, QVariant &, ServerPlayer *nodoka) const
    {
        if (event == EventPhaseEnd)
        {
            QList<int> discard_ids = VariantList2IntList(nodoka->tag["suanlvDiscardList"].toList());
            QList<int> suanlv = VariantList2IntList(nodoka->tag["suanlvSuanList"].toList());
            nodoka->tag.remove("suanlvDiscardList");
            nodoka->tag.remove("suanlvSuanList");
            QList<int> to_get;

            foreach (int id, discard_ids)
            {
                foreach (int suan, suanlv)
                {
                    if (!to_get.contains(id) && room->getCardPlace(id) == Player::DiscardPile && Sanguosha->getEngineCard(suan)->getColor() == Sanguosha->getEngineCard(id)->getColor())
                        to_get << id;
                }
            }

            if (!to_get.isEmpty())
            {
                room->getThread()->delay();
                DummyCard dummy(to_get);
                room->obtainCard(nodoka, &dummy);
            }
        }
        return false;
    }
};

class Sugong : public TriggerSkill
{
public:
    Sugong() : TriggerSkill("sugong")
    {
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const
    {
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Start && player->canDiscard(player, "h"))
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (room->askForCard(player, "TrickCard,EquipCard", "@sugong_discard", QVariant(), objectName()))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        player->setPhase(Player::Play);
        room->broadcastProperty(player, "phase");

        RoomThread *thread = room->getThread();
        if (!thread->trigger(EventPhaseStart, room, player))
            thread->trigger(EventPhaseProceeding, room, player);
        thread->trigger(EventPhaseEnd, room, player);

        player->setPhase(Player::Start);
        room->broadcastProperty(player, "phase");
        return false;
    }
};

class Moyun : public TriggerSkill
{
public:
    Moyun() : TriggerSkill("moyun")
    {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual void record(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player == NULL || player->isDead() || !player->ownSkill(this))
            return;

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to == player)
        {
            if (move.to_place == Player::PlaceHand && player->getHandcardNum() > 0)
                room->setPlayerMark(player, "moyunHand", 1);
            else if (move.to_place == Player::PlaceEquip && player->getEquips().length() > 0)
                room->setPlayerMark(player, "moyunEquip", 1);
            else if (move.to_place == Player::PlaceDelayedTrick && player->getJudgingArea().length() > 0)
                room->setPlayerMark(player, "moyunDelayedTrick", 1);
        }
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        QStringList triggers;

        if (player->getHandcardNum() == 0)
        {
            auto current = room->getCurrent();
            if (TriggerSkill::triggerable(player) && move.from != NULL && move.from == player && current == NULL ? true : current != player && player->getMark("moyunHand") >= 1)
            {
                triggers << objectName();
            }
            room->setPlayerMark(player, "moyunHand", 0);
        }

        if (player->getEquips().length() == 0)
        {
            if (TriggerSkill::triggerable(player) && move.from != NULL && move.from == player && player->getMark("moyunEquip") >= 1)
                triggers << objectName();
            room->setPlayerMark(player, "moyunEquip", 0);
        }

        if (player->getJudgingArea().length() == 0)
        {
            if (TriggerSkill::triggerable(player) && player->getMark("moyunDelayedTrick") >= 1)
            {
                triggers << objectName();
            }
            room->setPlayerMark(player, "moyunDelayedTrick", 0);
        }

        return triggers;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (room->askForSkillInvoke(player, objectName()))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        player->drawCards(1);
        return false;
    }
};

class Shanzhu : public PhaseChangeSkill
{
public:
    Shanzhu() : PhaseChangeSkill("shanzhu")
    {
        frequency = Frequent;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const
    {
        if (!PhaseChangeSkill::triggerable(player)) return QStringList();
        if (player->getRoom()->getDiscardPile().length() / 14 >= 1 && player->getPhase() == Player::Play && !player->isNude())
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (room->askForSkillInvoke(player, objectName()))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const
    {
        Room *room = player->getRoom();

        QList<int> cards = room->getNCards(room->getDiscardPile().length() / 14);

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = player;
        log.card_str = IntList2StringList(cards).join("+");
        room->doNotify(player, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());

        room->askForGuanxing(player, cards, Room::GuanxingUpOnly);

        return false;
    }
};

class Kongyun : public TriggerSkill
{
public:
    Kongyun() : TriggerSkill("kongyun")
    {
        events << AskForRetrial;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        return (TriggerSkill::triggerable(player) && judge->card != NULL && (judge->card->isRed() ? !player->isKongcheng() : true)) ? QStringList(objectName()) : QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        const Card *card = NULL;
        if (judge->card->isRed())
        {
            QStringList prompt_list;
            prompt_list << "@kongyun-card" << judge->who->objectName()
                << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
            QString prompt = prompt_list.join(":");

            card = room->askForCard(player, ".|.|.|hand", prompt, data, Card::MethodResponse, judge->who, true);
        }
        else if (room->askForSkillInvoke(player, objectName()))
        {
            room->getThread()->trigger(FetchDrawPileCard, room, NULL);
            if (room->getDrawPile().isEmpty())
                room->swapPile();
            card = Sanguosha->getEngineCard(room->getDrawPile().takeLast());
            room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, room->getDrawPile().length());
        }

        if (card != NULL)
        {
            room->broadcastSkillInvoke(objectName(), player);
            if (judge->card->isRed())
                room->retrial(card, player, judge, objectName(), true);
            else
            {
                const Card *oldJudge = judge->card;
                CardsMoveStruct move1(card->getId(), judge->who, Player::PlaceJudge,
                    CardMoveReason(CardMoveReason::S_REASON_RETRIAL, player->objectName(), objectName(), QString()));
                CardsMoveStruct move2(oldJudge->getId(), judge->who, NULL, Player::PlaceUnknown, Player::DrawPileBottom,
                    CardMoveReason(CardMoveReason::S_REASON_OVERRIDE, player->objectName(), objectName(), QString()));

                LogMessage log;
                log.type = "$ChangedJudge";
                log.arg = objectName();
                log.from = player;
                log.to << judge->who;
                log.card_str = QString::number(card->getEffectiveId());
                room->sendLog(log);

                QList<CardsMoveStruct> moves;
                moves.append(move2);
                moves.append(move1);
                room->moveCardsAtomic(moves, true);

                judge->card = card;
            }
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const
    {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        judge->updateResult();
        return false;
    }
};

// Aocai by QSanguosha V2 , maybe Para. Modified by OmnisReen

#include "json.h"
class LaoyueViewAsSkill : public ZeroCardViewAsSkill
{
public:
    LaoyueViewAsSkill() : ZeroCardViewAsSkill("laoyue")
    {
    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (player->getPhase() != Player::NotActive || player->hasFlag("Global_LaoyueFailed")) return false;
        if (pattern == "slash")
            return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
        else if (pattern == "peach")
            return player->getMark("Global_PreventPeach") == 0;
        else if (pattern.contains("analeptic"))
            return true;
        return false;
    }
    virtual const Card *viewAs() const
    {
        LaoyueCard *laoyue_card = new LaoyueCard;
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "peach+analeptic" && Self->getMark("Global_PreventPeach") > 0)
            pattern = "analeptic";
        laoyue_card->setUserString(pattern);
        laoyue_card->setShowSkill("laoyue");
        return laoyue_card;
    }
};

using namespace QSanProtocol;

class Laoyue : public TriggerSkill
{
public:
    Laoyue() : TriggerSkill("laoyue")
    {
        events << CardAsked;
        view_as_skill = new LaoyueViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        QString pattern = data.toStringList().first();
        if (player->getPhase() == Player::NotActive && (pattern == "slash" || pattern == "jink"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return room->askForSkillInvoke(player, objectName(), data);
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        QList<int> ids;

        for (int i = 0; i < 2; i++)
        {
            room->getThread()->trigger(FetchDrawPileCard, room, NULL);
            if (room->getDrawPile().isEmpty())
                room->swapPile();
            ids << room->getDrawPile().takeLast();
        }

        QList<int> enabled, disabled;
        foreach (int id, ids)
        {
            if (Sanguosha->getCard(id)->objectName().contains(data.toStringList().first()))
                enabled << id;
            else
                disabled << id;
        }
        int id = Laoyue::view(room, player, ids, enabled, disabled);

        if (id != -1)
        {
            const Card *card = Sanguosha->getCard(id);
            room->provide(card);
            return true;
        }
        return false;
    }

    static int view(Room *room, ServerPlayer *player, QList<int> &ids, QList<int> &enabled, QList<int> &disabled)
    {
        int result = -1, index = -1;
        QList<int> &drawPile = room->getDrawPile();

        // Remove log because of it's hide for others

        room->broadcastSkillInvoke("laoyue");
        room->notifySkillInvoked(player, "laoyue");

        room->fillAG(ids, player);
        room->getThread()->delay(2500);
        room->clearAG(player);

        QString choice = player->getHandcardNum() >= 2 ? room->askForChoice(player, "laoyue", "use+put+replace+cancel") : room->askForChoice(player, "laoyue", "use+replace+cancel");
        if (choice == "use")
        {
            if (enabled.isEmpty())
            {
                room->fillAG(ids, player, disabled);
                room->getThread()->delay(2000);
                room->clearAG(player);
            }
            else
            {
                room->fillAG(ids, player, disabled);
                int id = room->askForAG(player, enabled, true, "laoyue");
                if (id != -1)
                {
                    index = ids.indexOf(id);
                    ids.removeOne(id);
                    result = id;
                }
                room->clearAG(player);
            }
            for (int i = ids.length() - 1; i >= 0; i--)
                drawPile.append(ids.at(i));
        }
        else if (choice == "put")
        {
            QList<int> ex = room->askForExchange(player, "laoyue", 2, 2, "@laoyue-put", "", ".");
            if (ex.length() == 2)
            {
                CardsMoveStruct move1(QList<int>(), player, NULL, Player::PlaceHand, Player::DrawPileBottom,
                    CardMoveReason(CardMoveReason::S_REASON_OVERRIDE, player->objectName(), "laoyue", QString()));
                CardsMoveStruct move2(QList<int>(), NULL, player, Player::DrawPileBottom, Player::PlaceHand,
                    CardMoveReason(CardMoveReason::S_REASON_OVERRIDE, player->objectName(), "laoyue", QString()));
                move2.card_ids.append(ids);
                move1.card_ids.append(ex);
                QList<CardsMoveStruct> moves;
                moves.append(move2);
                moves.append(move1);
                room->moveCardsAtomic(moves, false);
            }
        }
        else if (choice == "replace")
        {
            auto topCards = room->getNCards(2, true);
            CardsMoveStruct move1(QList<int>(), NULL, NULL, Player::DrawPile, Player::DrawPileBottom,
                CardMoveReason(CardMoveReason::S_REASON_OVERRIDE, player->objectName(), "laoyue", QString()));
            CardsMoveStruct move2(QList<int>(), NULL, NULL, Player::DrawPileBottom, Player::DrawPile,
                CardMoveReason(CardMoveReason::S_REASON_OVERRIDE, player->objectName(), "laoyue", QString()));
            move2.card_ids.append(ids);
            move1.card_ids.append(topCards);
            QList<CardsMoveStruct> moves;
            moves.append(move2);
            moves.append(move1);
            room->moveCardsAtomic(moves, false);
            for (int i = ids.length() - 1; i >= 0; i--)
                drawPile.prepend(ids.at(i));
        }
        else if (choice == "cancel")
        {
            for (int i = ids.length() - 1; i >= 0; i--)
                drawPile.append(ids.at(i));
            return -1;
        }

        room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, drawPile.length());

        if (result == -1)
            room->setPlayerFlag(player, "Global_LaoyueFailed");
        else if (choice == "use")
        {
            LogMessage log;
            log.type = "#LaoyueUse";
            log.from = player;
            log.arg = "laoyue";
            log.arg2 = QString("CAPITAL(%1)").arg(index + 1);
            room->sendLog(log);
        }

        if (choice == "put")
        {
            LogMessage log;
            log.type = "#LaoyuePut";
            log.from = player;
            log.arg = "laoyue";
            room->sendLog(log);
        }
        else if (choice == "replace")
        {
            LogMessage log;
            log.type = "#LaoyueReplace";
            log.from = player;
            log.arg = "laoyue";
            room->sendLog(log);
        }

        return result;
    }
};

LaoyueCard::LaoyueCard()
{
}

bool LaoyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool LaoyueCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
        return true;
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFixed();
}

bool LaoyueCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetsFeasible(targets, Self);
}

const Card *LaoyueCard::validateInResponse(ServerPlayer *user) const
{
    Room *room = user->getRoom();
    QList<int> ids;
    for (int i = 0; i < 2; i++)
    {
        room->getThread()->trigger(FetchDrawPileCard, room, NULL);
        if (room->getDrawPile().isEmpty())
            room->swapPile();
        ids << room->getDrawPile().takeLast();
    }
    QStringList names = toString().split(":").last().split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";
    QList<int> enabled, disabled;
    foreach (int id, ids)
    {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }
    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "laoyue";
    room->sendLog(log);
    int id = Laoyue::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

const Card *LaoyueCard::validate(CardUseStruct &cardUse) const
{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *user = cardUse.from;
    Room *room = user->getRoom();
    QList<int> ids;
    for (int i = 0; i < 2; i++)
    {
        room->getThread()->trigger(FetchDrawPileCard, room, NULL);
        if (room->getDrawPile().isEmpty())
            room->swapPile();
        ids << room->getDrawPile().takeLast();
    }
    QStringList names = toString().split(":").last().split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";
    QList<int> enabled, disabled;
    foreach (int id, ids)
    {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }
    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "laoyue";
    room->sendLog(log);
    int id = Laoyue::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

class Jiandao : public TriggerSkill
{
public:
    Jiandao() : TriggerSkill("jiandao")
    {
        events << TargetChoosing;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player) || player->hasFlag("jiandao_used"))
            return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card != NULL && use.card->isKindOf("Slash") && use.to.length() == 1)
        {
            ServerPlayer *target = use.to.first();
            if (target == NULL)
                return QStringList();
            bool invoke = false;
            QList<ServerPlayer *> others = room->getOtherPlayers(player);
            if (others.contains(target))
                others.removeOne(target);
            int handcardNum = target->getHandcardNum();
            int equipcardNum = target->getEquips().length();
            foreach (ServerPlayer *p, others)
            {
                if (p->getHandcardNum() == handcardNum || p->getEquips().length() == equipcardNum)
                {
                    invoke = true;
                    break;
                }
            }
            if (invoke)
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *target = use.to.first();
        QList<ServerPlayer *> others = room->getOtherPlayers(player);
        QList<ServerPlayer *> handVictims;
        QList<ServerPlayer *> equipVictims;
        if (others.contains(target))
            others.removeOne(target);
        int handcardNum = target->getHandcardNum();
        int equipcardNum = target->getEquips().length();
        foreach (ServerPlayer *p, others)
        {
            if (p->getHandcardNum() == handcardNum && !use.to.contains(p))
                handVictims << p;
            if (p->getEquips().length() == equipcardNum && !use.to.contains(p))
                equipVictims << p;
        }
        if (player->askForSkillInvoke(this, data))
        {
            QStringList choices;
            if (handVictims.length() > 0)
                choices << "hand";
            if (equipVictims.length() > 0)
                choices << "equip";
            auto choice = room->askForChoice(player, objectName(), choices.join("+"), data);
            if (choice == "hand")
            {
                room->setPlayerFlag(player, "jiandao_used");
                player->tag["jiandao_targets"] = QVariant::fromValue(handVictims);
                room->broadcastSkillInvoke("jiandao", 1);
                return true;
            }
            else if (choice == "equip")
            {
                room->setPlayerFlag(player, "jiandao_used");
                player->tag["jiandao_targets"] = QVariant::fromValue(equipVictims);
                room->broadcastSkillInvoke("jiandao", 2);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> targets = player->tag["jiandao_targets"].value<QList<ServerPlayer *>>();
        if (targets.length() > 0)
        {
            QList<const Player*> others;
            foreach(ServerPlayer *other, use.to)
                others << other;
            foreach (ServerPlayer *target, targets)
            {
                if (!use.to.contains(target) && player->canSlash(target, false, 0, others))
                {
                    use.to << target;
                    others << target;
                }
            }
            data.setValue(use);
        }
        return false;
    }
};

//class Fengyin : public TriggerSkill
//{
//public:
//    Fengyin() : TriggerSkill("fengyin")
//    {
//        events << EventPhaseStart << CardsMoveOneTime;
//        frequency = NotFrequent;
//    }
//
//    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
//    {
//        QMap<ServerPlayer *, QStringList> skill_list;
//        if (event == EventPhaseStart)
//        {
//            if (player->getPhase() == Player::Draw)
//            {
//                QList<ServerPlayer *> sakuras = room->findPlayersBySkillName(objectName());
//                foreach (ServerPlayer *sakura, sakuras)
//                    if (player != sakura && !sakura->isNude())
//                        skill_list.insert(sakura, QStringList(objectName()));
//            }
//        }
//        else
//        {
//            if (!TriggerSkill::triggerable(player)) return skill_list;
//            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
//            if (move.to != NULL && move.to->hasFlag("fengyin_on") && move.to->getPhase() == Player::Draw
//                && (move.reason.m_reason == CardMoveReason::S_REASON_GOTCARD || move.reason.m_reason == CardMoveReason::S_REASON_DRAW))
//                foreach (int id, move.card_ids)
//                    if (id != -1)
//                        Sanguosha->getCard(id)->setFlags("clowcards");
//        }
//        return skill_list;
//    }
//
//    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const
//    {
//        const Card *card = room->askForCard(ask_who, "..", "@fengyin_put", QVariant::fromValue(player), Card::MethodNone, NULL, false, objectName(), false);
//        if (card != NULL)
//        {
//            auto reason = CardMoveReason(CardMoveReason::S_REASON_PUT, ask_who->objectName(), NULL, "fengyin", NULL);
//            room->moveCardTo(card, ask_who, NULL, Player::DrawPile, reason, false);
//            return true;
//        }
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const
//    {
//        player->setFlags("fengyin_on"); // this is coupling into isAvailable
//        return false;
//    }
//};
//
//class Jiechu : public TriggerSkill
//{
//public:
//    Jiechu() : TriggerSkill("jiechu")
//    {
//        events << TargetChoosing << DrawNCards << EventPhaseChanging;
//        frequency = NotFrequent;
//    }
//
//    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
//    {
//        QMap<ServerPlayer *, QStringList> skill_list;
//        if (event == TargetChoosing)
//        {
//            CardUseStruct use = data.value<CardUseStruct>();
//            if (use.from != NULL && use.to.contains(use.from) && !use.card->isKindOf("SkillCard"))
//            {
//                QList<ServerPlayer *> sakuras = room->findPlayersBySkillName(objectName());
//                foreach (ServerPlayer *sakura, sakuras)
//                    if (!sakura->hasFlag("jiechu_used"))
//                        skill_list.insert(sakura, QStringList(objectName()));
//            }
//        }
//        else if (event == DrawNCards)
//        {
//            if (TriggerSkill::triggerable(player) && player->getMark("@jiechu_times") > 0)
//            {
//                if (data.toInt() > 0)
//                    skill_list.insert(player, QStringList(objectName()));
//                else
//                    room->setPlayerMark(player, "@jiechu_times", 0);
//            }
//        }
//        else if (event == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::NotActive)
//        {
//            foreach (auto p, room->getAlivePlayers())
//            {
//                p->setFlags("-jiechu_used");
//            }
//        }
//        return skill_list;
//    }
//
//    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
//    {
//        if (event == TargetChoosing)
//        {
//            if (ask_who->askForSkillInvoke(objectName(), data))
//            {
//                ask_who->setFlags("jiechu_used");
//                room->broadcastSkillInvoke(objectName());
//                return true;
//            }
//        }
//        else if (event == DrawNCards)
//        {
//            if (data.toInt() > 0)
//                return true;
//            else
//                room->setPlayerMark(player, "@jiechu_times", 0);
//        }
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
//    {
//        if (event == TargetChoosing)
//        {
//            room->drawCards(ask_who, 1, objectName());
//            room->setPlayerMark(ask_who, "@jiechu_times", ask_who->getMark("@jiechu_times") + 1);
//        }
//        else
//        {
//            data = qMax(data.toInt() - player->getMark("@jiechu_times"), 0);
//            room->setPlayerMark(player, "@jiechu_times", 0);
//        }
//        return false;
//    }
//};

class Fengyin : public TriggerSkill
{
public:
    Fengyin() : TriggerSkill("fengyin")
    {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player) || event != DamageCaused)
            return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to != NULL && damage.to->hasShownOneGeneral() && damage.card != NULL && damage.card->isKindOf("Slash") && player->getPhase() == Player::Play
            && !player->isFriendWith(damage.to) && !player->willBeFriendWith(damage.to) && !damage.transfer && !damage.chain 
            && !damage.to->getActualGeneral2Name().contains("sujiang") && !player->hasFlag("fengyin_used"))
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data))
        {
            room->setPlayerFlag(player, "fengyin_used");
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), data.value<DamageStruct>().to->objectName());
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        QStringList choices;
        if (damage.to->hasEquip())
            choices << "discard";
        choices << "remove";
        QString choice = room->askForChoice(damage.to, objectName(), choices.join("+"));
        if (choice == "discard") {
            damage.to->throwAllEquips();
            room->loseHp(damage.to);
        } else {
            damage.to->removeGeneral(false);
        }
        return true;
    }
};

class Jiechu : public TriggerSkill
{
public:
    Jiechu() : TriggerSkill("jiechu")
    {
        events << EventPhaseStart << GeneralShown << GeneralHidden << GeneralRemoved << EventPhaseChanging;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data) const
    {
        if (event == GeneralShown || event == GeneralHidden || event == GeneralRemoved)
        {
            auto handle = data.value<GeneralHandleStruct>();
            if (handle.reason != "transform")
            {
                foreach(auto p, room->getAlivePlayers())
                {
                    room->setPlayerMark(p, "jiechu_num", p->getMark("jiechu_num") + 1);
                }
            }
        }
        else if (event == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::NotActive)
        {
            foreach (auto p, room->getAlivePlayers())
            {
                room->setPlayerMark(p, "jiechu_num", 0);
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const
    {
        if (event == EventPhaseStart && TriggerSkill::triggerable(player) && player->getMark("jiechu_num") > 0 && player->getPhase() == Player::Finish && player->canShowGeneral("d"))
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (player->askForSkillInvoke("transform"))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->transformDeputyGeneral(player);
        room->getThread()->delay(1500);

        room->drawCards(player, player->getMark("jiechu_num"), objectName());
        return false;
    }
};

void MoesenPackage::addComicGenerals()
{

    General *sakura = new General(this, "sakura", "shu", 3, false); // C001
    sakura->addSkill(new Fengyin);
    sakura->addSkill(new Jiechu);

    General *hinagiku = new General(this, "hinagiku", "shu", 4, false); // C002
    hinagiku->addSkill(new Jiandao);

    General *nagi = new General(this, "nagi", "shu", 3, false); // C003
    nagi->addSkill(new Tianzi);
    nagi->addSkill(new Yuzhai);
    nagi->addSkill(new TianziMaxCards);
    insertRelatedSkills("tianzi", "#tianzi-maxcard");

    General *izumi = new General(this, "izumi", "shu", 3, false); // C004
    izumi->addSkill(new Mizou);
    izumi->addSkill(new Duhun);

    General *suiseiseki = new General(this, "suiseiseki", "shu", 3, false); // C005
    suiseiseki->addSkill(new Shuimeng);
    suiseiseki->addSkill(new rosesuiseiseki);
    suiseiseki->addSkill(new meijiesuiseiseki);
    suiseiseki->addCompanion("shinku");

    General *suigintou = new General(this, "suigintou", "shu", 4, false); // C006
    suigintou->addSkill(new Mingming);
    suigintou->addSkill(new rosesuigintou);
    suigintou->addSkill(new meijiesuigintou);

    General *shinku = new General(this, "shinku", "shu", 3, false); // C007
    shinku->addSkill(new Heli);
    shinku->addSkill(new roseshinku);
    shinku->addSkill(new meijieshinku);

    General *akari = new General(this, "akari", "shu", 3, false); // C008
    akari->addSkill(new Wucun);
    akari->addSkill(new Kongni);

    General *t_kyouko = new General(this, "t_kyouko", "shu", 3, false); // C009
    t_kyouko->addSkill(new Ziwo);
    t_kyouko->addSkill(new Baozou);

    General *tsukasa = new General(this, "tsukasa", "shu", 3, false); // C010
    tsukasa->addSkill(new Zhiyu);
    tsukasa->addSkill(new Maoshi);
    tsukasa->addCompanion("kagami");

    General *kagami = new General(this, "kagami", "shu", 3, false); // C011
    kagami->addSkill(new Tsukkomi);
    kagami->addSkill(new Aolin);

    General *konata = new General(this, "konata", "shu", 3, false); // C012
    konata->addSkill(new Xipin);
    konata->addSkill(new Zhaihun);

    General *ika = new General(this, "ika", "shu", 5, false); // C013
    ika->addSkill(new Qinlve);

    General *toki = new General(this, "toki", "shu", 3, false); // C014
    toki->addSkill(new Tiruo);
    toki->addSkill(new Yujian);

    General *saki = new General(this, "saki", "shu", 4, false); // C015
    saki->addSkill(new Lingshang);
    saki->addSkill(new Kaihua);
    saki->addCompanion("nodoka");

    General *nodoka = new General(this, "nodoka", "shu", 3, false); // C016
    nodoka->addSkill(new Suanlv);
    nodoka->addSkill(new Sugong);

    General *shizuno = new General(this, "shizuno", "shu", 3, false); // C017
    shizuno->addSkill(new Moyun);
    shizuno->addSkill(new Shanzhu);

    General *koromo = new General(this, "koromo", "shu", 3, false); // C018
    koromo->addSkill(new Kongyun);
    koromo->addSkill(new Laoyue);

    addMetaObject<MizouCard>();
    addMetaObject<ShuimengCard>();
    addMetaObject<MingmingCard>();
    addMetaObject<HeliCard>();
    addMetaObject<ZhiyuCard>();
    addMetaObject<BaozouCard>();
    addMetaObject<LingshangCard>();
    addMetaObject<KaihuaCard>();
    addMetaObject<SuanlvCard>();
    addMetaObject<LaoyueCard>();
    addMetaObject<TianziCard>();
}
