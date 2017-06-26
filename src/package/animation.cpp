#include "animation.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "standard-shu-generals.h"
#include "engine.h"
#include "client.h"
#include "roomthread.h"
//this file is the general skills for animation.

//mami    lieqiang, molu -SE
class Lieqiang : public TriggerSkill
{
public:
    Lieqiang() : TriggerSkill("lieqiang")
    {
        frequency = Compulsory;
        events << TargetChosen;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        // Debug by hmqgg because TargetChosen trigger player changes to use.from
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL || !player->isAlive())
            return skill_list;

        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card == NULL || !use.card->isKindOf("Slash") || use.to.length() == 0)
            return skill_list;

        QList<ServerPlayer *> mamis = room->findPlayersBySkillName(objectName());

        QStringList targets;
        foreach(ServerPlayer *to, use.to)
            targets << to->objectName();

        foreach (ServerPlayer *mami, mamis)
        {
            int n = 0;
            if (mami->isKongcheng()) n++;
            if (!mami->hasEquip()) n++;
            if (mami->getJudgingArea().isEmpty()) n++;

            if (n >= 2 && (mami->isFriendWith(player) || mami->willBeFriendWith(player)))
                skill_list.insert(mami, QStringList(objectName() + "->" + targets.join("+")));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        bool invoke = ask_who->hasShownSkill(this) ? true : ask_who->askForSkillInvoke(objectName(), data);
        if (invoke)
        {
            if (ask_who->hasShownSkill(this))
            {
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

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        QVariantList jink_list = use.from->tag["Jink_" + use.card->toString()].toList();

        doLieqiang(player, use, jink_list);

        use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }

private:
    static void doLieqiang(ServerPlayer *target, CardUseStruct use, QVariantList &jink_list)
    {
        int index = use.to.indexOf(target);
        LogMessage log;
        log.type = "#NoJink";
        log.from = target;
        target->getRoom()->sendLog(log);
        jink_list.replace(index, QVariant(0));
    }
};

class Molu : public TriggerSkill
{
public:
    Molu() : TriggerSkill("molu")
    {
        events << AskForPeaches;
        relate_to_place = "head";
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        if (player != dying.who || player->getActualGeneral2Name().contains("sujiang"))
            return QStringList();
        if (player->getHp() > 0)
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(objectName(), data))
        {
            room->broadcastSkillInvoke(objectName());
            room->doLightbox("$MoluAnimate", 3000);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        RecoverStruct recover;
        recover.recover = player->getMaxHp() - player->getHp();
        recover.who = player;
        room->recover(player, recover);
        player->removeGeneral(false);
        return false;
    }
};

//yingqiang-Slob
class Yingqiang : public ViewAsSkill
{
public:
    Yingqiang() : ViewAsSkill("yingqiang")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        foreach (const Card *card, selected)
        {
            if (to_select->getSuit() == card->getSuit())
                return false;
        }
        return !to_select->hasFlag("using") && !to_select->isEquipped() && to_select->getSuit() != Card::NoSuit;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->isKongcheng() && Slash::IsAvailable(player) && !player->hasUsed(objectName());
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() < 2 || cards.length() > 4)
            return NULL;

        Card *slash = new Slash(Card::SuitToBeDecided, -1);
        slash->addSubcards(cards);
        slash->setSkillName(objectName());
        slash->setShowSkill(objectName());
        return slash;
    }
};

class YingqiangSpade : public TriggerSkill
{
public:
    YingqiangSpade() : TriggerSkill("yingqiang_spade")
    {
        events << Damage;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (triggerEvent == Damage)
        {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from != NULL && damage.from->isAlive() && damage.card != NULL && damage.card->getSkillName() == "yingqiang"
                && damage.to != NULL && damage.from->distanceTo(damage.to) <= 1)
                foreach (int cardid, damage.card->getSubcards())
                if (Sanguosha->getCard(cardid)->getSuit() == Card::Spade && player->isWounded())
                {
                    QStringList skill_list;
                    for (int i = 0; i < damage.damage; i++)
                        skill_list << objectName();
                    return skill_list;
                }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->notifySkillInvoked(player, "yingqiang");
        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);

        return false;
    }
};

class YingqiangHeart : public TriggerSkill
{
public:
    YingqiangHeart() : TriggerSkill("yingqiang_heart")
    {
        events << Damage;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer * &) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from != NULL && damage.from->isAlive() && damage.card && damage.card->getSkillName() == "yingqiang" && damage.to->isAlive())
            foreach (int cardid, damage.card->getSubcards())
            if (Sanguosha->getCard(cardid)->getSuit() == Card::Heart)
                return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        room->notifySkillInvoked(player, "yingqiang");
        DamageStruct damage = data.value<DamageStruct>();
        int n = qMin(qMax(damage.to->getHp(), 0), 5);
        if (n)
            damage.to->drawCards(n);
        damage.to->turnOver();
        return false;
    }
};

class YingqiangClub : public TriggerSkill
{
public:
    YingqiangClub() : TriggerSkill("yingqiang_club")
    {
        events << TargetChosen;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from && use.from->isAlive() && use.from == player && use.card && use.card->getSkillName() == "yingqiang")
        {
            //room->broadcastSkillInvoke("yingqiang");//the broadcart is written here
            foreach (int cardid, use.card->getSubcards())
                if (Sanguosha->getCard(cardid)->getSuit() == Card::Club)
                    return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        room->notifySkillInvoked(player, "yingqiang");
        CardUseStruct use = data.value<CardUseStruct>();
        foreach (ServerPlayer *p, use.to.toSet())
        {
            p->addQinggangTag(use.card);
        }
        return false;
    }
};

class YingqiangDiamond : public TriggerSkill
{
public:
    YingqiangDiamond() : TriggerSkill("yingqiang_diamond")
    {
        events << TargetChosen;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.from && use.from == player && player->isAlive() && use.card && use.card->getSkillName() == "yingqiang")
            foreach (int cardid, use.card->getSubcards())
            if (Sanguosha->getCard(cardid)->getSuit() == Card::Diamond)
                return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->notifySkillInvoked(player, "yingqiang");
        player->drawCards(1);
        return false;
    }
};

class YingqiangTargetMod : public TargetModSkill
{
public:
    YingqiangTargetMod() : TargetModSkill("#yingqiang-target")
    {
    }
    virtual int getDistanceLimit(const Player *, const Card *card) const
    {
        if (card->getSkillName() == "yingqiang")
            return 1000;
        else
            return 0;
    }
};

//madoka    cibei, renmin -SE
class Cibei : public TriggerSkill
{
public:
    Cibei() : TriggerSkill("cibei")
    {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (player->canDiscard(damage.to, "he"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (player->askForSkillInvoke(objectName(), QVariant::fromValue(damage.to)))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        DummyCard *dummy = new DummyCard;
        dummy->deleteLater();
        if (player->canDiscard(target, "h"))
        {
            int id = room->askForCardChosen(player, target, "h", objectName(), false, Card::MethodDiscard);
            dummy->addSubcard(id);
        }
        if (player->canDiscard(target, "e"))
        {
            int id = room->askForCardChosen(player, target, "e", objectName(), false, Card::MethodDiscard);
            dummy->addSubcard(id);
        }
        if (!dummy->getSubcards().isEmpty())
            room->throwCard(dummy, target, player);
        return true;
    }
};

class Renmin : public TriggerSkill
{
public:
    Renmin() : TriggerSkill("renmin")
    {
        events << CardsMoveOneTime;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        bool can_invoke = false;
        foreach (int id, move.card_ids)
            if (room->getCardPlace(id) == Player::DiscardPile)
            {
                can_invoke = true;
                break;
            }
        if (!can_invoke || !move.from || move.from->isDead() || move.to_place != Player::DiscardPile 
            || ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_DISCARD))
            return QStringList();
        if ((player->isFriendWith(move.from) || player->willBeFriendWith(move.from)) && move.from->getMark("renmin" + player->objectName()) == 0)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(objectName(), data))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *target = room->findPlayer(move.from->objectName());
        room->addPlayerMark(target, "@renmin_used");
        room->addPlayerMark(target, "renmin" + player->objectName());
        CardsMoveStruct newmove(move.card_ids, move.from, Player::PlaceHand, CardMoveReason(CardMoveReason::S_REASON_RECYCLE,
            player->objectName(),
            objectName(),
            QString()));
        room->moveCardsAtomic(newmove, true);
        return false;
    }
};

//wuwei-Slob
WuweiCard::WuweiCard()
{
}

