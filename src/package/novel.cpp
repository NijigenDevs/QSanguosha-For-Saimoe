#include "novel.h"
#include "skill.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "engine.h"
#include "client.h"

//weihao & zhuyi by SE
WeihaoCard::WeihaoCard() {
    target_fixed = true;
}

void WeihaoCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
     //broadcast
    source->drawCards(1);
    if (source->getMark("@zhenhao") == 0)
        source->gainMark("@weihao", 1);
    else
        source->loseMark("@zhenhao");
}

class Weihao: public ZeroCardViewAsSkill {
public:
    Weihao(): ZeroCardViewAsSkill("weihao") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMaxHp();//need Changed! should be getMaxCards but it does not exist.
    }

    virtual const Card *viewAs() const{
        WeihaoCard *card = new WeihaoCard;
        card->setShowSkill(objectName());
        return card;
    }
};

ZhuyiCard::ZhuyiCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool ZhuyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ZhuyiCard::onEffect(const CardEffectStruct &effect) const{
     //broadcast
    effect.from->getRoom()->useCard(CardUseStruct(this,effect.to,effect.to,false));
    if (effect.from->getMark("@weihao") == 0)
        effect.from->gainMark("@zhenhao", 1);
    else
        effect.from->loseMark("@weihao");
}

class Zhuyi: public OneCardViewAsSkill {
public:
    Zhuyi(): OneCardViewAsSkill("zhuyi") {
        filter_pattern = "EquipCard|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *) const {
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        ZhuyiCard *zhuyi = new ZhuyiCard();
        zhuyi->addSubcard(originalCard);
        zhuyi->setShowSkill(objectName());
        return zhuyi;
    }
};

class AzusaMaxCards: public MaxCardsSkill {
public:
    AzusaMaxCards(): MaxCardsSkill("#azusa-maxcard") {
    }

    virtual int getExtra(const Player *target) const{
        return target->getMark("@zhenhao") - target->getMark("@weihao");
    }
};

class AzusaTrigger: public TriggerSkill {
public:
    AzusaTrigger(): TriggerSkill("#azusa-cardHandle") {
        events << EventPhaseEnd;
    }

    virtual QStringList triggerable(TriggerEvent , Room *room, ServerPlayer *player, QVariant &, ServerPlayer * &) const {
        if (player->getPhase() == Player::Finish){
            room->setPlayerMark(player, "@weihao", 0);
            room->setPlayerMark(player, "@zhenhao", 0);
        }
        return QStringList();
    }
};

HaoqiCard::HaoqiCard() {
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
    m_skillName = "_haoqi";
}

bool HaoqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (to_select == Self)
        return false;
    return targets.isEmpty();
}

void HaoqiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(),
        targets.first()->objectName(), "haoqi", QString());
    room->moveCardTo(this, targets.first(), Player::PlaceHand, reason);
    QStringList choices;
    if (targets.first()->getHandcardNum() > 0)
        choices << "halfcards";
    if (!targets.first()->hasShownOneGeneral())
        choices << "showallgenerals";
    choices << "turnoverself";
    QString choice = room->askForChoice(targets.first(),objectName(),choices.join("+"));
    if (choice == "halfcards"){
        const Card *cards = room->askForExchange(targets.first(), objectName(), ( (targets.first()->getHandcardNum() + 1) / 2) );
        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, targets.first()->objectName(),
        source->objectName(), "haoqi", QString());
        room->moveCardTo(cards, source, Player::PlaceHand, reason);
    } else if (choice == "showallgenerals"){
        targets.first()->showGeneral();
        targets.first()->showGeneral(false);
    } else if (choice == "turnoverself") {
        targets.first()->turnOver();
    }

}

class Haoqi : public ViewAsSkill {
public:
    Haoqi() : ViewAsSkill("haoqi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() > 1;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (to_select->isEquipped())
            return false;

        int length = Self->getHandcardNum() / 2;
        return selected.length() < length;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != Self->getHandcardNum() / 2)
            return NULL;

