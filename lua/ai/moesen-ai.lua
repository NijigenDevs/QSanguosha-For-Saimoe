--molu
sgs.ai_skill_invoke.molu = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()
	return self:getAllPeachNum() + self:getCardsNum("Analeptic") < peaches
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
		if target:hasSkills(sgs.masochism_skill) then value = value - 5 end
		if target:hasSkills(sgs.need_kongcheng) and target:getHandcardNum() ==  1 then value = value + 25 end
		if value > 0 then return true end
	else
		if target:getEquips():length() == 0 then value = value -40 end
		if target:getArmor() and target:getArmor():isKindOf("SilverLion") and target:getLostHp() == 0 then value = value + 15 end
		if target:getArmor() and target:getArmor():isKindOf("SilverLion") and target:getLostHp() > 0 and target:getEquips():length() == 1 then value = value - 30 end
		if self:needToThrowArmor(target) then value = value + 10 end
		if target:getHandcardNum() < 2 and target:getHp() > 2 then value = value + 5 end
		if target:hasSkills(sgs.lose_equip_skill) then value = value - 20 end
		if target:hasSkills(sgs.need_equip_skill) then value = value + 15 end
		if target:hasSkills(sgs.cardneed_skill) then value = value + 10 end
		if target:hasSkills(sgs.masochism_skill) then value = value + 10 end
		if target:hasSkills(sgs.need_kongcheng) and target:getHandcardNum() ==  1 then value = value - 30 end
		if value > 0 then return true end
	end
end


