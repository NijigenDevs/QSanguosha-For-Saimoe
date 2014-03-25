local class = require "middleclass"

--- a SkillCard class
--
-- @usage
--
--  TuxiCard = class("TuxiCard", SkillCard)
--
--    function TuxiCard:initialize()
--      SkillCard.initialize(self)
--      self.name = "tuxi"
--      self.target_fixed = false
--    end
--
--    function TuxiCard:on_effect(card, effect)
--      ...
--    end
--
--  tuxi_card = TuxiCard:create() -- need at file scope
--
SkillCard = class("SkillCard")

  function SkillCard.static:create()
	return self:new():to_card()
  end

  function SkillCard:initialize()
	self.name = nil
	self.target_fixed = false
	self.will_throw = true
  end

  function SkillCard:to_card()
	assert(type(self.name) == "string")

	local card = sgs.LuaSkillCard(self.name)
	card:setTargetFixed(self.target_fixed)
	card:setWillThrow(self.will_throw)

	local methods = {"filter", "feasible", "on_use", "on_effect"}
	for _,method in ipairs(methods) do
	  if self[method] then
		card[method] = function(...)
		  return self[method](self, ...)
		end
	  end
	end

	return card
  end

Skill = class("Skill")

  function Skill.static:create()
	return self:new():to_skill()
  end

  function Skill:initialize()
	self.name = nil
	self.frequency = sgs.Skill_NotFrequent
  end

--- a ViewAsSkill class
--
-- @usage
--
--   Tuxi = class("Tuxi", ViewAsSkill)
--
--     function Tuxi:initialize()
--       ViewAsSkill.initialize(self)
--       self.name = "foo"
--       self.n = 0
--     end
--
--     function Tuxi:view_as(skill, cards)
--       ...
--     end
--
--   tuxi = Tuxi:create() -- need at file scope
--   zhangliao:addSkill(tuxi)
--
ViewAsSkill = class("ViewAsSkill", Skill)

  function ViewAsSkill:initialize()
	Skill.initialize(self)
	self.n = 0
  end

  function ViewAsSkill:to_skill()
	assert(type(self.name) == "string")

	local skill = sgs.LuaViewAsSkill(self.name)

	skill.view_as = function(this, cards)
	  return self.view_as(self, this, cards)
	end

	skill.view_filter = function(this, selected, to_select)
	  if #selected >= self.n then
		return false
	  end

	  return self.view_filter(self, this, selected, to_select)
	end

	local methods = {"enabled_at_play", "enabled_at_response", "enabled_at_nullification"}
	for _,method in ipairs(methods) do
	  if self[method] then
		skill[method] = function(...)
		  return self[method](self, ...)
		end
	  end
	end

	return skill
  end

--- a TriggerSkill class
--
-- @usage
--
--   Hujia = class("Hujia", TriggerSkill)
--
--     function Hujia:initialize()
--       self.name = "hujia"
--       self.events = {sgs.CardAsked}
--     end
--
--     function Hujia:on_trigger(skill, event, player, data)
--       ...
--     end
--
--   hujia = Hujia:create()
TriggerSkill = class("TriggerSkill", Skill)

  function TriggerSkill:initialize()
	Skill.initialize(self)
	self.events = {}
	self.priority = 1
  end

  function TriggerSkill:to_skill()
	assert(type(self.name) == "string")
	assert(type(self.on_trigger) == "function")

	local skill = sgs.LuaTriggerSkill(self.name, self.frequency)
	skill.priority = self.priority

	if type(self.events) == "number" then
	  skill:addEvent(self.events)
	elseif type(self.events) == "table" then
	  for _, event in ipairs(self.events) do
		skill:addEvent(event)
	  end
	end

	if self.view_as_skill then
	  skill:setViewAsSkill(self.view_as_skill)
	end

	local methods = {"on_trigger", "can_trigger"}
	for _,method in ipairs(methods) do
	  if self[method] then
		skill[method] = function(...)
		  return self[method](self, ...)
		end
	  end
	end

	return skill
  end


ProhibitSkill = class("ProhibitSkill", Skill)

  function ProhibitSkill:to_skill()
	assert(type(self.name) == "string")
	assert(type(self.is_prohibited) == "function")

	local skill = sgs.LuaProhibitSkill(self.name)

	skill.is_prohibited = function(...)
	  return self.is_prohibited(self, ...)
	end

	return skill
  end

DistanceSkill = class("DistanceSkill", Skill)

  function DistanceSkill:to_skill()
	assert(type(self.name) == "string")
	assert(type(self.correct_func) == "function")

	local skill = sgs.LuaDistanceSkill(self.name)

	skill.correct_func = function(...)
	  return self.correct_func(self, ...)
	end

	return skill
  end

MaxCardsSkill = class("MaxCardsSkill", Skill)

  function MaxCardsSkill:to_skill()
	assert(type(self.name) == "string")
	assert(type(self.extra_func) == "function")

	local skill = sgs.LuaMaxCardsSkill(self.name)

	skill.extra_func = function(...)
	  return self.extra_func(self, ...)
	end

	return skill
  end

GameStartSkill = class("GameStartSkill", TriggerSkill)

  function GameStartSkill:initialize()
	TriggerSkill.initialize(self)
	self.events = {sgs.GameStart}
  end

  function GameStartSkill:on_trigger(skill, event, player, data)
	self.on_gamestart(self, skill, player)
	return false
  end

  function GameStartSkill:to_skill()
	assert(type(self.on_gamestart) == "function")

	return TriggerSkill.to_skill(self)
  end

MasochismSkill = class("MasochismSkill", TriggerSkill)

  function MasochismSkill:initialize()
	TriggerSkill.initialize(self)
	self.events = {sgs.Damaged}
	self.priority = -1
  end

  function MasochismSkill:on_trigger(skill, event, player, data)
	self.on_damaged(self, skill, player)
	return false
  end

  function MasochismSkill:to_skill()
	assert(type(self.on_damaged) == "function")

	return TriggerSkill.to_skill(self)
  end
