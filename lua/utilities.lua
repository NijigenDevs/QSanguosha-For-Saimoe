--[[********************************************************************
	Copyright (c) 2013-2015 Mogara

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
*********************************************************************]]

function sgs.insertRelatedSkills(Package, main_skill, ...)
	local t = {...}
	local skill_name = (type(main_skill) == "string") and main_skill or main_skill:objectName()
	for _, s in ipairs(t) do
		Package:insertRelatedSkills(skill_name, (type(s) == "string") and s or s:objectName())
	end
end

sgs.isBigKingdom = function(player,skill_name)
	if not player:hasShownOneGeneral() then
		return false
	end
	local big_kingdoms = player:getBigKingdoms(skill_name,sgs.Max)
	local invoke = #big_kingdoms > 0
	if invoke then
		if #big_kingdoms == 1 and big_kingdoms[1]:startsWith("sgs") then
			invoke = table.contains(big_kingdoms,player:objectName())
		elseif player:getRole() == "careerist" then
			invoke = false
		else
			invoke = table.contains(big_kingdoms,player:getKingdom())
		end
	end
	return invoke
end

sgs.addSkillToEngine = function(skill)
	local skill_list = sgs.SkillList()
	if type(skill) == "table" then
		for _,ski in pairs(skill)do
			if not sgs.Sanguosha:getSkill(ski:objectName()) then
				skill_list:append(ski)
			end
		end
		sgs.Sanguosha:addSkills(skill_list)
		return true
	end
	if not sgs.Sanguosha:getSkill(skill:objectName()) then
		skill_list:append(skill)
		sgs.Sanguosha:addSkills(skill_list)
		return true
	end
	return false
end

sgs.addNewKingdom = function(kingdom_name,color)
	assert(type(kingdom_name) == "string")
	assert(type(color) == "string")
	require "lua.config"
	if not table.contains(config.kingdoms,kingdom_name) then
		table.insert(config.kingdoms,kingdom_name)
		config.kingdom_colors[kingdom_name] = string.upper(color)
		return true
	else
		return false
	end
end