        HaoqiCard *card = new HaoqiCard;
        card->addSubcards(cards);
        card->setShowSkill(objectName());
        return card;
    }
};

class Jinzhi : public PhaseChangeSkill {
public:
    Jinzhi() : PhaseChangeSkill("jinzhi") {
        frequency = Frequent;
    }

    virtual bool canPreshow() const{
        return true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer * &) const{
        if (!PhaseChangeSkill::triggerable(player))
            return QStringList();

        if (player->getPhase() == Player::Draw)
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *source) const{
        Room *room = source->getRoom();

        JudgeStruct judge;
        judge.reason = objectName();
        judge.play_animation = false;
        judge.who = source;
        judge.time_consuming = true;

        int lastno = 0 ;
        do {
            if (judge.card)
                lastno = judge.card->getNumber();
            room->judge(judge);
        } while (judge.card->getNumber() > lastno  && source->askForSkillInvoke(objectName()));

        QList<int> card_list = VariantList2IntList(source->tag[objectName()].toList());
        source->tag.remove(objectName());
        QList<int> subcards;
        foreach(int id, card_list)
            if (room->getCardPlace(id) == Player::PlaceTable && !subcards.contains(id))
                subcards << id;
        if (subcards.length() != 0){
            DummyCard dummy(subcards);
            source->obtainCard(&dummy);
        }
        return true;
    }
};

class JinzhiMove : public TriggerSkill {
public:
    JinzhiMove() : TriggerSkill("#jinzhi-move") {
        events << FinishJudge;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (player != NULL) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == "jinzhi") {
                QVariantList jinzhi_list = player->tag["jinzhi"].toList();
                jinzhi_list << judge->card->getEffectiveId();
                player->tag["jinzhi"] = jinzhi_list;

                if (room->getCardPlace(judge->card->getEffectiveId()) == Player::PlaceJudge) {
                    return QStringList(objectName());
                }
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *eru, QVariant &data, ServerPlayer *) const{
        JudgeStruct *judge = data.value<JudgeStruct *>();
        CardMoveReason reason(CardMoveReason::S_REASON_JUDGEDONE, eru->objectName(), QString(), judge->reason);
        room->moveCardTo(judge->card, NULL, Player::PlaceTable, reason, true);

        return false;
    }
};

JisuiCard::JisuiCard() {
}

bool JisuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if (to_select == Self || (!to_select->hasShownOneGeneral())) 
        return false;
    bool invoke = true;
    foreach (const Player *p , targets){
        if (to_select->isFriendWith(p)){
            invoke = false;
            break;            
        }
    }
    return invoke;
}

void JisuiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
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
    foreach(QVariant card_id, ag_list)
        card2_ids << card_id.toInt();

    QList<int> guanxing_ids;

    do {
        room->fillAG(card2_ids);
        int card_id = room->askForAG(source, card2_ids, true, "jisui");
        if (card_id == -1) {
            room->clearAG(source);
            break;
        }

        card2_ids.removeOne(card_id);
        ag_list.removeOne(card_id);
        guanxing_ids << card_id;
        room->clearAG(source);
    } while (!card2_ids.isEmpty());

    if (guanxing_ids.length() > 0 )
        room->askForGuanxing(source, guanxing_ids , Room::GuanxingUpOnly);

    if (card2_ids.length() > 0 ) {
        DummyCard dummy(VariantList2IntList(ag_list));
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "jisui", QString());
        room->throwCard(&dummy, reason , NULL);
    }
}

void JisuiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QVariantList ag_list = room->getTag("Jisui_Card").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, ag_list)
        card_ids << card_id.toInt();
    if (effect.to->getHandcardNum() > 0 ){

        QList<CardsMoveStruct> exchangeMove;

        const Card *excard = room->askForExchange(effect.to,"jisui",1,false,NULL,true);

        if (excard){
            int card_id = room->askForAG(effect.to, card_ids, false, objectName());
            card_ids.removeOne(card_id);

            CardsMoveStruct move1( excard->getEffectiveId() ,  NULL , Player::PlaceTable ,
                CardMoveReason(CardMoveReason::S_REASON_SWAP, effect.to->objectName(), NULL , "jisui" , NULL));
            CardsMoveStruct move2( card_id , effect.to , Player::PlaceHand ,
                CardMoveReason(CardMoveReason::S_REASON_SWAP, NULL , effect.to->objectName(), "jisui" , NULL));
            exchangeMove.push_back(move1);
            exchangeMove.push_back(move2);
            room->moveCardsAtomic(exchangeMove, true);

            card_ids << excard->getEffectiveId();

            ag_list.removeOne(card_id);
            ag_list << excard->getEffectiveId();

            room->setTag("Jisui_Card", ag_list);
        }
    }
}

class Jisui: public ZeroCardViewAsSkill {
public:
    Jisui(): ZeroCardViewAsSkill("jisui") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (!player->hasUsed("JisuiCard"));
    }

    virtual const Card *viewAs() const{
        JisuiCard *card = new JisuiCard;
        card->setShowSkill(objectName());
        return card;
    }
};

//Jingdi by SE
JingdiCard::JingdiCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool JingdiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    return targets.isEmpty() && to_select->getMark("aria_damaged") == 1;
}

void JingdiCard::onEffect(const CardEffectStruct &effect) const{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, effect.from->objectName(), effect.to->objectName(), "jingdi", QString());
    effect.from->getRoom()->obtainCard(effect.to, this, reason, false);
    effect.from->drawCards(1);
}

class Jingdi : public OneCardViewAsSkill{
public:
    Jingdi() : OneCardViewAsSkill("jingdi"){
        filter_pattern = ".|.|.|hand!";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.isEmpty() && !to_select->isTransferable();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (!player->isKongcheng() && !player->hasUsed("JingdiCard"));
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        JingdiCard *jc = new JingdiCard;
        jc->addSubcard(originalCard->getId());
        jc->setShowSkill(objectName());
        return jc;
    }
};

class JingdiDamage : public TriggerSkill {
public:
    JingdiDamage() : TriggerSkill("jingdi-damage") {
        events << Damaged;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent , Room *, ServerPlayer *, QVariant &data, ServerPlayer * &) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from->isAlive()){
            if (damage.to->hasSkill("jingdi")){
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        room->setPlayerMark(damage.from, "aria_damaged", 1);
        return false;
    }
};

