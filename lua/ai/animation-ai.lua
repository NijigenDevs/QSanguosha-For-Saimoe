
sgs.ai_skill_invoke.molu = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()
	return self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < peaches
end

--yingqiang

sgs.ai_skill_invoke.cibei = function(self, data)
	local target = data:toPlayer()
	local value = 0
	if self:isFriend(target) then
		if target:getEquips():length() == 0 then value = value + 15 end
		if target:getArmor() and target:getArmor():isKindOf("SilverLion") and target:getLostHp() > 0 then value = value + 30 end
		if self:needToThrowArmor(target) then value = value + 10 end
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
		if self:needToThrowArmor(target) then value = value + 5 end
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

--renmin
--wuwei

sgs.ai_skill_playerchosen.quanmian = function(self, targets)
	if #self.friends_noself == 0 then return self.player end
	return self:findPlayerToDraw(true, 1)
end

--miaolv

sgs.ai_skill_playerchosen.yinzhuang = function(self, targets)
	local i = self.room:getTag("YinzhuangCard"):toInt()
	if i == -1 then
		for _,p in sgs.qlist(targets) do
			if self:isEnemy(p) and not p:hasSkills(sgs.masochism_skill) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), p) then return p end
		end
		for _,p in sgs.qlist(targets) do
			if self:isEnemy(p) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), p) then return p end
		end
		for _,p in sgs.qlist(targets) do
			if self:isEnemy(p) then return p end
		end
		return targets:first()
	else
		local new_friends = {}
		for _, friend in ipairs(self.friends) do
			if not self:needKongcheng(friend, true) then table.insert(new_friends, friend) end
		end
		
		local cards = {}
		table.insert(cards, sgs.Sanguosha:getCard(i))
		
		if #new_friends > 0 then
			local card, target = self:getCardNeedPlayer(cards)
			if card and target then
				for _, friend in ipairs(new_friends) do
					if target:objectName() == friend:objectName() then
						return friend
					end
				end
			end
			return new_friends[1]
		else
			return self.player
		end
	end
end


sgs.ai_skill_invoke.yingan = true --TODO maybe some special situations?

sgs.ai_skill_invoke.yinren = function(self, data)
	if #self.enemies == 0 then return false end
	local suits = sgs.StringList()
	for _,card in sgs.qlist(self:getHandcards()) do
		if not suits.contains(card:getSuitString()) then
			suits.append(card:getSuitString())
		else
			return false
		end
	end
	return true
end

sgs.ai_skill_playerchosen.yinren = function(self, targets)
	for _,p in sgs.qlist(targets) do
		if self:isEnemy(p) and not p:hasSkills(sgs.masochism_skill) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), p) then return p end
	end
	for _,p in sgs.qlist(targets) do
		if self:isEnemy(p) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), p) then return p end
	end
	for _,p in sgs.qlist(targets) do
		if self:isEnemy(p) then return p end
	end
	return targets:first()
end

sgs.ai_skill_invoke.tongxin = true