bool WuweiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    Card *card = Sanguosha->getCard(getSubcards().first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->setSkillName("wuwei");
    slash->addSubcard(card);
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

const Card *WuweiCard::validate(CardUseStruct &use) const
{
    ServerPlayer *sayaka = use.from;
    Room *room = sayaka->getRoom();
    room->loseHp(sayaka);
    room->broadcastSkillInvoke("wuwei");
    Card *card = Sanguosha->getCard(getSubcards().first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->setSkillName("wuwei");
    slash->setShowSkill("wuwei");
    slash->addSubcard(card);
    return slash;
}

class WuweiViewAsSkill : public OneCardViewAsSkill
{
public:
    WuweiViewAsSkill() : OneCardViewAsSkill("wuwei")
    {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("WuweiCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        WuweiCard *wuwei = new WuweiCard();
        wuwei->addSubcard(originalCard);
        wuwei->setShowSkill(objectName());
        return wuwei;
    }
};

class Wuwei : public TriggerSkill
{
public:
    Wuwei() : TriggerSkill("wuwei")
    {
        events << DamageCaused << PreCardUsed;
        view_as_skill = new WuweiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (triggerEvent == PreCardUsed)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->getSkillName() == "wuwei" && use.from != NULL)
            {
                room->addPlayerHistory(player, use.card->getClassName(), -1);
                use.m_addHistory = false;
                data = QVariant::fromValue(use);
            }
        }
        else
        {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->getSkillName() == "wuwei")
            {
                return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        damage.damage++;
        data = QVariant::fromValue(damage);
        room->broadcastSkillInvoke("wuwei");
        return false;
    }
};

#include <gamerule.h>
//shiting -AK
class Shiting : public TriggerSkill
{
public:
    Shiting() : TriggerSkill("shiting")
    {
        events << TargetConfirmed << CardUsed << CardFinished << Death;
    }

    virtual void record(TriggerEvent event, Room *, ServerPlayer *, QVariant &data) const
    {
        if (event == Death)
        {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != NULL && death.who->hasShownSkill(this))
                death.who->tag["shiting_list"] = QVariant();
        }
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (event == TargetConfirmed)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card == NULL || !use.card->isKindOf("Slash") || use.to.length() != 1 || player != use.to.first())
                return skill_list;
            QList<ServerPlayer *> homuras = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *homura, homuras)
                if (homura->canDiscard(homura, "h") && homura->getHandcardNum() > 1 && (homura->isFriendWith(player) || homura->willBeFriendWith(player)))
                    skill_list.insert(homura, QStringList(objectName()));
        }
        else if (event == CardUsed)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            QList<ServerPlayer *> homuras = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *homura, homuras)
            {
                if (homura == use.from && use.card && homura->hasFlag("shiting_use") && use.to.length() > 0 && !use.card->isKindOf("SkillCard") && !use.card->isKindOf("TransferCard") && !use.to.contains(homura))
                {
                    QVariantList shiting_list = homura->tag["shiting_list"].toList();
                    if (!shiting_list.contains(data))
                    {
                        skill_list.insert(homura, QStringList(objectName()));
                    }
                }
            }

        }
        else if (event == CardFinished)
        {
            CardUseStruct use = data.value<CardUseStruct>();

            QList<ServerPlayer *> homuras = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *homura, homuras)
            {
                if (use.card && use.card->hasFlag("shiting_slash") && homura->tag["shiting_list"].toList().length() > 0)
                    skill_list.insert(homura, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == TargetConfirmed)
        {
            if (room->askForDiscard(ask_who, objectName(), 2, 2, true, false, "@shiting-ask"))
            {
                room->broadcastSkillInvoke(objectName(), 1);
                LogMessage log;
                log.type = "#ShitingStart";
                log.from = ask_who;
                log.arg = objectName();
                room->sendLog(log);
                return true;
            }
        }
        else if (event == CardUsed)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            QVariantList shiting_list = ask_who->tag["shiting_list"].toList();
            shiting_list << data;
            ask_who->tag["shiting_list"] = shiting_list;
            use.card->setFlags("cardNotTriggerCardFinished");
            return true;
        }
        else if (event == CardFinished)
        {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == TargetConfirmed)
        {
            CardUseStruct use = data.value<CardUseStruct>();

            use.card->setFlags("shiting_slash");
            ask_who->setFlags("shiting_use");
            ServerPlayer *current = room->getCurrent();
            try
            {
                room->setCurrent(ask_who);
                room->getThread()->trigger(TurnStart, room, ask_who);
                room->setCurrent(current);
            }
            catch (TriggerEvent triggerEvent)
            {
                if (triggerEvent == TurnBroken)
                {
                    if (ask_who->getPhase() != Player::NotActive)
                    {
                        const GameRule *game_rule = NULL;
                        game_rule = qobject_cast<const GameRule *>(Sanguosha->getTriggerSkill("game_rule"));
                        if (game_rule)
                            room->getThread()->trigger(EventPhaseEnd, room, ask_who);
                        ask_who->changePhase(ask_who->getPhase(), Player::NotActive);
                    }
                    room->setCurrent(current);
                }
                throw triggerEvent;
            }
        }
        else if (event == CardUsed)
        {
            return true;
        }
        else if (event == CardFinished)
        {
            QVariantList shiting_list = ask_who->tag["shiting_list"].toList();

            ask_who->setFlags("-shiting_use");
            foreach (QVariant v_use, shiting_list)
            {
                CardUseStruct use = v_use.value<CardUseStruct>();
                if (use.card != NULL && use.from != NULL && use.from->isAlive() && !use.to.isEmpty() && use.card->isAvailable(use.from))
                {
                    QList<ServerPlayer *> newTargets;
                    QList<const Player *> validityCheckTargets;
                    foreach (auto to, use.to)
                    {
                        if (to != NULL && to->isAlive() && use.card->targetFilter(validityCheckTargets, to, use.from) && !use.from->isProhibited(to, use.card, validityCheckTargets))
                        {
                            newTargets << to;
                            validityCheckTargets << to;
                        }
                    }

                    if (!newTargets.isEmpty() && use.card->targetsFeasible(validityCheckTargets, use.from))
                    {
                        use.to = newTargets;
                        room->useCard(use);
                    }
                }
            }
            ask_who->tag["shiting_list"] = QVariant();
        }
        return false;
    }
};

//shizhi -SE
class Shizhi : public TriggerSkill
{
public:
    Shizhi() : TriggerSkill("shizhi")
    {
        events << PreDamageDone << CardFinished;
        frequency = Frequent;
    }

    virtual void record(TriggerEvent event, Room *, ServerPlayer *, QVariant &data) const
    {
        if (event == PreDamageDone)
        {
            auto damage = data.value<DamageStruct>();
            if (damage.card != NULL && damage.card->isKindOf("Slash") && damage.from != NULL)
            {
                damage.from->setFlags("shizhi_slash_damaged");
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        auto skills = QStringList();
        if (!TriggerSkill::triggerable(player))
            return skills;
        if (event == CardFinished)
        {
            auto use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->isKindOf("Slash") && !use.to.isEmpty() && !player->hasFlag("shizhi_slash_damaged"))
            {
                foreach (auto to, use.to)
                {
                    if (to != NULL && to->isAlive() && player->canDiscard(to, "h"))
                    {
                        skills << objectName();
                        break;
                    }
                }
            }
            player->setFlags("-shizhi_slash_damaged");
        }
        return skills;
    }
    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (room->askForSkillInvoke(player, objectName(), data))
        {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }
    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        auto use = data.value<CardUseStruct>();
        auto candidates = QList<ServerPlayer *>();

        foreach (auto to, use.to)
        {
            if (to != NULL && to->isAlive() && player->canDiscard(to, "h"))
            {
                candidates << to;
            }
        }

        if (candidates.isEmpty())
            return false;

        auto target = room->askForPlayerChosen(player, candidates, objectName(), "@shizhi-targetchoose", false, false);

        if (target != NULL && target->isAlive() && player->canDiscard(target, "h"))
        {
            int id = room->askForCardChosen(player, target, "h", objectName(), false, Card::MethodDiscard);
            if (id != -1)
            {
                room->throwCard(id, target, player, objectName());
            }
        }

        return false;
    }
};

//quanmian, miaolv -SE
QuanmianCard::QuanmianCard()
{
}

bool QuanmianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return targets.isEmpty() && to_select->hasEquip();
}

void QuanmianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *yui = targets.first();
    int cardid = room->askForCardChosen(source, yui, "e", "quanmian");
    yui->obtainCard(Sanguosha->getCard(cardid));
    if (yui != source)
        source->drawCards(1);
}

class Quanmian : public ZeroCardViewAsSkill
{
public:
    Quanmian() : ZeroCardViewAsSkill("quanmian")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("QuanmianCard");
    }

    virtual const Card *viewAs() const
    {
        QuanmianCard *card = new QuanmianCard;
        card->setShowSkill(objectName());
        return card;
    }
};

MiaolvCard::MiaolvCard()
{
    target_fixed = true;
    mute = true;
}

void MiaolvCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct new_use = card_use;
    foreach (ServerPlayer *p, room->getAlivePlayers())
    {
        if (p->getHp() == card_use.card->getSubcards().length())
        {
            new_use.to << p;
        }
    }
    Card::onUse(room, new_use);
}

void MiaolvCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    RecoverStruct recover;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}

class Miaolv : public ViewAsSkill
{
public:
    Miaolv() : ViewAsSkill("miaolv")
    {
        response_or_use = true;
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getCardCount(true) > 0;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() > 0)
        {
            MiaolvCard *card = new MiaolvCard;
            card->addSubcards(cards);
            card->setShowSkill(objectName());
            return card;
        }
        return NULL;
    }
};

// yinzhuang, xiuse -SE, slob
class Yinzhuang : public TriggerSkill
{
public:
    Yinzhuang() : TriggerSkill("yinzhuang")
    {
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *mio, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(mio)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from != NULL && move.from == mio && move.from_places.contains(Player::PlaceEquip)
            && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_RESPONSE)
        {
            QStringList skill_list;
            for (int i = 0; i < move.card_ids.length(); i++)
                if (move.from_places[i] == Player::PlaceEquip)
                {
                    const EquipCard *equip = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(move.card_ids[i]));
                    switch (equip->location())
                    {
                        case EquipCard::WeaponLocation:
                            skill_list << "#yinzhuang-weapon";
                            break;
                        case EquipCard::ArmorLocation:
                            skill_list << "#yinzhuang-armor";
                            break;
                        case EquipCard::DefensiveHorseLocation:
                        case EquipCard::OffensiveHorseLocation:
                            skill_list << "#yinzhuang-horse";
                            break;
                        default:
                            break;
                    }
                }
            return skill_list;
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const
    {
        return false;
    }
};

