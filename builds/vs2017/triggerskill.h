#pragma once
#include "skill.h"
#include <sol.hpp>

class LuaTriggerSkill : public TriggerSkill
{
	Q_OBJECT

public:
	LuaTriggerSkill(const char *name, Frequency frequency, const char *limit_mark);
	void addEvent(TriggerEvent triggerEvent);
	void setViewAsSkill(ViewAsSkill *view_as_skill);
	void setGlobal(bool global);
	void setCanPreshow(bool preshow);
	void setGuhuoType(const char *type);
	virtual QString getGuhuoBox() const;

	virtual int getPriority() const;
	virtual bool canPreshow() const;
	virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
	virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = nullptr) const;
	virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = nullptr) const;
	void onTurnBroken(const char *function_name, TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = nullptr) const;
	virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;

	sol::protected_function on_record;
	sol::protected_function can_trigger;
	sol::protected_function on_cost;
	sol::protected_function on_effect;
	sol::protected_function on_turn_broken;

	int priority;
	bool can_preshow;

	static void registerType(sol::state &state);
protected:
	QString guhuo_type;
};
