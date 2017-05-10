#include "novel.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"

//weihao & zhuyi by SE
WeihaoCard::WeihaoCard()
{
    target_fixed = true;
}

void WeihaoCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    //broadcast
    source->drawCards(1);
    source->gainMark("@weihao", 1);
}

class Weihao : public ZeroCardViewAsSkill
{
public:
    Weihao() : ZeroCardViewAsSkill("weihao")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("azusa_maxcards") > 0;
    }

    virtual const Card *viewAs() const
    {
        WeihaoCard *card = new WeihaoCard;
        card->setShowSkill(objectName());
        return card;
    }
};

ZhuyiCard::ZhuyiCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool ZhuyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ZhuyiCard::onEffect(const CardEffectStruct &effect) const
{
    //broadcast
    effect.from->getRoom()->moveCardTo(this, effect.from, effect.to, Player::PlaceEquip, CardMoveReason(CardMoveReason::S_REASON_PUT, effect.from->objectName(), "zhuyi", QString()));

    LogMessage log;
    log.type = "$ZhuyiEquip";
    log.from = effect.to;
    log.card_str = QString::number(getEffectiveId());
    effect.from->getRoom()->sendLog(log);

    effect.from->gainMark("@zhenhao", 1);
}

class Zhuyi : public OneCardViewAsSkill
{
public:
    Zhuyi() : OneCardViewAsSkill("zhuyi")
    {
        filter_pattern = "EquipCard|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        ZhuyiCard *zhuyi = new ZhuyiCard();
        zhuyi->addSubcard(originalCard);
        zhuyi->setShowSkill(objectName());
        return zhuyi;
    }
};

class AzusaMaxCards : public MaxCardsSkill
{
public:
    AzusaMaxCards() : MaxCardsSkill("#azusa-maxcard")
    {
    }

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const
    {
        return target->getMark("@zhenhao") - target->getMark("@weihao");
    }
};

class AzusaTrigger : public TriggerSkill
{
public:
    AzusaTrigger() : TriggerSkill("#azusa-cardHandle")
    {
        events << NonTrigger << GameStart << TurnStart << EventPhaseStart << EventPhaseProceeding << EventPhaseEnd << EventPhaseChanging << EventPhaseSkipping 
            << DrawNCards << AfterDrawNCards << PreHpRecover << HpRecover << PreHpLost << HpChanged << MaxHpChanged << PostHpReduced << HpLost << TurnedOver
            << EventLoseSkill << EventAcquireSkill << StartJudge << AskForRetrial << FinishRetrial << FinishJudge << PindianVerifying << Pindian << ChoiceMade
            << ChainStateChanged << RemoveStateChanged << ConfirmDamage << Predamage << DamageForseen << DamageCaused << DamageInflicted << PreDamageDone 
            << DamageDone << Damage << Damaged << DamageComplete << Dying << QuitDying << AskForPeaches << AskForPeachesDone << Death << BuryVictim 
            << BeforeGameOverJudge << GameOverJudge << GameFinished << SlashEffected << SlashProceed << SlashHit << SlashMissed << JinkEffect << CardAsked 
            << CardResponded << BeforeCardsMove << CardsMoveOneTime << PreCardUsed << CardUsed << TargetChoosing << TargetConfirming << TargetChosen 
            << TargetConfirmed << CardEffect << CardEffected << CardEffectConfirmed << PostCardEffected << CardFinished << TrickCardCanceling 
            << StageChange << FetchDrawPileCard << GeneralShown << GeneralHidden << GeneralRemoved;
        // Add all Events to make sure mark set
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player != NULL && player->isAlive())
        {
            if (event == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::NotActive && (player->getMark("@zhenhao") > 0 || player->getMark("@weihao") > 0))
            {
                room->setPlayerMark(player, "@zhenhao", 0);
                room->setPlayerMark(player, "@weihao", 0);
            }
            room->setPlayerMark(player, "azusa_maxcards", player->getMaxCards(MaxCardsType::Normal));
        }
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const
    {
        return QStringList();
    }
};

HaoqiCard::HaoqiCard()
{
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
    m_skillName = "_haoqi";
}

bool HaoqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (to_select == Self)
        return false;
    return targets.isEmpty();
}

void HaoqiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(),
        targets.first()->objectName(), "haoqi", QString());
    room->moveCardTo(this, targets.first(), Player::PlaceHand, reason);
    QStringList choices;
    if (targets.first()->getHandcardNum() > 0)
        choices << "halfcards";
    if (!targets.first()->hasShownOneGeneral())
        choices << "showallgenerals";
    choices << "turnoverself";
    QString choice = room->askForChoice(targets.first(), objectName(), choices.join("+"));
    if (choice == "halfcards")
    {
        int returnNum = (targets.first()->getHandcardNum() + 1) / 2;
        auto cards = room->askForExchange(targets.first(), objectName(), returnNum, returnNum, "@haoqi-halfcards", "", ".|.|.|hand");
        DummyCard dummy(cards);
        source->obtainCard(&dummy, false);
    }
    else if (choice == "showallgenerals")
    {
        targets.first()->showGeneral();
        targets.first()->showGeneral(false);
    }
    else if (choice == "turnoverself")
    {
        targets.first()->turnOver();
    }

}

class Haoqi : public ViewAsSkill
{
public:
    Haoqi() : ViewAsSkill("haoqi")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getHandcardNum() > 1 && !player->hasUsed("HaoqiCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (to_select->isEquipped())
            return false;

        int length = Self->getHandcardNum() / 2;
        return selected.length() < length;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() != Self->getHandcardNum() / 2)
            return NULL;

        HaoqiCard *card = new HaoqiCard;
        card->addSubcards(cards);
        card->setShowSkill(objectName());
        return card;
    }
};

class Jinzhi : public PhaseChangeSkill
{
public:
    Jinzhi() : PhaseChangeSkill("jinzhi")
    {
        frequency = Frequent;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer * &) const
    {
        if (!PhaseChangeSkill::triggerable(player))
            return QStringList();

        if (player->getPhase() == Player::Draw)
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(objectName()))
        {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *source) const
    {
        Room *room = source->getRoom();

        JudgeStruct judge;
        judge.reason = objectName();
        judge.play_animation = false;
        judge.who = source;
        judge.time_consuming = true;

        int lastno = 0;
        do
        {
            if (judge.card != NULL)
                lastno = judge.card->getNumber();
            room->judge(judge);
        } while (judge.card != NULL && judge.card->getNumber() > lastno && source->askForSkillInvoke(objectName()));

        QList<int> card_list = VariantList2IntList(source->tag[objectName()].toList());
        source->tag.remove(objectName());
        QList<int> subcards;
        foreach (int id, card_list)
            if (room->getCardPlace(id) == Player::PlaceTable && !subcards.contains(id))
                subcards << id;
        if (subcards.length() != 0)
        {
            DummyCard dummy(subcards);
            source->obtainCard(&dummy);
        }
        return true;
    }
};

class JinzhiMove : public TriggerSkill
{
public:
    JinzhiMove() : TriggerSkill("#jinzhi-move")
    {
        events << FinishJudge;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (player != NULL)
        {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == "jinzhi")
            {
                QVariantList jinzhi_list = player->tag["jinzhi"].toList();
                jinzhi_list << judge->card->getEffectiveId();
                player->tag["jinzhi"] = jinzhi_list;

                if (room->getCardPlace(judge->card->getEffectiveId()) == Player::PlaceJudge)
                {
                    return QStringList(objectName());
                }
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *eru, QVariant &data, ServerPlayer *) const
    {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        CardMoveReason reason(CardMoveReason::S_REASON_JUDGEDONE, eru->objectName(), QString(), judge->reason);
        room->moveCardTo(judge->card, NULL, Player::PlaceTable, reason, true);

        return false;
    }
};

JisuiCard::JisuiCard()
{
}

bool JisuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    if (to_select == Self || (!to_select->hasShownOneGeneral()))
        return false;
    bool invoke = true;
    foreach (const Player *p, targets)
    {
        if (to_select->isFriendWith(p))
        {
            invoke = false;
            break;
        }
    }
    return invoke;
}

void JisuiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    targets << source;
    QList<int> card_ids = room->getNCards(targets.length());
    room->fillAG(card_ids);
    room->setTag("Jisui_Card", IntList2VariantList(card_ids));
    room->sortByActionOrder(targets);
    Card::use(room, source, targets);
    room->clearAG();
    QVariantList ag_list = room->getTag("Jisui_Card").toList();
    if (ag_list.isEmpty()) return;

    QList<int> card2_ids;
    foreach (QVariant card_id, ag_list)
        card2_ids << card_id.toInt();

    QList<int> guanxing_ids;

    do
    {
        room->fillAG(card2_ids);
        int card_id = room->askForAG(source, card2_ids, true, "jisui");
        if (card_id == -1)
        {
            room->clearAG(source);
            break;
        }

        card2_ids.removeOne(card_id);
        ag_list.removeOne(card_id);
        guanxing_ids << card_id;
        room->clearAG(source);
    } while (!card2_ids.isEmpty());

    if (guanxing_ids.length() > 0)
        room->askForGuanxing(source, guanxing_ids, Room::GuanxingUpOnly);

    if (card2_ids.length() > 0)
    {
        DummyCard dummy(VariantList2IntList(ag_list));
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "jisui", QString());
        room->throwCard(&dummy, reason, NULL);
    }
}

void JisuiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    QVariantList ag_list = room->getTag("Jisui_Card").toList();
    QList<int> card_ids;
    foreach (QVariant card_id, ag_list)
        card_ids << card_id.toInt();
    if (effect.to->getHandcardNum() > 0)
    {

        QList<CardsMoveStruct> exchangeMove;

        auto excard = room->askForExchange(effect.to, "jisui", 1, 0, "@jisui-exchange", "", ".");

        if (excard.length() > 0)
        {
            int card_id = room->askForAG(effect.to, card_ids, false, objectName());
            card_ids.removeOne(card_id);

            CardsMoveStruct move1(excard.first(), NULL, Player::PlaceTable,
                CardMoveReason(CardMoveReason::S_REASON_SWAP, effect.to->objectName(), NULL, "jisui", NULL));
            CardsMoveStruct move2(card_id, effect.to, Player::PlaceHand,
                CardMoveReason(CardMoveReason::S_REASON_SWAP, NULL, effect.to->objectName(), "jisui", NULL));
            exchangeMove.push_back(move1);
            exchangeMove.push_back(move2);
            room->moveCardsAtomic(exchangeMove, true);

            card_ids << excard.first();

            ag_list.removeOne(card_id);
            ag_list << excard.first();

            room->setTag("Jisui_Card", ag_list);
        }
    }
}

class Jisui : public ZeroCardViewAsSkill
{
public:
    Jisui() : ZeroCardViewAsSkill("jisui")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return (!player->hasUsed("JisuiCard"));
    }

    virtual const Card *viewAs() const
    {
        JisuiCard *card = new JisuiCard;
        card->setShowSkill(objectName());
        return card;
    }
};

//Jingdi by SE
JingdiCard::JingdiCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool JingdiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return targets.isEmpty() && to_select->getMark("aria_damaged") == 1;
}

void JingdiCard::onEffect(const CardEffectStruct &effect) const
{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, effect.from->objectName(), effect.to->objectName(), "jingdi", QString());
    effect.from->getRoom()->obtainCard(effect.to, this, reason, false);
    effect.from->drawCards(1);
}

class Jingdi : public OneCardViewAsSkill
{
public:
    Jingdi() : OneCardViewAsSkill("jingdi")
    {
        filter_pattern = ".|.|.|hand!";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return selected.isEmpty() && !to_select->isTransferable();
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return (!player->isKongcheng() && !player->hasUsed("JingdiCard"));
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        JingdiCard *jc = new JingdiCard;
        jc->addSubcard(originalCard->getId());
        jc->setShowSkill(objectName());
        return jc;
    }
};

class JingdiDamage : public TriggerSkill
{
public:
    JingdiDamage() : TriggerSkill("jingdi-damage")
    {
        events << Damaged;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer * &) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from != NULL && damage.from->isAlive())
        {
            if (damage.to->hasSkill("jingdi"))
            {
                room->setPlayerMark(damage.from, "aria_damaged", 1);
                return QStringList();
            }
        }
        return QStringList();
    }
};

//wujie by SE
class Wujie : public TriggerSkill
{
public:
    Wujie() : TriggerSkill("wujie")
    {
        events << DamageCaused << DamageInflicted << TurnStart;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == TurnStart)
        {
            foreach (ServerPlayer *p, room->getAlivePlayers())
            {
                p->loseAllMarks("@wujie_used");
            }
            return QStringList();
        }
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to->getHp() - damage.damage < 1 && damage.from->getMark("@wujie_used") == 0)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (player->hasShownSkill(this) && damage.from->askForSkillInvoke(objectName(), QVariant::fromValue(damage.to)))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        RecoverStruct recover;
        recover.who = damage.to;
        room->recover(damage.to, recover, true);
        damage.from->gainMark("@wujie_used");
        damage.from->insertPhase(Player::Play);
        return false;
    }
};

