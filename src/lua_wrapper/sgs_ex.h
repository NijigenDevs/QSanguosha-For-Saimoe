#pragma once

#include "skill.h"
#include "standard.h"
#include <sol.hpp>

class LuaProhibitSkill : public ProhibitSkill
{
	Q_OBJECT

public:
	LuaProhibitSkill(const char *name);

	virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;

	sol::protected_function is_prohibited;
};

class LuaFixCardSkill : public FixCardSkill
{
	Q_OBJECT

public:
	LuaFixCardSkill(const char *name);

	virtual bool isCardFixed(const Player *from, const Player *to, const QString &flags, Card::HandlingMethod method) const;

	sol::protected_function is_cardfixed;
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

	sol::protected_function view_filter;
	sol::protected_function view_as;

	sol::protected_function enabled_at_play;
	sol::protected_function enabled_at_response;
	sol::protected_function enabled_at_nullification;
	sol::protected_function in_pile;

	virtual bool isEnabledAtPlay(const Player *player) const;
	virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
	virtual bool isEnabledAtNullification(const ServerPlayer *player) const;
	virtual QString getExpandPile() const;

protected:
	QString guhuo_type;

};

class LuaViewHasSkill : public ViewHasSkill
{
	Q_OBJECT

public:
	LuaViewHasSkill(const char *name);

	virtual bool ViewHas(const Player *player, const QString &skill_name, const QString &flag) const;
	sol::protected_function is_viewhas;
	inline void setGlobal(bool global)
	{
		this->global = global;
	}
};

class LuaFilterSkill : public FilterSkill
{
	Q_OBJECT

public:
	LuaFilterSkill(const char *name);

	virtual bool viewFilter(const Card *to_select, ServerPlayer *player) const;
	virtual const Card *viewAs(const Card *originalCard) const;

	sol::protected_function view_filter;
	sol::protected_function view_as;
};

class LuaDistanceSkill : public DistanceSkill
{
	Q_OBJECT

public:
	LuaDistanceSkill(const char *name);

	virtual int getCorrect(const Player *from, const Player *to) const;

	sol::protected_function correct_func;
};

class LuaMaxCardsSkill : public MaxCardsSkill
{
	Q_OBJECT

public:
	LuaMaxCardsSkill(const char *name);

	virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;
	virtual int getFixed(const ServerPlayer *target, MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;

	sol::protected_function extra_func;
	sol::protected_function fixed_func;
};

class LuaTargetModSkill : public TargetModSkill
{
	Q_OBJECT

public:
	LuaTargetModSkill(const char *name, const char *pattern);

	virtual int getResidueNum(const Player *from, const Card *card) const;
	virtual int getDistanceLimit(const Player *from, const Card *card) const;
	virtual int getExtraTargetNum(const Player *from, const Card *card) const;

	sol::protected_function residue_func;
	sol::protected_function distance_limit_func;
	sol::protected_function extra_target_func;
};

class LuaAttackRangeSkill : public AttackRangeSkill
{
	Q_OBJECT

public:
	LuaAttackRangeSkill(const char *name);

	virtual int getExtra(const Player *target, bool include_weapon) const;
	virtual int getFixed(const Player *target, bool include_weapon) const;

	sol::protected_function extra_func;
	sol::protected_function fixed_func;
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
	sol::protected_function filter;
	sol::protected_function feasible;
	sol::protected_function about_to_use;
	sol::protected_function on_use;
	sol::protected_function on_effect;
	sol::protected_function on_validate;
	sol::protected_function on_validate_in_response;
	sol::protected_function extra_cost;
	sol::protected_function on_turn_broken;
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
	sol::protected_function filter;
	sol::protected_function feasible;
	sol::protected_function available;
	sol::protected_function about_to_use;
	sol::protected_function on_use;
	sol::protected_function on_effect;

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
	sol::protected_function filter;
	sol::protected_function feasible;
	sol::protected_function available;
	sol::protected_function is_cancelable;
	sol::protected_function about_to_use;
	sol::protected_function on_use;
	sol::protected_function on_effect;
	sol::protected_function on_nullified;

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
	sol::protected_function on_install;
	sol::protected_function on_uninstall;

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
	sol::protected_function on_install;
	sol::protected_function on_uninstall;

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
	sol::protected_function on_install;
	sol::protected_function on_uninstall;

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

	sol::protected_function on_assign;
	sol::protected_function relation;
	sol::protected_function on_tag_set;
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
