--molu
sgs.ai_skill_invoke.molu = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()
	return self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < peaches
end



--yingqiang(INCREDIBLE HARD)

--cibei
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
end

--renmin
sgs.ai_skill_invoke.renmin = function(self, data)
	local value = 0
	local move = data:toMoveOneTime()
	if self:needKongcheng(move.from, true) then return false end
	for _,id in sgs.qlist(move.card_ids) do
		--TODO
	end
	
	if value > 0 then return true end
	return false
end


--wuwei VS SKILL need DEBUG

local wuwei_skill = {}
wuwei_skill.name = "wuwei"
table.insert(sgs.ai_skills, wuwei_skill)
wuwei_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("WuweiCard") and not self.player:isKongcheng() then
		return sgs.Card_Parse("@WuweiCard=.&wuwei")
	end
end

sgs.ai_skill_use_func.WuweiCard = function(card, use, self)
	if self.player:isKongcheng() then return end
	self:sort(self.enemies, "hp")
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _, enemy in ipairs(self.enemies) do
		if self:objectiveLevel(enemy) > 2 and not self:cantbeHurt(enemy) 
		and self:damageIsEffective(enemy) and not enemy:hasSkills(sgs.masochism_skill) and not (enemy:getArmor() 
		and enemy:getArmor():isKindOf("SilverLion")) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), enemy) then
			if self.player:distanceTo(enemy) <= self.player:getAttackRange() and self.player:getHp() > 1 then
				use.card = sgs.Card_Parse("@WuweiCard=" .. cards[1]:getId() .. "&wuwei")
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end

sgs.ai_use_value.WuweiCard = 2.5
sgs.ai_card_intention.WuweiCard = 80
sgs.wuwei_keep_value = {
	Peach = 6,
	Jink = 5.1,
	Weapon = 5
}

--quanmian
sgs.ai_skill_playerchosen.quanmian = function(self, targets)
	if #self.friends_noself == 0 then return self.player end
	return self:findPlayerToDraw(true, 1)
end

--miaolv

local miaolv_skill = {}
miaolv_skill.name = "miaolv"
table.insert(sgs.ai_skills, miaolv_skill)
miaolv_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("MiaolvCard") then
		return sgs.Card_Parse("@MiaolvCard=.&miaolv")
	end
end

sgs.ai_skill_use_func.MiaolvCard = function(card, use, self)
	for _,friend in ipairs(self.friends) do
		if friend:hasSkills(sgs.lose_equip_skill) and friend:getEquips():length() > 0 then
			use.card = sgs.Card_Parse("@MiaolvCard=.&miaolv")
			if use.to then
				use.to:append(friend)
			end
			return
		end
		if friend:hasSkill("lieqiang") and friend:getEquips():length() == 1 then
			use.card = sgs.Card_Parse("@MiaolvCard=.&miaolv")
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	self:sort(self.friends, "hp")
	for _,friend in ipairs(self.friends) do
		if friend:getArmor() and friend:getArmor():isKindOf("SilverLion") and friend:getHp() < friend:getMaxHp() and friend:getEquips():length() > 0 then
			use.card = sgs.Card_Parse("@MiaolvCard=.&miaolv")
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	self:sort(self.enemies, "hp")
	for _,enemy in ipairs(self.enemies) do
		if enemy:getEquips():length() > 0 and not enemy:hasSkills(sgs.lose_equip_skill) and enemy:getArmor() and not (enemy:getArmor():isKindOf("SilverLion") and enemy:getHp() < enemy:getMaxHp()) then
			use.card = sgs.Card_Parse("@MiaolvCard=.&miaolv")
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
	if self.player:getEquips():length() > 0 then
		use.card = sgs.Card_Parse("@MiaolvCard=.&miaolv")
		if use.to then
			use.to:append(self.player)
		end
	end
	return
end

