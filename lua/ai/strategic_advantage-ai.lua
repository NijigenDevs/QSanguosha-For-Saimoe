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

--transfer
local transfer_skill = {}
transfer_skill.name = "transfer"
table.insert(sgs.ai_skills, transfer_skill)
transfer_skill.getTurnUseCard = function(self, inclusive)
	for _, c in sgs.qlist(self.player:getCards("h")) do
		if c:isKindOf("BurningCamps") then continue end
		if c:isTransferable() then return sgs.Card_Parse("@TransferCard=.") end
	end
end

sgs.ai_skill_use_func.TransferCard = function(transferCard, use, self)

	local friends, friends_other = {}, {}
	local targets = sgs.PlayerList()
	for _, friend in ipairs(self.friends_noself) do
		if transferCard:targetFilter(targets, friend, self.player) and not self:needKongcheng(friend, true) then
			if friend:hasShownOneGeneral() then
				table.insert(friends, friend)
			else
				table.insert(friends_other, friend)
			end
		end
	end

	local cards = {}
	local oneJink = self.player:hasSkill("kongcheng")
	for _, c in sgs.qlist(self.player:getCards("he")) do
		if c:isTransferable() and (not isCard("Peach", c, self.player) or #friends > 0) then
			if not oneJink and isCard("Jink", c, self.player) then
				oneJink = true
				continue
			elseif c:getNumber() > 10 and self.player:hasSkills("tianyi|quhu|shuangren|lieren") then
				continue
			end
			table.insert(cards, c)
		end
	end
	if #cards == 0 then return end

	if #friends > 0 then
		self:sortByUseValue(cards)
		if #friends > 0 then
			local card, target = self:getCardNeedPlayer(cards, friends, "transfer")
			if card and target then
				use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
				if use.to then use.to:append(target) end
				return
			end
		end
	end

	if #friends_other > 0 then
		local card, target = self:getCardNeedPlayer(cards, friends_other, "transfer")
		if card and target then
			use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
			if use.to then use.to:append(target) end
			return
		end
	end

	for _, card in ipairs(cards) do
		if card:isKindOf("ThreatenEmperor") then
			local anjiang = 0
			for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if sgs.isAnjiang(p) then anjiang = anjiang + 1 end
			end

			local big_kingdoms = self.player:getBigKingdoms("AI")
			local big_kingdom = #big_kingdoms > 0 and big_kingdoms[1]
			local maxNum = (big_kingdom and (big_kingdom:startsWith("sgs") and 99 or self.player:getPlayerNumWithSameKingdom("AI", big_kingdom)))
							or (anjiang == 0 and 99)
							or 0

			for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if p:hasShownOneGeneral() and transferCard:targetFilter(targets, p, self.player)
					and p:objectName() ~= big_kingdom and (not table.contains(big_kingdoms, p:getKingdom()) or p:getRole() == "careerist")
					and (maxNum == 99 or p:getPlayerNumWithSameKingdom("AI") + anjiang < maxNum) then
					use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
					if use.to then use.to:append(p) end
					return
				end
			end
		elseif card:isKindOf("BurningCamps") then
			local gameProcess = sgs.gameProcess()
			if string.find(gameProcess, self.player:getKingdom() .. ">") then
				for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					if transferCard:targetFilter(targets, p, self.player) and (self:isFriend(p) or (p:hasShownOneGeneral() and self:willSkipPlayPhase(p))) then
						use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
						if use.to then use.to:append(p) end
						return
					end
				end
			else
				for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					if p:hasShownOneGeneral() and transferCard:targetFilter(targets, p, self.player) and card:isAvailable(p) then
						local np = p:getNextAlive()
						if not self:isFriend(np) and (not np:isChained() or self:isGoodChainTarget(np, p, sgs.DamageStruct_Fire, 1, use.card)) then
							use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
							if use.to then use.to:append(p) end
							return
						end
					end
				end
			end
		end
	end
end

sgs.ai_use_priority.TransferCard = -99
sgs.ai_card_intention.TransferCard = -10

--Drowning
function SmartAI:useCardDrowning(card, use)
	if not card:isAvailable(self.player) then return end

	self:sort(self.enemies, "equip_defense")

	local players = sgs.PlayerList()
	for _, enemy in ipairs(self.enemies) do
		if card:targetFilter(players, enemy, self.player) and not players:contains(enemy) and enemy:hasEquip()
			and self:hasTrickEffective(card, enemy) and self:damageIsEffective(enemy, sgs.DamageStruct_Thunder, self.player) and self:canAttack(enemy)
			and not self:getDamagedEffects(enemy, self.player) and not self:needToLoseHp(enemy, self.player) and not self:needToThrowArmor(enemy)
			and not (enemy:hasArmorEffect("PeaceSpell") and (enemy:getHp() > 1 or self:needToLoseHp(enemy, self.player)))
			and not (enemy:hasArmorEffect("Breastplate") and enemy:getHp() == 1) then
			local dangerous
			local chained = {}
			 if enemy:isChained() --[[and not self.player:hasShownSkill("jueqing")]] then
				for _, p in sgs.qlist(self.room:getOtherPlayers(enemy)) do
					if not self:isGoodChainTarget(enemy, p, sgs.DamageStruct_Thunder) and self:damageIsEffective(p, sgs.DamageStruct_Thunder) and self:isFriend(p) then
						table.insert(chained, p)
						if self:isWeak(p) then dangerous = true end
					end
				end
			end
			if #chained >= 2 then dangerous = true end
			if not dangerous then
				players:append(enemy)
				if use.to then use.to:append(enemy) end
			end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if card:targetFilter(players, friend, self.player) and not players:contains(friend) and self:needToThrowArmor(friend) and friend:getEquips():length() == 1 then
			players:append(friend)
			if use.to then use.to:append(friend) end
		end
	end

	if not players:isEmpty() then
		use.card = card
		return
	end
end

sgs.ai_card_intention.Drowning = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if not self:hasTrickEffective(card, to, from) or not self:damageIsEffective(to, sgs.DamageStruct_Thunder, from)
			or self:needToThrowArmor(to) then
		else
			sgs.updateIntention(from, to, 80)
		end
	end
end

sgs.ai_skill_choice.drowning = function(self, choices, data)
	local effect = data:toCardEffect()
	if not self:damageIsEffective(self.player, sgs.DamageStruct_Thunder, effect.from)
		or self:needToLoseHp(self.player, effect.from)
		or self:getDamagedEffects(self.player, effect.from) then return "damage" end

	if self.player:getHp() == 1 and not self.player:hasArmorEffect("Breastplate") then return "throw" end

	local value = 0
	for _, equip in sgs.qlist(self.player:getEquips()) do
		if equip:isKindOf("Weapon") then value = value + (self:evaluateWeapon(equip) / 2)
		elseif equip:isKindOf("Armor") then
			value = value + self:evaluateArmor(equip)
			if self:needToThrowArmor() then value = value - 5
			elseif equip:isKindOf("Breastplate") and self.player:getHp() <= 1 then value = value + 99
			elseif equip:isKindOf("PeaceSpell") then value = value - 2
			end
		elseif equip:isKindOf("OffensiveHorse") then value = value + 2.5
		elseif equip:isKindOf("DefensiveHorse") then value = value + 5
		end
	end
	if value < 8 then return "throw" else return "damage" end
end

sgs.ai_nullification.Drowning = function(self, card, from, to, positive, keep)
	if positive then
		if self:isFriend(to) then
			if self:needToThrowArmor(to) then return end
			if to:getEquips():length() >= 2 then return true, true end
		end
	else
		if self:isFriend(from) and (self:getOverflow() > 0 or self:getCardsNum("Nullification") > 1) then return true, true end
	end
	return
end

sgs.ai_use_value.Drowning = 5.1
sgs.ai_use_priority.Drowning = sgs.ai_use_priority.Dismantlement + 0.05
sgs.ai_keep_value.Drowning = 3.4

--IronArmor
function sgs.ai_armor_value.IronArmor(player, self)
	if self:isWeak(player) then
		for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
			if p:hasShownSkill("huoji") and self:isEnemy(player, p) then
				return 5
			end
		end
	end
	return 2.5
end

sgs.ai_use_priority.IronArmor = 0.82

--BurningCamps
function SmartAI:useCardBurningCamps(card, use)
	if not card:isAvailable(self.player) then return end

	local player = self.room:nextPlayer(self.player)
	if self:isFriendWith(player) then return end

	local players = player:getFormation()
	if players:isEmpty() then return end
	local shouldUse
	for i = 0 , players:length() - 1 do
		player = findPlayerByObjectName(players:at(i):objectName())
		if not self:hasTrickEffective(card, player, self.player) then
			continue
		end
		local damage = {}
		damage.from = self.player
		damage.to = player
		damage.nature = sgs.DamageStruct_Fire
		damage.damage = 1
		if self:damageIsEffective_(damage) then
			if player:isChained() and self:isGoodChainTarget_(damage) then
				shouldUse = true
			elseif self:objectiveLevel(player) > 3 or self:evaluateKingdom(player) == "unknown" then
				shouldUse = true
			else
				return
			end
		end
	end
	if shouldUse then
		use.card = card
	end
end

sgs.ai_nullification.BurningCamps = function(self, card, from, to, positive, keep)
	local targets = sgs.SPlayerList()
	local players = self.room:getTag("targets" .. card:toString()):toList()
	for _, q in sgs.qlist(players) do
		targets:append(q:toPlayer())
	end
	if positive then
		if from:objectName() == self.player:objectName() then return false end
		local chained = {}
		local dangerous
		if self:damageIsEffective(to, sgs.DamageStruct_Fire) and to:isChained() --[[and not from:hasShownSkill("jueqing")]] then
			for _, p in sgs.qlist(self.room:getOtherPlayers(to)) do
				if not self:isGoodChainTarget(to, p, sgs.DamageStruct_Fire) and self:damageIsEffective(p, sgs.DamageStruct_Fire) and self:isFriend(p) then
					table.insert(chained, p)
					if self:isWeak(p) then dangerous = true end
				end
			end
		end
		if to:hasArmorEffect("Vine") and #chained > 0 then dangerous = true end
		local friends = {}
		if self:isFriend(to) then
			for _, p in sgs.qlist(targets) do
				if self:damageIsEffective(p, sgs.DamageStruct_Fire) then
					table.insert(friends, p)
					if self:isWeak(p) or p:hasArmorEffect("Vine") then dangerous = true end
				end
			end
		end
		if #chained + #friends > 2 or dangerous then return true, #friends <= 1 end
		if keep then return false end
		if self:isFriendWith(to) and self:isEnemy(from) then return true, #friends <= 1 end
	else
		if not self:isFriend(from) then return false end
		local chained = {}
		local dangerous
		local enemies = {}
		local good
		if self:damageIsEffective(to, sgs.DamageStruct_Fire) and to:isChained() --[[and not from:hasShownSkill("jueqing")]] then
			for _, p in sgs.qlist(self.room:getOtherPlayers(to)) do
				if not self:isGoodChainTarget(to, p, sgs.DamageStruct_Fire) and self:damageIsEffective(p, sgs.DamageStruct_Fire) and self:isFriend(p) then
					table.insert(chained, p)
					if self:isWeak(p) then dangerous = true end
				end
				if not self:isGoodChainTarget(to, p, sgs.DamageStruct_Fire) and self:damageIsEffective(p, sgs.DamageStruct_Fire) and self:isEnemy(p) then
					table.insert(enemies, p)
					if self:isWeak(p) then good = true end
				end
			end
		end
		if to:hasArmorEffect("Vine") and #chained > 0 then dangerous = true end
		if to:hasArmorEffect("Vine") and #enemies > 0 then good = true end
		local friends = {}
		if self:isFriend(to) then
			for _, p in sgs.qlist(targets) do
				if self:damageIsEffective(p, sgs.DamageStruct_Fire) then
					table.insert(friends, p)
					if self:isWeak(p) or p:hasArmorEffect("Vine") then dangerous = true end
				end
			end
		end
		if self:isEnemy(to) then
			for _, p in sgs.qlist(targets) do
				if self:damageIsEffective(p, sgs.DamageStruct_Fire) then
					if self:isWeak(p) or p:hasArmorEffect("Vine") then good = true end
				end
			end
		end
		if #chained + #friends > 2 or dangerous then return false end
		if keep then
			local nulltype = self.room:getTag("NullificatonType"):toBool()
			if nulltype and targets:length() > 1 then good = true end
			if good then keep = false end
		end
		if keep then return false end
		if self:isFriend(from) and self:isEnemy(to) then return true, true end
	end
	return
end

sgs.ai_use_value.BurningCamps = 7.1
sgs.ai_use_priority.BurningCamps = 4.7
sgs.ai_keep_value.BurningCamps = 3.38
sgs.ai_card_intention.BurningCamps = 10

--Breastplate
sgs.ai_skill_invoke.Breastplate = true

function sgs.ai_armor_value.Breastplate(player, self)
	if player:getHp() >= 3 then
		return 2
	else
		return 5.5
	end
end

sgs.ai_use_priority.Breastplate = 0.9

--LureTiger
function SmartAI:useCardLureTiger(LureTiger, use)
	sgs.ai_use_priority.LureTiger = 4.9
	if not LureTiger:isAvailable(self.player) then return end

	local players = sgs.PlayerList()

	local card = self:getCard("BurningCamps")
	if card and card:isAvailable(self.player) then
		local nextp = self.room:nextPlayer(self.player)
		local first
		while true do
			if LureTiger:targetFilter(players, nextp, self.player) and self:hasTrickEffective(LureTiger, nextp, self.player) then
				if not first then
					if self:isEnemy(nextp) then
						first = nextp
					else
						players:append(nextp)
					end
				else
					if not first:isFriendWith(nextp) then
						players:append(nextp)
					end
				end
				nextp = self.room:nextPlayer(nextp)
			else
				break
			end
		end
		if first then
			use.card = LureTiger
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	card = self:getCard("ArcheryAttack")
	if card and card:isAvailable(self.player) and self:getAoeValue(card) > 0 then
		self:sort(self.friends_noself, "hp")
		for _, friend in ipairs(self.friends_noself) do
			if self:isFriendWith(friend) and LureTiger:targetFilter(players, friend, self.player) and self:hasTrickEffective(LureTiger, friend, self.player) then
				players:append(friend)
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if LureTiger:targetFilter(players, friend, self.player) and not players:contains(friend) and self:hasTrickEffective(LureTiger, friend, self.player) then
				players:append(friend)
			end
		end
		if players:length() > 0 then
			sgs.ai_use_priority.LureTiger = sgs.ai_use_priority.ArcheryAttack + 0.2
			use.card = LureTiger
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	card = self:getCard("SavageAssault")
	if card and card:isAvailable(self.player) and self:getAoeValue(card) > 0 then
		self:sort(self.friends_noself, "hp")
		for _, friend in ipairs(self.friends_noself) do
			if self:isFriendWith(friend) and LureTiger:targetFilter(players, friend, self.player) and self:aoeIsEffective(LureTiger, friend, self.player) then
				players:append(friend)
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if LureTiger:targetFilter(players, friend, self.player) and not players:contains(friend) and self:aoeIsEffective(LureTiger, friend, self.player) then
				players:append(friend)
			end
		end
		if players:length() > 0 then
			sgs.ai_use_priority.LureTiger = sgs.ai_use_priority.SavageAssault + 0.2
			use.card = LureTiger
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	card = self:getCard("Slash")
	if card and self:slashIsAvailable(self.player, card) then
		local dummyuse = { isDummy = true, to = sgs.SPlayerList() }
		self.player:setFlags("slashNoDistanceLimit")
		self:useCardSlash(card, dummyuse)
		self.player:setFlags("-slashNoDistanceLimit")
		if dummyuse.card then
			local total_num = 2 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, LureTiger)
			local function getPlayersFromTo(one)
				local targets1 = sgs.PlayerList()
				local targets2 = sgs.PlayerList()
				local nextp = self.room:nextPlayer(self.player)
				while true do
					if LureTiger:targetFilter(targets1, nextp, self.player) and self:hasTrickEffective(LureTiger, nextp, self.player) then
						if one:objectName() ~= nextp:objectName() then
							targets1:append(nextp)
						else
							break
						end
						nextp = self.room:nextPlayer(nextp)
					else
						targets1 = sgs.PlayerList()
						break
					end
				end
				nextp = self.room:nextPlayer(one)
				while true do
					if LureTiger:targetFilter(targets2, nextp, self.player) and self:hasTrickEffective(LureTiger, nextp, self.player) then
						if self.player:objectName() ~= nextp:objectName() then
							targets2:append(nextp)
						else
							break
						end
						nextp = self.room:nextPlayer(nextp)
					else
						targets2 = sgs.PlayerList()
						break
					end
				end
				if targets1:length() > 0 and targets2:length() >= targets1:length() and targets1:length() <= total_num then
					return targets1
				elseif targets2:length() > 0 and targets1:length() >= targets2:length() and targets2:length() <= total_num then
					return targets2
				end
				return
			end

			for _, to in sgs.qlist(dummyuse.to) do
				if self.player:distanceTo(to) > self.player:getAttackRange() and self.player:distanceTo(to, -total_num) <= self.player:getAttackRange() then
					local sps = getPlayersFromTo(to)
					if sps then
						sgs.ai_use_priority.LureTiger = 3
						use.card = LureTiger
						if use.to then use.to = sgs.PlayerList2SPlayerList(sps) end
						return
					end
				end
			end
		end

	end

	players = sgs.PlayerList()

	card = self:getCard("GodSalvation")
	if card and card:isAvailable(self.player) then
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if LureTiger:targetFilter(players, enemy, self.player) and self:hasTrickEffective(LureTiger, enemy, self.player) then
				players:append(enemy)
			end
		end
		if players:length() > 0 then
			sgs.ai_use_priority.LureTiger = sgs.ai_use_priority.GodSalvation + 0.1
			use.card = LureTiger
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	if self.player:objectName() == self.room:getCurrent():objectName() then
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if LureTiger:targetFilter(players, player, self.player) and self:hasTrickEffective(LureTiger, player, self.player) then
				sgs.ai_use_priority.LureTiger = 0.3
				use.card = LureTiger
				if use.to then use.to:append(player) end
				return
			end
		end
	end