//wujie by SE
class Wujie : public TriggerSkill {
public:
    Wujie() : TriggerSkill("wujie") {
        events << DamageCaused << DamageInflicted << TurnStart;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        if (event == TurnStart){
            foreach (ServerPlayer *p, room->getAlivePlayers()){
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

    virtual bool cost(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (player->hasShownSkill(this) && damage.from->askForSkillInvoke(objectName(), QVariant::fromValue(damage.to))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        RecoverStruct recover;
        recover.who = damage.to;
        room->recover(damage.to, recover, true);
        damage.from->gainMark("@wujie_used");
        damage.from->insertPhase(Player::Play);
        return false;
    }
};

//huxiao
class Huxiao : public TriggerSkill {
public:
    Huxiao() : TriggerSkill("huxiao") {
        events << CardsMoveOneTime;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        QList<ServerPlayer *> list;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if (!p->getJudgingArea().isEmpty() || !p->getEquips().isEmpty()){
                list.append(p);
            }
        }
        if (list.isEmpty()){
            return QStringList();
        }
        if (move.from == player && move.from_places.contains(Player::PlaceEquip)) {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        int n = 0;
        foreach(Player::Place place, move.from_places){
            if (place == Player::PlaceEquip){
                n++;
            }
        }
        for (int i = 0; i < n; i++){
            QList<ServerPlayer *> list;
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if (!p->getJudgingArea().isEmpty() || (p->getEquips().length() > 1 || (p->getEquips().length() == 1 && p->getTreasure() == NULL))){
                    list.append(p);
                }
            }
            if (list.isEmpty()){
                return false;
            }

            if (i > 0){
                if (!player->askForSkillInvoke(objectName())){
                    return false;
                }
            }

            ServerPlayer *from = room->askForPlayerChosen(player, list, objectName());
            if (from == NULL){
                return false;
            }

            QList<int> cardids;
            if (from->getWeapon())
                cardids.append(from->getWeapon()->getEffectiveId());
            if (from->getArmor())
                cardids.append(from->getArmor()->getEffectiveId());
            if (from->getDefensiveHorse())
                cardids.append(from->getDefensiveHorse()->getEffectiveId());
            if (from->getOffensiveHorse())
                cardids.append(from->getOffensiveHorse()->getEffectiveId());
            foreach(const Card *c, from->getJudgingArea()){
                cardids.append(c->getEffectiveId());
            }
            if (cardids.length() == 0)
                return false;
            room->fillAG(cardids, player);
            int id = room->askForAG(player, cardids, true, objectName());
            room->clearAG(player);
            if (id == -1){
                return false;
            }

            Card *card = Sanguosha->getCard(id);
            Player::Place place = room->getCardPlace(id);

            int type = -1;
            if (place == Player::PlaceEquip){
                if (card->isKindOf("Weapon")){
                    type = 1;
                }
                else if (card->isKindOf("Armor")){
                    type = 2;
                }
                else if (card->isKindOf("DefensiveHorse")){
                    type = 3;
                }
                else if (card->isKindOf("OffensiveHorse")){
                    type = 4;
                }
            }

            QList<ServerPlayer *> tos;
            list = room->getAlivePlayers();

            foreach(ServerPlayer *p, list){
                switch (type){
                case -1:
                    if (!player->isProhibited(p, card) && !p->containsTrick(card->objectName())){
                        tos.append(p);
                    }
                    break;
                case 1:
                    if (!p->getWeapon()){
                        tos.append(p);
                    }
                    break;
                case 2:
                    if (!p->getArmor()){
                        tos.append(p);
                    }
                    break;
                case 3:
                    if (!p->getDefensiveHorse()){
                        tos.append(p);
                    }
                    break;
                case 4:
                    if (!p->getOffensiveHorse()){
                        tos.append(p);
                    }
                    break;
                }
            }
            if (tos.isEmpty()){
                return false;
            }

            ServerPlayer *to = room->askForPlayerChosen(player, tos, objectName());
            if (!to)
                return false;
            CardMoveReason reason = CardMoveReason(CardMoveReason::S_REASON_TRANSFER, player->objectName(), objectName(), QString());
            room->moveCardTo(card, from, to, place, reason);
        }
        return false;
    }
};

//yexi
class Yexi : public TriggerSkill {
public:
    Yexi() : TriggerSkill("yexi") {
        events << CardUsed;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.to.length() != 1)
            return QStringList();
        const Card *card = use.card;
        if (card->isKindOf("Slash") && card->isBlack() && use.from->objectName() == player->objectName() && use.to.length() == 1 && player->distanceTo(use.to.at(0)) == 1){
            bool trigger = false;
            if (use.to.at(0)->getWeapon() && !player->getWeapon())
                trigger = true;
            if (use.to.at(0)->getArmor() && !player->getArmor())    
                trigger = true;
            if (use.to.at(0)->getDefensiveHorse() && !player->getDefensiveHorse())
                trigger = true;
            if (use.to.at(0)->getOffensiveHorse() && !player->getOffensiveHorse())
                trigger = true;
            if (trigger){
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (player->askForSkillInvoke(objectName(), QVariant::fromValue(use.to.at(0)))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<int> cardids;
        if (use.to.at(0)->getWeapon() && !player->getWeapon())
            cardids.append(use.to.at(0)->getWeapon()->getEffectiveId());
        if (use.to.at(0)->getArmor() && !player->getArmor())
            cardids.append(use.to.at(0)->getArmor()->getEffectiveId());
        if (use.to.at(0)->getDefensiveHorse() && !player->getDefensiveHorse())
            cardids.append(use.to.at(0)->getDefensiveHorse()->getEffectiveId());
        if (use.to.at(0)->getOffensiveHorse() && !player->getOffensiveHorse())
            cardids.append(use.to.at(0)->getOffensiveHorse()->getEffectiveId());
        if (cardids.length() == 0)
            return false;
        room->fillAG(cardids, player);
        int id = room->askForAG(player, cardids, true, objectName());
        room->clearAG(player);
        if (id == -1)
            return false;
        CardMoveReason reason = CardMoveReason(CardMoveReason::S_REASON_TRANSFER, player->objectName(), objectName(), QString());
        room->moveCardTo(Sanguosha->getCard(id), use.to.at(0), player, Player::PlaceEquip, reason);
        return true;
    }
};

//feiyan
FeiyanCard::FeiyanCard() {
}

bool FeiyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setNature(DamageStruct::Fire);
    slash->setSkillName("feiyan");
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

const Card *FeiyanCard::validate(CardUseStruct &use) const{
    ServerPlayer *shana = use.from;
    Room *room = shana->getRoom();
    shana->turnOver();
    room->broadcastSkillInvoke(objectName());
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setNature(DamageStruct::Fire);
    slash->setSkillName("feiyan");
    return slash;
}

class FeiyanViewAsSkill : public ZeroCardViewAsSkill {
public:
    FeiyanViewAsSkill() : ZeroCardViewAsSkill("feiyan") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return !player->hasUsed("FeiyanCard");
    }

    virtual const Card *viewAs() const {
        FeiyanCard *feiyan = new FeiyanCard();
        feiyan->setShowSkill(objectName());
        return feiyan;
    }
};

class Feiyan : public TriggerSkill{
public:
    Feiyan() : TriggerSkill("feiyan"){
        events << SlashMissed << PreCardUsed;
        view_as_skill = new FeiyanViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == PreCardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->getSkillName() == "feiyan" && use.from != NULL){
                room->addPlayerHistory(player, use.card->getClassName(), -1);
                use.m_addHistory = false;
                data = QVariant::fromValue(use);
            }
        }
        else {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.slash && effect.slash->getSkillName() == "feiyan") {
                return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (!player->faceUp()){
            player->turnOver();
        }
        return false;
    }
};

//bianchi
BianchiCard::BianchiCard() {
}

bool BianchiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->inMyAttackRange(to_select) && Self->objectName() != to_select->objectName();
}

void BianchiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    damage.reason = "bianchiDamage";
    room->damage(damage);
    if (effect.to->isAlive()){
        effect.to->drawCards(effect.to->getLostHp());
    }
}

class BianchiVS : public ZeroCardViewAsSkill {
public:
    BianchiVS() : ZeroCardViewAsSkill("bianchi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("BianchiCard");
    }

