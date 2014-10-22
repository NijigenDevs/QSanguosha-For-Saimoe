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
        const Card *cards = room->askForExchange(targets.first(), objectName(), (targets.first()->getHandcardNum() + 1 / 2));
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
    foreach ( const Player *p , targets){
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


void MoesenPackage::addNovelGenerals()
{
    /*General *shana = new General(this, "shana", "qun", 3, false); // Novel 001

    General *louise = new General(this, "louise", "qun", 3, false); // Novel 002

    General *ruri = new General(this, "ruri", "qun", 3, false); // Novel 003

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

    General *taiga = new General(this, "taiga", "qun", 3, false); // Novel 009

    General *aria = new General(this, "aria", "qun", 3, false); // Novel 010

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
}
