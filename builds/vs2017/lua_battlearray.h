#pragma once

#include "skill.h"
#include <sol.hpp>

class LuaBattleArraySkill : public BattleArraySkill
{
	Q_OBJECT

public:
	LuaBattleArraySkill(const char *name, Frequency frequency, const char *limit_mark, HegemonyMode::ArrayType array_type);
	void addEvent(TriggerEvent triggerEvent);
	void setViewAsSkill(ViewAsSkill *view_as_skill);

	virtual int getPriority() const;

	virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
	virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
	void onTurnBroken(const char *function_name, TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
	virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;

	sol::protected_function on_record;
	sol::protected_function can_trigger;
	sol::protected_function on_cost;
	sol::protected_function on_effect;
	sol::protected_function on_turn_broken;

	int priority;

	static void registerType(sol::state &state);
};