class YinzhuangWeapon : public TriggerSkill
{
public:
    YinzhuangWeapon() : TriggerSkill("#yinzhuang-weapon")
    {
        events << NonTrigger;
        frequency = Compulsory;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        bool invoke = mio->hasShownSkill(this) ? true : room->askForSkillInvoke(mio, objectName());
        if (invoke)
        {
            room->broadcastSkillInvoke("yinzhuang", 2, mio);
            if (mio->hasShownSkill(this))
            {
                room->notifySkillInvoked(mio, "yinzhuang");

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mio;
                log.arg = "yinzhuang";
                room->sendLog(log);
            }
            else
                mio->showGeneral(mio->inHeadSkills("yinzhuang"));
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        room->drawCards(mio, 1, "yinzhuang");
        return false;
    }
};

class YinzhuangArmor : public TriggerSkill
{
public:
    YinzhuangArmor() : TriggerSkill("#yinzhuang-armor")
    {
        events << NonTrigger;
        frequency = Compulsory;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        bool invoke = mio->hasShownSkill(this) ? true : room->askForSkillInvoke(mio, objectName());
        if (invoke)
        {
            if (mio->hasShownSkill(this))
            {
                room->notifySkillInvoked(mio, "yinzhuang");
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mio;
                log.arg = "yinzhuang";
                room->sendLog(log);
            }
            else
                mio->showGeneral(mio->inHeadSkills("yinzhuang"));
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_yinzhuang");
        QList<ServerPlayer *> can_slashers;
        foreach (ServerPlayer *p, room->getOtherPlayers(mio))
        {
            if (mio->canSlash(p, slash, false))
            {
                can_slashers << p;
            }
        }
        if (can_slashers.isEmpty())
        {
            delete slash;
            //log
            return false;
        }
        ServerPlayer *slasher = room->askForPlayerChosen(mio, can_slashers, objectName(), "@yinzhuang-slash");
        room->useCard(CardUseStruct(slash, mio, slasher));
        return false;
    }
};

class YinzhuangHorse : public TriggerSkill
{
public:
    YinzhuangHorse() : TriggerSkill("#yinzhuang-horse")
    {
        events << NonTrigger;
        frequency = Compulsory;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        bool invoke = mio->hasShownSkill(this) ? true : room->askForSkillInvoke(mio, objectName());
        if (invoke)
        {
            room->broadcastSkillInvoke("yinzhuang", 3, mio);
            if (mio->hasShownSkill(this))
            {
                room->notifySkillInvoked(mio, "yinzhuang");

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mio;
                log.arg = "yinzhuang";
                room->sendLog(log);
            }
            else
                mio->showGeneral(mio->inHeadSkills("yinzhuang"));
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        if (mio->isWounded())
        {
            RecoverStruct recover;
            recover.who = mio;
            room->recover(mio, recover);
        }
        return false;
    }
};

class Xiuse : public TriggerSkill
{
public:
    Xiuse() : TriggerSkill("xiuse")
    {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(mio)) return QStringList();
        ServerPlayer *current = room->getCurrent();
        if (!current || current->isDead() || current->getPhase() == Player::NotActive)
            return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == mio && move.from_places.contains(Player::PlaceHand) && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD
            && !(move.to == mio && (move.to_place == Player::PlaceHand || move.to_place == Player::PlaceEquip)) && !current->hasFlag("xiuse_used"))
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        if (mio->askForSkillInvoke(objectName()))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *mio, QVariant &, ServerPlayer *) const
    {
        auto current = room->getCurrent();
        if (current != NULL && current->isAlive())
            room->setPlayerFlag(current, "xiuse_used");
        mio->drawCards(1);
        return false;
    }
};

//yingan -SE
class Yingan : public TriggerSkill
{
public:
    Yingan() : TriggerSkill("yingan")
    {
        events << CardFinished;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.from || use.from->isDead() || !use.card || !use.card->isKindOf("EquipCard"))
            return skill_list;

        QList<ServerPlayer *> yuis = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *yui, yuis)
        {
            if (yui->isFriendWith(use.from) || yui->willBeFriendWith(use.from))
                skill_list.insert(yui, QStringList(objectName()));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        if (ask_who->askForSkillInvoke(objectName(), data))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        ask_who->drawCards(1);
        QStringList big_kingdoms = ask_who->getBigKingdoms(objectName(), MaxCardsType::Normal);
        QList<ServerPlayer *> bigs, smalls;
        foreach (ServerPlayer *p, room->getAllPlayers())
        {
            QString kingdom = p->objectName();
            if (big_kingdoms.length() == 1 && big_kingdoms.first().startsWith("sgs"))
            { // for JadeSeal
                if (big_kingdoms.contains(kingdom))
                    bigs << p;
                else
                    smalls << p;
            }
            else
            {
                if (!p->hasShownOneGeneral())
                {
                    smalls << p;
                    continue;
                }
                if (p->getRole() == "careerist")
                    kingdom = "careerist";
                else
                    kingdom = p->getKingdom();
                if (big_kingdoms.contains(kingdom))
                    bigs << p;
                else
                    smalls << p;
            }
        }
        if ((!smalls.contains(ask_who)) && ask_who->canDiscard(ask_who, "he"))
            room->askForDiscard(ask_who, objectName(), 1, 1, false, true, "@yingan_friend");

        return false;
    }
};

//yinren by Fsu0413
class Yinren : public TriggerSkill
{
public:
    Yinren() : TriggerSkill("yinren")
    {
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const
    {
        if (TriggerSkill::triggerable(player) && (player->getPhase() == Player::Start || player->getPhase() == Player::Finish) && player->canDiscard(player, "h"))
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        return room->askForDiscard(player, objectName(), 1, 1, true, false, "@yinren-invoke");
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->getHandcardNum() == 0)
            return false;

        room->showAllCards(player);
        QList<Card::Suit> suit_list;
        bool duplicate = false;
        foreach (const Card *c, player->getHandcards())
        {
            Card::Suit s = c->getSuit();
            if (!suit_list.contains(s))
                suit_list << s;
            else
            {
                duplicate = true;
                break;
            }
        }
        if (!duplicate)
        {
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName("_yinren");
            QList<ServerPlayer *> can_slashers;
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
            {
                if (player->canSlash(p, slash, false))
                {
                    can_slashers << p;
                }
            }
            if (can_slashers.isEmpty())
            {
                delete slash;
                //log
                return false;
            }
            ServerPlayer *slasher = room->askForPlayerChosen(player, can_slashers, objectName(), "@yinren-slash");
            room->broadcastSkillInvoke(objectName());
            room->useCard(CardUseStruct(slash, player, slasher));
        }
        return false;
    }
};

