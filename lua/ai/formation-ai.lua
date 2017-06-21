sgs.weapon_range.DragonPhoenix = 2
sgs.ai_use_priority.DragonPhoenix = 2.400
function sgs.ai_weapon_value.DragonPhoenix(self, enemy, player)
	local lordliubei = nil
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:hasShownSkill("zhangwu") then
			lordliubei = p
			break
		end
	end
	if lordliubei and player:getWeapon() and not self:hasSkill("xiaoji", player) then
		return -10
	end
	if enemy and enemy:getHp() <= 1 and (sgs.card_lack[enemy:objectName()]["Jink"] == 1 or getCardsNum("Jink", enemy, self.player) == 0) then
		return 4.1
	end
end

function sgs.ai_slash_weaponfilter.DragonPhoenix(self, to, player)
	if player:distanceTo(to) > math.max(sgs.weapon_range.DragonPhoenix, player:getAttackRange()) then return end
	return getCardsNum("Peach", to, self.player) + getCardsNum("Jink", to, self.player) < 1
		and (sgs.card_lack[to:objectName()]["Jink"] == 1 or getCardsNum("Jink", to, self.player) == 0)
end

sgs.ai_skill_invoke.DragonPhoenix = function(self, data)
	if data:toString() == "revive" then return true end
	local death = data:toDeath()
	if death.who then return true
	else
		local to = data:toPlayer()
		return self:doNotDiscard(to) == self:isFriend(to)
	end
end

sgs.ai_skill_choice.DragonPhoenix = function(self, choices, data)
	local kingdom = data:toString()
	local choices_pri = {}
	choices_t = string.split(choices, "+")
	if (kingdom == "wei") then
		if (string.find(choices, "guojia")) then
			table.insert(choices_pri,"guojia") end
		if (string.find(choices, "xunyu")) then
			table.insert(choices_pri,"xunyu") end
		if (string.find(choices, "lidian")) then
			table.insert(choices_pri,"lidian") end
		if (string.find(choices, "zhanghe")) then
			table.insert(choices_pri,"zhanghe") end
		if (string.find(choices, "caopi")) then
			table.insert(choices_pri,"caopi") end
		if (string.find(choices, "zhangliao")) then
			table.insert(choices_pri,"zhangliao") end

		table.removeOne(choices_t, "caohong")
		table.removeOne(choices_t, "zangba")
		table.removeOne(choices_t, "xuchu")
		table.removeOne(choices_t, "dianwei")
		table.removeOne(choices_t, "caoren")

	elseif (kingdom == "shu") then
		if (string.find(choices, "mifuren")) then
			table.insert(choices_pri,"mifuren") end
		if (string.find(choices, "pangtong")) then
			table.insert(choices_pri,"pangtong") end
		if (string.find(choices, "lord_liubei")) then
			table.insert(choices_pri,"lord_liubei") end
		if (string.find(choices, "liushan")) then
			table.insert(choices_pri, "liushan") end
		if (string.find(choices, "jiangwanfeiyi")) then
			table.insert(choices_pri, "jiangwanfeiyi") end
		if (string.find(choices, "wolong")) then
			table.insert(choices_pri, "wolong") end

		table.removeOne(choices_t, "guanyu")
		table.removeOne(choices_t, "zhangfei")
		table.removeOne(choices_t, "weiyan")
		table.removeOne(choices_t, "zhurong")
		table.removeOne(choices_t, "madai")

	elseif (kingdom == "wu") then
		if (string.find(choices, "zhoutai")) then
			table.insert(choices_pri, "zhoutai") end
		if (string.find(choices, "lusu")) then
			table.insert(choices_pri, "lusu") end
		if (string.find(choices, "taishici")) then
			table.insert(choices_pri, "taishici") end
		if (string.find(choices, "sunjian")) then
			table.insert(choices_pri, "sunjian") end
		if (string.find(choices, "sunshangxiang")) then
			table.insert(choices_pri, "sunshangxiang") end

		table.removeOne(choices_t, "sunce")
		table.removeOne(choices_t, "chenwudongxi")
		table.removeOne(choices_t, "luxun")
		table.removeOne(choices_t, "huanggai")

	elseif (kingdom == "qun") then
		if (string.find(choices, "yuji")) then
			table.insert(choices_pri,"yuji") end
		if (string.find(choices, "caiwenji")) then
			table.insert(choices_pri,"caiwenji") end
		if (string.find(choices, "mateng")) then
			table.insert(choices_pri,"mateng") end
		if (string.find(choices, "kongrong")) then
			table.insert(choices_pri,"kongrong") end
		if (string.find(choices, "lord_zhangjiao")) then
			table.insert(choices_pri,"lord_zhangjiao") end
		if (string.find(choices, "huatuo")) then
			table.insert(choices_pri,"huatuo") end

		table.removeOne(choices_t, "dongzhuo")
		table.removeOne(choices_t, "tianfeng")
		table.removeOne(choices_t, "zhangjiao")

	end
	
	if #choices_pri > 0 then 
		return choices_pri[math.random(1, #choices_pri)] 
	end
	if #choices_t == 0 then choices_t = string.split(choices, "+") end
	return choices_t[math.random(1, #choices_t)]
end

sgs.ai_skill_discard.DragonPhoenix = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = sgs.QList2Table(self.player:getCards("he"))

	if #to_discard == 1 then
		return {to_discard[1]:getEffectiveId()}
	end

	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_PlaceEquip then
			if card:isKindOf("SilverLion") and self.player:isWounded() then return -2 end

			if card:isKindOf("Weapon") then
				if self.player:getHandcardNum() < discard_num + 2 and not self:needKongcheng() then return 0
				else return 2 end
			elseif card:isKindOf("OffensiveHorse") then
				if self.player:getHandcardNum() < discard_num + 2 and not self:needKongcheng() then return 0
				else return 1 end
			elseif card:isKindOf("DefensiveHorse") then return 3
			elseif card:isKindOf("Armor") then
				if self.player:hasSkill("bazhen") then return 0
				else return 4 end
			else return 0 --@to-do: add the corrsponding value of Treasure
			end
		else
			if self.player:getMark("@qianxi_red") > 0 and card:isRed() and not card:isKindOf("Peach") then return 0 end
			if self.player:getMark("@qianxi_black") > 0 and card:isBlack() then return 0 end
			if self:isWeak() then return 5 else return 0 end
		end
	end

	local compare_func = function(card1, card2)
		local card1_aux = aux_func(card1)
		local card2_aux = aux_func(card2)
		if card1_aux ~= card2_aux then return card1_aux < card2_aux end
		return self:getKeepValue(card1) < self:getKeepValue(card2)
	end

	table.sort(to_discard, compare_func)

	for _, card in ipairs(to_discard) do
		if not self.player:isJilei(card) then return {card:getEffectiveId()} end
	end
end