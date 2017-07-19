#include "lua_simpleskills.h"
#include "util.h"

LuaProhibitSkill::LuaProhibitSkill(const char *name)
	: ProhibitSkill(name)
{
}

bool LuaProhibitSkill::isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others /*= QList<const Player *>()*/) const
{
	if (this->is_prohibited.valid()) {
		sol::table t(is_prohibited.lua_state(), sol::create);
		foreach(auto &e, others)
			t.add(e);

		auto result = this->is_prohibited(this, from, to, card, t);
		if (result.valid()) {
			bool r = result.get<bool>();
			return r;
		}
		else {
			sol::error e = result;
			outputError(e.what());
			return ProhibitSkill::isProhibited(from, to, card, others);
		}
	}
	else {
		return ProhibitSkill::isProhibited(from, to, card, others);
	}
}

void LuaProhibitSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char *>> c;
	sol::usertype<LuaProhibitSkill> u(c,
		sol::base_classes, sol::bases<ProhibitSkill>(),

		"is_prohibited", &LuaProhibitSkill::is_prohibited
	);
	sgs.new_usertype<LuaProhibitSkill>("LuaProhibitSkill", u);
}

LuaFixCardSkill::LuaFixCardSkill(const char *name)
	: FixCardSkill(name)
{
}

bool LuaFixCardSkill::isCardFixed(const Player *from, const Player *to, const QString &flags, Card::HandlingMethod method) const
{
	if (is_cardfixed.valid()) {
		auto result = this->is_cardfixed(this, from, to, flags.toUtf8(), method);
		if (result.valid()) {
			bool r = result.get<bool>();
			return r;
		}
		else {
			sol::error e = result;
			outputError(e.what());
			return FixCardSkill::isCardFixed(from, to, flags, method);
		}
	}
	else {
		return FixCardSkill::isCardFixed(from, to, flags, method);
	}
}

void LuaFixCardSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char *>> s;
	sol::usertype<LuaFixCardSkill>u(s,
		sol::base_classes, sol::bases<FixCardSkill>(),

		"is_cardfixed", &LuaFixCardSkill::is_cardfixed
	);

	sgs.new_usertype<LuaFixCardSkill>("LuaFixCardSkill", u);

}

LuaViewHasSkill::LuaViewHasSkill(const char *name)
	: ViewHasSkill(name)
{
}

bool LuaViewHasSkill::ViewHas(const Player *player, const QString &skill_name, const QString &flag) const
{
	if (this->is_viewhas.valid()) {
		auto result = this->is_viewhas(this, player, skill_name.toUtf8(), flag.toUtf8());
		if (result.valid()) {
			bool r = result.get<bool>();
			return r;
		}
		else {
			sol::error e = result;
			outputError(e.what());
			return ViewHasSkill::ViewHas(player, skill_name, flag);
		}
	}
	else {
		return ViewHasSkill::ViewHas(player, skill_name, flag);
	}
}

void LuaViewHasSkill::setGlobal(bool global)
{
	this->global = global;
}

void LuaViewHasSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char*>> c;
	sol::usertype<LuaViewHasSkill> u(c,
		sol::base_classes, sol::bases<ViewHasSkill>(),

		"is_viewhas", &LuaViewHasSkill::is_viewhas,

		"setGlobal", &LuaViewHasSkill::setGlobal

	);
	sgs.new_usertype<LuaViewHasSkill>("LuaViewHasSkill", u);
}

LuaFilterSkill::LuaFilterSkill(const char *name)
	: FilterSkill(name)
{
}

bool LuaFilterSkill::viewFilter(const Card *to_select, ServerPlayer *player) const
{
	if (this->view_filter.valid()) {
		auto result = this->view_filter(this, to_select, player);
		if (result.valid()) {
			bool r = result.get<bool>();
			return r;
		}
		else {
			sol::error e = result;
			outputError(e.what());
		}
	}
	return FilterSkill::viewFilter(to_select, player);
}

const Card * LuaFilterSkill::viewAs(const Card *originalCard) const
{
	if (this->view_as.valid()) {
		auto result = this->view_as(this, originalCard);
		if (result.valid()) {
			const Card *r = result.get<const Card *>();
			return r;
		}
		else {
			sol::error e = result;
			outputError(e.what());
		}
	}
	return FilterSkill::viewAs(originalCard);
}

void LuaFilterSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];

	sol::constructors<sol::types<const char *>> c;
	sol::usertype<LuaFilterSkill> u(c,
		sol::base_classes, sol::bases<FilterSkill>(),

		"view_filter", &LuaFilterSkill::view_filter,
		"view_as", &LuaFilterSkill::view_as
	);

	sgs.new_usertype<LuaFilterSkill>("LuaFilterSkill", u);
}

LuaDistanceSkill::LuaDistanceSkill(const char *name)
	: DistanceSkill(name)
{
}