class Tongxin : public TriggerSkill
{
public:
    Tongxin() : TriggerSkill("tongxin")
    {
        events << Death << GameStart;
        frequency = Limited;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const
    {
        if (triggerEvent == GameStart)
        {
            if (player->ownSkill(objectName()))
            {
                room->setPlayerProperty(player, "tongxin", 1);
            }
        }
        else if ((triggerEvent == Death) && (player != NULL && player->isDead()))
        {
            foreach (ServerPlayer *p, room->getAllPlayers(true))
            {
                if (p->isDead())
                {
                    bool ok = false;
                    int tongxin = p->property("tongxin").toInt(&ok);
                    if (ok && tongxin > 0)
                    {
                        DeathStruct death = data.value<DeathStruct>();
                        if ((death.who->isFriendWith(p) || death.who->willBeFriendWith(p)) && death.who != p)
                        {
                            ask_who = p;
                            return QStringList(objectName());
                        }
                    }
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who /* = NULL */) const
    {
        if (ask_who->askForSkillInvoke(objectName(), data))
        {
            room->broadcastSkillInvoke(objectName());
            //animate
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who /* = NULL */) const
    {
        room->setPlayerProperty(ask_who, "hp", player->getHp() + 2); //don't use the normal recover method
        room->revivePlayer(ask_who);
        room->detachSkillFromPlayer(ask_who, "yinren", false, false, ask_who->inHeadSkills(this));
        ask_who->drawCards(2);

        room->setPlayerProperty(ask_who, "tongxin", 0);

        return false;
    }
};

//xiehang by SE    NEED DEBUG

XiehangCard::XiehangCard()
{
}

bool XiehangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && !to_select->isKongcheng() && !Self->hasFlag("xiehang_used_" + to_select->objectName()) && to_select != Self;
}

void XiehangCard::use(Room *room, ServerPlayer *asuka, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.first();
    room->setPlayerFlag(asuka, "xiehang_used_" + target->objectName());
    PindianStruct *pindian = asuka->pindianSelect(target, "xiehang");
    //get the ids before pindian
    QList<int> cardids;

    bool win = asuka->pindian(pindian);

    if (pindian->from_card == NULL || pindian->to_card == NULL)
        return;

    cardids.append(pindian->from_card->getEffectiveId());
    cardids.append(pindian->to_card->getEffectiveId());

    ServerPlayer *user = asuka;

    if (!win)
    {
        if (pindian->from_number == pindian->to_number)
            return;
        if (!target->getAcquiredSkills().contains("xiehangAnother"))
            room->attachSkillToPlayer(target, "xiehangAnother");
        user = target;
    }

    QList<int> disabled_ids;

    foreach (int id, cardids)
    {
        auto tryCard = Sanguosha->getEngineCard(id);
        auto toTry = Sanguosha->cloneCard(tryCard);
        if (toTry != NULL)
        {
            toTry->setCanRecast(false);
            toTry->setSkillName("xiehang");
            if (!toTry->isAvailable(user))
            {
                disabled_ids << id;
            }
            delete toTry;
        }
    }

    room->fillAG(cardids, user, disabled_ids);
    int id = room->askForAG(user, cardids, true, objectName());
    room->clearAG(user);
    if (id == -1)
        return;
    Card *card = Sanguosha->getCard(id);
    if (card == NULL)
        return;

    room->setPlayerMark(user, "xiehangCardId", id);
    room->askForUseCard(user, "@@xiehang", objectName());
}

XiehangUseCard::XiehangUseCard()
{
}

bool XiehangUseCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (Self == NULL)
        return false;
    Card* card = Sanguosha->getCard(Self->getMark("xiehangCardId"));
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool XiehangUseCard::targetFixed() const
{
    if (Self == NULL)
        return false;
    Card* card = Sanguosha->getCard(Self->getMark("xiehangCardId"));
    return card && card->targetFixed();
}

bool XiehangUseCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    if (Self == NULL)
        return false;
    Card* card = Sanguosha->getCard(Self->getMark("xiehangCardId"));
    return card && card->targetsFeasible(targets, Self);
}

const Card * XiehangUseCard::validate(CardUseStruct &cardUse) const
{
    if (cardUse.from == NULL)
        return NULL;

    Card* card = Sanguosha->getCard(cardUse.from->getMark("xiehangCardId"));

    if (card == NULL)
        return NULL;

    card->setSkillName("xiehang");
    card->setCanRecast(false);

    bool ok = true;
    auto source = cardUse.from;
    auto room = source->getRoom();
    QList<ServerPlayer *> targets;

    if (card->isKindOf("AwaitExhausted"))
    {
        foreach(ServerPlayer *p, room->getOtherPlayers(source))
            if (!source->isProhibited(p, card) && source->isFriendWith(p))
                targets << p;
    }
    else if (card->getSubtype() == "global_effect" && !card->isKindOf("FightTogether"))
    {
        foreach(ServerPlayer *p, room->getAlivePlayers())
            if (!source->isProhibited(p, card))
                targets << p;
    }
    else if (card->isKindOf("FightTogether"))
    {
        QStringList big_kingdoms = source->getBigKingdoms("xiehang", MaxCardsType::Normal);
        bool can_use = !big_kingdoms.isEmpty() && !source->isCardLimited(this, Card::MethodUse);
        QStringList choices;
        QList<ServerPlayer *> bigs, smalls, bigs_void, smalls_void;
        if (can_use) {
            foreach(ServerPlayer *p, room->getAllPlayers()) {
                QString kingdom = p->objectName();
                if (big_kingdoms.length() == 1 && big_kingdoms.first().startsWith("sgs")) { // for JadeSeal
                    if (big_kingdoms.contains(kingdom)) {
                        if (room->isProhibited(source, p, this))
                            bigs_void << p;
                        else
                            bigs << p;
                    }
                    else {
                        if (room->isProhibited(source, p, this))
                            smalls_void << p;
                        else
                            smalls << p;
                    }
                }
                else {
                    if (!p->hasShownOneGeneral()) {
                        if (room->isProhibited(source, p, this))
                            smalls_void << p;
                        else
                            smalls << p;
                        continue;
                    }
                    if (p->getRole() == "careerist")
                        kingdom = "careerist";
                    else
                        kingdom = p->getKingdom();
                    if (big_kingdoms.contains(kingdom)) {
                        if (room->isProhibited(source, p, this))
                            bigs_void << p;
                        else
                            bigs << p;
                    }
                    else {
                        if (room->isProhibited(source, p, this))
                            smalls_void << p;
                        else
                            smalls << p;
                    }
                }
            }
            if (!bigs.isEmpty() || !smalls.isEmpty())
            {
                ok = true;
                targets.clear();
            }
        }
    }
    else if (card->getSubtype() == "aoe" && !card->isKindOf("BurningCamps"))
    {
        foreach(ServerPlayer *p, room->getOtherPlayers(source))
            if (!source->isProhibited(p, card))
                targets << p;
    }
    else if (card->isKindOf("BurningCamps"))
    {
        QList<const Player *> players = source->getNextAlive()->getFormation();
        foreach(const Player *p, players)
            if (!source->isProhibited(p, card))
                targets << room->findPlayerbyobjectName(p->objectName());
    }

    if (!targets.isEmpty())
        cardUse.to << targets;

    foreach(ServerPlayer *to, cardUse.to)
    {
        if (source->isProhibited(to, card))
        {
            ok = false;
            break;
        }
    }

    ok = ok && card->isAvailable(source);

    if (!ok)
        return NULL;
    return card;
}

class XiehangResidue : public TargetModSkill
{
public:
    XiehangResidue() : TargetModSkill("#xiehang-residue")
    {
        pattern = "BasicCard,TrickCard";
    }

    virtual int getResidueNum(const Player *, const Card *card) const
    {
        if (card->getSkillName() == "xiehang")
            return 1000;
        else
            return 0;
    }
};

class Xiehang : public ZeroCardViewAsSkill
{
public:
    Xiehang() : ZeroCardViewAsSkill("xiehang")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->isKongcheng();
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@xiehang";
    }

    virtual const Card *viewAs() const
    {
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern() == "@@xiehang")
        {
            return new XiehangUseCard;
        }
        XiehangCard *card = new XiehangCard;
        card->setCanRecast(false);
        card->setShowSkill(objectName());
        return card;
    }
};

class XiehangAnother : public ZeroCardViewAsSkill
{
public:
    XiehangAnother() : ZeroCardViewAsSkill("#xiehangAnother")
    {
        attached_lord_skill = true;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@xiehang";
    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    virtual const Card *viewAs() const
    {
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern() == "@@xiehang")
        {
            return new XiehangUseCard;
        }
        return NULL;
    }
};

//powei by SE
class Powei : public TriggerSkill
{
public:
    Powei() : TriggerSkill("powei")
    {
        events << SlashEffected << TargetConfirming << CardFinished;
        frequency = NotFrequent;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data) const
    {
        if (event == CardFinished)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.to.isEmpty())
                return;
            foreach(ServerPlayer *p, use.to)
            {
                if (p != NULL && p->isAlive() && p->getMark("powei") > 0)
                    room->setPlayerMark(p, "powei", 0);
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        if (triggerEvent == TargetConfirming)
        {
            CardUseStruct use = data.value<CardUseStruct>();
            Player *right = player->getNextAlive(1, false);
            Player *left = player->getLastAlive(1, false);
            if (use.card != NULL && use.card->isKindOf("Slash") && use.to.contains(player) && left->hasShownOneGeneral() && right->hasShownOneGeneral()
                && !(player->isFriendWith(right) || player->willBeFriendWith(right)) && !(player->isFriendWith(left) || player->willBeFriendWith(left)))
                return QStringList(objectName());
        }
        else if (triggerEvent == SlashEffected)
        {
            if (player->getMark("powei") > 0)
            {
                player->setMark("powei", 0);
                if (!player->hasShownSkill(this))
                    return QStringList();
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == SlashEffected)
            return true;
        bool invoke = room->askForSkillInvoke(player, objectName(), data);
        if (invoke)
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *asuka, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == SlashEffected)
        {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            LogMessage log;
            log.type = "#DanlaoAvoid";
            log.from = effect.to;
            log.arg2 = objectName();
            log.arg = effect.slash->objectName();
            room->sendLog(log);
            return true;
        }

        CardUseStruct use = data.value<CardUseStruct>();
        room->notifySkillInvoked(asuka, objectName());

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = asuka;
        log.arg = objectName();
        room->sendLog(log);

        QVariant dataforai = QVariant::fromValue(asuka);
        QString choice = room->askForChoice(use.from, "powei", "powei_discard+powei_cancel", dataforai);
        if (choice == "powei_discard")
        {
            if (!room->askForCard(use.from, ".|.|.|.", "@powei-discard", dataforai))
                asuka->addMark("powei");
        }
        else
        {
            asuka->addMark("powei");
        }

        return false;
    }
};

////lingchang by SE
//class Lingchang : public TriggerSkill
//{
//public:
//    Lingchang() : TriggerSkill("lingchang")
//    {
//        events << EventPhaseStart;
//    }
//
//    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
//    {
//        QMap<ServerPlayer *, QStringList> skill_list;
//        if (player == NULL) return skill_list;
//        if (player->getPhase() == Player::RoundStart)
//        {
//            if (!player->isWounded() || player->isKongcheng())
//                return skill_list;
//            QList<ServerPlayer *> inoris = room->findPlayersBySkillName(objectName());
//            foreach (ServerPlayer *inori, inoris)
//                if (inori->isAlive() && inori->canDiscard(player, "h") && (player->isFriendWith(inori) || inori->willBeFriendWith(player)))
//                    skill_list.insert(inori, QStringList(objectName()));
//            return skill_list;
//        }
//        return skill_list;
//    }
//
//    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const
//    {
//        if (ask_who->askForSkillInvoke(objectName(), QVariant::fromValue(player)))
//        {
//            room->broadcastSkillInvoke(objectName());
//            return true;
//        }
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const
//    {
//        int cardid = room->askForCardChosen(ask_who, player, "h", "lingchang");
//        room->showCard(ask_who, cardid);
//        room->getThread()->delay(1400);
//        room->throwCard(cardid, player, ask_who);
//        if (Sanguosha->getCard(cardid)->isRed())
//        {
//            RecoverStruct recover;
//            recover.who = player;
//            room->recover(player, recover);
//        }
//
//        return false;
//    }
//};
//
////bajian by SE
//BajianCard::BajianCard()
//{
//}
//
//bool BajianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
//{
//    Slash *slash = new Slash(Card::NoSuit, 0);
//    slash->deleteLater();
//    return slash->targetFilter(targets, to_select, Self);
//}
//
//void BajianCard::use(Room *room, ServerPlayer *syuu, QList<ServerPlayer *> &targets) const
//{
//    QList<ServerPlayer*> inoris = room->findPlayersBySkillName("bajian");
//    foreach (ServerPlayer *i, inoris)
//    {
//        if (i->isKongcheng() || i->objectName() == syuu->objectName())
//            inoris.removeOne(i);
//    }
//    if (inoris.isEmpty())
//        return;
//    ServerPlayer *inori = room->askForPlayerChosen(syuu, room->findPlayersBySkillName("bajian"), "bajianVS", "bajian-multipleInoris");
//    Card* card = Sanguosha->getCard(room->askForCardChosen(syuu, inori, "h", "bajianVS"));
//    Slash *slash = new Slash(card->getSuit(), card->getNumber());
//    slash->addSubcard(card);
//    slash->setShowSkill("bajianVS");
//    slash->setSkillName("bajianVS");
//    room->useCard(CardUseStruct(slash, syuu, targets), true);
//}
//
//class BajianViewAsSkill : public ZeroCardViewAsSkill
//{
//public:
//    BajianViewAsSkill() : ZeroCardViewAsSkill("bajianVS")
//    {
//    }
//
//    virtual bool isEnabledAtPlay(const Player *player) const
//    {
//        foreach (const Player *p, player->getSiblings())
//        {
//            if (p->hasSkill("bajian") && !p->isKongcheng() && p->objectName() != player->objectName())
//                return !player->hasUsed("BajianCard") && Slash::IsAvailable(player);
//        }
//        return false;
//    }
//
//    virtual const Card *viewAs() const
//    {
//        BajianCard *bajian = new BajianCard();
//        bajian->setShowSkill(objectName());
//        return bajian;
//    }
//};
//
//class Bajian : public TriggerSkill
//{
//public:
//    Bajian() : TriggerSkill("bajian")
//    {
//        events << GeneralShown << GeneralHidden << GeneralRemoved << Death;
//    }
//
//    virtual bool canPreshow() const
//    {
//        return false;
//    }
//
//    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
//    {
//        if (player == NULL)
//            return QStringList();
//        if (triggerEvent == Death)
//        {
//            DeathStruct death = data.value<DeathStruct>();
//            if (death.who->hasSkill(objectName()))
//            {
//                foreach (ServerPlayer *p, room->getAllPlayers())
//                    if (p->getMark("bajian") > 0)
//                    {
//                        room->setPlayerMark(p, "bajian", 0);
//                        room->detachSkillFromPlayer(p, "bajianVS", true, true);
//                    }
//                return QStringList();
//            }
//            else
//            {
//                if (death.who->getMark("bajian") > 0)
//                {
//                    room->setPlayerMark(death.who, "bajian", 0);
//                    room->detachSkillFromPlayer(death.who, "bajianVS", true, true);
//                    return QStringList();
//                }
//            }
//        }
//        foreach (ServerPlayer *p, room->getAllPlayers())
//            if (p->getMark("bajian") > 0)
//            {
//                room->setPlayerMark(p, "bajian", 0);
//                room->detachSkillFromPlayer(p, "bajianVS", true, true);
//            }
//
//        QList<ServerPlayer *> inoris = room->findPlayersBySkillName(objectName());
//        foreach (ServerPlayer *inori, inoris)
//            if (inori->hasShownSkill(this))
//            {
//                foreach (ServerPlayer *p, room->getOtherPlayers(inori))
//                    if (inori->isFriendWith(p) || inori->willBeFriendWith(p))
//                    {
//                        room->setPlayerMark(p, "bajian", 1);
//                        room->attachSkillToPlayer(p, "bajianVS");
//                    }
//            }
//
//        return QStringList();
//    }
//};



//liufei by AK
class Liufei : public TriggerSkill
{
public:
    Liufei() : TriggerSkill("liufei")
    {
        events << TargetConfirming;
        frequency = Compulsory;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL)
            return skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card || !use.card->isKindOf("Slash") || !use.to.contains(player))
            return skill_list;

