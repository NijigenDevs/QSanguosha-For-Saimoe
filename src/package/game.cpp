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
        events << EventPhaseEnd;
        frequency = NotFrequent;
        view_as_skill = new HaixingViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == EventPhaseEnd && player->getPhase() == Player::Draw)
            if (TriggerSkill::triggerable(player))
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

class Taozui: public TriggerSkill {
public:
    Taozui(): TriggerSkill("taozui") {
        events << EventPhaseStart;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if (triggerEvent == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Start){
                 foreach (const Card *c, player->getJudgingArea()){
                    if (c->isKindOf("Indulgence"))
                        return QStringList();
                }
                foreach (ServerPlayer *p, room->getOtherPlayers(player)){
                    if (!p->getJudgingArea().isEmpty()){
                        foreach (const Card *c, p->getJudgingArea()){
                            if (c->isKindOf("Indulgence"))
                                return QStringList(objectName());
                        }
                    }
                }
                foreach(const Card *c, player->getJudgingArea()){
                    if (c->isKindOf("SupplyShortage"))
                        return QStringList(objectName());
                }
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

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *fuuko, QVariant &data, ServerPlayer *) const{
        if (triggerEvent == EventPhaseStart){
            fuuko->skip(Player::Play);
            bool dis =  false, get = false;
            const Card *to_do;
            foreach(const Card *c, fuuko->getJudgingArea()){
                    if (c->isKindOf("SupplyShortage")){
                        dis = true;
                        to_do = c;
                    }
             }


            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *p, room->getOtherPlayers(fuuko)){
                if (!p->getJudgingArea().isEmpty()){
                    foreach(const Card *card, p->getJudgingArea()){
                        if (card->isKindOf("Indulgence")){
                            targets.append(p);
                            get = true;
                        }
                    }
                }
            }
            QString choice = "taozui_get";
            if (dis && get){
                choice = room->askForChoice(fuuko, objectName(), "taozui_get+taozui_discard");
            }else if(dis && !get){
                choice = "taozui_discard";
            }

            if (choice == "taozui_discard"){
                room->throwCard(to_do, CardMoveReason(CardMoveReason::S_REASON_DISCARD, fuuko->objectName(), "taozui", QString()), fuuko, fuuko);
            }else{
                if (targets.isEmpty())
                    return false;
                ServerPlayer *target = room->askForPlayerChosen(fuuko, targets, "taozui_from");
                foreach(const Card *card,target->getJudgingArea()){
                    if (card->isKindOf("Indulgence"))
                        to_do = card;
                }
                if (!to_do)
                    return false;
                room->obtainCard(fuuko, to_do,CardMoveReason(CardMoveReason::S_REASON_GOTCARD, fuuko->objectName(), "taozui", QString()));
            }
        }
        return false;
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

    General *haruka = new General(this, "haruka", "wu", 3, false); // Game 010

    General *miki = new General(this, "miki", "wu", 3, false); // Game 011

    General *n_rin = new General(this, "n_rin", "wu", 3, false); // Game 012

    General *misuzu = new General(this, "misuzu", "wu", 3, false); // Game 013

    General *setsuna = new General(this, "setsuna", "wu", 3, false); // Game 014

    General *kazusa = new General(this, "kazusa", "wu", 3, false); // Game 015

    General *ayu = new General(this, "ayu", "wu", 3, false); // Game 016

    General *hayate = new General(this, "hayate", "wu", 3, false); // Game 017

    General *komari = new General(this, "komari", "wu", 3, false); // Game 018
    */

   addMetaObject<HaixingCard>();
}