end

sgs.ai_nullification.LureTiger = function(self, card, from, to, positive)
	--[[
	if positive then
		if self:isFriendWith(to) and self:isEnemy(from) then return true end
	else
		if self:isFriend(from) and self:isEnemy(to) then return true end
	end
	--]]
	return false
end

sgs.ai_use_value.LureTiger = 5
sgs.ai_use_priority.LureTiger = 4.9
sgs.ai_keep_value.LureTiger = 3.22

--FightTogether
function SmartAI:useCardFightTogether(card, use)
	self.FightTogether_choice = nil
	if not card:isAvailable(self.player) then return end

	--@todo: consider hongfa

	local big_kingdoms = self.player:getBigKingdoms("AI")
	local bigs, smalls = {}, {}
	local isBig, isSmall
	for _, p in sgs.qlist(self.room:getAllPlayers()) do
		if self:hasTrickEffective(card, p, self.player) then
			local kingdom = p:objectName()
			if #big_kingdoms == 1 and big_kingdoms[1]:startsWith("sgs") then
				if table.contains(big_kingdoms, kingdom) then
					table.insert(bigs, p)
					if p:objectName() == self.player:objectName() then isBig = true end
				else
					if not(p:hasArmorEffect("IronArmor") and not p:isChained()) then
						table.insert(smalls, p)
						if p:objectName() == self.player:objectName() then isSmall = true end
					end
				end
			else
				if not p:hasShownOneGeneral() and not(p:hasArmorEffect("IronArmor") and not p:isChained()) then
					if p:objectName() == self.player:objectName() then isSmall = true end
					table.insert(smalls, p)
					continue
				elseif p:getRole() == "careerist" then
					kingdom = "careerist"
				else
					kingdom = p:getKingdom()
				end
				if table.contains(big_kingdoms, kingdom) then
					if p:objectName() == self.player:objectName() then isBig = true end
					table.insert(bigs, p)
				else
					if not(p:hasArmorEffect("IronArmor") and not p:isChained()) then
						if p:objectName() == self.player:objectName() then isSmall = true end
						table.insert(smalls, p)
					end
				end
			end
		end
	end

	local choices = {}
	if #bigs > 0 then table.insert(choices, "big") end
	if #smalls > 0 then table.insert(choices, "small") end

	if #choices > 0 then
		local v_big, v_small = 0, 0
		for _, p in ipairs(bigs) do
			v_big = v_big + (p:isChained() and -1 or 1)
		end
		if isBig then v_big = -v_big end
		for _, p in ipairs(smalls) do
			v_small = v_small + (p:isChained() and -1 or 1)
		end
		if isSmall then v_small = -v_small end

		if #choices == 1 then
			if table.contains(choices, "big") then
				if v_big > 0 then self.FightTogether_choice = "big" end
			else
				if v_small > 0 then self.FightTogether_choice = "small" end
			end
		else
			if isBig then
				if v_big > 0 and v_big == #bigs then self.FightTogether_choice = "big"
				elseif v_small > 0 then self.FightTogether_choice = "small"
				elseif v_big > 0 then self.FightTogether_choice = "big"
				end
			elseif isSmall then
				if v_small > 0 and v_small == #smalls then self.FightTogether_choice = "small"
				elseif v_big >= 0 then self.FightTogether_choice = "big"
				elseif v_small > 0 then self.FightTogether_choice = "small"
				end
			else
				if v_big > v_small and v_big > 0 then self.FightTogether_choice = "big"
				elseif v_small > v_big and v_small > 0 then self.FightTogether_choice = "small"
				elseif v_big == v_small and v_big > 0 then
					if #bigs > #smalls then self.FightTogether_choice = "big"
					elseif #bigs < #smalls then self.FightTogether_choice = "small"
					else
						self.FightTogether_choice = math.random(1, 2) == 1 and "big" or "small"
					end
				end
			end
		end
	end
	if (self.FightTogether_choice == "big" and #bigs == 1) or (self.FightTogether_choice == "small" and #smalls == 1) then
		local check
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if p:isChained() and self:isEnemy(p) then
				check = true
				break
			end
		end
		if not check then self.FightTogether_choice = nil end
	end

	if not self.FightTogether_choice and not self.player:isCardLimited(card, sgs.Card_MethodRecast) then
		self.FightTogether_choice = "recast"
	end
	if self.FightTogether_choice then
		use.card = card
	end
end

sgs.ai_skill_choice["fight_together"] = function(self, choices)
	choices = choices:split("+")
	if self.FightTogether_choice and table.contains(choices, self.FightTogether_choice) then
		return self.FightTogether_choice
	end
	return choices[#choices]
end

sgs.ai_nullification.FightTogether = function(self, card, from, to, positive, keep)
	local targets = sgs.SPlayerList()
	local players = self.room:getTag("targets" .. card:toString()):toList()
	for _, q in sgs.qlist(players) do
		if q:toPlayer():objectName() ~= to:objectName() and to:isFriendWith(q:toPlayer()) then
			targets:append(q:toPlayer())
		end
	end
	local ed, no = 0, 0
	if positive then
		if to:isChained() then
			local single = true
			if self:isEnemy(to) and to:hasShownSkills(sgs.cardneed_skill .. "|" .. sgs.priority_skill .. "|" .. sgs.wizard_harm_skill) then
				for _, p in sgs.qlist(targets) do
					if p:isChained() then ed = ed + 1 else no = no + 1 end
				end
				if targets:length() > 0 and ed > no then single = false end
				return true, single
			end
		else
			if self:isFriendWith(to) and to:hasArmorEffect("Vine") then
				for _, p in sgs.qlist(targets) do
					if p:isChained() then ed = ed + 1 else no = no + 1 end
				end
				if targets:length() > 0 and no >= ed then single = false end
				return true, single
			end
		end
	else
		if self:isFriendWith(to) and to:isChained() then return true, true end
	end
	return
end

sgs.ai_use_value.FightTogether = 5.2
sgs.ai_use_priority.FightTogether = 8.9
sgs.ai_keep_value.FightTogether = 3.24

--AllianceFeast
function SmartAI:useCardAllianceFeast(card, use)
	if not card:isAvailable(self.player) then return end
	local targets = {}
	local isEnemy
	for _, friend in ipairs(self.friends_noself) do
		if not self.player:isFriendWith(friend) and friend:hasShownOneGeneral() and self:hasTrickEffective(card, friend, self.player) then
			table.insert(targets, friend)
		end
	end
	if #targets == 0 then
		for _, target in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if not self.player:isFriendWith(target) and target:hasShownOneGeneral() and not self:isEnemy(target) and self:hasTrickEffective(card, target, self.player) then
				table.insert(targets, target)
			end
		end
	end
	if #targets == 0 then
		isEnemy = true
		for _, enemy in ipairs(self.enemies) do
			if not self.player:isFriendWith(enemy) and enemy:hasShownOneGeneral() and self:hasTrickEffective(card, enemy, self.player) then
				table.insert(targets, enemy)
			end
		end
	end

	if #targets > 0 then
		local function cmp_k(a, b)
			local v1 = a:getPlayerNumWithSameKingdom("AI")
			local v2 = b:getPlayerNumWithSameKingdom("AI")
			return v1 > v2
		end
		table.sort(targets, cmp_k)
		local target = targets[1]
		if isEnemy then
			target = nil
			targets = sgs.reverse(targets)
			for _, t in ipairs(targets) do
				local v = 0
				for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					if p:isFriendWith(t) then
						if p:isWounded() then
							v = v - 2
						else
							v = v + 1
						end
					end
				end
				if v > 0 then target = t break end
			end
		end
		if target then
			use.card = card
			if use.to then use.to:append(target) end
			return
		end
	end
end

sgs.ai_skill_choice["alliance_feast"] = function(self, choices)
	if self.player:isWounded() then
		return "recover"
	else
		return "draw"
	end
end
sgs.ai_use_value.AllianceFeast = 7.5
sgs.ai_use_priority.AllianceFeast = 8.8
sgs.ai_keep_value.AllianceFeast = 3.26

sgs.ai_nullification.AllianceFeast = function(self, card, from, to, positive, keep)
	local targets = sgs.SPlayerList()
	local players = self.room:getTag("targets" .. card:toString()):toList()
	for _, q in sgs.qlist(players) do
		targets:append(q:toPlayer())
	end
	if positive then
		if self:isEnemy(to) then
			local wounded
			for _, p in sgs.qlist(targets) do
				if p:objectName() ~= from:objectName() and (p:isWounded() or not p:faceUp()) then wounded = true end
				if p:objectName() ~= from:objectName() and self:isWeak(p) then keep = false end
			end
			if keep then return false end
			local hegnull = self:getCard("HegNullification") or (self.room:getTag("NullifyingTimes"):toInt() > 0 and self.room:getTag("NullificatonType"):toBool())
			if to:objectName() ~= from:objectName() and wounded and hegnull then
				return true, players:length() <= 1
			end
			if to:objectName() ~= from:objectName() and (to:isWounded() or not to:faceUp()) then
				return true, true
			end
			if to:objectName() == from:objectName() and to:getMark("alliance_feast") >= 2 then return true, true end
		end
	else
		if self:isFriend(to) then
			if to:objectName() ~= from:objectName() and (to:isWounded() or not to:faceUp()) then return true, true end
			if keep then return false end
			if to:objectName() == from:objectName() and to:getMark("alliance_feast") >= 2 then return true, true end
		end
	end
	return
end


--ThreatenEmperor
function SmartAI:useCardThreatenEmperor(card, use)
	if not card:isAvailable(self.player) then return end
	if self.player:getCardCount(true) < 2 then return end
	if not self:hasTrickEffective(card, self.player, self.player) then return end
	use.card = card
end
sgs.ai_use_value.ThreatenEmperor = 8
sgs.ai_use_priority.ThreatenEmperor = 0
sgs.ai_keep_value.ThreatenEmperor = 3.2

sgs.ai_nullification.ThreatenEmperor = function(self, card, from, to, positive, keep)
	if positive then
		if self:isEnemy(from) and not from:isNude() then return true, true end
	else
		if from:getCards("he"):length() == 1 and self.player:objectName() == from:objectName() then
			if self:getCard("Nullification"):getEffectiveId() == self.player:getCards("he"):first():getEffectiveId() then return false end
		end
		if self:isFriend(from) and not from:isNude() then return true, true end
	end
	return
end

sgs.ai_skill_cardask["@threaten_emperor"] = function(self)
	if self.player:isNude() then return "." end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		if not card:isKindOf("JadeSeal") then
			return card:getEffectiveId()
		end
	end
	return cards[1]:getEffectiveId()
end

--ImperialOrder
function SmartAI:useCardImperialOrder(card, use)
	if not card:isAvailable(self.player) then return end
	use.card = card
end
sgs.ai_use_value.ImperialOrder = 0
sgs.ai_use_priority.ImperialOrder = 8.9
sgs.ai_keep_value.ImperialOrder = 0

sgs.ai_nullification.ImperialOrder = function(self, card, from, to, positive)
	return
end

sgs.ai_skill_cardask["@imperial_order-equip"] = function(self)
	if self:needToThrowArmor() then
		return self.player:getArmor():getEffectiveId()
	end
	local discard
	local kingdom = self:evaluateKingdom(self.player)
	if kingdom == "unknown" then discard = true
	else
		kingdom = kingdom:split("?")
		discard = #kingdom / #sgs.KingdomsTable >= 0.5
	end
	if self.player:getPhase() == sgs.Player_NotActive and discard then
		local cards = self.player:getCards("he")
		local cards = sgs.QList2Table(self.player:getCards("he"))
			for _, card in ipairs(cards) do
				if not self:willShowForAttack() and ((card:isKindOf("Weapon") and self.player:getHandcardNum() < 3) or card:isKindOf("OffensiveHorse")) then
					return card:getEffectiveId()
				elseif not self:willShowForDefence() and ((card:isKindOf("Armor") and self.player:getHp() > 1) or card:isKindOf("DefensiveHorse")) then
					return card:getEffectiveId()
				end
			end
	end
	return "."
end

sgs.ai_skill_choice.imperial_order = function(self)
	if self.player:getPhase() ~= sgs.Player_NotActive then return "show" end
	if self:needToLoseHp() then return "losehp" end
	if not self.player:isWounded() and self.player:getCards("he"):length() > 6 then return "losehp" end
	return "show"
end


--JadeSeal
sgs.ai_skill_use["@@JadeSeal!"] = function(self, prompt, method)
	local card = sgs.cloneCard("known_both")
	local dummyuse = { isDummy = true, to = sgs.SPlayerList() }
	self:useCardKnownBoth(card, dummyuse)
	local tos = {}
	if dummyuse.card and not dummyuse.to:isEmpty() then
		for _, to in sgs.qlist(dummyuse.to) do
			table.insert(tos, to:objectName())
		end
		return "known_both:JadeSeal[no_suit:0]=.&->" .. table.concat(tos, "+")
	end
	self:sort(self.enemies, "handcard")
	self.enemies = sgs.reverse(self.enemies)
	local targets = sgs.PlayerList()
	for _, enemy in ipairs(self.enemies) do
		if self:getKnownNum(enemy, self.player) ~= enemy:getHandcardNum() and card:targetFilter(targets, enemy, self.player) and not targets:contains(enemy) then
			targets:append(enemy)
			table.insert(tos, enemy:objectName())
			self.knownboth_choice[enemy:objectName()] = "handcards"
		end
	end
	self:sort(self.friends_noself, "handcard")
	self.friends_noself = sgs.reverse(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if self:getKnownNum(friend, self.player) ~= friend:getHandcardNum() and card:targetFilter(targets, friend, self.player) and not targets:contains(friend) then
			targets:append(friend)
			table.insert(tos, friend:objectName())
			self.knownboth_choice[friend:objectName()] = "handcards"
		end
	end

	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(players, "handcard")
	players = sgs.reverse(players)
	for _, player in ipairs(players) do
		if card:targetFilter(targets, player, self.player) and not targets:contains(player) then
			targets:append(player)
			table.insert(tos, player:objectName())
			self.knownboth_choice[player:objectName()] = "handcards"
		end
	end
	assert(#tos > 0)
	return "known_both:JadeSeal[no_suit:0]=.&->" .. table.concat(tos, "+")
end

sgs.ai_use_priority.JadeSeal = 5.6
sgs.ai_keep_value.JadeSeal = 4

--Halberd
sgs.ai_view_as.Halberd = function(card, player, card_place)
	if card_place == sgs.Player_PlaceHand and card:isKindOf("Slash") and not player:hasFlag("Global_HalberdFailed") and not player:hasFlag("slashDisableExtraTarget")
		and sgs.Sanguosha:getCurrentCardUseReason() == sgs.CardUseStruct_CARD_USE_REASON_RESPONSE_USE and player:getMark("Equips_Nullified_to_Yourself") == 0 then
		return "@HalberdCard=."
	end
end

local Halberd_skill = {}
Halberd_skill.name = "Halberd"
table.insert(sgs.ai_skills, Halberd_skill)
Halberd_skill.getTurnUseCard = function(self, inclusive)
	if self.player:hasFlag("Global_HalberdFailed") or not self:slashIsAvailable() or self.player:getMark("Equips_Nullified_to_Yourself") > 0 then return end
	if self:getCard("Slash") then
		local HalberdCard = sgs.Card_Parse("@HalberdCard=.")
		assert(HalberdCard)
		return HalberdCard
	end
end

sgs.ai_skill_use_func.HalberdCard = function(card, use, self)
	local slash = self:getCard("Slash")
	self:useCardSlash(slash, use)
	if use.card then
		if use.card:isKindOf("Analeptic") then
			return
		elseif not use.card:isKindOf("Slash") then
			use.card = nil
		else
			use.card = card
		end
	end
	if use.to then use.to = sgs.SPlayerList() end
end

sgs.ai_use_priority.HalberdCard = sgs.ai_use_priority.Slash + 0.2

sgs.ai_skill_playerchosen.Halberd = sgs.ai_skill_playerchosen.slash_extra_targets

sgs.ai_skill_cardask["@Halberd"] = function(self)
	local cards = self:getCards("Slash")
	self:sortByUseValue(cards)
	for _, slash in ipairs(cards) do
		if slash:isKindOf("HalberdCard") then continue end
		local use = { to = sgs.SPlayerList() }
		local target
		if self.player:hasFlag("slashTargetFix") then
			for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if player:hasFlag("SlashAssignee") then
					if self.player:canSlash(player, slash) then
						use.to:append(player)
						target = player
						break
					else
						return "."
					end
				end
			end
		end
		self:useCardSlash(slash, use)
		if not use.card or not use.card:isKindOf("Slash") then return "." end
		local targets = {}
		for _, p in sgs.qlist(use.to) do
			table.insert(targets, p:objectName())
		end
		if #targets > 0 and (not target or table.contains(targets, target:objectName())) then return slash:toString() .. "->" .. table.concat(targets, "+") end
	end
	return "."
end

function sgs.ai_weapon_value.Halberd(self, enemy, player)
	return 2.1
end

--WoodenOx
local wooden_ox_skill = {}
wooden_ox_skill.name = "WoodenOx"
table.insert(sgs.ai_skills, wooden_ox_skill)
wooden_ox_skill.getTurnUseCard = function(self)
	self.wooden_ox_assist = nil
	if self.player:hasUsed("WoodenOxCard") or self.player:isKongcheng() or not self.player:hasTreasure("WoodenOx") then return end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	local card, friend = self:getCardNeedPlayer(cards, self.friends_noself, "WoodenOx")
	if card and friend and friend:objectName() ~= self.player:objectName() and (self:getOverflow() > 0 or self:isWeak(friend)) then
		self.wooden_ox_assist = friend
		return sgs.Card_Parse("@WoodenOxCard=" .. card:getEffectiveId())
	end
	if self:getOverflow() > 0 or (self:needKongcheng() and #cards == 1) then
		self.wooden_ox_assist = nil
		return sgs.Card_Parse("@WoodenOxCard=" .. cards[1]:getEffectiveId())
	end
end

sgs.ai_skill_use_func.WoodenOxCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_playerchosen.WoodenOx = function(self, targets)
	return self.wooden_ox_assist
end

sgs.ai_playerchosen_intention.WoodenOx = -10

sgs.ai_use_priority.WoodenOxCard = 0

--Blade
function sgs.ai_weapon_value.Blade(self, enemy, player)
	if not enemy then
		return 0
	else
		local v = 0
		if not enemy:hasShownGeneral1() then v = v + 1 end
		if not enemy:hasShownGeneral2() then v = v + 1 end
		return v
	end
end