        QList<ServerPlayer *> mayus = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *mayu, mayus)
        {
            if ((mayu->isFriendWith(player) || mayu->willBeFriendWith(player)) && mayu != player && use.from->inMyAttackRange(mayu) && mayu->getHandcardNum() < mayu->getHp())
                skill_list.insert(mayu, QStringList(objectName()));
        }
        return skill_list;
    }
    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        bool invoke = ask_who->hasShownSkill(this) ? true : ask_who->askForSkillInvoke(objectName(), data);
        if (invoke)
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }
    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        room->notifySkillInvoked(ask_who, objectName());
        LogMessage log;
        if (use.from)
        {
            log.type = "$CancelTarget";
            log.from = use.from;
        }
        else
        {
            log.type = "$CancelTargetNoUser";
        }
        log.to << player;
        log.arg = use.card->objectName();
        room->sendLog(log);

        use.to.removeOne(player);
        data = QVariant::fromValue(use);
        return false;
    }
};

//pianxian by AK

class PianxianViewAsSkill : public ViewAsSkill
{
public:
    PianxianViewAsSkill() : ViewAsSkill("pianxian")
    {
        response_pattern = "@@pianxian";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() < Self->getHp())
            return NULL;
        DummyCard *card = new DummyCard;
        card->setShowSkill("pianxian");
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class Pianxian : public PhaseChangeSkill
{
public:
    Pianxian() : PhaseChangeSkill("pianxian")
    {
        view_as_skill = new PianxianViewAsSkill;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer * &) const
    {
        if (!PhaseChangeSkill::triggerable(player))
            return QStringList();
        if (player->getPhase() != Player::Start || player->isNude())
            return QStringList();
        int x = 0;
        foreach (const Card *card, player->getCards("he"))
        {
            if (player->isJilei(card))
                x++;
        }
        if (player->getCardCount(true) - x < player->getHp())
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(objectName()))
        {
            room->broadcastSkillInvoke(objectName());
            const Card * card = room->askForUseCard(player, "@@pianxian", "@pianxian", -1, Card::MethodDiscard);
            if (card)
            {
                player->tag["pianxiannum"] = card->subcardsLength();
                return true;
            }
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const
    {
        Room *room = player->getRoom();
        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);
        int num = player->tag["pianxiannum"].toInt();
        if (num >= 2)
        {
            Duel *duel = new Duel(Card::NoSuit, 0);
            duel->setSkillName("_pianxian");
            QList<ServerPlayer *> can_duelers;
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
            {
                if (duel->targetFilter(QList<const Player *>(), p, player) && !player->isProhibited(p, duel))
                    can_duelers << p;
            }
            if (can_duelers.isEmpty())
            {
                delete duel;
                //log
                return false;
            }
            ServerPlayer *dest = room->askForPlayerChosen(player, can_duelers, objectName(), "@pianxian-duel");
            room->broadcastSkillInvoke(objectName());
            room->useCard(CardUseStruct(duel, player, dest));
        }
        return false;
    }
};

//geji by SE
GejiCard::GejiCard()
{
}

bool GejiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (to_select->isNude() || !to_select->hasShownOneGeneral() || to_select->objectName() == Self->objectName())
        return false;
    foreach (const Player *p, targets)
    {
        if (p->isFriendWith(to_select))
            return false;
    }
    return true;
}

void GejiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    targets.append(source);
    QList<ServerPlayer *> drawPlayers;
    //broadcast
    foreach (ServerPlayer *p, targets)
    {
        int cardid = room->askForCardChosen(source, p, "he", "geji", false, Card::MethodDiscard);
        if (cardid != -1)
        {
            room->throwCard(cardid, p, source, "geji");
            if (Sanguosha->getCard(cardid)->getSuit() == Card::Spade)
            {
                drawPlayers.append(p);
            }
        }
    }
    //log
    if (drawPlayers.length() > 0)
    {
        foreach (ServerPlayer *kira, drawPlayers)
        {
            kira->drawCards(1);
        }
    }
}

class Geji : public ZeroCardViewAsSkill
{
public:
    Geji() : ZeroCardViewAsSkill("geji")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("GejiCard") && !player->isNude();
    }

    virtual const Card *viewAs() const
    {
        GejiCard *card = new GejiCard;
        card->setShowSkill(objectName());
        return card;
    }
};

//pinghe by SE
class Pinghe : public TriggerSkill
{
public:
    Pinghe() : TriggerSkill("pinghe")
    {
        events << CardUsed;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL)
            return skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card || !use.from || !use.card->isKindOf("Slash"))
            return skill_list;

        QList<ServerPlayer *> lacuses = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *lacus, lacuses)
        {
            skill_list.insert(lacus, QStringList(objectName()));
        }
        return skill_list;
    }
    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        bool invoke = ask_who->askForSkillInvoke(objectName(), data);
        if (invoke)
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }
    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        room->notifySkillInvoked(ask_who, objectName());
        
        LogMessage log;
        log.type = "#CardNullified";
        log.from = use.to.first();
        log.arg = use.card->objectName();
        room->sendLog(log);

        const Card *card = room->askForCard(use.from, "slash", "@pinghe", QVariant(), Card::MethodNone);
        if (card != NULL)
        {
            room->showCard(use.from, card->getEffectiveId());
            room->detachSkillFromPlayer(ask_who, objectName(), false, false, ask_who->inHeadSkills(this));
        }
        return true;
    }
};

ShowMashu::ShowMashu()
    : ShowDistanceCard()
{
}

Mashu::Mashu(const QString &owner) : DistanceSkill("mashu_" + owner)
{
}

int Mashu::getCorrect(const Player *from, const Player *) const
{
    if (from->hasSkill(objectName()) && from->hasShownSkill(this))
        return -1;
    else
        return 0;
}

//tengyue by SE
class TengyueTrigger : public TriggerSkill
{
public:
    TengyueTrigger() : TriggerSkill("#tengyue-trigger")
    {
        events << EventLoseSkill << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data, ServerPlayer * &) const
    {
        if (triggerEvent == EventLoseSkill)
        {
            if (target && target->getMark("@TengyueExtraSlashes") > 0 && data.toString() == "tengyue")
            {
                room->setPlayerMark(target, "@TengyueExtraSlashes", 0);
            }
        }
        else if (triggerEvent == EventPhaseChanging)
        {
            if (target && target->getMark("@TengyueExtraSlashes") > 0 && data.value<PhaseChangeStruct>().to == Player::NotActive)
            {
                room->setPlayerMark(target, "@TengyueExtraSlashes", 0);
            }
        }
        return QStringList();
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const
    {
        return 2;
    }
};

Tengyue::Tengyue() : DrawCardsSkill("tengyue")
{
    frequency = NotFrequent;
}

bool Tengyue::cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
{
    if (player->askForSkillInvoke(objectName()))
    {
        room->broadcastSkillInvoke(objectName());
        return true;
    }
    return false;
}

int Tengyue::getDrawNum(ServerPlayer *player, int n) const
{
    QString choices = "tengyue1";
    for (int i = 1; i < n; i++)
    {
        choices += "+tengyue" + QString::number(i + 1);
    }
    QString choice = player->getRoom()->askForChoice(player, "tengyue", choices);
    int num = choice.section('e', -1).toInt();

    player->getRoom()->setPlayerMark(player, "@TengyueExtraSlashes", num);
    return n - num;
}

class TengyueTargetMod : public TargetModSkill
{
public:
    TengyueTargetMod() : TargetModSkill("#tengyue-target")
    {
    }

