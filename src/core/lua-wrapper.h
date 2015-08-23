/********************************************************************
    Copyright (c) 2013-2015 - Mogara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    Mogara
    *********************************************************************/

#ifndef _LUA_WRAPPER_H
#define _LUA_WRAPPER_H

#include "skill.h"
#include "standard.h"


struct lua_State;
typedef int LuaFunction;

class LuaTriggerSkill : public TriggerSkill
{
    Q_OBJECT

public:
    LuaTriggerSkill(const char *name, Frequency frequency, const char *limit_mark);
    inline void addEvent(TriggerEvent triggerEvent)
    {
        events << triggerEvent;
    }
    inline void setViewAsSkill(ViewAsSkill *view_as_skill)
    {
        this->view_as_skill = view_as_skill;
    }
    inline void setGlobal(bool global)
    {
        this->global = global;
    }
    inline void setCanPreshow(bool preshow)
    {
        this->can_preshow = preshow;
    }
    inline void setGuhuoType(const char *type)
    {
        this->guhuo_type = type;
    }
    virtual QString getGuhuoBox() const;

    virtual int getPriority() const;
    virtual bool canPreshow() const;
    virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    void onTurnBroken(const char *function_name, TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_record;
    LuaFunction can_trigger;
    LuaFunction on_cost;
    LuaFunction on_effect;
    LuaFunction on_turn_broken;

    int priority;
    bool can_preshow;
protected:
    QString guhuo_type;
};

class LuaBattleArraySkill : public BattleArraySkill
{
    Q_OBJECT

public:
    LuaBattleArraySkill(const char *name, Frequency frequency, const char *limit_mark, HegemonyMode::ArrayType array_type);
    inline void addEvent(TriggerEvent triggerEvent)
    {
        events << triggerEvent;
    }
    inline void setViewAsSkill(ViewAsSkill *view_as_skill)
    {
        this->view_as_skill = view_as_skill;
    }

    virtual int getPriority() const;

    virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    void onTurnBroken(const char *function_name, TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;

    LuaFunction on_record;
    LuaFunction can_trigger;
    LuaFunction on_cost;
    LuaFunction on_effect;
    LuaFunction on_turn_broken;

    int priority;
};

class LuaViewAsSkill : public ViewAsSkill
{
    Q_OBJECT

public:
    LuaViewAsSkill(const char *name, const char *response_pattern, bool response_or_use, const char *expand_pile, const char *limit_mark);

    inline void setGuhuoType(const char *type)
    {
        this->guhuo_type = type;
    }
    virtual QString getGuhuoBox() const;

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card *viewAs(const QList<const Card *> &cards) const;


    void pushSelf(lua_State *L) const;

    LuaFunction view_filter;
    LuaFunction view_as;

    LuaFunction enabled_at_play;
    LuaFunction enabled_at_response;
    LuaFunction enabled_at_nullification;

    virtual bool isEnabledAtPlay(const Player *player) const;
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
    virtual bool isEnabledAtNullification(const ServerPlayer *player) const;

protected:
    QString guhuo_type;

};

class LuaFilterSkill : public FilterSkill
{
    Q_OBJECT

public:
    LuaFilterSkill(const char *name);

    virtual bool viewFilter(const Card *to_select) const;
    virtual const Card *viewAs(const Card *originalCard) const;

    LuaFunction view_filter;
    LuaFunction view_as;
};

class LuaDistanceSkill : public DistanceSkill
{
    Q_OBJECT

public:
    LuaDistanceSkill(const char *name);

    virtual int getCorrect(const Player *from, const Player *to) const;

    LuaFunction correct_func;
};

class LuaMaxCardsSkill : public MaxCardsSkill
{
    Q_OBJECT

public:
    LuaMaxCardsSkill(const char *name);

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;
    virtual int getFixed(const ServerPlayer *target, MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;

    LuaFunction extra_func;
    LuaFunction fixed_func;
};

class LuaTargetModSkill : public TargetModSkill
{
    Q_OBJECT

public:
    LuaTargetModSkill(const char *name, const char *pattern);

    virtual int getResidueNum(const Player *from, const Card *card) const;
    virtual int getDistanceLimit(const Player *from, const Card *card) const;
    virtual int getExtraTargetNum(const Player *from, const Card *card) const;

    LuaFunction residue_func;
    LuaFunction distance_limit_func;
    LuaFunction extra_target_func;
};

class LuaAttackRangeSkill : public AttackRangeSkill
{
    Q_OBJECT

public:
    LuaAttackRangeSkill(const char *name);

    virtual int getExtra(const Player *target, bool include_weapon) const;
    virtual int getFixed(const Player *target, bool include_weapon) const;

    LuaFunction extra_func;
    LuaFunction fixed_func;
};

class LuaSkillCard : public SkillCard
{
    Q_OBJECT

public:
    LuaSkillCard(const char *name, const char *skillName);
    LuaSkillCard *clone() const;
    inline void setTargetFixed(bool target_fixed)
    {
        this->target_fixed = target_fixed;
    }
    inline void setWillThrow(bool will_throw)
    {
        this->will_throw = will_throw;
    }
    inline void setCanRecast(bool can_recast)
    {
        this->can_recast = can_recast;
    }
    inline void setHandlingMethod(Card::HandlingMethod handling_method)
    {
        this->handling_method = handling_method;
    }
    inline void setMute(bool isMute)
    {
        this->mute = isMute;
    }

    // member functions that do not expose to Lua interpreter
    static LuaSkillCard *Parse(const QString &str);
    void pushSelf(lua_State *L) const;

    virtual QString toString(bool hidden = false) const;