////huxiao
//class Huxiao : public TriggerSkill
//{
//public:
//    Huxiao() : TriggerSkill("huxiao")
//    {
//        events << CardsMoveOneTime;
//        frequency = NotFrequent;
//    }
//
//    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
//    {
//        if (!TriggerSkill::triggerable(player)) return QStringList();
//        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
//        QList<ServerPlayer *> list;
//        foreach (ServerPlayer *p, room->getAlivePlayers())
//        {
//            if (!p->getJudgingArea().isEmpty() || !p->getEquips().isEmpty())
//            {
//                list.append(p);
//            }
//        }
//        if (list.isEmpty())
//        {
//            return QStringList();
//        }
//        if (move.from == player && move.from_places.contains(Player::PlaceEquip))
//        {
//            return QStringList(objectName());
//        }
//        return QStringList();
//    }
//
//    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
//    {
//        if (player->askForSkillInvoke(objectName()))
//        {
//            room->broadcastSkillInvoke(objectName());
//            return true;
//        }
//
//        return false;
//    }
//
//    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
//        int n = 0;
//        foreach (Player::Place place, move.from_places)
//        {
//            if (place == Player::PlaceEquip)
//            {
//                n++;
//            }
//        }
//        for (int i = 0; i < n; i++)
//        {
//            QList<ServerPlayer *> list;
//            foreach (ServerPlayer *p, room->getAlivePlayers())
//            {
//                if (!p->getJudgingArea().isEmpty() || (p->getEquips().length() > 1 || (p->getEquips().length() == 1 && p->getTreasure() == NULL)))
//                {
//                    list.append(p);
//                }
//            }
//            if (list.isEmpty())
//            {
//                return false;
//            }
//
//            if (i > 0)
//            {
//                if (!player->askForSkillInvoke(objectName()))
//                {
//                    return false;
//                }
//            }
//
//            ServerPlayer *from = room->askForPlayerChosen(player, list, objectName(), "@huxiao_from");
//            if (from == NULL)
//            {
//                return false;
//            }
//
//            QList<int> cardids;
//            if (from->getWeapon())
//                cardids.append(from->getWeapon()->getEffectiveId());
//            if (from->getArmor())
//                cardids.append(from->getArmor()->getEffectiveId());
//            if (from->getDefensiveHorse())
//                cardids.append(from->getDefensiveHorse()->getEffectiveId());
//            if (from->getOffensiveHorse())
//                cardids.append(from->getOffensiveHorse()->getEffectiveId());
//            foreach (const Card *c, from->getJudgingArea())
//            {
//                cardids.append(c->getEffectiveId());
//            }
//            if (cardids.length() == 0)
//                return false;
//            room->fillAG(cardids, player);
//            int id = room->askForAG(player, cardids, true, objectName());
//            room->clearAG(player);
//            if (id == -1)
//            {
//                return false;
//            }
//
//            Card *card = Sanguosha->getCard(id);
//            Player::Place place = room->getCardPlace(id);
//
//            int type = -1;
//            if (place == Player::PlaceEquip)
//            {
//                if (card->isKindOf("Weapon"))
//                {
//                    type = 1;
//                }
//                else if (card->isKindOf("Armor"))
//                {
//                    type = 2;
//                }
//                else if (card->isKindOf("DefensiveHorse"))
//                {
//                    type = 3;
//                }
//                else if (card->isKindOf("OffensiveHorse"))
//                {
//                    type = 4;
//                }
//            }
//
//            QList<ServerPlayer *> tos;
//            list = room->getAlivePlayers();
//
//            foreach (ServerPlayer *p, list)
//            {
//                switch (type)
//                {
//                    case -1:
//                        if (!player->isProhibited(p, card) && !p->containsTrick(card->objectName()))
//                        {
//                            tos.append(p);
//                        }
//                        break;
//                    case 1:
//                        if (!p->getWeapon())
//                        {
//                            tos.append(p);
//                        }
//                        break;
//                    case 2:
//                        if (!p->getArmor())
//                        {
//                            tos.append(p);
//                        }
//                        break;
//                    case 3:
//                        if (!p->getDefensiveHorse())
//                        {
//                            tos.append(p);
//                        }
//                        break;
//                    case 4:
//                        if (!p->getOffensiveHorse())
//                        {
//                            tos.append(p);
//                        }
//                        break;
//                }
//            }
//            if (tos.isEmpty())
//            {
//                return false;
//            }
//
//            ServerPlayer *to = room->askForPlayerChosen(player, tos, objectName(), "@huxiao_to");
//            if (!to)
//                return false;
//            CardMoveReason reason = CardMoveReason(CardMoveReason::S_REASON_TRANSFER, player->objectName(), objectName(), QString());
//            room->moveCardTo(card, from, to, place, reason, false);
//        }
//        return false;
//    }
//};
//
////yexi
//class Yexi : public TriggerSkill
//{
//public:
//    Yexi() : TriggerSkill("yexi")
//    {
//        events << CardUsed;
//    }
//
//    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
//    {
//        CardUseStruct use = data.value<CardUseStruct>();
//        if (use.to.length() != 1 || !player->hasSkill(objectName()))
//            return QStringList();
//        const Card *card = use.card;
//        if (card->isKindOf("Slash") && card->isBlack() && use.from->objectName() == player->objectName() && use.to.length() == 1 && player->distanceTo(use.to.at(0)) == 1)
//        {
//            bool trigger = false;
//            if (use.to.at(0)->getWeapon() && !player->getWeapon())
//                trigger = true;
//            if (use.to.at(0)->getArmor() && !player->getArmor())
//                trigger = true;
//            if (use.to.at(0)->getDefensiveHorse() && !player->getDefensiveHorse())
//                trigger = true;
//            if (use.to.at(0)->getOffensiveHorse() && !player->getOffensiveHorse())
//                trigger = true;
//            if (trigger)
//            {
//                return QStringList(objectName());
//            }
//        }
//        return QStringList();
//    }
//
//    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
//    {
//        CardUseStruct use = data.value<CardUseStruct>();
//        if (player->askForSkillInvoke(objectName(), QVariant::fromValue(use.to.at(0))))
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
//        QList<int> cardids;
//        if (use.to.at(0)->getWeapon() && !player->getWeapon())
//            cardids.append(use.to.at(0)->getWeapon()->getEffectiveId());
//        if (use.to.at(0)->getArmor() && !player->getArmor())
//            cardids.append(use.to.at(0)->getArmor()->getEffectiveId());
//        if (use.to.at(0)->getDefensiveHorse() && !player->getDefensiveHorse())
//            cardids.append(use.to.at(0)->getDefensiveHorse()->getEffectiveId());
//        if (use.to.at(0)->getOffensiveHorse() && !player->getOffensiveHorse())
//            cardids.append(use.to.at(0)->getOffensiveHorse()->getEffectiveId());
//        if (cardids.length() == 0)
//            return false;
//        room->fillAG(cardids, player);
//        int id = room->askForAG(player, cardids, true, objectName());
//        room->clearAG(player);
//        if (id == -1)
//            return false;
//        CardMoveReason reason = CardMoveReason(CardMoveReason::S_REASON_TRANSFER, player->objectName(), objectName(), QString());
//        room->moveCardTo(Sanguosha->getCard(id), use.to.at(0), player, Player::PlaceEquip, reason);
//        return true;
//    }
//};

class Huxiao : public MasochismSkill
{
public:
    Huxiao() : MasochismSkill("huxiao")
    {
        frequency = Frequent;
        relate_to_place = "head";
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (TriggerSkill::triggerable(player))
        {
            DamageStruct damage = data.value<DamageStruct>();
            QStringList trigger_list;
            for (int i = 1; i <= damage.damage; i++)
            {
                trigger_list << objectName();
            }

            return trigger_list;
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *taiga, QVariant &data, ServerPlayer *) const
    {
        if (taiga->askForSkillInvoke(this, data))
        {
            room->broadcastSkillInvoke(objectName(), taiga);
            return true;
        }

        return false;
    }

    virtual void onDamaged(ServerPlayer *taiga, const DamageStruct &) const
    {
        taiga->setMaxHp(taiga->getMaxHp() + 1);
    }
};

class Yexi : public PhaseChangeSkill
{
public:
    Yexi() : PhaseChangeSkill("yexi")
    {
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *player) const
    {
        return TriggerSkill::triggerable(player) && player->getLostHp() > 0 && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this))
        {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }

        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *taiga) const
    {
        Room *room = taiga->getRoom();
        int x = taiga->getLostHp();
        QList<int> guanxing = room->getNCards(x);

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = taiga;
        log.card_str = IntList2StringList(guanxing).join("+");
        room->doNotify(taiga, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());

        room->askForGuanxing(taiga, guanxing, Room::GuanxingBothSides);

        int max = taiga->getMaxHp();
        int now = taiga->getHp();

        if (max != now) // use max != now here instead of x > 0, because there may be hp > maxhp situation (which is in AnimeMod)
        {
            QStringList choices;
            choices << "maxhptohp";
            if (taiga->canDiscard(taiga, "h") && taiga->getHandcardNum() >= x)
            {
                choices << "hptomaxhp";
            }

            choices << "cancel";

            auto choice = room->askForChoice(taiga, objectName(), choices.join("+"));

            if (choice == "maxhptohp")
            {
                taiga->setMaxHp(now);
                room->drawCards(taiga, x, objectName());
            }
            else if (choice == "hptomaxhp")
            {
                if (room->askForDiscard(taiga, objectName(), x, x, false, false, "@yexi-discard"))
                {
                    // use setHp because it's 'set hp' in description
                    taiga->setHp(max);
                }
            }
        }

        return false;
    }
};

//bianchi
BianchiCard::BianchiCard()
{
}

bool BianchiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && Self->inMyAttackRange(to_select) && Self->objectName() != to_select->objectName();
}

void BianchiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    damage.reason = "bianchiDamage";
    room->damage(damage);
    if (effect.to->isAlive())
    {
        effect.to->drawCards(effect.to->getLostHp());
    }
}

class BianchiVS : public ZeroCardViewAsSkill
{
public:
    BianchiVS() : ZeroCardViewAsSkill("bianchi")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("BianchiCard");
    }

    virtual const Card *viewAs() const
    {
        BianchiCard *bc = new BianchiCard;
        bc->setShowSkill(objectName());
        return bc;
    }
};