    virtual int getResidueNum(const Player *from, const Card *card) const
    {
        if (card->isKindOf("Slash") && from->getMark("@TengyueExtraSlashes") > 0)
            return (from->getMark("@TengyueExtraSlashes"));
        else
            return 0;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const
    {
        if (card->isKindOf("Slash") && from->getMark("@TengyueExtraSlashes") > 0)
            return 1;
        else
            return 0;
    }
};

//mogai by SE
class Mogai : public TriggerSkill
{
public:
    Mogai() : TriggerSkill("mogai")
    {
        events << TargetConfirming << CardUsed;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *miho, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(miho)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isKindOf("Slash") && miho->canDiscard(miho, "e"))
        {
            QList<const char*> list;
            foreach (const Card *card, miho->getEquips())
            {
                if (card->isKindOf("Weapon"))
                    list.append("Weapon");
                if (card->isKindOf("Armor"))
                    list.append("Armor");
                if (card->isKindOf("OffensiveHorse"))
                    list.append("OffensiveHorse");
                if (card->isKindOf("DefensiveHorse"))
                    list.append("DefensiveHorse");
            }
            QList<int> cardids = room->getDiscardPile();
            QList<int> useids;
            foreach (int cardid, cardids)
            {
                foreach (const char* kind, list)
                {
                    if (Sanguosha->getCard(cardid)->isKindOf(kind))
                    {
                        useids.append(cardid);
                    }
                }
            }
            if (useids.isEmpty())
            {
                return QStringList();
            }

            if (triggerEvent == TargetConfirming)
            {
                if (use.to.contains(miho))
                    return QStringList(objectName());
            }
            else if (triggerEvent == CardUsed)
            {
                if (use.from == miho)
                {
                    return QStringList(objectName());
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *miho, QVariant &data, ServerPlayer *) const
    {
        bool invoke = miho->askForSkillInvoke(objectName(), data);
        if (invoke)
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *miho, QVariant &, ServerPlayer *) const
    {
        QList<const char*> list;
        foreach (const Card *card, miho->getEquips())
        {
            if (card->isKindOf("Weapon"))
                list.append("Weapon");
            if (card->isKindOf("Armor"))
                list.append("Armor");
            if (card->isKindOf("OffensiveHorse"))
                list.append("OffensiveHorse");
            if (card->isKindOf("DefensiveHorse"))
                list.append("DefensiveHorse");
        }
        QList<int> cardids = room->getDiscardPile();
        QList<int> useids;
        foreach (int cardid, cardids)
        {
            foreach (const char* kind, list)
            {
                if (Sanguosha->getCard(cardid)->isKindOf(kind))
                {
                    useids.append(cardid);
                }
            }
        }
        if (useids.isEmpty())
        {
            //arg
            return false;
        }
        room->fillAG(useids, miho);
        int cid = room->askForAG(miho, useids, true, objectName());
        room->clearAG(miho);
        if (cid <= 0)
            return false;
        if (Sanguosha->getCard(cid)->isKindOf("Weapon"))
        {
            room->throwCard(miho->getWeapon(), CardMoveReason(CardMoveReason::S_REASON_RESPONSE, miho->objectName()), miho);
        }
        else if (Sanguosha->getCard(cid)->isKindOf("Armor"))
        {
            room->throwCard(miho->getArmor(), CardMoveReason(CardMoveReason::S_REASON_RESPONSE, miho->objectName()), miho);
        }
        else if (Sanguosha->getCard(cid)->isKindOf("DefensiveHorse"))
        {
            room->throwCard(miho->getDefensiveHorse(), CardMoveReason(CardMoveReason::S_REASON_RESPONSE, miho->objectName()), miho);
        }
        else if (Sanguosha->getCard(cid)->isKindOf("OffensiveHorse"))
        {
            room->throwCard(miho->getOffensiveHorse(), CardMoveReason(CardMoveReason::S_REASON_RESPONSE, miho->objectName()), miho);
        }
        else if (Sanguosha->getCard(cid)->isKindOf("Treasure"))
        {
            room->throwCard(miho->getTreasure(), CardMoveReason(CardMoveReason::S_REASON_RESPONSE, miho->objectName()), miho);
        }

        room->moveCardTo(Sanguosha->getCard(cid), miho, Player::PlaceEquip, CardMoveReason(CardMoveReason::S_REASON_CHANGE_EQUIP, miho->objectName()));
        return false;
    }
};

//ruhun by SE
class Ruhun : public TriggerSkill
{
public:
    Ruhun() : TriggerSkill("ruhun")
    {
        events << TargetConfirmed;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *miho, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(miho))
            return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card != NULL && use.card->isKindOf("Slash") && miho->canDiscard(miho, "h"))
        {
            if (use.from != NULL && use.from == miho && !use.to.isEmpty())
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *miho, QVariant &data, ServerPlayer *) const
    {
        if (miho->askForSkillInvoke(objectName(), data))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *miho, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> left = use.to;
        const Card *card = room->askForCard(miho, ".", "@ruhun", QVariant(), objectName());
        if (card != NULL && !use.to.isEmpty())
        {
            room->throwCard(card, CardMoveReason(CardMoveReason::S_REASON_DISCARD, miho->objectName()), miho, miho);
            QString pattern = ".|" + card->getSuitString() + "|.|.";
            foreach (auto to, use.to)
            {
                room->setPlayerCardLimitation(to, "use,response", pattern, true);
            }

            LogMessage log;
            log.type = "#RuhunLimit";
            log.from = miho;
            log.to << use.to;
            log.arg = card->getSuitString();
            log.arg2 = objectName();
            room->sendLog(log);
        }
        return false;
    }
};
//
////qiyuan for menma
//class Qiyuan : public TriggerSkill
//{
//public:
//    Qiyuan() : TriggerSkill("qiyuan")
//    {
//        frequency = NotFrequent;
//        events << EventPhaseStart << DrawNCards << CardsMoveOneTime << EventPhaseEnd << EventPhaseChanging;
//    }
//
//    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
//    {
//        if (event == CardsMoveOneTime)
//        {
//            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
//            if (move.from != NULL && move.from == player && player->getPhase() == Player::Discard && move.to_place == Player::DiscardPile
//                && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
//                room->setPlayerMark(player, "qiyuan-discard", player->getMark("qiyuan-discard") + move.card_ids.length());
//        }
//        else if (event == EventPhaseChanging)
//        {
//            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
//            if (change.to == Player::Discard || change.from == Player::Discard)
//            {
//                room->setPlayerMark(player, "qiyuan-discard", 0);
//            }
//            if (change.to == Player::NotActive || change.from == Player::NotActive)
//            {
//                room->setPlayerMark(player, "@qiyuan-draw", 0);
//            }
//        }
//    }
//
//    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
//    {
//        TriggerList skill_list;
//        if (player == NULL)
//            return skill_list;
//        if (event == EventPhaseStart)
//        {
//            if (player->getPhase() != Player::Start)
//                return skill_list;
//            QList<ServerPlayer *> menmas = room->findPlayersBySkillName(objectName());
//            foreach (ServerPlayer *menma, menmas)
//            {
//                if (menma->canDiscard(menma, "h") && menma != player)
//                    skill_list.insert(menma, QStringList(objectName()));
//            }
//        }
//        else if (event == EventPhaseEnd)
//        {
//            if (player->getPhase() != Player::Discard)
//                return skill_list;
//            QList<ServerPlayer *> menmas = room->findPlayersBySkillName(objectName());
//            foreach (ServerPlayer *menma, menmas)
//            {
//                if (menma != player && player->getMark("qiyuan-discard") > menma->getHandcardNum())
//                    skill_list.insert(menma, QStringList(objectName()));
//            }
//        }
//        else if (event == DrawNCards)
//        {
//            if (player->getMark("@qiyuan-draw") > 0)
//            {
//                data = data.toInt() + player->getMark("@qiyuan-draw");
//                room->setPlayerMark(player, "@qiyuan-draw", 0);
//            }
//        }
//        return skill_list;
//    }
//
//    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
//    {
//        if (event == EventPhaseStart)
//        {
//            ServerPlayer *menma = ask_who;
//            if (menma != NULL && room->askForDiscard(menma, objectName(), 1, 1, true, false, "@qiyuan-discard", true))
//            {
//                room->broadcastSkillInvoke(objectName(), 1, menma);
//                return true;
//            }
//        }
//        else if (event == EventPhaseEnd)
//        {
//            ServerPlayer *menma = ask_who;
//            if (menma != NULL && room->askForSkillInvoke(menma, objectName()))
//            {
//                room->broadcastSkillInvoke(objectName(), 2, menma);
//                return true;
//            }
//        }
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const
//    {
//        if (event == EventPhaseStart)
//        {
//            room->setPlayerMark(player, "@qiyuan-draw", player->getMark("@qiyuan-draw") + 1);
//        }
//        else if (event == EventPhaseEnd)
//        {
//            if (ask_who != NULL && ask_who->isAlive())
//                ask_who->drawCards(1, objectName());
//        }
//        return false;
//    }
//};
//
//// Huaming -- menma
//class Huaming : public TriggerSkill
//{
//public:
//    Huaming() : TriggerSkill("huaming")
//    {
//        events << Death;
//    }
//
//    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
//    {
//        if (player == NULL || !player->ownSkill(this))
//            return QStringList();
//
//        DeathStruct death = data.value<DeathStruct>();
//        if (death.who != NULL && player == death.who)
//        {
//            QList<ServerPlayer *> alives = room->getAlivePlayers();
//            if ((death.damage->from != NULL && death.damage->from->isAlive() && alives.contains(death.damage->from) && alives.length() > 1) 
//                || (death.damage->from == NULL && alives.length() > 0))
//            {
//                return QStringList(objectName());
//            }
//        }
//        return QStringList();
//    }
//
//    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        QList<ServerPlayer *> alives = room->getAlivePlayers();
//        DeathStruct death = data.value<DeathStruct>();
//        if ((death.damage->from != NULL && death.damage->from->isAlive() && alives.contains(death.damage->from)))
//        {
//            alives.removeOne(death.damage->from);
//            ServerPlayer *target = room->askForPlayerChosen(player, alives, objectName(), "@huaming-choose", true, true);
//            if (target != NULL && target->isAlive())
//            {
//                player->tag["huaming-tar"] = QVariant::fromValue(target);
//                return true;
//            }
//        }
//        else if (death.damage->from == NULL && alives.length() > 0)
//        {
//            ServerPlayer *target = room->askForPlayerChosen(player, alives, objectName(), "@huaming-choose", true, true);
//            if (target != NULL && target->isAlive())
//            {
//                player->tag["huaming-tar"] = QVariant::fromValue(target);
//                return true;
//            }
//        }
//        player->tag["huaming-tar"] = QVariant();
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        ServerPlayer *target = player->tag["huaming-tar"].value<ServerPlayer *>();
//        DeathStruct death = data.value<DeathStruct>();
//        ServerPlayer *killer = death.damage->from;
//        if (target == NULL || !target->isAlive()) return false;
//        if (killer != NULL && killer->isAlive() && !killer->isNude())
//        {
//            int id = room->askForCardChosen(target, killer, "he", objectName(), false);
//            if (id != -1)
//                target->obtainCard(Sanguosha->getEngineCard(id), false);
//        }
//        RecoverStruct recover;
//        recover.who = player;
//        recover.recover = 1;
//        room->recover(target, recover);
//        player->tag["huaming-tar"] = QVariant();
//        return false;
//    }
//};

//qiyuan for menma
class Qiyuan : public TriggerSkill
{
public:
    Qiyuan() : TriggerSkill("qiyuan")
    {
        frequency = NotFrequent;
        events << CardUsed << CardResponded;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList list;
        if (player == NULL || !player->isAlive())
            return list;
        if (event == CardUsed || event == CardResponded)
        {
            ServerPlayer *from = NULL;
            const Card *card = NULL;
            if (event == CardUsed)
            {
                CardUseStruct use = data.value<CardUseStruct>();
                from = use.from;
                card = use.card;
            }
            else
            {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                from = player;
                card = resp.m_card;
            }
            if (from != NULL && from->isAlive() && card != NULL && card->isKindOf("Jink"))
            {
                foreach (auto menma, room->findPlayersBySkillName(objectName()))
                {
                    list.insert(menma, QStringList(objectName()));
                }
            }
        }
        return list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const
    {
        if (room->askForSkillInvoke(ask_who, objectName(), qVariantFromValue(player)))
        {
            room->broadcastSkillInvoke(objectName(), ask_who);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        player->drawCards(1, objectName());
        return false;
    }
};

class Huaming : public TriggerSkill
{
public:
    Huaming() : TriggerSkill("huaming")
    {
        frequency = Compulsory;
        events << DamageInflicted << GeneralShown;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        QStringList skill_list;
        if (player == NULL || player->isDead())
            return skill_list;
        if (event == DamageInflicted)
        {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.damage < 1 || !player->hasShownSkill(this))
                return skill_list;

            bool has_hide = false;
            foreach (auto p, room->getOtherPlayers(player))
            {
                if (!p->hasShownAllGenerals() && p->canShowGeneral())
                {
                    has_hide = true;
                    break;
                }
            }

            if (has_hide)
                skill_list << objectName();
        }
        else if (event == GeneralShown)
        {
            GeneralHandleStruct handle = data.value<GeneralHandleStruct>();
            if (handle.who == player && handle.generalName == "inori")
            {
                foreach (auto p, room->getOtherPlayers(player))
                {
                    if (p->isFriendWith(player))
                    {
                        player->loseSkill(objectName(), player->inHeadSkills(this));
                        LogMessage log;
                        log.type = "#HuamingLose";
                        log.from = player;
                        log.to << p;
                        log.arg = objectName();
                        room->sendLog(log);
                        break;
                    }
                }
            }
        }

        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        foreach (auto p, room->getOtherPlayers(player))
        {
            if (!p->hasShownAllGenerals() && p->canShowGeneral() && p->askForGeneralShow(true, true))
            {
                room->broadcastSkillInvoke(objectName());
                p->drawCards(1, objectName());
                player->tag["huaming_source"] = qVariantFromValue(p);
                return true;
            }
        }

        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();

        auto source = player->tag["huaming_source"].value<ServerPlayer *>();

        LogMessage log;
        log.type = "#HuamingReduce";
        log.from = player;
        log.to << source;
        log.arg = objectName();
        log.arg2 = QString::number(--damage.damage);
        room->sendLog(log);

        if (damage.damage == 0)
        {
            damage.prevented = true;
            return true;
        }

        data = QVariant::fromValue(damage);
        return false;
    }
};

// Chidun --ayanami 
class Chidun : public TriggerSkill
{
public:
    Chidun() : TriggerSkill("chidun")
    {
        frequency = NotFrequent;
        events << DamageInflicted << DamageComplete;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player == NULL) return skill_list;
        DamageStruct damage = data.value<DamageStruct>();
        if (triggerEvent == DamageInflicted && player->isAlive())
        {
            if (damage.damage < 1 || damage.transfer)
                return skill_list;

            QList<ServerPlayer *> ayanamis = room->findPlayersBySkillName(objectName());

            foreach (ServerPlayer *ayanami, ayanamis)
                if ((ayanami->isFriendWith(player) || ayanami->willBeFriendWith(player)) && ayanami != player)
                    skill_list.insert(ayanami, QStringList(objectName()));
        }
        else if (triggerEvent == DamageComplete && damage.transfer && damage.transfer_reason == objectName() && damage.to == player && player->isAlive())
            skill_list.insert(player, QStringList(objectName()));
        return skill_list;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        if (triggerEvent == DamageInflicted && ask_who->askForSkillInvoke(objectName(), data))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        else if (triggerEvent == DamageComplete)
        {
            ServerPlayer *slasher = NULL;
            DamageStruct damage = data.value<DamageStruct>();
            foreach (ServerPlayer *tar, room->getAlivePlayers())
                if (tar->hasFlag("chidun_tar"))
                    slasher = tar;
            if (slasher)
                slasher->setFlags("-chidun_tar");
            QString prompt = "@chidun:" + damage.from->objectName();
            if (room->askForUseSlashTo(slasher, damage.from, prompt, false, false, false))
                return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == DamageInflicted)
        {
            ask_who->drawCards(1, objectName());

            DamageStruct damage = data.value<DamageStruct>();
            damage.to->setFlags("chidun_tar");
            damage.transfer = true;
            damage.to = ask_who;
            damage.transfer_reason = objectName();

            player->tag["TransferDamage"] = QVariant::fromValue(damage);

            LogMessage log;
            log.type = "#ChidunTransfer";
            log.from = ask_who;
            log.to << damage.to;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);