sgs.ai_skill_cardchosen.miaolv = function(self, who, flags)
	local cards = sgs.QList2Table(who:getEquips())
	if self:isFriend(who) then
		for _,card in ipairs(cards) do
			if card:isKindOf("SilverLion") and who:getHp() < who:getMaxHp() then
				return card
			end
		end
		for _,card in ipairs(cards) do
			if not card:isKindOf("Armor") then
				return card
			end
		end
		return cards[1]
	else
		for _,card in ipairs(cards) do
			if not (card:isKindOf("SilverLion") and who:getHp() < who:getMaxHp()) and card:isKindOf("Armor") then
				return card
			end
		end
	end
	return cards[1]
end

sgs.ai_use_value.MiaolvCard = 4.2
sgs.ai_use_priority.MiaolvCard = 6.5
sgs.ai_card_intention.MiaolvCard = function(self, card, from, to)
	sgs.updateIntention(from, to[1], from:isFriendWith(to[1]) and -30 or 30)
end

--yinzhuang
sgs.ai_skill_playerchosen.yinzhuang = function(self, targets)
	local i = self.room:getTag("YinzhuangCard"):toInt()
	if i == -1 then
		for _,p in sgs.qlist(targets) do
			if not self:isFriend(p) and not p:hasSkills(sgs.masochism_skill) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), p) then return p end
		end
		for _,p in sgs.qlist(targets) do
			if not self:isFriend(p) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), p) then return p end
		end
		for _,p in sgs.qlist(targets) do
			if not self:isFriend(p) then return p end
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
	if data:toString() == "gainacard" then
		if (self:isFriend(self.room:getCurrent())) then
			return false
		end
		return true
	end
	if #self.enemies == 0 then return false end
	local suits = {}
	for _,card in sgs.qlist(self.player:getHandcards()) do
		if not table.contains(suits, card:getSuitString()) then
			table.insert(suits, card:getSuitString())
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
--....
sgs.ai_skill_invoke.powei = true
--lingchang
sgs.ai_skill_invoke.lingchang = function(self, data)--IMBA AI...
	local p = data:toPlayer()
	local cards = p:getHandcards()
	local reds = 0
	local blacks = 0
	for _,card in sgs.qlist(cards) do
		if card:isRed() then
			reds = reds + 1
		else
			blacks = blacks + 1
		end
	end
	if p:objectName() == self.player:objectName() then
		if reds > 0 then return true end
	end
	if reds <= 1 then return false end
	if reds > blacks then return true end
	if p:getHandcardNum() < p:getHp() then return false end
	if blacks < reds then return true end
	return false
end

sgs.ai_skill_cardchosen.lingchang = function(self, who, flags)
	if who:objectName() == self.player:objectName() then
		local cards = who.getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _,card in ipairs(cards) do
			if card:isRed() then return card end
		end
	end
	return cards[1]
end

--mengyin


sgs.ai_skill_invoke.lvdong = true

sgs.ai_skill_invoke.liufei = true

--tengyue
sgs.ai_skill_invoke.tengyue = function(self, data)
	local slashes = 0
	local targets = 0
	local cards = sgs.QList2Table(self.player:getHandcards())
	for _,card in ipairs(cards) do
		if card:isKindOf("Slash") then slashes = slashes + 1 end
	end
	for _,enemy in ipairs(self.enemies) do
		if self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), enemy) and self.player:distanceTo(enemy) <= self.player:getAttackRange() then
			targets = targets + 10
		end
	end
	if targets == 0 or slashes == 0 then return false end
	if targets == 1 and (self:isWeak(self.player) or slashes <= 1) then return false end
	return true
end

sgs.ai_skill_choice.tengyue = function(self, choices)
	local slashes = 0
	local targets = 0
	local cards = sgs.QList2Table(self.player:getHandcards())
	for _,card in ipairs(cards) do
		if card:isKindOf("Slash") then slashes = slashes + 1 end
	end
	for _,enemy in ipairs(self.enemies) do
		if self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), enemy) and self.player:distanceTo(enemy) <= self.player:getAttackRange() then
			targets = targets + 10
		end
	end
	if targets > 1 and slashes > 1 then return "tengyue2" end
	return "tengyue1"
	
end

--mogai

