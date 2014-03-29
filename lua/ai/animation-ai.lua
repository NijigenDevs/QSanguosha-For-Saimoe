
sgs.ai_skill_invoke.molu = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()
	return self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < peaches
end

--Yingqiang(complex)

sgs.ai_skill_invoke.cibei = function(self, data)
	local target = data:toPlayer()
	local value = 0
	if self:isFriend(target) then
		if target:getEquips():length() == 0 then value = value + 15 end
		if target:getArmor() and target:getArmor():isKindOf("SilverLion") and target:getLostHp() > 0 then value = value + 30 end
		if target:getHandcardNum() < 2 and target:getHp() > 2 then value = value -10 end
		if target:getHp() == target:getMaxHp() then value = value -10 end
		if target:hasSkills(sgs.lose_equip_skill) then value = value + 20 end
		if target:hasSkills(sgs.need_equip_skill) then value = value - 12 end
		if target:hasSkills(sgs.cardneed_skill) then value = value - 8 end
		if target:hasSkills(sgs.need_kongcheng) and target:getHandcardNum() ==  1 then value = value + 15 end
		if value > 0 then return true end
	else
		if target:getEquips():length() == 0 then value = value -40 end
		if target:getArmor() and target:getArmor():isKindOf("SilverLion") and target:getLostHp() == 0 then value = value + 15 end
		if target:getArmor() and target:getArmor():isKindOf("SilverLion") and target:getLostHp() > 0 and target:getEquips():length() == 1 then value = value - 30 end
		if target:getHandcardNum() < 2 and target:getHp() > 2 then value = value + 5 end
		if target:hasSkills(sgs.lose_equip_skill) then value = value - 20 end
		if target:hasSkills(sgs.need_equip_skill) then value = value + 15 end
		if target:hasSkills(sgs.cardneed_skill) then value = value + 10 end
		if target:hasSkills(sgs.need_kongcheng) and target:getHandcardNum() ==  1 then value = value - 30 end
		if value > 0 then return true end
	end
end

sgs.ai_skill_cardchosen.cibei = function(self, who, flags) --TODO!!
	if flags == "e" then
		if self:isFriend(who) then
			if who:getArmor() and who:getArmor():isKindOf("SilverLion") and who:getLostHp() > 0 then return who:getArmor() end
			return who:getEquips():at(0)
		else
			if who:getArmor() and not (who:getArmor():isKindOf("SilverLion") and who:getLostHp() > 0) then return who:getArmor() end
			return who:getEquips():at(0)
	end
end

