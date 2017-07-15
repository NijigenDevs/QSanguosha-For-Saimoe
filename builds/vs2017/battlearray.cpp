#include "sgs_ex.h"
#include "util.h"
#include "battlearray.h"

LuaBattleArraySkill::LuaBattleArraySkill(const char *name, Frequency frequency, const char *limit_mark, HegemonyMode::ArrayType array_type)
	: BattleArraySkill(name, array_type)
{
	this->frequency = frequency;
	this->limit_mark = limit_mark;
}

void LuaBattleArraySkill::addEvent(TriggerEvent triggerEvent)
{
	events << triggerEvent;
}

void LuaBattleArraySkill::setViewAsSkill(ViewAsSkill *view_as_skill)
{
	this->view_as_skill = view_as_skill;
}

int LuaBattleArraySkill::getPriority() const
{
	return priority;
}

TriggerList LuaBattleArraySkill::triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
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
		TriggerList r;
		if (BattleArraySkill::triggerable(player))
			r[player] << objectName();

		return r;
	}
}

bool LuaBattleArraySkill::cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /*= NULL*/) const
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
				return true;
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

bool LuaBattleArraySkill::effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /*= NULL*/) const
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
				return BattleArraySkill::effect(triggerEvent, room, player, data, ask_who);
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
		return BattleArraySkill::effect(triggerEvent, room, player, data, ask_who);
	}

}

void LuaBattleArraySkill::onTurnBroken(const char *function_name, TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /*= NULL*/) const
{
	if (this->on_turn_broken.valid()) {
		auto result = this->on_turn_broken(this, function_name, triggerEvent, room, player, data, ask_who);
		if (!result.valid()) {
			sol::error e = result;
			room->output(e.what());
		}
	}
}

void LuaBattleArraySkill::record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
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

void LuaBattleArraySkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char *, Frequency, const char*, HegemonyMode::ArrayType>> c;
	sol::usertype<LuaBattleArraySkill> u(c,
		sol::base_classes, sol::bases<BattleArraySkill>(),

		"addEvent", &LuaBattleArraySkill::addEvent,
		"addEvent", &LuaBattleArraySkill::addEvent,
		"setViewAsSkill", &LuaBattleArraySkill::setViewAsSkill,

		"onTurnBroken", &LuaBattleArraySkill::onTurnBroken,

		"on_record", &LuaBattleArraySkill::on_record,
		"can_trigger", &LuaBattleArraySkill::can_trigger,
		"on_cost", &LuaBattleArraySkill::on_cost,
		"on_effect", &LuaBattleArraySkill::on_effect,
		"on_turn_broken", &LuaBattleArraySkill::on_turn_broken,

		"priority", &LuaBattleArraySkill::priority
	);
	sgs.new_usertype<LuaBattleArraySkill>("LuaBattleArraySkill", u);

}