            room->sendLog(log);

            return true;
        }
        return false;
    }
};

class WuxinAya : public TriggerSkill
{
public:
    WuxinAya() : TriggerSkill("wuxinAya")
    {
        frequency = Limited;
        limit_mark = "@wuxin";
        events << DamageInflicted;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if ((triggerEvent == DamageInflicted) && TriggerSkill::triggerable(player) && player->getMark("@wuxin") > 0)
        {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.damage >= player->getHp())
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who /* = NULL */) const
    {
        if (ask_who->askForSkillInvoke(objectName(), data))
        {
            ask_who->loseAllMarks("@wuxin");
            room->doSuperLightbox("rei", "Evangelion");
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer * /* = NULL */) const
    {
        RecoverStruct recover;
        recover.who = player;
        recover.recover = player->getLostHp();
        room->recover(player, recover);
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "@wuxin_text", false, true);
        room->acquireSkill(target, "yongjue");
        room->setPlayerMark(target, "@yongjue", 1);
        return true;
    }
};

class Qinyin : public TriggerSkill
{
public:
    Qinyin() : TriggerSkill("qinyin")
    {
        frequency = NotFrequent;
        events << CardUsed << CardResponded;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        const Card *card = NULL;
        if (event == CardUsed)
        {
            card = data.value<CardUseStruct>().card;
        }
        else
        {
            card = data.value<CardResponseStruct>().m_card;
        }
        if (card == NULL)
            return QStringList();

        int black = card->isBlack();
        foreach (ServerPlayer *p, room->getOtherPlayers(player))
        {
            if (black && p->getHp() > player->getHp() && player->canDiscard(p, "he"))
            {
                black++;
                break;
            }
        }
        if ((card->isRed() && !player->hasFlag("Qinyin_Red")) || (black == 2 && !player->hasFlag("Qinyin_Black")))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        const Card *card = NULL;
        if (event == CardUsed)
        {
            card = data.value<CardUseStruct>().card;
        }
        else
        {
            card = data.value<CardResponseStruct>().m_card;
        }
        if (card == NULL)
            return false;

        bool red = card->isRed();
        bool black = card->isBlack();
        QList<ServerPlayer *> reds;
        QList<ServerPlayer *> blacks;
        foreach (ServerPlayer *p, room->getOtherPlayers(player))
        {
            if (black && p->getHp() > player->getHp() && player->canDiscard(p, "he"))
            {
                blacks << p;
            }
            else if (red && p->getHp() >= player->getHp())
            {
                reds << p;
            }

        }
        reds << player;

        ServerPlayer *target = NULL;
        
        if (red && reds.length() > 0)
        {
            player->setFlags("AI_qinyin_red"); // for AI
            target = room->askForPlayerChosen(player, reds, objectName(), "@qinyin-red", true, false);
        }
        else if (black && blacks.length() > 0)
        {
            player->setFlags("AI_qinyin_black");
            target = room->askForPlayerChosen(player, blacks, objectName(), "@qinyin-black", true, false);
        }
        player->setFlags("-AI_qinyin_red");
        player->setFlags("-AI_qinyin_black");

        if (target != NULL)
        {
            player->tag["Qinyin_target"] = QVariant::fromValue(target);
            player->tag["Qinyin_effect"] = QVariant::fromValue(red ? true : false);
            room->broadcastSkillInvoke(objectName());
            player->setFlags(blacks.isEmpty() ? "Qinyin_Red" : "Qinyin_Black");
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        ServerPlayer *target = player->tag["Qinyin_target"].value<ServerPlayer *>();
        bool red = player->tag["Qinyin_effect"].value<bool>();
        if (target == NULL)
            return false;

        LogMessage log;

        if (!red)
        {
            int id = room->askForCardChosen(player, target, "he", objectName(), false, Card::MethodDiscard);
            if (id != -1)
            {
                room->throwCard(Sanguosha->getCard(id), target, player, objectName());
                log.type = "#QinyinBlack";
                log.from = player;
                log.to << target;
                log.arg = Sanguosha->getCard(id)->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
            }
        }
        else
        {
            target->drawCards(1, objectName());
            log.type = "#QinyinRed";
            log.from = player;
            log.to << target;
            log.arg = objectName();
            room->sendLog(log);
        }
        return false;
    }
};

class Yongjue : public TriggerSkill
{
public:
    Yongjue() : TriggerSkill("yongjue")
    {
        events << CardUsed << CardResponded << CardsMoveOneTime << EventPhaseStart;
        frequency = Frequent;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual void record(TriggerEvent event, Room *, ServerPlayer *player, QVariant &) const
    {
        if (event == EventPhaseStart)
        {
            if (player != NULL && player->isAlive() && player->getMark("yongjue") > 0 && player->getPhase() == Player::Play)
                player->setMark("yongjue", 0);
        }
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const
    {
        if (player == NULL || !player->isAlive())
            return QStringList();
        QList<ServerPlayer *> owners = room->findPlayersBySkillName(objectName());
        if (triggerEvent == CardUsed || triggerEvent == CardResponded)
        {
            ServerPlayer *from = NULL;
            bool is_use = false;
            const Card *card = NULL;
            if (triggerEvent == CardUsed)
            {
                is_use = true;
                CardUseStruct use = data.value<CardUseStruct>();
                from = use.from;
                card = use.card;
            }
            else
            {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                is_use = resp.m_isUse;
                from = player;
                card = resp.m_card;
            }
            if (from->getPhase() == Player::Play && from->getMark(objectName()) == 0 && is_use)
            {
                if (!card->isKindOf("SkillCard"))
                    from->addMark(objectName());
                if (card->isKindOf("Slash"))
                {
                    from->tag.remove("yongjue_id");
                    int yongjue_id = -1;
                    if (!card->isVirtualCard())
                        yongjue_id = card->getId();
                    else if (card->subcardsLength() == 1)
                    {
                        const Card *c = Sanguosha->getCard(card->getSubcards().first());
                        if (c->isKindOf("Slash"))
                            yongjue_id = c->getId();
                    }

                    if (yongjue_id != -1)
                        from->tag["yongjue_id"] = yongjue_id;
                }
            }
        }
        else if (triggerEvent == CardsMoveOneTime)
        {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != NULL && move.from->tag.contains("yongjue_id") && player == move.from
                && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_USE)
                && move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile)
            {
                if (move.card_ids.length() == 1)
                {
                    bool ok = false;
                    int yongjue_id = player->tag["yongjue_id"].toInt(&ok);
                    if (ok && yongjue_id == move.card_ids.first())
                    {
                        ask_who = player;
                        foreach(ServerPlayer *p, owners)
                        {
                            if (p->isFriendWith(player))
                                return QStringList(objectName());
                        }
                    }
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        QList<ServerPlayer *> owners = room->findPlayersBySkillName(objectName());
        ServerPlayer *owner = NULL;
        foreach(ServerPlayer *p, owners)
        {
            if (player->isFriendWith(p))
            {
                owner = p;
                break;
            }
        }

        if (owner != NULL)
        {
            player->tag.remove("yongjue_id");
            if (player->askForSkillInvoke(this))
            {
                LogMessage log;
                log.type = "#InvokeOthersSkill";
                log.from = player;
                log.to << owner;
                log.arg = objectName();
                room->sendLog(log);
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, owner->objectName(), player->objectName());
                room->broadcastSkillInvoke(objectName(), owner);
                if (owner != player)
                    room->notifySkillInvoked(owner, objectName());

                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        DummyCard dummy(move.card_ids);
        player->obtainCard(&dummy);
        return false;
    }
};

class Jianqiao : public TriggerSkill
{
public:
    Jianqiao() : TriggerSkill("jianqiao")
    {
        events << EventPhaseStart << CardsMoveOneTime << CardUsed << EventPhaseChanging;
    }

    // @TODO: set the jianqiao source playerlist for jianqiao target, make 2nd effect "change equips" only trigger for those source.
    // now I use flag which doesn't include the information of skill source, it's wrong.

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == CardUsed)
        {
            auto use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->isKindOf("Slash") && use.m_addHistory && player->getMark("@jianqiao") > 0)
            {
                room->addPlayerHistory(player, use.card->getClassName(), -1);
                use.m_addHistory = false;
                data = QVariant::fromValue(use);
                room->removePlayerMark(player, "@jianqiao", 1);
            }
        }
        else if (event == EventPhaseChanging)
        {
            if (player->getMark("@jianqiao") > 0)
            {
                room->setPlayerMark(player, "@jianqiao", 0);
            }
        }
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList list;

        if (event == EventPhaseStart)
        {
            if (player->getPhase() == Player::Play)
            {
                auto inoris = room->findPlayersBySkillName(objectName());

                foreach (auto inori, inoris)
                {
                    if (inori != player && !inori->isKongcheng())
                    {
                        list.insert(inori, QStringList(objectName()));
                    }
                }
            }
        }
        else if (event == CardsMoveOneTime)
        {
            auto move = data.value<CardsMoveOneTimeStruct>();
            if (move.card_ids.length() == 1 && move.from_places.contains(Player::PlaceEquip) && move.to_place == Player::PlaceTable
                && move.reason.m_reason == CardMoveReason::S_REASON_CHANGE_EQUIP && move.from != NULL && move.from->hasFlag("jianqiao_target"))
            {
                auto inoris = room->findPlayersBySkillName(objectName());

                foreach(auto inori, inoris)
                {
                    if (inori != player && !inori->isKongcheng())
                    {
                        list.insert(inori, QStringList(objectName()));
                    }
                }
            }
        }

        return list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == EventPhaseStart)
        {
            auto card = room->askForCard(ask_who, "Slash,EquipCard|.|.|hand", "@jianqiao-invoke", NULL, Card::MethodNone, player, false, objectName());
            if (card != NULL)
            {
                player->obtainCard(card, true);
                room->broadcastSkillInvoke(objectName(), 1, ask_who);
                return true;
            }
        }
        else if (event == CardsMoveOneTime)
        {
            auto move = data.value<CardsMoveOneTimeStruct>();
            int id = move.card_ids.first();
            if (room->getCardPlace(id) == Player::PlaceTable && ask_who->askForSkillInvoke(this))
            {
                room->broadcastSkillInvoke(objectName(), 2, ask_who);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == EventPhaseStart)
        {
            player->setFlags("jianqiao_target");
            room->addPlayerMark(player, "@jianqiao", 1);
        }
        else if (event == CardsMoveOneTime)
        {
            auto move = data.value<CardsMoveOneTimeStruct>();
            int id = move.card_ids.first();
            if (room->getCardPlace(id) == Player::PlaceTable)
            {
                auto equip = Sanguosha->getEngineCard(id);
                if (equip != NULL)
                {
                    ask_who->obtainCard(equip);
                    if (!ask_who->isProhibited(player, equip) && equip->isAvailable(ask_who))
                        room->useCard(CardUseStruct(equip, ask_who, ask_who));
                }
            }
        }
        return false;
    }
};

class Wange : public TriggerSkill
{
public:
    Wange() : TriggerSkill("wange")
    {
        events << AskForPeaches;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        auto dying = data.value<DyingStruct>();
        if (dying.who != player || dying.who->getHp() > 0)
            return QStringList();

        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (room->askForSkillInvoke(player, objectName()))
        {
            int toDrawNum = 0;
            foreach (auto p, room->getAlivePlayers())
            {
                if (player->isFriendWith(p) || player->willBeFriendWith(p))
                    toDrawNum++;
            }
            if (toDrawNum > 0)
            {
                room->drawCards(player, toDrawNum, objectName());
                room->broadcastSkillInvoke(objectName(), player);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        foreach (auto p, room->getOtherPlayers(player))
        {
            if (!player->isFriendWith(p))
                continue;

            QString prompt = "@wange:" + p->objectName();
            auto card = room->askForExchange(player, objectName(), 1, 1, prompt, "", ".|.|.|hand");
            if (!card.isEmpty())
            {
                DummyCard dummy(card);
                p->obtainCard(&dummy, false);
            }

            if (player->isKongcheng())
                break;
        }
        return false;
    }
};

RendeCard::RendeCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

void RendeCard::extraCost(Room *room, const CardUseStruct &card_use) const
{
    ServerPlayer *target = card_use.to.first();
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, card_use.from->objectName(), target->objectName(), "rende", QString());
    room->obtainCard(target, this, reason, false);
}

void RendeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende", new_value);