--renmin
sgs.ai_skill_invoke.renmin = function(self, data)
	local value = 0
	local move = data:toMoveOneTime()
	if self:needKongcheng(move.from, true) then return false end
	for _,id in sgs.qlist(move.card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if self:getUseValue(card) > 5 then
			value = value + 1
		end
		if self:getKeepValue(card) >= 4.1 then
			value = value + 2
		end
		if card:isKindOf("Peach") then
			value = value + 1
		end
	end
	if self:isWeak(move.from) then
		value = value + 3
	end
	if value > 6 then return true end
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

sgs.ai_use_value.WuweiCard = 6
sgs.ai_use_priority.WuweiCard = 2.5
sgs.ai_card_intention.WuweiCard = 80
sgs.wuwei_keep_value = {
	Peach = 6,
	Jink = 5.1,
	Weapon = 5
}

--shiting
sgs.ai_skill_invoke.shiting = function(self, data)
	local use = data:toCardUse()
	local value = 0
	if self.player:hasSkills("pianxian|tengyue") then
		value = value + 2
	end
	for _,card in sgs.qlist(self.player:getHandcards()) do
		if self:getUseValue(card) > 5 then
			value = value + 1
		end
		if self:getKeepValue(card) >= 4.1 then
			value = value + 2
		end
		if card:isKindOf("Peach") then
			value = value + 2
		end
	end
	if value < 3 then return true end

	if not self:isWeak(self.player) and value < 7 then
		if self.player:hasSkills("wuwei|quanmian|yinren|geji|xiuse") then
			return true
		end
	end
	return false
end

sgs.ai_skill_invoke.shizhi = function(self, data)
	if not self:needKongcheng(self.player, true) then
		return true
	end
end

--quanmian
local quanmian_skill = {}
quanmian_skill.name = "quanmian"
table.insert(sgs.ai_skills, quanmian_skill)
quanmian_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("QuanmianCard") then
		return sgs.Card_Parse("@QuanmianCard=.&quanmian")
	end
end

sgs.ai_skill_use_func.QuanmianCard = function(card, use, self)
	for _,friend in ipairs(self.friends) do
		if friend:hasSkills(sgs.lose_equip_skill) and friend:getEquips():length() > 0 then
			use.card = sgs.Card_Parse("@QuanmianCard=.&quanmian")
			if use.to then
				use.to:append(friend)
			end
			return
		end
		if friend:hasSkill("lieqiang") and friend:getEquips():length() == 1 then
			use.card = sgs.Card_Parse("@QuanmianCard=.&quanmian")
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	self:sort(self.friends, "hp")
	for _,friend in ipairs(self.friends) do
		if friend:getArmor() and friend:getArmor():isKindOf("SilverLion") and friend:getHp() < friend:getMaxHp() and friend:getEquips():length() > 0 then
			use.card = sgs.Card_Parse("@QuanmianCard=.&quanmian")
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	self:sort(self.enemies, "hp")
	for _,enemy in ipairs(self.enemies) do
		if enemy:getEquips():length() > 0 and not enemy:hasSkills(sgs.lose_equip_skill) and enemy:getArmor() and not (enemy:getArmor():isKindOf("SilverLion") and enemy:getHp() < enemy:getMaxHp()) and not (enemy:getArmor():objectName() == "PeaceSpell" and enemy:getHp() > 1) then
			use.card = sgs.Card_Parse("@QuanmianCard=.&quanmian")
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
	for _,enemy in ipairs(self.enemies) do
		if enemy:getEquips():length() > 0 and not enemy:hasSkills(sgs.lose_equip_skill) then
			use.card = sgs.Card_Parse("@QuanmianCard=.&quanmian")
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
	if self.player:getEquips():length() > 0 then
		use.card = sgs.Card_Parse("@QuanmianCard=.&quanmian")
		if use.to then
			use.to:append(self.player)
		end
	end
	return
end

sgs.ai_skill_cardchosen.quanmian = function(self, who, flags)
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
		for _,card in ipairs(cards) do
			if card:isKindOf("DefensiveHorse") then
				return card
			end
		end
	end
	return cards[1]
end

sgs.ai_use_value.QuanmianCard = 4.2
sgs.ai_use_priority.QuanmianCard = 6.5
sgs.ai_card_intention.QuanmianCard = function(self, card, from, to)
	sgs.updateIntention(from, to[1], from:isFriendWith(to[1]) and -30 or 30)
end

--miaolv
local miaolv_skill = {}
miaolv_skill.name = "miaolv"
table.insert(sgs.ai_skills, miaolv_skill)
miaolv_skill.getTurnUseCard = function(self)
	local hp = 1
	for hp = 1, 3 do
		if self.player:getHandcardNum() - self:getCardsNum("Peach") < hp then return end
		local benefit = 0
		for _, friend in ipairs(self.friends) do
			if friend:getHp() == hp then
				benefit = benefit + 1
			end
			if friend:hasSkills(sgs.masochism_skill) then
				benefit = benefit + 1
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if enemy:getHp() == hp then
				benefit = benefit - 1
			end
			if enemy:hasSkills(sgs.masochism_skill) then
				benefit = benefit - 1
			end
		end
		if benefit > hp - 1 then
			return sgs.Card_Parse("@MiaolvCard=.&miaolv")
		end
	end
end

sgs.ai_skill_use_func.MiaolvCard = function(card, use, self)
	local hp = 1
	for hp = 1, 3 do
		local benefit = 0
		if self.player:getHandcardNum() - self:getCardsNum("Peach") < hp then return end
		for _, friend in ipairs(self.friends) do
			if friend:getHp() == hp then
				benefit = benefit + 1
			end
			if friend:hasSkills(sgs.masochism_skill) then
				benefit = benefit + 1
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if enemy:getHp() == hp then
				benefit = benefit - 1
			end
			if enemy:hasSkills(sgs.masochism_skill) then
				benefit = benefit - 1
			end
		end
		if benefit > hp - 1 then
			break
		end
	end

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards, true)
	if hp == 1 and #cards > 0 then
		use.card = sgs.Card_Parse("@MiaolvCard=" .. cards[1]:getId() .. "&wuwei")
	elseif hp == 2 and #cards > 1 then
		use.card = sgs.Card_Parse("@MiaolvCard=" .. cards[1]:getId() .."+".. cards[2]:getId().. "&wuwei")
	elseif hp == 3 and #cards > 2 then
		use.card = sgs.Card_Parse("@MiaolvCard=" .. cards[1]:getId().."+"..cards[2]:getId().."+"..cards[3]:getId() .. "&wuwei")
	end
	return
end

--yinzhuang
sgs.ai_skill_playerchosen.yinzhuang = sgs.ai_skill_playerchosen.zero_card_as_slash

--yingan
function willBeBigKingdom(self)
	t = {["wei"] = 0, ["shu"] = 0,["wu"] = 0,["qun"] = 0}
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:getKingdom() == "wei" then
			t.wei = t.wei + 1
		end
		if p:getKingdom() == "shu" then
			t.shu = t.shu + 1
		end
		if p:getKingdom() == "wu" then
			t.wu = t.wu + 1
		end
		if p:getKingdom() == "qun" then
			t.qun = t.qun + 1
		end
	end
	local ki = self.player:getKingdom()
	local mx = math.max(t.wei, t.shu, t.wu, t.qun)
	if t[ki] < mx then return true end
	return false
