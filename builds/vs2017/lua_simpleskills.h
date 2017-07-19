#pragma once

#include "skill.h"
#include <sol.hpp>

class LuaProhibitSkill : public ProhibitSkill
{
	Q_OBJECT

public:
	LuaProhibitSkill(const char *name);

	virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;

	sol::protected_function is_prohibited;

	static void registerType(sol::state &state);
};

class LuaFixCardSkill : public FixCardSkill
{
	Q_OBJECT

public:
	LuaFixCardSkill(const char *name);

	virtual bool isCardFixed(const Player *from, const Player *to, const QString &flags, Card::HandlingMethod method) const;

	sol::protected_function is_cardfixed;

	static void registerType(sol::state &state);
};

class LuaViewHasSkill : public ViewHasSkill
{
	Q_OBJECT

public:
	LuaViewHasSkill(const char *name);

	virtual bool ViewHas(const Player *player, const QString &skill_name, const QString &flag) const;
	sol::protected_function is_viewhas;
	void setGlobal(bool global);

	static void registerType(sol::state &state);
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

	static void registerType(sol::state &state);
};

class LuaDistanceSkill : public DistanceSkill
{
	Q_OBJECT

public:
	LuaDistanceSkill(const char *name);

	virtual int getCorrect(const Player *from, const Player *to) const;

	sol::protected_function correct_func;

	static void registerType(sol::state &state);
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

	static void registerType(sol::state &state);
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

	static void registerType(sol::state &state);
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

	static void registerType(sol::state &state);
};