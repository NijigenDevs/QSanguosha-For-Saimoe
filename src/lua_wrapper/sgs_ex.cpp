#include "sgs_ex.h"
#include "util.h"

LuaProhibitSkill::LuaProhibitSkill(const char *name)
	: ProhibitSkill(name)
{
}

bool LuaProhibitSkill::isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others /*= QList<const Player *>()*/) const
{
	if (this->is_prohibited.valid()) {
		sol::table t(is_prohibited.lua_state(),sol::create);
		foreach(auto &e, others)
			t.add(e);
		
		auto result = this->is_prohibited(this, from, to, card, t);
		if (result.valid()) {

		}
		else {
			sol::error e = result;
			
		}
	}
	else {
		return ProhibitSkill::isProhibited(from, to, card, others);
	}
}

LuaFixCardSkill::LuaFixCardSkill(const char *name)
	: FixCardSkill(name)
{
}

bool LuaFixCardSkill::isCardFixed(const Player *from, const Player *to, const QString &flags, Card::HandlingMethod method) const
{

}