end


sgs.ai_skill_invoke.yingan = function(self, data)
	if willBeBigKingdom(self) and #self.friends < 2 then return false end
	return true
end 

--yinren
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
	local slash = sgs.cloneCard("slash")
	local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
	self.player:setFlags("slashNoDistanceLimit")
	self:useBasicCard(slash, dummy_use)
	self.player:setFlags("-slashNoDistanceLimit")

	if dummy_use.card and not dummy_use.to:isEmpty() then
		return dummy_use.to:at(0)
	end
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
--wuxin
sgs.ai_skill_invoke.wuxin = true
sgs.ai_skill_playerchosen.wuxin = function(self, targets)
	local giveyou = sgs.findPlayerByShownSkillName("xiehang")
	if giveyou and self:isFriend(giveyou) then return giveyou end
	giveyou = sgs.findPlayerByShownSkillName("tengyue")
	if giveyou and self:isFriend(giveyou) then return giveyou end
	giveyou = sgs.findPlayerByShownSkillName("pianxian")
	if giveyou and self:isFriend(giveyou) then return giveyou end
	giveyou = sgs.findPlayerByShownSkillName("wuwei")
	if giveyou and self:isFriend(giveyou) then return giveyou end
	for _,p in ipairs(self.friends) do
		if not self:isWeak(p) then
			return p
		end
	end
	return self.player
end

--chidun
sgs.ai_skill_invoke.chidun = function(self, data)
	local hp = self.player:getHp()
	local damage = data:toDamage()
	local dm = damage.damage
	if dm == 1 then
		if hp == 3 and not damage.to:hasSkills(sgs.masochism_skill) and damage.to:getHandcardNum() > 1 then return true end
		if hp == 2 and not damage.to:hasSkills(sgs.masochism_skill) and self:isWeak(damage.to) then return true end
	else
		--any situation for not use this skill?
		if hp == 3  and damage.to:hasSkills(sgs.masochism_skill) and dm == 2 and not self:isWeak(damage.to) then return false end
		return true
	end
	return false
end 



--xiehang
function sgs.ai_skill_pindian.xiehang(minusecard, self, requestor)
	local maxcard = self:getMaxCard()
	return self:isFriend(requestor) and self:getMinCard() or (maxcard:getNumber() < 6 and minusecard or maxcard)
end

--powei
sgs.ai_skill_invoke.powei = function(self, data)
	if not self:willShowForDefence() then
		return false
	end
	return true
end

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
		local cards = who:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _,card in ipairs(cards) do
			if card:isRed() then return card end
		end
	end
	return cards[1]
end
--bajian
local bajianVS_skill = {}
bajianVS_skill.name = "bajianVS"
table.insert(sgs.ai_skills, bajianVS_skill)
bajianVS_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("BajianCard") then return end
	if self:getCardsNum("Slash") > 0 then return end
	local inori = sgs.findPlayerByShownSkillName("bajian")
	if not inori then return end
	if inori:getHandcardNum() < 2 or (self:isWeak(inori) and inori:getHandcardNum() < 3) then return end
	local slash = sgs.cloneCard("slash", sgs.Card_Spade, 1)
	if not slash:isAvailable(self.player) then return end
	local hasGoodTarget = false
	for _,enemy in ipairs(self.enemies) do
		if self:slashIsEffective(slash, enemy) and self.player:distanceTo(enemy) <= self.player:getAttackRange() and self:isWeak(enemy) then
			hasGoodTarget = true
		end
	end
	
	if hasGoodTarget then
		return sgs.Card_Parse("@BajianCard=.&bajianVS")
	end
end

sgs.ai_skill_use_func.BajianCard = function(card, use, self)
	local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
	local slash = sgs.cloneCard("slash", sgs.Card_Spade, 1)
	self:useBasicCard(slash, dummy_use)
	use.to = dummy_use.to
	use.card = sgs.Card_Parse("@BajianCard=.&bajianVS")
	return
end
sgs.ai_use_value.BajianCard = sgs.ai_use_value.Slash - 2
sgs.ai_use_priority.BajianCard = sgs.ai_use_priority.Slash