int LuaDistanceSkill::getCorrect(const Player *from, const Player *to) const
{
	if (this->correct_func.valid()) {
		auto r = this->correct_func(this, from, to);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error e = r;
			outputError(e.what());
		}
	}
	return DistanceSkill::getCorrect(from, to);
}

void LuaDistanceSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char *>> c;
	sol::usertype<LuaDistanceSkill> u(c,
		sol::base_classes, sol::bases<DistanceSkill>(),

		"correct_func", &LuaDistanceSkill::correct_func
	);
	sgs.new_usertype<LuaDistanceSkill>("LuaDistanceSkill", u);
}

LuaMaxCardsSkill::LuaMaxCardsSkill(const char *name)
	: MaxCardsSkill(name)
{
}

int LuaMaxCardsSkill::getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount type /*= MaxCardsType::Max*/) const
{
	if (this->extra_func.valid()) {
		auto r = this->extra_func(this, target, type);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error e = r;
			outputError(e.what());
		}
	}
	return MaxCardsSkill::getExtra(target, type);
}

int LuaMaxCardsSkill::getFixed(const ServerPlayer *target, MaxCardsType::MaxCardsCount type /*= MaxCardsType::Max*/) const
{
	if (this->fixed_func.valid()) {
		auto r = this->fixed_func(this, target, type);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error e = r;
			outputError(e.what());
		}
	}
	return MaxCardsSkill::getFixed(target, type);
}

void LuaMaxCardsSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char *>> c;
	sol::usertype<LuaMaxCardsSkill> u(c,
		sol::base_classes, sol::bases<MaxCardsSkill>(),

		"extra_func", &LuaMaxCardsSkill::extra_func,
		"fixed_func", &LuaMaxCardsSkill::fixed_func
	);
	sgs.new_usertype<LuaMaxCardsSkill>("LuaMaxCardsSkill", u);
}

LuaTargetModSkill::LuaTargetModSkill(const char *name, const char *pattern)
	: TargetModSkill(name)
{
	this->pattern = pattern;
}

int LuaTargetModSkill::getResidueNum(const Player *from, const Card *card) const
{
	if (this->residue_func.valid()) {
		auto r = this->residue_func(this, from, card);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error e = r;
			outputError(e.what());
		}
	}
	return TargetModSkill::getResidueNum(from, card);
}

int LuaTargetModSkill::getDistanceLimit(const Player *from, const Card *card) const
{
	if (this->distance_limit_func.valid()) {
		auto r = this->distance_limit_func(this, from, card);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error e = r;
			outputError(e.what());
		}
	}
	return TargetModSkill::getDistanceLimit(from, card);
}

int LuaTargetModSkill::getExtraTargetNum(const Player *from, const Card *card) const
{
	if (this->extra_target_func.valid()) {
		auto r = this->extra_target_func(this, from, card);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error e = r;
			outputError(e.what());
		}
	}
	return TargetModSkill::getExtraTargetNum(from, card);
}

void LuaTargetModSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char*, const char *>> c;
	sol::usertype<LuaTargetModSkill> u(c,
		sol::base_classes, sol::bases<TargetModSkill>(),

		"residue_func", &LuaTargetModSkill::residue_func,
		"distance_limit_func", &LuaTargetModSkill::distance_limit_func,
		"extra_target_func", &LuaTargetModSkill::extra_target_func

	);
	sgs.new_usertype<LuaTargetModSkill>("LuaTargetModSkill", u);
}

LuaAttackRangeSkill::LuaAttackRangeSkill(const char *name)
	: AttackRangeSkill(name)
{
}

int LuaAttackRangeSkill::getExtra(const Player *target, bool include_weapon) const
{
	if (this->extra_func.valid()) {
		auto r = this->extra_func(this, target, include_weapon);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error r = r;
			outputError(r.what());
		}
	}
	return AttackRangeSkill::getExtra(target, include_weapon);
}

int LuaAttackRangeSkill::getFixed(const Player *target, bool include_weapon) const
{
	if (this->fixed_func.valid()) {
		auto r = this->fixed_func(this, target, include_weapon);
		if (r.valid()) {
			int v = r.get<int>();
			return v;
		}
		else {
			sol::error r = r;
			outputError(r.what());
		}
	}
	return AttackRangeSkill::getFixed(target, include_weapon);
}

void LuaAttackRangeSkill::registerType(sol::state &state)
{
	sol::table sgs = state["sgs"];
	sol::constructors<sol::types<const char *>> c;
	sol::usertype<LuaAttackRangeSkill> u(c,
		sol::base_classes, sol::bases<AttackRangeSkill>(),

		"extra_func", &LuaAttackRangeSkill::extra_func,
		"fixed_func", &LuaAttackRangeSkill::fixed_func
	);
	sgs.new_usertype<LuaAttackRangeSkill>("LuaAttackRangeSkill", u);
}