class Bianchi : public TriggerSkill
{
public:
    Bianchi() : TriggerSkill("bianchi")
    {
        events << QuitDying;
        view_as_skill = new BianchiVS;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer * &) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage && dying.damage->getReason() == "bianchiDamage")
        {
            ServerPlayer *p = dying.damage->from;
            if (p->isAlive())
            {
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        dying.damage->from->turnOver();
        return false;
    }
};

//Dushe
class DusheVS : public OneCardViewAsSkill
{
public:
    DusheVS() : OneCardViewAsSkill("dushe")
    {
        response_or_use = true;
    }

    virtual bool viewFilter(const Card *to_select) const
    {
        return to_select->isKindOf("Slash");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        Dismantlement *dismantlement = new Dismantlement(originalCard->getSuit(), originalCard->getNumber());
        dismantlement->addSubcard(originalCard->getId());
        dismantlement->setSkillName(objectName());
        dismantlement->setShowSkill(objectName());
        return dismantlement;
    }
};

class Dushe : public TriggerSkill
{
public:
    Dushe() : TriggerSkill("dushe")
    {
        events << PreCardUsed;
        view_as_skill = new DusheVS;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer * &) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Dismantlement") && use.card->getSkillName() == objectName())
        {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (player->askForSkillInvoke(objectName(), QVariant::fromValue(use.to)))
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        foreach (ServerPlayer *p, room->getAlivePlayers())
        {
            if (player->inMyAttackRange(p) && !use.to.contains(p) && !p->isNude())
            {
                use.to.append(p);
            }
        }
        data.setValue(use);
        return false;
    }
};

//shanguang
class Shanguang : public MasochismSkill
{
public:
    Shanguang() : MasochismSkill("shanguang")
    {
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *asuna, QVariant &data, ServerPlayer * &) const
    {
        if (MasochismSkill::triggerable(asuna))
        {
            ServerPlayer *from = data.value<DamageStruct>().from;
            return (from && !from->isKongcheng()) ? QStringList(objectName()) : QStringList();
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *asuna, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (!damage.from->isKongcheng() && asuna->askForSkillInvoke(this, data))
        {
            room->broadcastSkillInvoke(objectName(), asuna);
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, asuna->objectName(), damage.from->objectName());
            return true;
        }
        return false;
    }

    virtual void onDamaged(ServerPlayer *asuna, const DamageStruct &damage) const
    {
        Room *room = asuna->getRoom();
        int card_id = room->askForCardChosen(damage.from, damage.from, "h", objectName());
        QList<int> ids;
        ids.append(card_id);
        asuna->addToPile("shuo", ids);
    }
};

class ShanguangMaxCards : public MaxCardsSkill
{
public:
    ShanguangMaxCards() : MaxCardsSkill("#shanguang-maxcard")
    {
    }

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const
    {
        if (!target->hasShownSkill("shanguang"))
        {
            return 0;
        }
        return target->getPile("shuo").length();
    }
};

//zhuanyu
ZhuanyuCard::ZhuanyuCard()
{
    target_fixed = true;
}

void ZhuanyuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->fillAG(source->getPile("shuo"), source);
    int id = room->askForAG(source, source->getPile("shuo"), false, objectName());
    room->clearAG(source);
    if (id == -1)
    {
        return;
    }
    room->throwCard(Sanguosha->getCard(id), source, source, objectName());
    source->drawCards(source->getLostHp());
}

class Zhuanyu : public ZeroCardViewAsSkill
{
public:
    Zhuanyu() : ZeroCardViewAsSkill("zhuanyu")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ZhuanyuCard") && player->getPile("shuo").length() > 0;
    }

    virtual const Card *viewAs() const
    {
        ZhuanyuCard *zy = new ZhuanyuCard;
        zy->setShowSkill(objectName());
        return zy;
    }
};

//xianqun
XianqunCard::XianqunCard()
{
}

bool XianqunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && Self->objectName() != to_select->objectName() && to_select->getEquips().length() > 0;
}

void XianqunCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.at(0);
    int id = room->askForCardChosen(source, target, "e", objectName());
    if (id == -1)
    {
        return;
    }
    QList<int> ids;
    ids.append(id);
    target->addToPile("ruiko_qun", ids);
}

class XianqunVS : public ZeroCardViewAsSkill
{
public:
    XianqunVS() : ZeroCardViewAsSkill("xianqun")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("XianqunCard");
    }

    virtual const Card *viewAs() const
    {
        XianqunCard *xq = new XianqunCard;
        xq->setShowSkill("xianqun");
        return xq;
    }
};

class Xianqun : public TriggerSkill
{
public:
    Xianqun() : TriggerSkill("xianqun")
    {
        events << EventPhaseStart << Death << EventLoseSkill;
        view_as_skill = new XianqunVS;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (event == EventPhaseStart)
        {
            if (player != NULL && player->getPhase() == Player::Finish && player->getPile("ruiko_qun").length() > 0)
            {
                QList<ServerPlayer *> ruikos = room->findPlayersBySkillName(objectName());
                foreach (ServerPlayer *ruiko, ruikos)
                {
                    if (ruiko != NULL && player != ruiko)
                    {
                        if (player->getPile("ruiko_qun").length() == 0)
                        {
                            continue;
                        }
                        foreach (int id, player->getPile("ruiko_qun"))
                        {
                            if (Sanguosha->getCard(id)->isKindOf("Weapon"))
                            {
                                if (player->getWeapon())
                                {
                                    room->throwCard(player->getWeapon(), player, ruiko, objectName());
                                }
                            }
                            if (Sanguosha->getCard(id)->isKindOf("Armor"))
                            {
                                if (player->getArmor())
                                {
                                    room->throwCard(player->getArmor(), player, ruiko, objectName());
                                }
                            }
                            if (Sanguosha->getCard(id)->isKindOf("DefensiveHorse"))
                            {
                                if (player->getDefensiveHorse())
                                {
                                    room->throwCard(player->getDefensiveHorse(), player, ruiko, objectName());
                                }
                            }
                            if (Sanguosha->getCard(id)->isKindOf("OffensiveHorse"))
                            {
                                if (player->getOffensiveHorse())
                                {
                                    room->throwCard(player->getOffensiveHorse(), player, ruiko, objectName());
                                }
                            }
                            if (Sanguosha->getCard(id)->isKindOf("Treasure"))
                            {
                                if (player->getTreasure())
                                {
                                    room->throwCard(player->getTreasure(), player, ruiko, objectName());
                                }
                            }
                        }
                        CardsMoveStruct move;
                        move.from = player;
                        move.to = player;
                        move.to_place = Player::PlaceEquip;
                        move.from_pile_name = "ruiko_qun";
                        move.card_ids = player->getPile("ruiko_qun");
                        move.reason = CardMoveReason(CardMoveReason::S_REASON_PUT, ruiko->objectName(), objectName() + "Back", QString());
                        room->moveCards(move, true);
                        foreach (int id, move.card_ids)
                        {
                            LogMessage log;
                            log.type = "$Xianqun_back";
                            log.from = ruiko;
                            log.arg = objectName();
                            log.arg2 = player->objectName();
                            log.card_str = Sanguosha->getCard(id)->toString();
                            room->sendLog(log);
                        }
                    }

                }
            }
        }
        else
        {
            QList<ServerPlayer *> ruikos = room->findPlayersBySkillName(objectName());
            if (ruikos.length() == 0)
            {
                foreach (ServerPlayer *p, room->getAlivePlayers())
                {
                    if (p->getPile("ruiko_qun").length() > 0)
                    {
                        p->clearOnePrivatePile("ruiko_qun");
                    }
                }
            }
        }

        return skill_list;
    }
};