--mengyin
local mengyin_skill = {}
mengyin_skill.name = "mengyin"
table.insert(sgs.ai_skills, mengyin_skill)
mengyin_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("MengyinCard") or self.player:getHandcardNum() < 2 then
		return nil
	end
	cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local values = self:getKeepValue(cards[1]) + self:getKeepValue(cards[2])
	if values <= 3.6 * 2 + 1.5 then
		return sgs.Card_Parse("@MengyinCard=" .. cards[1]:getId() .. "+" .. cards[2]:getId() .. "&mengyin")
	end
end

sgs.ai_skill_use_func.MengyinCard = function(card, use, self)
	use.card = card
end

sgs.ai_use_value.MengyinCard = 4
sgs.ai_use_priority.MengyinCard = sgs.ai_use_priority.Slash + 0.1

sgs.ai_skill_playerchosen.mengyin = sgs.ai_skill_playerchosen.zero_card_as_slash

--lvdong
sgs.ai_skill_invoke.lvdong = true

--pianxian
sgs.ai_skill_use["@@pianxian"] = function(self, prompt)
	cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	cardnum = self.player:getHp()

	needed = {}
	
	if cardnum > #cards then return end
	if self.player:hasSkill("lieqiang") then
		equips = self.player:getCards("e")
		equips = sgs.QList2Table(equips)
		if #equips <= cardnum then
			for _,equip in ipairs(equips) do
				table.insert(needed, equip:getId())
			end
			cardnum = cardnum - #equips
		end
	end

	
	if cardnum >= 1 then table.insert(needed, tostring(cards[1]:getId())) end
	if cardnum >= 2 then table.insert(needed, tostring(cards[2]:getId())) end
	if cardnum >= 3 then table.insert(needed, tostring(cards[3]:getId())) end
	return ("@DummyCard=" .. table.concat(needed, "+").. "&pianxian")
end

sgs.ai_skill_invoke.pianxian = function(self, data)
	if self.player:getHp() == 1 and self.player:getCards("he") >= 1 then return true end
	if self.player:getHp() == 2 then
		local value = 7 + 3.7
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		value = value - self:getKeepValue(cards[1]) - self:getKeepValue(cards[2])
		if value > 0 then return true end
	end
	return false
end

--liufei
sgs.ai_skill_invoke.liufei = true

--geji
local geji_skill = {}
geji_skill.name = "geji"
table.insert(sgs.ai_skills, geji_skill)
geji_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("GejiCard") and self.player:getCards("he"):length() > 0 then
		if self:isWeak() then
			local spade = false
			for _,card in self.player:getCards("he") do
				if card:getSuit() == sgs.Card_Spade and not card:isKindOf("Armor") and not card:isKindOf("DefensiveHorse") then
					spade = true
				end
			end
			if spade or self.player:getCards("he"):length() > 1 then
				return sgs.Card_Parse("@GejiCard=.&geji")
			else
				return
			end
		end
		return sgs.Card_Parse("@GejiCard=.&geji")
	end
end

sgs.ai_skill_use_func.GejiCard = function(card, use, self)
	targets = sgs.SPlayerList()
	local hasShowns = sgs.SPlayerList()
	for _,other in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if other:hasShownOneGeneral() then
			hasShowns:append(other)
		end
	end
	for _,other in sgs.qlist(hasShowns) do
		if other:getWeapon() and not self:isFriend(other) and other:getWeapon():getSuit() ~= sgs.Card_Spade and not other:hasSkills(sgs.lose_equip_skill) then
			local ok = true
			for _,p in sgs.qlist(targets) do
				if p:objectName() == other:objectName() or p:isFriendWith(other) then
					ok = false
				end
			end
			if ok then
				targets:append(other)
			end
		end
	end
	for _,other in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if other:getArmor() and not self:isFriend(other) and other:getArmor():getSuit() ~= sgs.Card_Spade and not other:hasSkills(sgs.lose_equip_skill) then
			local ok = true
			for _,p in sgs.qlist(targets) do
				if p:objectName() == other:objectName() or p:isFriendWith(other) then
					ok = false
				end
			end
			if ok then
				targets:append(other)
			end
		end
	end
	for _,other in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if not self:isFriend(other) then
			local ok = true
			for _,p in sgs.qlist(targets) do
				if p:objectName() == other:objectName() or p:isFriendWith(other) then
					ok = false
				end
			end
			if ok then
				targets:append(other)
			end
		end
	end
	use.to = targets
	use.card = card
	return
end