function table.Shuffle(list)
	local result = {}
	while #list > 0 do
		local value = list[math.random(1,#list)]
		table.insert(result,value)
		table.removeOne(list,value)
	end
	return result
end
-- utilities, i.e: convert QList<const Card> to Lua's native table
function sgs.QList2Table(qlist)
	local t = {}
	for i = 0, qlist:length() - 1 do
		table.insert(t, qlist:at(i))
	end

	return t
end

-- the iterator of QList object
local qlist_iterator = function(list, n)
	if n < list:length() - 1 then
		return n + 1, list:at(n + 1) -- the next element of list
	end
end

function sgs.qlist(list)
	return qlist_iterator, list, -1
end

-- more general iterator
function sgs.list(list)
	if type(list) == "table" then
		return ipairs(list)
	else
		return sgs.qlist(list)
	end
end

function sgs.reverse(list)
	local new = {}
	for i = #list, 1, -1 do
		table.insert(new, list[i])
	end
	return new
end

-- copied from "Well House Consultants"
-- used to split string into a table, similar with php' explode function
function string:split(delimiter)
	local result = {}
	local from = 1
	local delim_from, delim_to = string.find(self, delimiter, from)
	while delim_from do
		table.insert(result, string.sub(self, from, delim_from - 1))
		from  = delim_to + 1
		delim_from, delim_to = string.find(self, delimiter, from)
	end
	table.insert(result, string.sub(self, from))
	return result
end

function table:contains(element)
	if #self == 0 or type(self[1]) ~= type(element) then return false end
	for _, e in ipairs(self) do
		if e == element then return true end
	end
end

function table:removeOne(element)
	if #self == 0 or type(self[1]) ~= type(element) then return false end

	for i = 1, #self do
		if self[i] == element then
			table.remove(self, i)
			return true
		end
	end
	return false
end

function table:removeAll(element)
	if #self == 0 or type(self[1]) ~= type(element) then return 0 end
	local n = 0
	for i = 1, #self do
		if self[i] == element then
			table.remove(self, i)
			n = n + 1
		end
	end
	return n
end

function table:insertTable(list)
	for _, e in ipairs(list) do
		table.insert(self, e)
	end
end

function table:removeTable(list)
	for _, e in ipairs(list) do
		table.removeAll(self, e)
	end
end

function table.copyFrom(list)
	local l = {}
	for _, e in ipairs(list) do
		table.insert(l, e)
	end
	return l
end

function table:indexOf(value, from)
	from = from or 1
	for i = from, #self do
		if self[i] == value then return i end
	end
	return -1
end

function table:toSet()
	local set = {}
	for _,ele in pairs(self)do
		if not table.contains(set,ele) then
			table.insert(set,ele)
		end
	end
	return set
end

function string:matchOne(option)
	return self:match("^" .. option .. "%p") or self:match("%p" .. option .. "%p") or self:match("%p" .. option .. "$")
end

function string:startsWith(substr)
	local len = string.len(substr)
	if len == 0 or len > string.len(self) then return false end
	return string.sub(self, 1, len) == substr
end

function string:endsWith(substr)
	local len = string.len(substr)
	if len == 0 or len > string.len(self) then return false end
	return string.sub(self, -len, -1) == substr
end

function math:mod(num)
	return math.fmod(self, num)
end

sgs.CommandType = {
	"S_COMMAND_UNKNOWN",
	"S_COMMAND_CHOOSE_CARD",
	"S_COMMAND_PLAY_CARD",
	"S_COMMAND_RESPONSE_CARD",
	"S_COMMAND_SHOW_CARD",
	"S_COMMAND_SHOW_ALL_CARDS",
	"S_COMMAND_EXCHANGE_CARD",
	"S_COMMAND_DISCARD_CARD",
	"S_COMMAND_INVOKE_SKILL",
	"S_COMMAND_MOVE_FOCUS",
	"S_COMMAND_CHOOSE_GENERAL",
	"S_COMMAND_CHOOSE_KINGDOM",
	"S_COMMAND_CHOOSE_SUIT",
	"S_COMMAND_CHOOSE_DIRECTION",
	"S_COMMAND_CHOOSE_PLAYER",
	"S_COMMAND_CHOOSE_ORDER",
	"S_COMMAND_ASK_PEACH",
	"S_COMMAND_SET_MARK",
	"S_COMMAND_SET_FLAG",
	"S_COMMAND_CARD_FLAG",
	"S_COMMAND_NULLIFICATION",
	"S_COMMAND_MULTIPLE_CHOICE",
	"S_COMMAND_PINDIAN",
	"S_COMMAND_AMAZING_GRACE",
	"S_COMMAND_SKILL_YIJI",
	"S_COMMAND_SKILL_GUANXING",
	"S_COMMAND_SKILL_GONGXIN",
	"S_COMMAND_SET_PROPERTY",
	"S_COMMAND_CHANGE_HP",
	"S_COMMAND_CHANGE_MAXHP",
	"S_COMMAND_CHEAT",
	"S_COMMAND_SURRENDER",
	"S_COMMAND_ENABLE_SURRENDER",
	"S_COMMAND_GAME_OVER",
	"S_COMMAND_GAME_START",
	"S_COMMAND_MOVE_CARD",
	"S_COMMAND_GET_CARD",
	"S_COMMAND_LOSE_CARD",
	"S_COMMAND_LOG_EVENT",
	"S_COMMAND_LOG_SKILL",
	"S_COMMAND_UPDATE_CARD",
	"S_COMMAND_CARD_LIMITATION",
	"S_COMMAND_ADD_HISTORY",
	"S_COMMAND_SET_EMOTION",
	"S_COMMAND_FILL_AMAZING_GRACE",
	"S_COMMAND_CLEAR_AMAZING_GRACE",
	"S_COMMAND_TAKE_AMAZING_GRACE",
	"S_COMMAND_FIXED_DISTANCE",
	"S_COMMAND_KILL_PLAYER",
	"S_COMMAND_REVIVE_PLAYER",
	"S_COMMAND_ATTACH_SKILL",
	"S_COMMAND_NULLIFICATION_ASKED",
	"S_COMMAND_EXCHANGE_KNOWN_CARDS",
	"S_COMMAND_SET_KNOWN_CARDS",
	"S_COMMAND_UPDATE_PILE",
	"S_COMMAND_RESET_PILE",
	"S_COMMAND_UPDATE_HANDCARD_NUM",
	"S_COMMAND_UPDATE_STATE_ITEM",
	"S_COMMAND_SPEAK",
	"S_COMMAND_ARRANGE_GENERAL",
	"S_COMMAND_FILL_GENERAL",
	"S_COMMAND_TAKE_GENERAL",
	"S_COMMAND_RECOVER_GENERAL",
	"S_COMMAND_REVEAL_GENERAL",
	"S_COMMAND_AVAILABLE_CARDS",
	"S_COMMAND_ANIMATE",
	"S_COMMAND_LUCK_CARD",
	"S_COMMAND_VIEW_GENERALS",
	"S_COMMAND_SET_DASHBOARD_SHADOW",
	"S_COMMAND_PRESHOW",
	"S_COMMAND_TOGGLE_READY",
	"S_COMMAND_ADD_ROBOT",
	"S_COMMAND_FILL_ROBOTS",
	"S_COMMAND_TRUST",
	"S_COMMAND_PAUSE",
	"S_COMMAND_NETWORK_DELAY_TEST",
	"S_COMMAND_CHECK_VERSION",
	"S_COMMAND_SETUP",
	"S_COMMAND_ADD_PLAYER",
	"S_COMMAND_REMOVE_PLAYER",
	"S_COMMAND_START_IN_X_SECONDS",
	"S_COMMAND_ARRANGE_SEATS",
	"S_COMMAND_WARN",
	"S_COMMAND_SIGNUP",
	"S_COMMAND_DISABLE_SHOW",
	"S_COMMAND_TRIGGER_ORDER",
	"S_COMMAND_MIRROR_GUANXING_STEP",
	"S_COMMAND_CHANGE_SKIN",
	"S_COMMAND_SKILL_MOVECARDS",
    "S_COMMAND_MIRROR_MOVECARDS_STEP",
	"S_COMMAND_SET_VISIBLE_CARDS",
	"S_COMMAND_SET_ACTULGENERAL",
	"S_COMMAND_GLOBAL_CHOOSECARD"
}

local i = 0
for _, command in ipairs(sgs.CommandType) do
	sgs.CommandType[command] = i
	i = i + 1
end

json = require("json")