    virtual const Card *viewAs() const{
        BianchiCard *bc = new BianchiCard;
        bc->setShowSkill(objectName());
        return bc;
    }
};

class Bianchi : public TriggerSkill{
public:
    Bianchi() : TriggerSkill("bianchi"){
        events << QuitDying;
        view_as_skill = new BianchiVS;
    }

    virtual QStringList triggerable(TriggerEvent , Room *, ServerPlayer *, QVariant &data, ServerPlayer * &) const{
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage && dying.damage->getReason() == "bianchiDamage"){
            ServerPlayer *p = dying.damage->from;
            if (p->isAlive()){
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer *) const {
        DyingStruct dying = data.value<DyingStruct>();
        dying.damage->from->turnOver();
        return false;
    }
};

//Dushe
class DusheVS : public OneCardViewAsSkill {
public:
    DusheVS() : OneCardViewAsSkill("dushe") {
        response_or_use = true;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isKindOf("Slash");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Dismantlement *dismantlement = new Dismantlement(originalCard->getSuit(), originalCard->getNumber());
        dismantlement->addSubcard(originalCard->getId());
        dismantlement->setSkillName(objectName());
        dismantlement->setShowSkill(objectName());
        return dismantlement;
    }
};

class Dushe : public TriggerSkill{
public:
    Dushe() : TriggerSkill("dushe"){
        events << PreCardUsed;
        view_as_skill = new DusheVS;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer * &) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Dismantlement") && use.card->getSkillName() == objectName()){
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (player->hasShownSkill(this) && player->askForSkillInvoke(objectName(), QVariant::fromValue(use.to))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardUseStruct use = data.value<CardUseStruct>();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if (player->inMyAttackRange(p) && !use.to.contains(p) && !p->isNude()){
                use.to.append(p);
            }
        }
        data.setValue(use);
        return false;
    }
};

//Duotian


void MoesenPackage::addNovelGenerals()
{
    General *shana = new General(this, "shana", "qun", 4, false); // Novel 001
    shana->addSkill(new Feiyan);

    
    General *louise = new General(this, "louise", "qun", 3, false); // Novel 002
    louise->addSkill(new Bianchi);

    
    General *ruri = new General(this, "ruri", "qun", 3, false); // Novel 003
    ruri->addSkill(new Dushe);
    /*
    General *tsukiko = new General(this, "tsukiko", "qun", 3, false); // Novel 004
    */
    General *a_azusa = new General(this, "a_azusa", "qun", 3, false); // Novel 005
    a_azusa->addSkill(new Weihao);
    a_azusa->addSkill(new Zhuyi);
    a_azusa->addSkill(new AzusaMaxCards);
    a_azusa->addSkill(new AzusaTrigger);


    General *eru = new General(this, "eru", "qun", 3, false); // Novel 013
    eru->addSkill(new Haoqi);
    eru->addSkill(new Jinzhi);
    eru->addSkill(new JinzhiMove);
    insertRelatedSkills("jinzhi", "#jinzhi-move");

    General *holo = new General(this, "holo", "qun", 4, false); // Novel 018
    holo->addSkill(new Jisui);

    /*
    General *yuki = new General(this, "yuki", "qun", 3, false); // Novel 006

    General *haruhi = new General(this, "haruhi", "qun", 3, false); // Novel 007

    General *watashi = new General(this, "watashi", "qun", 3, false); // Novel 008
    */
    General *taiga = new General(this, "taiga", "qun", 3, false); // Novel 009
    taiga->addSkill(new Huxiao);
    taiga->addSkill(new Yexi);

    General *aria = new General(this, "aria", "qun", 3, false); // Novel 010
    aria->addSkill(new Jingdi);
    aria->addSkill(new Wujie);
    skills << new JingdiDamage;
    
    /*
    General *ruiko = new General(this, "ruiko", "qun", 3, false); // Novel 011

    General *mikoto = new General(this, "mikoto", "qun", 3, false); // Novel 012

    General *asuna = new General(this, "asuna", "qun", 3, false); // Novel 014

    General *sena = new General(this, "sena", "qun", 3, false); // Novel 015

    General *hitagi = new General(this, "hitagi", "qun", 3, false); // Novel 016

    General *rikka = new General(this, "rikka", "qun", 3, false); // Novel 017

    */
    addMetaObject<WeihaoCard>();
    addMetaObject<ZhuyiCard>();
    addMetaObject<HaoqiCard>();
    addMetaObject<JisuiCard>();
    addMetaObject<JingdiCard>();
    addMetaObject<FeiyanCard>();
    addMetaObject<BianchiCard>();
}