sgs.ai_use_value.GejiCard = 7
sgs.ai_use_priority.GejiCard = 7

--pinghe
sgs.ai_skill_invoke.pinghe = function(self, data)
	local use = data:toCardUse()
	for _,pp in sgs.qlist(use.to) do
		if self:isFriend(pp) then
			if self:isEnemy(use.from) then
				if not self:slashIsEffective(use.card, pp, use.from) then return false end
				if self:isWeak(use.from) then
					return true
				end
				if use.from:getHandcardNum() <= 1 then
					return true
				end
				if use.from:getMark("drank") > 0 and (pp:getHandcardNum() <= 2 or pp:getHp() <= 2) then
					return true
				end
			end
		end
	end
	return false
end


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
	local slashes = self:getCardsNum("Slash")
	local targets = 0
	for _,enemy in ipairs(self.enemies) do
		if self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), enemy) and self.player:distanceTo(enemy) <= self.player:getAttackRange() then
			targets = targets + 1
		end
	end
	if targets > 1 and slashes > 1 then return "tengyue2" end
	return "tengyue1"
end

--mogai
sgs.ai_skill_invoke.mogai = function(self, data)
	local hasA,hasW,hasS,hasL
	for _,cardid in sgs.qlist(room:getDiscardPile()) do
		local card = sgs.Sanguosha:getCard(cardid)
		if card:isKindOf("Weapon") then hasW = true end
		if card:isKindOf("Armor") then hasA = true end
		if card:isKindOf("SilverLion") then hasS = true end
		if card:isKindOf("KylinBow") or card:isKindOf("Halberd") or card:isKindOf("Axe") or card:isKindOf("Spear")  then hasL = true end
	end
	if hasA and self.player:getArmor() and self.player:getArmor():objectName() == "SilverLion" and self.player:getHp() < self.player:getMaxHp() then
		return true
	end
	if hasA and self.player:getArmor() and self.player:getArmor():objectName() ~= "SilverLion" and hasS and self.player:getHp() < self.player:getMaxHp() then
		return true
	end
	if self.player:getAttackRange() * 2 < room:getAlivePlayers():length() and hasL then return true end
	return false
end

sgs.ai_skill_askforag.mogai = function(self, card_ids)
	local hasA,hasW,hasS,hasL
	for _,cardid in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(cardid)
		if card:isKindOf("Weapon") then hasW = true end
		if card:isKindOf("Armor") then hasA = true end
		if card:isKindOf("SilverLion") then hasS = true end
		if card:isKindOf("KylinBow") or card:isKindOf("Halberd") or card:isKindOf("Axe") or card:isKindOf("Spear")  then hasL = true end
	end
	if hasA and self.player:getArmor() and self.player:getArmor():objectName() == "SilverLion" and self.player:getHp() < self.player:getMaxHp() then
		for _,cardid in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(cardid)
			if card:isKindOf("Armor") then return cardid end
		end
	end
	if hasA and self.player:getArmor() and self.player:getArmor():objectName() ~= "SilverLion" and hasS and self.player:getHp() < self.player:getMaxHp() then
		for _,cardid in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(cardid)
			if card:isKindOf("SilverLion") then return cardid end
		end
	end
	if self.player:getAttackRange() * 2 < room:getAlivePlayers():length() and hasL then
		for _,cardid in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(cardid)
			if card:isKindOf("KylinBow") or card:isKindOf("Halberd") or card:isKindOf("Axe") or card:isKindOf("Spear") then return cardid end
		end
	end
	return card_ids[1]
end

--ruhun
sgs.ai_skill_invoke.ruhun = function(self, data)
	if not self:willShowForAttack() then return false end
	local use = data:toCardUse()
	if self.player:getHandcardNum() == 1 and self.player:hasSkill("lieqiang") and not self:isWeak() then return true end
	if self.player:getHandcardNum() > player:getHp() then
		if self:isWeak(use.to:at(0)) then return true end
	end
	return false
end

--qinyin
sgs.ai_skill_playerchosen.qinyin = function(self, targets)
	if self.player:hasFlag("AI_qinyin_red") then
		return self:findPlayerToDraw(true, 1)
	elseif self.player:hasFlag("AI_qinyin_black") then
		return self:findPlayerToDiscard("he", true, sgs.Card_MethodDiscard, targets)
	end
	return nil
end