//fangzhu
class Fangzhu6 : public MasochismSkill
{
public:
    Fangzhu6() : MasochismSkill("fangzhu_rikka")
    {
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const
    {
        if (TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who).contains(objectName()))
        {
            DamageStruct damage = data.value<DamageStruct>();
            QStringList trigger_list;
            for (int i = 1; i <= damage.damage; i++)
            {
                trigger_list << objectName();
            }

            return trigger_list;
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        ServerPlayer *to = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "fangzhu_rikka-invoke", true, true);
        if (to != NULL)
        {
            room->broadcastSkillInvoke(objectName(), (to->faceUp() ? 1 : 2), player);
            player->tag["fangzhu_rikka_invoke"] = QVariant::fromValue(to);
            return true;
        }
        return false;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &) const
    {
        ServerPlayer *to = player->tag["fangzhu_rikka_invoke"].value<ServerPlayer *>();
        player->tag.remove("fangzhu_rikka_invoke");
        if (to)
        {
            if (player->isWounded())
                to->drawCards(player->getLostHp(), objectName());
            to->turnOver();
        }
    }
};

//xieyu
XieyuSummon::XieyuSummon()
    : ArraySummonCard("xieyu")
{
}

class Xieyu : public BattleArraySkill
{
public:
    Xieyu() : BattleArraySkill("xieyu", HegemonyMode::Formation)
    {
        events << EventPhaseStart << Death << EventLoseSkill << EventAcquireSkill
            << GeneralShown << GeneralHidden << GeneralRemoved << RemoveStateChanged;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (player == NULL) return QStringList();

        if (triggerEvent == EventPhaseStart)
        {
            if (player->getPhase() != Player::RoundStart)
                return QStringList();
        }
        else if (triggerEvent == Death)
        {
            DeathStruct death = data.value<DeathStruct>();
            if (player != death.who)
                return QStringList();
        }

        foreach(ServerPlayer *p, room->getPlayers())
        {
            if (p->getMark("xieyu_dis") > 0)
            {
                room->setPlayerMark(p, "xieyu_dis", 0);
                foreach(ServerPlayer *target, room->getAllPlayers())
                {
                    room->setFixedDistance(p, target, -1);
                }
            }
        }

        if (triggerEvent == EventLoseSkill && data.toString().split(":").first() == "xieyu")
            return QStringList();
        if (triggerEvent == GeneralHidden && player->ownSkill(this) && player->inHeadSkills(objectName()) == data.toBool())
            return QStringList();
        if (triggerEvent == GeneralRemoved && data.toString() == "rikka")
            return QStringList();
        if (player->aliveCount() < 4)
            return QStringList();

        QList<ServerPlayer *> rikkas = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *rikka, rikkas)
        {
            foreach(ServerPlayer *p, room->getAlivePlayers())
            {
                if (rikka->hasShownSkill(this) && rikka->inFormationRalation(p))
                {
                    room->doBattleArrayAnimate(rikka);
                    room->setPlayerMark(p ,"xieyu_dis", 1);
                    foreach(ServerPlayer *target, room->getOtherPlayers(p))
                    {
                        room->setFixedDistance(p, target, 1);
                    }
                }
            }
        }

        return QStringList();
    }
};

class XieyuTargetMod : public TargetModSkill
{
public:
    XieyuTargetMod() : TargetModSkill("#xieyu-target")
    {
        pattern = "SingleTargetTrick";
    }

    virtual int getExtraTargetNum(const Player *from, const Card *) const
    {
        if (from->getMark("xieyu_dis") == 1)
            return 1;
        else
            return 0;
    }
};

//Duran for Yukino
class Duran : public TriggerSkill
{
public:
    Duran() : TriggerSkill("duran")
    {
        events << EventPhaseStart;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Play)
            return QStringList();

        auto players = room->getOtherPlayers(player);
        QStringList kingdoms;
        foreach (ServerPlayer *p, players)
        {
            if (!p->hasShownOneGeneral()) continue;

            if (p->getRole() == "careerist")
                kingdoms << p->objectName();
            else if (!kingdoms.contains(p->getKingdom()))
            {
                kingdoms << p->getKingdom();
            }
        }

        if (kingdoms.contains(player->getKingdom()))
            kingdoms.removeAll(player->getKingdom());

        foreach (ServerPlayer *p, players)
        {
            if (!p->hasShownOneGeneral()) continue;

            if (p->inMyAttackRange(player))
            {
                if (p->getRole() == "careerist" && kingdoms.contains(p->objectName()))
                {
                    kingdoms.removeAll(p->objectName());
                }
                if (kingdoms.contains(p->getKingdom()))
                {
                    kingdoms.removeAll(p->getKingdom());
                }
            }
        }

        if (kingdoms.length() > 0)
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        bool invoke = player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
        if (invoke)
        {
            room->broadcastSkillInvoke("duran");
            if (player->hasShownSkill(this))
            {
                room->notifySkillInvoked(player, "duran");
            }
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        auto players = room->getOtherPlayers(player);
        QStringList kingdoms;
        foreach (ServerPlayer *p, players)
        {
            if (!p->hasShownOneGeneral()) continue;

            if (p->getRole() == "careerist")
                kingdoms << p->objectName();
            else if (!kingdoms.contains(p->getKingdom()))
            {
                kingdoms << p->getKingdom();
            }
        }

        if (player->getRole() == "careerist")
        {
            kingdoms.removeAll(player->objectName());
        }
        else if (kingdoms.contains(player->getKingdom()))
        {
            kingdoms.removeAll(player->getKingdom());
        }

        foreach (ServerPlayer *p, players)
        {
            if (!p->hasShownOneGeneral()) continue;

            if (p->inMyAttackRange(player))
            {
                if (p->getRole() == "careerist" && kingdoms.contains(p->objectName()))
                {
                    kingdoms.removeAll(p->objectName());
                }
                if (kingdoms.contains(p->getKingdom()))
                {
                    kingdoms.removeAll(p->getKingdom());
                }
            }
        }

        if (kingdoms.length() > 0)
        {
            LogMessage log;
            log.type = "#DuranCounter";
            log.from = player;
            log.arg = kingdoms.length();
            log.arg2 = objectName();
            room->sendLog(log);
            player->drawCards(kingdoms.length(), objectName());
        }

        return false;
    }

};

class DuranDis : public DistanceSkill
{
public:
    DuranDis() : DistanceSkill("#duran-dis")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const
    {
        if (from->getMaxHp() > to->getMaxHp())
        {
            if (from->hasShownSkill("duran"))
                return 1;
            if (to->hasShownSkill("duran"))
                return 1;
        }
        return 0;
    }
};

//Jieao for yukino
class Jieao : public TriggerSkill
{
public:
    Jieao() : TriggerSkill("jieao")
    {
        events << BuryVictim;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer * &ask_who) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if ((death.damage == NULL) || !TriggerSkill::triggerable(death.damage->from))
            return QStringList();
        ask_who = death.damage->from;
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        auto invoke = ask_who->hasShownSkill(this) ? true : room->askForSkillInvoke(ask_who, objectName());
        if (invoke)
        {
            room->broadcastSkillInvoke("jieao");
            if (ask_who->hasShownSkill(this))
            {
                room->notifySkillInvoked(ask_who, "jieao");
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = ask_who;
                log.arg = "jieao";
                room->sendLog(log);
            }
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        ask_who->setFlags("Jieao_flag");
        return false;
    }
};

//Duanzui for shana
class DuanzuiVS : public OneCardViewAsSkill
{
public:
    DuanzuiVS() : OneCardViewAsSkill("duanzui")
    {
        filter_pattern = "EquipCard";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasFlag("duanzui_used");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        auto *fs = new FireSlash(originalCard->getSuit(), originalCard->getNumber());
        fs->addSubcard(originalCard);
        fs->setSkillName(objectName());
        fs->setShowSkill(objectName());
        return fs;
    }
};

