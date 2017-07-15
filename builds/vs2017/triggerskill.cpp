#include "sgs_ex.h"
#include "util.h"
#include "triggerskill.h"

LuaTriggerSkill::LuaTriggerSkill(const char *name, Frequency frequency, const char *limit_mark)
	: TriggerSkill(name), priority(3)
{
	this->frequency = frequency;
	this->limit_mark = limit_mark;
	this->guhuo_type = "";
}

void LuaTriggerSkill::addEvent(TriggerEvent triggerEvent)
{
	events << triggerEvent;
}

void LuaTriggerSkill::setViewAsSkill(ViewAsSkill *view_as_skill)
{
	this->view_as_skill = view_as_skill;
}

void LuaTriggerSkill::setGlobal(bool global)
{
	this->global = global;
}

void LuaTriggerSkill::setCanPreshow(bool preshow)
{
	this->can_preshow = preshow;
}

void LuaTriggerSkill::setGuhuoType(const char *type)
{
	this->guhuo_type = type;
}

int LuaTriggerSkill::getPriority() const
{
	return priority;
}

bool LuaTriggerSkill::canPreshow() const
{
	return can_preshow;
}

TriggerList LuaTriggerSkill::triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
{
	if (this->can_trigger.valid()) {
		try {
			auto result = this->can_trigger(this, triggerEvent, room, player, data);
			if (result.valid()) {
				auto r = result.get<std::tuple<std::string, std::string>>();
				QString trigger_str = QString::fromUtf8(std::get<0>(r).data());
				QString obj_name_str = QString::fromUtf8(std::get<1>(r).data());
				QStringList obj_name_list = obj_name_str.split("|");
				QStringList who_skill_list = trigger_str.split("|");
				TriggerList skill_list;
				int index = 0;
				while (who_skill_list.size() > index) {
					ServerPlayer *who = player;
					if (obj_name_list.size() > index)
						who = room->findPlayer(obj_name_list.at(index), true);
					if (who)
						skill_list.insert(who, who_skill_list.at(index).split(","));
					index++;
				}
				return skill_list;
			}
			else {
				sol::error e = result;
				room->output(e.what());
				return TriggerSkill::triggerable(triggerEvent, room, player, data);
			}
		}
		catch (TriggerEvent e)
		{
			if (e == TurnBroken || e == StageChange)
				onTurnBroken("can_trigger", triggerEvent, room, player, data, nullptr);
			throw e;
		}
	}
	else {
		return TriggerSkill::triggerable(triggerEvent, room, player, data);
	}
}

bool LuaTriggerSkill::cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /*= NULL*/) const
{
	if (this->on_cost.valid()) {
		try {
			auto result = this->on_cost(this, triggerEvent, room, player, data, ask_who);
			if (result.valid()) {
				auto r = result.get<bool>();
				return r;
			}
			else {
				sol::error e = result;
				room->output(e.what());
				return TriggerSkill::cost(triggerEvent, room, player, data, ask_who);
			}
		}
		catch (TriggerEvent e)
		{
			if (e == TurnBroken || e == StageChange)
				onTurnBroken("on_cost", triggerEvent, room, player, data, ask_who);
			throw e;
		}
	}
	else {
		return TriggerSkill::cost(triggerEvent, room, player, data, ask_who);
	}
}

bool LuaTriggerSkill::effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /*= NULL*/) const
{
	if (this->on_effect.valid()) {
		try {
			auto result = this->on_effect(this, triggerEvent, room, player, data, ask_who);
			if (result.valid()) {
				auto r = result.get<bool>();
				return r;
			}
			else {
				sol::error e = result;
				room->output(e.what());
				return TriggerSkill::effect(triggerEvent, room, player, data, ask_who);
			}
		}
		catch (TriggerEvent e)
		{
			if (e == TurnBroken || e == StageChange)
				onTurnBroken("on_effect", triggerEvent, room, player, data, ask_who);
			throw e;
		}
	}
	else {
		return TriggerSkill::effect(triggerEvent, room, player, data, ask_who);
	}
}

void LuaTriggerSkill::onTurnBroken(const char *function_name, TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /*= nullptr*/) const
{
	if (this->on_turn_broken.valid()) {
		auto result = this->on_turn_broken(this, function_name, triggerEvent, room, player, data, ask_who);
		if (!result.valid()) {
			sol::error e = result;
			room->output(e.what());
		}
	}
}

void LuaTriggerSkill::record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
{
	if (this->on_record.valid()) {
		try {
			auto result = this->on_record(this, triggerEvent, room, player, data);
			if (!result.valid()) {
				sol::error e = result;
				room->output(e.what());
			}
		}
		catch (TriggerEvent e)
		{
			if (e == TurnBroken || e == StageChange)
				onTurnBroken("on_record", triggerEvent, room, player, data, nullptr);
			throw e;
		}
	}
}

void LuaTriggerSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char*, Skill::Frequency, const char*>> cs;
	sol::usertype<LuaTriggerSkill> u(cs,
		sol::base_classes, sol::bases<TriggerSkill>(),

		"addEvent", &LuaTriggerSkill::addEvent,
		"setViewAsSkill", &LuaTriggerSkill::setViewAsSkill,
		"setGlobal", &LuaTriggerSkill::setGlobal,
		"setCanPreshow", &LuaTriggerSkill::setCanPreshow,
		"setGuhuoType", &LuaTriggerSkill::setGuhuoType,
		"getGuhuoBox", &LuaTriggerSkill::getGuhuoBox,

		"onTurnBroken", &LuaTriggerSkill::onTurnBroken,

		"on_record", &LuaTriggerSkill::on_record,
		"can_trigger", &LuaTriggerSkill::can_trigger,
		"on_cost", &LuaTriggerSkill::on_cost,
		"on_effect", &LuaTriggerSkill::on_effect,
		"on_turn_broken", &LuaTriggerSkill::on_turn_broken,

		"priority", &LuaTriggerSkill::priority,
		"can_preshow", &LuaTriggerSkill::can_preshow
	);

	sgs.new_usertype<LuaTriggerSkill>("LuaTriggerSkill", u);
}

QString LuaTriggerSkill::getGuhuoBox() const
{
	return guhuo_type;
}