sgs.ai_skill_cardchosen.qinyin = function(self, who, flags)
	return self:askForCardChosen(who, flags, "dismantlement")
end

--qiyuan
sgs.ai_skill_discard.qiyuan = function(self, discard_num, min_num, optional, include_equip)
	local current = self.room:getCurrent()
	if not current then return {} end
	if self:isFriend(current) and self.player:getHandcardNum() > 1 and not self:isWeak() then
		local handCards = self.player:getHandcards()
		self:sortByKeepValue(handCards)
		if self:getKeepValue(handCards[1]) < 3 then
			return { handCards[1]:getEffectiveId() }
		end
	end
	return {}
end

sgs.ai_skill_invoke.qiyuan = function(self, data)
	if not self:needKongcheng(self.player, true) then
		return true
	end
end

--huaming
sgs.ai_skill_playerchosen.huaming = function(self, targets)
	return self:findPlayerToDraw(false, 1)
end

sgs.ai_skill_cardchosen.huaming = function(self, who, flags)
	return self:askForCardChosen(who, flags, "snatch")
end

--fengyin
sgs.ai_skill_cardask["@fengyin_put"] = function(self, data)
	if not self:willShowForDefence() then return "." end
	local player = data:toPlayer()
	if self.player:getHandcardNum() >= 1 and self:isEnemy(player) and not self:isWeak() then
		local handCards = self.player:getHandcards()
		for _, card in sgs.qlist(handCards) do
			if self:getKeepValue(card) < 3 and self:getUseValue(card) < 3 then
				return "$" .. card:getId()
			end
		end
	end
	return "."
end

--jiechu
sgs.ai_skill_invoke.jiechu = function(self, data)
	if not self:needKongcheng(self.player, true) then
		return true
	end
end