    // these functions are defined at swig/luaskills.i
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self,
        int &maxVotes) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
    void onTurnBroken(const char *function_name, Room *room, QVariant &value) const;

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;
    LuaFunction on_validate;
    LuaFunction on_validate_in_response;
    LuaFunction extra_cost;
    LuaFunction on_turn_broken;
};

class LuaBasicCard : public BasicCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LuaBasicCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype);
    LuaBasicCard *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    inline void setTargetFixed(bool target_fixed)
    {
        this->target_fixed = target_fixed;
    }
    inline void setCanRecast(bool can_recast)
    {
        this->can_recast = can_recast;
    }

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    inline virtual QString getClassName() const
    {
        return QString(class_name);
    }
    inline virtual QString getSubtype() const
    {
        return QString(subtype);
    }
    inline virtual bool isKindOf(const char *cardType) const
    {
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else
            return Card::isKindOf(cardType);
    }

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction available;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;

private:
    QString class_name, subtype;
};

class LuaTrickCard : public TrickCard
{
    Q_OBJECT

public:
    enum SubClass
    {
        TypeNormal, TypeSingleTargetTrick, TypeDelayedTrick, TypeAOE, TypeGlobalEffect
    };

    Q_INVOKABLE LuaTrickCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype);
    LuaTrickCard *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    inline void setTargetFixed(bool target_fixed)
    {
        this->target_fixed = target_fixed;
    }
    inline void setCanRecast(bool can_recast)
    {
        this->can_recast = can_recast;
    }

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onNullified(ServerPlayer *target) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    inline virtual QString getClassName() const
    {
        return class_name;
    }
    inline void setSubtype(const char *subtype)
    {
        this->subtype = subtype;
    }
    inline virtual QString getSubtype() const
    {
        return subtype;
    }
    inline void setSubClass(SubClass subclass)
    {
        this->subclass = subclass;
    }
    inline SubClass getSubClass() const
    {
        return subclass;
    }
    inline virtual bool isKindOf(const char *cardType) const
    {
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else {
            if (Card::isKindOf(cardType)) return true;
            switch (subclass) {
                case TypeSingleTargetTrick: return strcmp(cardType, "SingleTargetTrick") == 0; break;
                case TypeDelayedTrick: return strcmp(cardType, "DelayedTrick") == 0; break;
                case TypeAOE: return strcmp(cardType, "AOE") == 0; break;
                case TypeGlobalEffect: return strcmp(cardType, "GlobalEffect") == 0; break;
                case TypeNormal:
                default:
                    return false;
                    break;
            }
        }
    }

    // the lua callbacks
    LuaFunction filter;
    LuaFunction feasible;
    LuaFunction available;
    LuaFunction is_cancelable;
    LuaFunction about_to_use;
    LuaFunction on_use;
    LuaFunction on_effect;
    LuaFunction on_nullified;

private:
    SubClass subclass;
    QString class_name, subtype;
};

class LuaWeapon : public Weapon
{
    Q_OBJECT

public:
    Q_INVOKABLE LuaWeapon(Card::Suit suit, int number, int range, const char *obj_name, const char *class_name);
    LuaWeapon *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    inline virtual QString getClassName() const
    {
        return class_name;
    }
    inline virtual bool isKindOf(const char *cardType) const
    {
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else
            return Card::isKindOf(cardType);
    }

    // the lua callbacks
    LuaFunction on_install;
    LuaFunction on_uninstall;

private:
    QString class_name;
};

class LuaArmor : public Armor
{
    Q_OBJECT

public:
    Q_INVOKABLE LuaArmor(Card::Suit suit, int number, const char *obj_name, const char *class_name);
    LuaArmor *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    inline virtual QString getClassName() const
    {
        return class_name;
    }
    inline virtual bool isKindOf(const char *cardType) const
    {
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else
            return Card::isKindOf(cardType);
    }

    // the lua callbacks
    LuaFunction on_install;
    LuaFunction on_uninstall;

private:
    QString class_name;
};


class LuaTreasure : public Treasure
{
    Q_OBJECT

public:
    Q_INVOKABLE LuaTreasure(Card::Suit suit, int number, const char *obj_name, const char *class_name);
    LuaTreasure *clone(Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;

    // member functions that do not expose to Lua interpreter
    void pushSelf(lua_State *L) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    inline virtual QString getClassName() const
    {
        return class_name;
    }
    inline virtual bool isKindOf(const char *cardType) const
    {
        if (strcmp(cardType, "LuaCard") == 0 || QString(cardType) == class_name)
            return true;
        else
            return Card::isKindOf(cardType);
    }

    // the lua callbacks
    LuaFunction on_install;
    LuaFunction on_uninstall;

private:
    QString class_name;
};

#include "ai.h"
#include "scenario.h"

class LuaScenario : public Scenario
{
    Q_OBJECT

public:
    LuaScenario(const char *name);

    void setRule(LuaTriggerSkill *rule);

    inline virtual bool exposeRoles() const
    {
        return expose_role;
    }
    inline virtual int getPlayerCount() const
    {
        return player_count;
    }
    virtual QString getRoles() const;
    virtual void assign(QStringList &generals, QStringList &generals2, QStringList &roles, Room *room) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
    virtual void onTagSet(Room *room, const char *key) const;
    inline virtual bool generalSelection() const
    {
        return general_selection;
    }

    inline void setRandomSeat(bool random)
    {
        random_seat = random;
    }

    bool expose_role;
    int player_count;
    bool general_selection;

    LuaFunction on_assign;
    LuaFunction relation;
    LuaFunction on_tag_set;
};

class LuaSceneRule : public ScenarioRule
{
public:
    LuaSceneRule(LuaScenario *parent, TriggerSkill *t);

    virtual int getPriority() const
    {
        return origin->getPriority();
    }

    virtual bool effect(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;

protected:
    TriggerSkill *origin;
};

#endif
