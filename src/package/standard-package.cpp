#include "standard-package.h"
#include "exppattern.h"
#include "card.h"
#include "skill.h"

//Xusine: we can put some global skills in here,for example,the Global FakeMove.
//just for convenience.

class GlobalFakeMoveSkill : public TriggerSkill { 
public:
    GlobalFakeMoveSkill() : TriggerSkill("global-fake-move") {
        events << BeforeCardsMove << CardsMoveOneTime;
        global = true;
    }

    virtual int getPriority() const{
        return 10;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *target, QVariant &, ServerPlayer * &) const{
        return (target != NULL) ? QStringList(objectName()) : QStringList();
    }

    virtual bool effect(TriggerEvent , Room *room, ServerPlayer *, QVariant &, ServerPlayer *) const{
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (p->hasFlag("Global_InTempMoving"))
                return true;
        }

        return false;
    }

};

StandardPackage::StandardPackage()
    : Package("standard")
{
    skills << new GlobalFakeMoveSkill;

    patterns["."] = new ExpPattern(".|.|.|hand");
    patterns[".S"] = new ExpPattern(".|spade|.|hand");
    patterns[".C"] = new ExpPattern(".|club|.|hand");
    patterns[".H"] = new ExpPattern(".|heart|.|hand");
    patterns[".D"] = new ExpPattern(".|diamond|.|hand");

    patterns[".black"] = new ExpPattern(".|black|.|hand");
    patterns[".red"] = new ExpPattern(".|red|.|hand");

    patterns[".."] = new ExpPattern(".");
    patterns["..S"] = new ExpPattern(".|spade");
    patterns["..C"] = new ExpPattern(".|club");
    patterns["..H"] = new ExpPattern(".|heart");
    patterns["..D"] = new ExpPattern(".|diamond");

    patterns[".Basic"] = new ExpPattern("BasicCard");
    patterns[".Trick"] = new ExpPattern("TrickCard");
    patterns[".Equip"] = new ExpPattern("EquipCard");

    patterns[".Weapon"] = new ExpPattern("Weapon");
    patterns["slash"] = new ExpPattern("Slash");
    patterns["jink"] = new ExpPattern("Jink");
    patterns["peach"] = new  ExpPattern("Peach");
    patterns["nullification"] = new ExpPattern("Nullification");
    patterns["peach+analeptic"] = new ExpPattern("Peach,Analeptic");
}

ADD_PACKAGE(Standard)


TestPackage::TestPackage()
: Package("test")
{
    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);

    new General(this, "anjiang", "god", 5, true, true, true);
    new General(this, "anjiang_head", "god", 5, true, true, true);
    new General(this, "anjiang_deputy", "god", 5, true, true, true);

    // developers
    new General(this, "slob", "programmer", 9, true, true, true);
}

ADD_PACKAGE(Test)


StandardCardPackage::StandardCardPackage()
: Package("standard_cards", Package::CardPack)
{
    addWeiSkills();
    addShuSkills();
    addWuSkills();
    addQunSkills();

    QList<Card *> cards;

    cards << basicCards() << equipCards() << trickCards();

    foreach (Card *card, cards)
        card->setParent(this);

    addEquipSkills();
}

ADD_PACKAGE(StandardCard)