    if (old_value < 3 && new_value >= 3 && source->isWounded()) {
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

void MoesenPackage::addAnimationGenerals()
{
    General *madoka = new General(this, "madoka", "wei", 4, false); // A001
    madoka->addSkill(new Cibei);
    madoka->addSkill(new Renmin);

    General *mami = new General(this, "mami", "wei", 4, false); // A002
    mami->addSkill(new Lieqiang);
    mami->addSkill(new Molu);

    General *s_kyouko = new General(this, "s_kyouko", "wei", 4, false); // A003
    s_kyouko->addCompanion("sayaka");
    s_kyouko->addSkill(new Yingqiang);
    s_kyouko->addSkill(new YingqiangTargetMod);
    related_skills.insertMulti("yingqiang", "#yingqiang-target");
    skills << new YingqiangSpade
        << new YingqiangHeart
        << new YingqiangClub
        << new YingqiangDiamond;

    General *sayaka = new General(this, "sayaka", "wei", 4, false); // A004
    sayaka->addSkill(new Wuwei);

    General *homura = new General(this, "homura", "wei", 3, false);  // A005
    homura->addSkill(new Shiting);
    homura->addSkill(new Shizhi);

    General *n_azusa = new General(this, "n_azusa", "wei", 3, false); // A006
    n_azusa->addSkill(new Quanmian);
    n_azusa->addSkill(new Miaolv);

    General *mio = new General(this, "mio", "wei", 3, false); // A007
    mio->addSkill(new Yinzhuang);
    mio->addSkill(new YinzhuangWeapon);
    mio->addSkill(new YinzhuangArmor);
    mio->addSkill(new YinzhuangHorse);
    related_skills.insertMulti("yinzhuang", "#yinzhuang-weapon");
    related_skills.insertMulti("yinzhuang", "#yinzhuang-armor");
    related_skills.insertMulti("yinzhuang", "#yinzhuang-horse");
    mio->addSkill(new Xiuse);

    General *yui = new General(this, "yui", "wei", 4, false); // A008
    yui->addSkill(new Yingan);

    General *kanade = new General(this, "kanade", "wei", 4, false); // A009
    kanade->addSkill(new Yinren);
    kanade->addSkill(new Tongxin);

    General *rei = new General(this, "rei", "wei", 3, false); // A010
    rei->addSkill(new WuxinAya);
    rei->addSkill(new Chidun);
    rei->addRelateSkill("yongjue");

    General *asuka = new General(this, "asuka", "wei", 3, false); // A011
    asuka->addSkill(new Xiehang);
    skills << new XiehangAnother;
    asuka->addSkill(new XiehangResidue);
    related_skills.insertMulti("Xiehang", "XiehangAnother");
    related_skills.insertMulti("Xiehang", "#xiehang-residue");
    asuka->addSkill(new Powei);

    General *inori = new General(this, "inori", "wei", 3, false); // A012
    inori->addSkill(new Jianqiao);
    inori->addSkill(new Wange);

    General *n_maki = new General(this, "n_maki", "wei", 3, false); // A013
    n_maki->addSkill(new Qinyin);

    General *mayu = new General(this, "mayu", "wei", 3, false); // A014
    mayu->addSkill(new Pianxian);
    mayu->addSkill(new Liufei);

    General *lacus = new General(this, "lacus", "wei", 3, false); // A015
    lacus->addSkill(new Geji);
    lacus->addSkill(new Pinghe);

    General *sawa = new General(this, "sawa", "wei", 4, false); // A016
    sawa->addSkill(new Mashu("sawa"));
    sawa->addSkill(new Tengyue);
    sawa->addSkill(new TengyueTrigger);
    sawa->addSkill(new TengyueTargetMod);
    related_skills.insertMulti("tengyue", "#tengyue-trigger");
    related_skills.insertMulti("tengyue", "#tengyue-target");

    General *menma = new General(this, "menma", "wei", 3, false); // A017
    menma->addSkill(new Qiyuan);
    menma->addSkill(new Huaming);

    General *miho = new General(this, "miho", "wei", 3, false); // A018
    miho->addSkill(new Mogai);
    miho->addSkill(new Ruhun);

    skills << new Yongjue;

    addMetaObject<RendeCard>();
    addMetaObject<WuweiCard>();
    addMetaObject<MiaolvCard>();
    addMetaObject<QuanmianCard>();
    //addMetaObject<BajianCard>();
    addMetaObject<XiehangCard>();
    addMetaObject<XiehangUseCard>();
    addMetaObject<GejiCard>();
}