class Duanzui : public TriggerSkill
{
public:
    Duanzui() : TriggerSkill("duanzui")
    {
        view_as_skill = new DuanzuiVS;
        events << PreCardUsed << CardUsed << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        if (event == PreCardUsed)
        {
            auto use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->getSkillName() == "duanzui" && player->getPhase() == Player::Play)
            {
                room->addPlayerHistory(player, use.card->getClassName(), -1);
                room->setPlayerFlag(player, "duanzui_used");
                use.m_addHistory = false;
                data = QVariant::fromValue(use);
            }
        }
        else
        {
            auto change = data.value<PhaseChangeStruct>();
            if (change.from == Player::Play && player->hasFlag("duanzui_used"))
                player->setFlags("-duanzui_used");
        }
        return QStringList();
    }
};

class DuanzuiTM : public TargetModSkill
{
public:
    DuanzuiTM() : TargetModSkill("#duanzui-target")
    {
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const
    {
        if (from->hasShownSkill("duanzui") && card->getSkillName() == "duanzui")
            return 1;
        else
            return 0;
    }
};

//Honglian for shana
class Honglian : public TriggerSkill
{
public:
    Honglian() : TriggerSkill("honglian")
    {
        events << CardsMoveOneTime << Damaged;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        if (event == CardsMoveOneTime)
        {
            auto move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && move.from_places.contains(Player::PlaceEquip))
                return QStringList(objectName());
        }
        else
        {
            auto damage = data.value<DamageStruct>();
            if (damage.to != NULL && damage.to == player && damage.nature != DamageStruct::Normal)
                return QStringList(objectName());
        }
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->drawCards(player, 1, objectName());
        return false;
    }
};

//Dianji for mikoto
class Dianji : public TriggerSkill
{
public:
    Dianji() : TriggerSkill("dianji")
    {
        events << EventPhaseEnd << PreCardUsed;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(player))
            return QStringList();

        if (event == EventPhaseEnd)
        {
            if (!player->hasFlag("DianjiUsedSlash") && player->getPhase() == Player::Play)
            {
                return QStringList(objectName());
            }
            player->setFlags("-DianjiUsedSlash");
        }
        else if (event == PreCardUsed)
        {
            if (player->getPhase() == Player::Play)
            {
                const Card *card = data.value<CardUseStruct>().card;
                if (card != NULL && card->isKindOf("Slash") && !player->hasFlag("DianjiUsedSlash"))
                    player->setFlags("DianjiUsedSlash");
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (room->askForSkillInvoke(player, objectName()))
        {
            player->turnOver();
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "@dianji_target");
        if (target != NULL)
            room->damage(DamageStruct(objectName(), player, target, 1, DamageStruct::Thunder));
        return false;
    }
};

//Cichang for mikoto
class Cichang : public TriggerSkill
{
public:
    Cichang() : TriggerSkill("cichang")
    {
        events << TurnedOver;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *target, QVariant &) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;

        if (target == NULL || !target->isAlive())
            return skill_list;

        if (target->faceUp())
            return skill_list;

        QList<ServerPlayer *> mikotos = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *mikoto, mikotos)
        {
            if (mikoto->canDiscard(mikoto, "h"))
            {
                skill_list.insert(mikoto, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *target, QVariant &, ServerPlayer *ask_who) const
    {
        const Card *card = room->askForCard(ask_who, "BasicCard", "@cichang_discard", QVariant::fromValue(target), Card::MethodDiscard);
        if (card != NULL)
        {
            target->setFaceUp(true);
            room->broadcastProperty(target, "faceup");
            ask_who->tag["cichang-card"] = QVariant::fromValue(card);
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        const Card *card = ask_who->tag["cichang-card"].value<const Card *>();

        if (card->isKindOf("ThunderSlash") || card->isKindOf("FireSlash"))
        {
            QList<ServerPlayer *> candidates;
            foreach (ServerPlayer *candidate, room->getAlivePlayers())
            {
                if (candidate != NULL && !candidate->isChained() && candidate->canBeChainedBy(ask_who))
                    candidates << candidate;
            }

            if (candidates.length() <= 1)
                return false;

            QList<ServerPlayer *> victims = room->askForPlayersChosen(ask_who, candidates, objectName(), 2, 2, "@cichang_choosetarget", false);
            foreach (ServerPlayer *victim, victims)
            {
                room->setPlayerProperty(victim, "chained", true);
            }
        }
        else if (card->isKindOf("Peach") || card->isKindOf("Analeptic"))
        {
            ask_who->drawCards(2, objectName());
        }
        return false;
    }
};

//Xianli for Yui
XianliCard::XianliCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool XianliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return (targets.isEmpty() && !to_select->hasShownAllGenerals());
}

void XianliCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    QStringList choices;
    choices << "drawonecard";
    choices << "showonegeneral";
    KnownBoth *kb = new KnownBoth(Card::NoSuit, 0);
    kb->setSkillName("xianli");
    QList<const Player *> empty;
    if (kb->isAvailable(effect.from) && kb->targetFilter(empty, effect.to, effect.from) && !effect.from->isProhibited(effect.to, kb, empty))
        choices << "knownboth";
    delete kb;
    auto choice = room->askForChoice(effect.to, objectName(), choices.join("+"));

    if (choice == "drawonecard")
    {
        effect.from->drawCards(1, "xianli");
    }
    else if (choice == "showonegeneral")
    {
        effect.to->askForGeneralShow(true, false);
    }
    else if (choice == "knownboth")
    {
        KnownBoth *knownBoth = new KnownBoth(Card::NoSuit, 0);
        knownBoth->setSkillName("xianli");
        room->useCard(CardUseStruct(knownBoth, effect.from, effect.to), false);
    }
};

class Xianli : public ZeroCardViewAsSkill
{
public:
    Xianli() : ZeroCardViewAsSkill("xianli")
    {
        relate_to_place = "head";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("XianliCard");
    }