--jiandao
sgs.ai_skill_invoke.jiandao = function(self, data)
	if not self:willShowForAttack() then return false end
	local slash = data:toCardUse()
	if slash then
		local to = slash:first()
		if to then
			local handVictims, equipVictims = {}, {}
			local handcardNum = to:getHandcardNum()
			local equipNum = to:getEquips():length()
			for _, p in sgs.qlist(room:getOtherPlayers(self.player)) do
				if p:objectName() ~= to:objectName() then
					if p:getHandcardNum() == handcardNum then
						table.insert(handVictims, p)
					end
					if p:getEquips():length() == equipNum then
						table.insert(equipVictims, p)
					end
				end
			end

			local nature = sgs.DamageStruct_Normal
			if (slash.card:isKindOf("FireSlash")) then
				nature = sgs.DamageStruct_Fire
			elseif (slash.card:isKindOf("ThunderSlash")) then
				nature = sgs.DamageStruct_Thunder
			end

			local dmg = self:hasHeavySlashDamage(self.player, slash.card, to, true)

			local value = 50

			for _, p in ipairs(handVictims) do
				if (self:isEnemy(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value + dmg * 10
					if (self:isWeak(p)) then
						value = value + dmg * 15
					end
				end
				if (self:isFriend(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value - dmg * 15 + self:getAllPeachNum() * 5
					if (self:isWeak(p)) then
						value = value - dmg * 20
					end
				end
			end

			local hand = value

			value = 50

			for _, p in ipairs(equipVictims) do
				if (self:isEnemy(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value + dmg * 10
					if (self:isWeak(p)) then
						value = value + dmg * 15
					end
				end
				if (self:isFriend(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value - dmg * 15 + self:getAllPeachNum() * 5
					if (self:isWeak(p)) then
						value = value - dmg * 20
					end
				end
			end

			local equip = value

			if hand >= 60 or equip >= 60 then
				return true
			end
		end
	end
	return false
end

sgs.ai_skill_choice.jiandao = function(self, choices, data)
	local slash = data:toCardUse()
	if slash then
		local to = slash:first()
		if to then
			local handVictims, equipVictims = {}, {}
			local handcardNum = to:getHandcardNum()
			local equipNum = to:getEquips():length()
			for _, p in sgs.qlist(room:getOtherPlayers(self.player)) do
				if p:objectName() ~= to:objectName() then
					if p:getHandcardNum() == handcardNum then
						table.insert(handVictims, p)
					end
					if p:getEquips():length() == equipNum then
						table.insert(equipVictims, p)
					end
				end
			end

			local nature = sgs.DamageStruct_Normal
			if (slash.card:isKindOf("FireSlash")) then
				nature = sgs.DamageStruct_Fire
			elseif (slash.card:isKindOf("ThunderSlash")) then
				nature = sgs.DamageStruct_Thunder
			end

			local dmg = self:hasHeavySlashDamage(self.player, slash.card, to, true)

			local value = 50

			for _, p in ipairs(handVictims) do
				if (self:isEnemy(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value + dmg * 10
					if (self:isWeak(p)) then
						value = value + dmg * 15
					end
				end
				if (self:isFriend(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value - dmg * 15 + self:getAllPeachNum() * 5
					if (self:isWeak(p)) then
						value = value - dmg * 20
					end
				end
			end

			local hand = value

			value = 50

			for _, p in ipairs(equipVictims) do
				if (self:isEnemy(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value + dmg * 10
					if (self:isWeak(p)) then
						value = value + dmg * 15
					end
				end
				if (self:isFriend(p) and self:damageIsEffective(p, nature, self.player)) then
					value = value - dmg * 15 + self:getAllPeachNum() * 5
					if (self:isWeak(p)) then
						value = value - dmg * 20
					end
				end
			end

			local equip = value
			
			if (hand >= equip) then
				return "hand"
			else
				return "equip"
			end
		end
	end
	return "hand"
end

--tianzi

sgs.ai_skill_invoke.tianzi = function(self)
	if not self:willShowForAttack() and not self:willShowForDefence() then
		return false
	end
	return true
end

sgs.tianzi_keep_value = {
	ExNihilo = 0,
	BefriendAttacking = 0,
	Indulgence = 0,
	SupplyShortage = 0,
	Snatch = 0,
	Dismantlement = 0,
	Duel = 0,
	Drownning = 0,
	BurningCamps = 0,
	Collateral = 0,
	ArcheryAttack = 0,
	SavageAssault = 0,
	KnownBoth = 0,
	IronChain = 0,
	GodSalvation = 0,
	Fireattack = 0,
	AllianceFeast = 0,
	FightTogether = 0,
	LureTiger = 0,
	ThreatenEmperor = 0,
	AwaitExhausted = 0,
	ImperialOrder = 0
}

-- AI don't use the TianziCard

--yuzhai
sgs.ai_skill_playerchosen.yuzhai = function(self, targets)
	return self:findPlayerToDiscard("h", false, sgs.Card_MethodDiscard, targets)
end

sgs.ai_skill_cardchosen.yuzhai = function(self, who, flags)
	return self:askForCardChosen(who, flags, "dismantlement")
end

--mizou
sgs.ai_skill_cardask["@mizou_discard"] = function(self, data)
	if not self:willShowForDefence() then return "." end
	local damage = data:toDamage()
	local from = damage.from
	local to = damage.to
	local dmg = damage.damage
	if (self.player:objectName() == from:objectName()) then
		if (self:isFriend(to)) then
			local todiscard = self:askForDiscard("mizou", 1, 0, true, false)
			if todiscard.length() == 1 and self:getKeepValue(sgs.Sanguosha:getEngineCard(todiscard[1])) <= 4.2 then
				return "$" .. todiscard[1]
			end
		end
	elseif (self.player:objectName() == to:objectName()) then
		if (self:isFriend(from)) then
			local todiscard = self:askForDiscard("mizou", 1, 0, true, false)
			if todiscard.length() == 1 and self:getKeepValue(sgs.Sanguosha:getEngineCard(todiscard[1])) <= 5.5 then
				return "$" .. todiscard[1]
			end
		elseif (self:isEnemy(from)) then
			local todiscard = self:askForDiscard("mizou", 1, 0, true, false)
			if todiscard.length() == 1 and self:getKeepValue(sgs.Sanguosha:getEngineCard(todiscard[1])) <= 4.5 then
				return "$" .. todiscard[1]
			end
		else
			local todiscard = self:askForDiscard("mizou", 1, 0, true, false)
			if todiscard.length() == 1 and self:getKeepValue(sgs.Sanguosha:getEngineCard(todiscard[1])) <= 4.8 then
				return "$" .. todiscard[1]
			end
		end
	end
	return "."
end


--wushu
sgs.ai_skill_invoke.wushu = function(self)
	if not self:willShowForAttack() and not self:willShowForDefence() then
		return false
	end
	return true
end

sgs.ai_skill_playerchosen.wushu = function(self, targets)
	return self:findPlayerToDraw(true, 1)
end
