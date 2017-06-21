--夜明珠
local Luminouspearl_skill = {}
Luminouspearl_skill.name = "Luminouspearl"
table.insert(sgs.ai_skills, Luminouspearl_skill)
Luminouspearl_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ZhihengCard") and not self.player:hasShownSkill("zhiheng") and self.player:hasTreasure("Luminouspearl") then
		return sgs.Card_Parse("@ZhihengCard=.")
	end
end

sgs.ai_use_priority.Luminouspearl = 7
sgs.ai_keep_value.Luminouspearl = 4.3

--变更武将相关

function SmartAI:getGeneralValue(player, position)
	local general
	if position then
		general = player:getGeneral()
	else
		general = player:getGeneral2()
	end
	if general:objectName() == "anjiang" then
		if self.player:objectName() ~= player:objectName() then return 3 end
	else
		if position then
			general = player:getActualGeneral1()
		else
			general = player:getActualGeneral2()
		end
	end
	local ajust = 0
	for _, skill in sgs.qlist(general:getVisibleSkillList(true, position)) do
		if skill:getFrequency() == sgs.Skill_Limited and skill:getLimitMark() ~= "" and player:getMark(skill:getLimitMark()) == 0 then
            ajust = ajust - 1
		end
	end
	for name, value in pairs(sgs.general_value) do
		if general:objectName() == name then
			return value + ajust
		end
	end
	return 3
end

function SmartAI:needToTransform()
	local g1 = self.player:getActualGeneral1()
	local g2 = self.player:getActualGeneral2()
	local current_value = 0
	for name, value in pairs(sgs.general_pair_value) do
		if g1:objectName() .. "+" .. g2:objectName() == name or g2:objectName() .. "+" .. g1:objectName() == name then
			current_value = value
			break
		end
	end
	local oringin_g1 = 3
	local oringin_g2 = 3
	for name, value in pairs(sgs.general_value) do
		if g1:objectName() == name then oringin_g1 = value end
		if g2:objectName() == name then oringin_g2 = value end
	end
	if current_value == 0 then current_value = oringin_g1 + oringin_g2 end
	local g2_v = current_value - (oringin_g2 - self:getGeneralValue(self.player, false)) - oringin_g1
	return g2_v < 3
end

sgs.ai_skill_invoke.transform = function(self, data)
	return self:needToTransform()
end