    virtual const Card *viewAs() const
    {
        XianliCard *card = new XianliCard();
        card->setShowSkill(objectName());
        return card;
    }
};

//Yuanxin for Yui
class Yuanxin : public TriggerSkill
{
public:
    Yuanxin() : TriggerSkill("yuanxin")
    {
        relate_to_place = "deputy";
        events << EventPhaseStart << HpRecover;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &) const
    {
        QMap<ServerPlayer *, QStringList> skill_list;

        if (player == NULL || !player->isAlive())
            return skill_list;

        if (event == HpRecover)
        {
            if (!player->hasSkill(objectName()))
                return skill_list;

            player->setFlags("yuanxin_turnrecover");
        }
        else
        {
            if (player->getPhase() == Player::Finish)
            {
                QList<ServerPlayer *> yuis = room->findPlayersBySkillName(objectName());
                foreach (ServerPlayer *yui, yuis)
                {
                    if (yui->hasFlag("yuanxin_turnrecover") && yui != player)
                        skill_list.insert(yui, QStringList(objectName()));
                }
            }
        }

        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        if (ask_who->askForSkillInvoke(objectName()))
        {
            room->broadcastSkillInvoke(objectName());
            room->loseHp(ask_who);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        ServerPlayer *luckyDog = room->askForPlayerChosen(ask_who, room->getOtherPlayers(ask_who), objectName(), "@yuanxin_choosetarget", false);
        if (luckyDog != NULL)
        {
            RecoverStruct recover;
            recover.recover = 1;
            recover.who = ask_who;
            room->recover(luckyDog, recover);
            ask_who->drawCards(2, objectName());
        }

        return false;
    }
};

//Qingyou for Hitagi
class Qingyou : public MasochismSkill
{
public:
    Qingyou() : MasochismSkill("qingyou")
    {
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (!TriggerSkill::triggerable(player))
        {
            return QStringList();
        }

        ServerPlayer *from = data.value<DamageStruct>().from;
        if (from != NULL)
        {
            return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        ServerPlayer *from = data.value<DamageStruct>().from;
        if (from != NULL && player->askForSkillInvoke(this, QVariant::fromValue(data)))
        {
            QList<ServerPlayer *> luckyDogs;
            if (player->hasShownSkill("zhongxie") && player->hasShownAllGenerals() && player->getMark("HasDedicatedMaxHp") == 1)
            {
                luckyDogs << player;
            }
            luckyDogs << from;
            room->drawCards(luckyDogs, 1, objectName());
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }

        return false;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const
    {
        Room *room = player->getRoom();
        ServerPlayer *from = damage.from;
        if (from == NULL) return;

        foreach (ServerPlayer *p, room->getAlivePlayers())
        {
            // one by one discard will reduce the amount of p, different from getting list first, due to design purpose 
            if (p->getHandcardNum() < from->getHandcardNum())
            {
                auto id = room->askForCardChosen(p, from, "he", objectName(), false, Card::MethodDiscard, QList<int>());
                if (id != -1)
                {
                    room->throwCard(id, from, p, objectName());
                }
            }
        }

        return;
    }
};

//Baoyan for Hitagi
BaoyanCard::BaoyanCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool BaoyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *self) const
{
    return targets.isEmpty() && to_select != self;
}

void BaoyanCard::onEffect(const CardEffectStruct &effect) const
{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, effect.from->objectName(), effect.to->objectName(), "baoyan", QString());
    effect.from->getRoom()->obtainCard(effect.to, this, reason, true);
    effect.from->drawCards(1, "baoyan");
}

class Baoyan : public OneCardViewAsSkill
{
public:
    Baoyan() : OneCardViewAsSkill("baoyan")
    {
        filter_pattern = ".|.|.|hand!";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return selected.isEmpty() && to_select->isKindOf("Slash");
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        if (player->isKongcheng())
            return false;

        if (player->hasShownSkill("zhongxie") && player->hasShownAllGenerals() && player->getMark("HasDedicatedMaxHp") == 0)
            return true;

        return !player->hasUsed("BaoyanCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        BaoyanCard *by = new BaoyanCard;
        by->addSubcard(originalCard->getId());
        by->setShowSkill(objectName());
        return by;
    }
};

//Zhongxie for Hitagi (Highly Coupling)
class Zhongxie : public TriggerSkill
{
public:
    Zhongxie() : TriggerSkill("zhongxie")
    {
        frequency = Compulsory;
        events << NonTrigger;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const
    {
        return QStringList();
    }
};

//Tiaoting for Watashi (Not me but watashi :D)
TiaotingCard::TiaotingCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool TiaotingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *self) const
{
    if (to_select->hasShownOneGeneral() && to_select != self)
    {
        return (targets.isEmpty() || (targets.length() == 1 && !to_select->isFriendWith(targets.first())));
    }

    return false;
}

bool TiaotingCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
    return targets.length() == 2;
}

void TiaotingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *axis = targets.at(0);
    ServerPlayer *allies = targets.at(1);

    auto armistice = room->getTag("tiaoting").value<QMap<ServerPlayer *, QMap<ServerPlayer *, bool>>>();
    QMap<ServerPlayer *, bool> belligerent;
    belligerent.insert(axis, false);
    belligerent.insert(allies, false);
    armistice[source] = belligerent;
    room->setTag("tiaoting", QVariant::fromValue(armistice));
}

class TiaotingVS : public ZeroCardViewAsSkill
{
public:
    TiaotingVS() : ZeroCardViewAsSkill("tiaoting")
    {
        response_pattern = "@@tiaoting";
    }

    virtual const Card *viewAs() const
    {
        TiaotingCard *tt = new TiaotingCard;
        tt->setShowSkill(objectName());
        return tt;
    }
};

class Tiaoting : public TriggerSkill
{
public:
    Tiaoting() : TriggerSkill("tiaoting")
    {
        events << EventPhaseStart << TurnStart << DamageCaused;
        view_as_skill = new TiaotingVS;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const
    {
        if (event == EventPhaseStart)
        {
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Finish)
            {
                QStringList kingdoms;
                foreach (ServerPlayer *p, room->getOtherPlayers(player))
                {
                    if (p->hasShownOneGeneral())
                    {
                        if (!kingdoms.contains(p->getRole()))
                        {
                            kingdoms << p->getRole();
                        }

                        if (kingdoms.length() > 1)
                        {
                            return QStringList(objectName());
                        }
                    }
                }
            }
        }
        else if (event == TurnStart)
        {
            if (TriggerSkill::triggerable(player))
            {
                auto armistice = room->getTag("tiaoting").value<QMap<ServerPlayer *, QMap<ServerPlayer *, bool>>>();
                armistice[player].clear();
                room->setTag("tiaoting", QVariant::fromValue(armistice));
            }
        }
        else
        {
            auto damage = data.value<DamageStruct>();
            if (damage.from != NULL && damage.to != NULL && damage.damage > 0)
            {
                auto armistice = room->getTag("tiaoting").value<QMap<ServerPlayer *, QMap<ServerPlayer *, bool>>>();
                foreach (ServerPlayer *p, room->getOtherPlayers(damage.from))
                {
                    if (!armistice.value(p).isEmpty())
                    {
                        auto belligerent = armistice.value(p);
                        if (belligerent.contains(damage.from) && belligerent.contains(damage.to) && !belligerent.value(damage.from))
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

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const
    {
        if (event == EventPhaseStart)
        {
            if (room->askForUseCard(player, "@@tiaoting", "@tiaoting-targetchoose") != NULL)
            {
                room->broadcastSkillInvoke(objectName(), 1, player);
                return true;
            }
        }
        else
        {
            room->broadcastSkillInvoke(objectName(), 2, ask_who);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == EventPhaseStart)
        {
            return false;
        }
        else
        {
            ask_who->setFlags("tiaoting_used");
            auto damage = data.value<DamageStruct>();
            auto armistice = room->getTag("tiaoting").value<QMap<ServerPlayer *, QMap<ServerPlayer *, bool>>>();
            (armistice[ask_who])[damage.from] = true;
            room->setTag("tiaoting", QVariant::fromValue(armistice));
            QList<ServerPlayer *> belligerent;
            if (damage.from->isAlive())
            {
                belligerent << damage.from;
            }
            if (damage.to->isAlive())
            {
                belligerent << damage.to;
            }
            if (belligerent.length() > 0)
            {
                room->drawCards(belligerent, 1, objectName());
            }
            return true;
        }

        return false;
    }
};

//Jilu for watashi
class Jilu : public TriggerSkill
{
public:
    Jilu() : TriggerSkill("jilu")
    {
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &ask_who) const
    {
        if (player == NULL || !player->isAlive())
        {
            return QStringList();
        }

        if (player->getPhase() == Player::Finish)
        {
            auto armistice = room->getTag("tiaoting").value<QMap<ServerPlayer *, QMap<ServerPlayer *, bool>>>();
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
            {
                if (!armistice.value(p).isEmpty())
                {
                    auto belligerent = armistice.value(p);
                    if (belligerent.contains(player) && !belligerent.value(player))
                    {
                        if (!p->hasFlag("tiaoting_used"))
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

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        if (room->askForSkillInvoke(ask_who, objectName()))
        {
            room->broadcastSkillInvoke(objectName(), ask_who);
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const
    {
        ask_who->drawCards(1, objectName());
        return false;
    }
};

//Zhizun for Haruhi (Coupling in getBigKingdoms)
class Zhizun : public TriggerSkill
{
public:
    Zhizun() : TriggerSkill("zhizun")
    {
        events << NonTrigger;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const
    {
        return QStringList();
    }
};

#include "strategic-advantage.h"

//Gexin for Haruhi
class GexinVS : public OneCardViewAsSkill
{
public:
    GexinVS() : OneCardViewAsSkill("gexin")
    {
        response_or_use = true;
    }

    virtual bool viewFilter(const Card *to_select) const
    {
        return to_select->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("gexin_lastplayer") == 0;
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        auto *te = new ThreatenEmperor(originalCard->getSuit(), originalCard->getNumber());
        te->addSubcard(originalCard);
        te->setSkillName("gexin");
        te->setShowSkill("gexin");
        return te;
    }
};

class Gexin : public TriggerSkill
{
public:
    Gexin() : TriggerSkill("gexin")
    {
        events << EventPhaseChanging;
        view_as_skill = new GexinVS;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        auto change = data.value<PhaseChangeStruct>();
        auto current = room->getCurrent();

        if (current != NULL && player != NULL && player == current && change.to == Player::NotActive)
        {
            auto haruhis = room->findPlayersBySkillName(objectName());

            foreach(auto haruhi, haruhis)
            {
                if (haruhi == current)
                {
                    room->setPlayerMark(haruhi, "gexin_lastplayer", 1);
                }
                else
                {
                    room->setPlayerMark(haruhi, "gexin_lastplayer", 0);
                }
            }
        }

        return QStringList();
    }
};

//Zixun for Yuki
class Zixun : public TriggerSkill
{
public:
    Zixun() : TriggerSkill("zixun")
    {
        frequency = Compulsory;
        events << DrawNCards << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (!TriggerSkill::triggerable(player))
        {
            return QStringList();
        }

        if (event == EventPhaseChanging)
        {
            auto change = data.value<PhaseChangeStruct>();
            if (change.from == Player::NotActive && player->hasShownSkill(this))
            {
                player->setMark("@zixun", player->getMark("@zixun") + 1);
            }
        }
        else
        {
            return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        bool invoke = player->hasShownSkill(this) ? true : player->askForSkillInvoke(objectName());
        if (invoke)
        {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        data = qMin(player->getMark("@zixun"), 3);
        return false;
    }
};

//Tonghe for Haruhi
TongheCard::TongheCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool TongheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *self) const
{
    if (targets.isEmpty())
    {
        return to_select->distanceTo(self) == 1 && to_select->canDiscard(to_select, "he");
    }

    return false;
}

void TongheCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    if (targets[0] == NULL)
        return;

    foreach (auto p, room->getAlivePlayers())
    {
        if (targets[0] != p && p->isFriendWith(targets[0]) && p->canDiscard(p, "he"))
        {
            targets << p;
        }
    }

    foreach(auto *p, targets)
    {
        room->askForDiscard(p, "tonghe", 1, 1, false, true, "@tonghe_discard", true);
    }

    auto big_kingdoms = source->getBigKingdoms("tonghe", MaxCardsType::Normal);

    if ((targets[0]->getRole() == "careerist" && big_kingdoms.contains(targets[0]->objectName()))
        || big_kingdoms.contains(targets[0]->getKingdom()))
    {
        room->drawCards(targets, 1, "tonghe");
    }
}

class Tonghe : public ZeroCardViewAsSkill
{
public:
    Tonghe() : ZeroCardViewAsSkill("tonghe")
    {
        relate_to_place = "deputy";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("TongheCard");
    }

    virtual const Card *viewAs() const
    {
        auto *th = new TongheCard;
        th->setShowSkill(objectName());
        return th;
    }
};

void MoesenPackage::addNovelGenerals()
{

    General *taiga = new General(this, "taiga", "qun", 4, false); // N001
    taiga->addSkill(new Huxiao);
    taiga->addSkill(new Yexi);
    taiga->setHeadMaxHpAdjustedValue(-1);

    General *shana = new General(this, "shana", "qun", 4, false); // N002
    shana->addSkill(new Duanzui);
    shana->addSkill(new DuanzuiTM);
    shana->addSkill(new Honglian);
    insertRelatedSkills("duanzui", "#duanzui-target");

    General *louise = new General(this, "louise", "qun", 3, false); // N003
    louise->addSkill(new Bianchi);

    General *aria = new General(this, "aria", "qun", 3, false); // N004
    aria->addSkill(new Jingdi);
    aria->addSkill(new Wujie);
    skills << new JingdiDamage;

    General *holo = new General(this, "holo", "qun", 4, false); // N005
    holo->addSkill(new Jisui);

    General *eru = new General(this, "eru", "qun", 3, false); // N006
    eru->addSkill(new Haoqi);
    eru->addSkill(new Jinzhi);
    eru->addSkill(new JinzhiMove);
    insertRelatedSkills("jinzhi", "#jinzhi-move");

    General *ruri = new General(this, "ruri", "qun", 4, false); // N007
    ruri->addSkill(new Dushe);

    General *a_azusa = new General(this, "a_azusa", "qun", 3, false); // N008
    a_azusa->addSkill(new Weihao);
    a_azusa->addSkill(new Zhuyi);
    a_azusa->addSkill(new AzusaMaxCards);
    a_azusa->addSkill(new AzusaTrigger);
    insertRelatedSkills("weihao", "#azusa-maxcard");
    insertRelatedSkills("weihao", "#azusa-cardHandle");
    insertRelatedSkills("zhuyi", "#azusa-maxcard");

    General *rikka = new General(this, "rikka", "qun", 3, false); // N009
    rikka->addSkill(new Fangzhu6);
    rikka->addSkill(new Xieyu);
    rikka->addSkill(new XieyuTargetMod);
    insertRelatedSkills("xieyu", "#xieyu-target");

    General *yukino = new General(this, "yukino", "qun", 3, false); // N010
    yukino->addSkill(new Duran);
    yukino->addSkill(new DuranDis);
    yukino->addSkill(new Jieao);
    insertRelatedSkills("duran", "#duran-dis");

    General *y_yui = new General(this, "y_yui", "qun", 4, false); // N011
    y_yui->addSkill(new Xianli);
    y_yui->setHeadMaxHpAdjustedValue(1);
    y_yui->addSkill(new Yuanxin);

    General *mikoto = new General(this, "mikoto", "qun", 3, false); // N012
    mikoto->addSkill(new Dianji);
    mikoto->addSkill(new Cichang);

    General *ruiko = new General(this, "ruiko", "qun", 4, false); // N013
    ruiko->addSkill(new Xianqun);

    General *asuna = new General(this, "asuna", "qun", 3, false); // N014
    asuna->addSkill(new Shanguang);
    asuna->addSkill(new ShanguangMaxCards);
    insertRelatedSkills("shanguang", "#shanguang-maxcard");
    asuna->addSkill(new Zhuanyu);

    General *hitagi = new General(this, "hitagi", "qun", 3, false); // N015
    hitagi->addSkill(new Baoyan);
    hitagi->addSkill(new Qingyou);
    hitagi->addSkill(new Zhongxie);

    General *watashi = new General(this, "watashi", "qun", 3, false); // N016
    watashi->addSkill(new Tiaoting);
    watashi->addSkill(new Jilu);

    General *haruhi = new General(this, "haruhi", "qun", 3, false); // N017
    haruhi->addSkill(new Zhizun);
    haruhi->addSkill(new Gexin);

    General *yuki = new General(this, "yuki", "qun", 4, false); // N018
    yuki->addSkill(new Zixun);
    yuki->addSkill(new Tonghe);
    yuki->setDeputyMaxHpAdjustedValue(-1);

    addMetaObject<WeihaoCard>();
    addMetaObject<ZhuyiCard>();
    addMetaObject<HaoqiCard>();
    addMetaObject<JisuiCard>();
    addMetaObject<JingdiCard>();
    addMetaObject<BianchiCard>();
    addMetaObject<ZhuanyuCard>();
    addMetaObject<XianqunCard>();
    addMetaObject<XieyuSummon>();
    addMetaObject<XianliCard>();
    addMetaObject<BaoyanCard>();
    addMetaObject<TiaotingCard>();
    addMetaObject<TongheCard>();
}
