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

sgs.ai_chat = {}

function speak(to, type)
	if not sgs.GetConfig("AIChat", false) then return end
	if to:getState() ~= "robot" then return end
	if sgs.GetConfig("OriginAIDelay", 0) == 0 then return end

	if table.contains(sgs.ai_chat, type) then
		local i = math.random(1, #sgs.ai_chat[type])
		to:speak(sgs.ai_chat[type][i])
	end
end

function speakTrigger(card, from, to, event)
	if sgs.GetConfig("OriginAIDelay", 0) == 0 then return end
	if type(to) == "table" then
		for _, t in ipairs(to) do
			speakTrigger(card, from, t, event)
		end
		return
	end

	if (event == "death") and from:hasShownSkill("ganglie") then
		speak(from, "ganglie_death")
	end

	if not card then return end

	if card:isKindOf("Indulgence") and (to:getHandcardNum() > to:getHp()) then
		speak(to, "indulgence")
	elseif card:isKindOf("LeijiCard") then
		speak(from, "leiji_jink")
	elseif card:isKindOf("QuhuCard") then
		speak(from, "quhu")
	elseif card:isKindOf("Slash") and to:hasShownSkill("yiji") and (to:getHp() <= 1) then
		speak(to, "guojia_weak")
	elseif card:isKindOf("SavageAssault") and (to:hasShownSkill("kongcheng") or to:hasShownSkill("huoji")) then
		speak(to, "daxiang")
	elseif card:isKindOf("FireAttack") and to:hasShownSkill("luanji") then
		speak(to, "yuanshao_fire")
	elseif card:isKindOf("Peach") and math.random() < 0.1 then
		speak(to, "usepeach")
	end
end

sgs.ai_chat_func[sgs.SlashEffected].blindness = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local effect = data:toSlashEffect()
	if not effect.from then return end

	local chat = {"<$2$>",
				"再杀我就输了，懂吗<$53$>",
				"我跟你拼命了<$38$>",
				"<$56$>要死要死"
				}

	if self:hasCrossbowEffect(effect.from) then
		table.insert(chat, "<$54$>身中数箭")
		table.insert(chat, "<$46$>害怕了害怕了")
		table.insert(chat, "<$44$>瑟瑟发抖")
	end

	if effect.from:getMark("drank") > 0 then
		table.insert(chat, "<$59$>我要上天了，再见")
	end

	if sgs.isAnjiang(effect.to) then
		table.insert(chat, "你再杀我就要弃光所有牌了<$56$>")
		table.insert(chat, "<$5$>我不会告诉别人我是跟你一伙的")
		table.insert(chat, "这盲狙瞎了啊！<$15$>")
		table.insert(chat, "<$52$>哭了，我是和你一伙儿的呀")
		table.insert(chat, "<$29$>心痛的无法呼吸")
		if not sgs.isAnjiang(effect.from) and effect.from:getRole() ~= "careerist" then
			table.insert(chat, "<$1$>至于吗，我也是" .. sgs.Sanguosha:translate(effect.from:getKingdom()))
		end
	end

	local index = 1 + (os.time() % #chat)

	if os.time() % 10 <= 3 and not effect.to:isLord() and math.random() < 0.9 then
		effect.to:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.Death].stupid_friend = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local damage = data:toDeath().damage
	local chat = {"MDZZ<$29$>",
				"<$54$>我没有你这个队友",
				"<$8$>我现在不想说话",
				"果然自古粉切黑<$39$>",
				"<$45$>老哥，别这样",
				}
	if damage and damage.from and player:isFriendWith(damage.from) and damage.to:objectName() == player:objectName() and ((not damage.card) or (damage.card:getSkillName() ~= "lijian")) then
		local index = 1 + (os.time() % #chat)
		damage.to:speak(chat[index])
	end
end
--[[
sgs.ai_chat_func[sgs.Dying].fuck_renegade = function(self, player, data)
	local dying = data:toDying()
	local chat = {"小内，你还不跳啊，要崩盘吧",
				"9啊，不9就输了",
				"999...999...",
				"小内，我死了，你也赢不了",
				"没戏了，小内不帮忙的话，我们全部托管吧",
				}
	if (self.role=="rebel" or self.role == "loyalist") and sgs.current_mode_players["renegade"] > 0 and dying.who:objectName() == player:objectName() then
		local index = 1 + (os.time() % #chat)
		player:speak(chat[index])
	end
end
]]
sgs.ai_chat_func[sgs.EventPhaseStart].ally = function(self, player, data)
	if player:getState() ~= "robot" then return end
	if player:getPhase() == sgs.Player_Play then
		local gameProcess = sgs.gameProcess()
		if string.find(gameProcess, ">>>") then
			local kingdom = gameProcess:split(">")[1]
			if player:getKingdom() == kingdom then return end
			kingdom = sgs.Sanguosha:translate(kingdom)
			local chat = {
				"现在大" .. kingdom .. "太可怕了，大家一起抄家伙上<$25$>",
				"快砍大" .. kingdom .. "的"
			}
			if os.time() % 10 < 1 then
				player:speak(chat[math.random(1, #chat)])
			end
		end
	end
end

sgs.ai_chat_func[sgs.EventPhaseStart].beset = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local chat = {
		"围观战五渣<$26$>",
		"不要一下弄死了，慢慢来<$40$>",
		"一人一下，赶紧弄死<$22$>",
		"你投降吧，免受皮肉之苦，投降给全尸<$23$>",
	}
	if #self.enemies == 1 and player:getPhase() == sgs.Player_Start and self:getKingdomCount() == 2 and player:getPlayerNumWithSameKingdom("AI") >= 3
		and self.enemies[1]:getPlayerNumWithSameKingdom("AI") == 1 and os.time() % 10 < 4 then
		local index = 1 + (os.time() % #chat)
		player:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.CardFinished].yaoseng = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local use = data:toCardUse()
	if use.card:isKindOf("OffensiveHorse") and use.from:objectName() == player:objectName() then
		for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
			if self:isEnemy(player, p) and player:distanceTo(p) == 1 and player:distanceTo(p, 1) == 2 and math.random() < 0.2 then
				player:speak("<$38$>" .. p:screenName() .. "你往哪里跑")
				return
			end
		end
	end
end

sgs.ai_chat_func[sgs.CardFinished].analeptic = function(self, player, data)
	local use = data:toCardUse()
	if use.card and use.card:isKindOf("Analeptic") and use.card:getSkillName() ~= "zhendu" then
		local to = use.to:first()
		if to:getMark("drank") == 0 then return end
		local suit = { "spade", "heart", "club", "diamond" }
		suit = suit[math.random(1, #suit)]
		local chat = {
			"酒杀还行<$41$>",
			"喜闻乐见<$28$>",
			"前排围观<$14$>",
			"<$47$>不要砍我，我有" .. "<b><font color = 'yellow'>" .. sgs.Sanguosha:translate("jink")
				.. string.format("[<img src='image/system/log/%s.png' height = 12/>", suit) .. math.random(1, 10) .. "] </font></b>",
			"菊花一紧<$9$>"
		}
		for _, p in ipairs(sgs.robot) do
			if p:objectName() ~= to:objectName() and not p:isFriendWith(to) and math.random() < 0.2 then
				if not p:isWounded() then
					table.insert(chat, "我满血，不慌<$27$>")
				end
				p:speak(chat[math.random(1, #chat)])
				return
			end
		end
	end
end


sgs.ai_chat_func[sgs.EventPhaseStart]["ai_chat_scenario"] = function(self, player, data)
	if player:getPhase() ~= sgs.Player_Start then end
	if sgs.ai_chat_scenario then return end
	sgs.ai_chat_scenario = true
	for _, p in ipairs(sgs.robot) do
		if math.random() < 0.05 then
			if p:hasSkill("luanji") then sgs.ai_yuanshao_ArcheryAttack = {} end
		end
	end
	for _, p in ipairs(sgs.robot) do
		if player:objectName() ~= self.room:getCurrent():objectName() and math.random() < 0.1 then
			local chat = {
				"首先声明，谁砍我我砍谁<$38$>",
			}
			player:speak(chat[math.random(1, #chat)])
			return
		end
	end
end

sgs.ai_chat_func[sgs.TargetConfirmed].imperial_order = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local use = data:toCardUse()
	if use.card:isKindOf("ImperialOrder") and use.from and use.from:objectName() == player:objectName() then
			local chat = {
				"开门！查水表！<$57$>",
			}
			if player:hasSkills("duangang") then
				table.insert(chat, "都亮出来我好放大招")
			end
			if player:getKingdom() == "shu" then
				table.insert(chat, "我就看看是不是大C")
			end
			player:speak(chat[math.random(1, #chat)])
	end
end

function SmartAI:speak(cardtype, isFemale)
	if not sgs.GetConfig("AIChat", false) then return end
	if self.player:getState() ~= "robot" then return end
	if sgs.GetConfig("OriginAIDelay", 0) == 0 then return end

	if sgs.ai_chat[cardtype] then
		if type(sgs.ai_chat[cardtype]) == "function" then
			sgs.ai_chat[cardtype](self)
		elseif type(sgs.ai_chat[cardtype]) == "table" then
			if isFemale and sgs.ai_chat[cardtype .. "_female"] then cardtype = cardtype .. "_female" end
			local i = math.random(1, #sgs.ai_chat[cardtype])
			self.player:speak(sgs.ai_chat[cardtype][i])
		end
	end
end

sgs.ai_chat_func[sgs.GeneralShown].show = function(self, player, data)
	if sgs.isRoleExpose() then return end
	local name1 =  sgs.Sanguosha:translate(self.player:getGeneralName())
	local name2 =  sgs.Sanguosha:translate(self.player:getGeneral2Name())
	local kingdom = sgs.Sanguosha:translate(self.player:getKingdom())
	local chat = {
		"<$30$>强无敌",
		"<$47$>卧槽，神将！"
	}
	local chat1 = {
		"亮一个<$8$>",
		"没想到吧<$31$>"
		}
	local notshown, shown= 0, 0
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
		if  not p:hasShownOneGeneral() then
			notshown = notshown + 1
		end
		if p:hasShownOneGeneral() then
			shown = shown + 1
		end
	end
	if shown == 1 then
		table.insert(chat,"首亮一时爽")
		if sgs.GetConfig("RewardTheFirstShowingPlayer", true) then
			table.insert(chat1,"我来摸两张<$27$>")
		end
		if not self.player:hasShownSkill("heli") then
			table.insert(chat1,"你们懂不懂，渣将首亮防真红")
		else table.insert(chat,"真红要砸人啦<$58$>")
		end
	end
	if shown < 3 then
		table.insert(chat,"亮这么早，小心被打")
		table.insert(chat,"这么快就亮了？")
	end
	if notshown < 3 then
		table.insert(chat,"终于亮了<$43$>")
		table.insert(chat,"竟然憋到现在<$45$>")
	end
	if self.player:getRole() == "careerist" then
		table.insert(chat,"野了<$28$>")
		table.insert(chat,"喜闻乐见野心家<$28$>")
		table.insert(chat1,"竟然野了<$26$>")
	end
	if not self.player:hasShownAllGenerals() then
		table.insert(chat,self.player:screenName() .."原来是"..kingdom)
		table.insert(chat,"大"..kingdom.."可怕<$44$>")
	elseif self.player:hasShownAllGenerals() then
		table.insert(chat, "果然是".. self.player:screenName() .."是"..name1..name2.."<$29$>")
		table.insert(chat,"<$41$>卧槽,"..name1..name2.."!")
	end
	for _, p in ipairs(sgs.robot) do
		if p:objectName() ~= self.player:objectName() and (math.random() < 0.06 or (self.player:getRole() == "careerist" and math.random() < 0.5)) then
			p:speak(chat[math.random(1, #chat)])
		elseif p:objectName() == self.player:objectName() and (math.random() < 0.1 or shown == 1)then
			p:speak(chat1[math.random(1, #chat1)])
		end
	end
end

sgs.ai_chat_func[sgs.DamageCaused].attackAnjiang = function(self, player, data)
	if sgs.isRoleExpose() then return end
	local damage = data:toDamage()
	local chat = {
			"看看局势再说<$1$>",
			"都不亮吗？<$10$>",
			}
	local chat1= {
			"不亮就打到亮<$22$>",
			"你敢说你不是萌虎？<$37$>"
			}
	local chat2= {
			"我说了我不卖<$24$>",
			"<$1$>别打我，打明的",
			}
	if damage and not damage.to:hasShownOneGeneral() then
		if damage.to:getMaxHp() == 3 then
			table.insert(chat, "3血不卖不是N<$22$>")
		end
		for _, p in ipairs(sgs.robot) do
			if not p:hasShownOneGeneral() then
				table.insert(chat, "<$31$>你们不亮，我也不亮")
				table.insert(chat, "萌战MOD，猥琐当先<$36$>")
			end
			if p:objectName() ~= damage.to:objectName() and math.random() < 0.05 then
				p:speak(chat[math.random(1, #chat)])
			elseif p:objectName() == damage.from:objectName() and math.random() < 0.05 then
				p:speak(chat1[math.random(1, #chat1)])
			elseif p:objectName() == damage.to:objectName() and math.random() < 0.1 then
				p:speak(chat2[math.random(1, #chat2)])
			end
		end
	end
end

sgs.ai_chat.yiji = {
"再用力一点<$33$>",
"要死了啊!<$56$>"
}

sgs.ai_chat.Snatch_female = {
"打土豪分财产！<$37$>",
"顺一个<$23$>" ,
"手牌什么的最讨厌了<$31$>"
}

sgs.ai_chat.Snatch = {
"帮你减负~<$26$>",
"有本事等会儿顺我啊<$21$>"
}

sgs.ai_chat.Dismantlement_female = sgs.ai_chat.Snatch_female

sgs.ai_chat.Dismantlement = sgs.ai_chat.Snatch

sgs.ai_chat.respond_hostile = {
"心痛的，无法呼吸<$29$>",
"头顶青天<$60$>，泪满面<$52$>",
"遭不住了<$45$>"
}

sgs.ai_chat.friendly = {
"。。。"
}

sgs.ai_chat.respond_friendly = {
 "老哥，稳<$24$>"
 }

sgs.ai_chat.Duel_female = {
"问你怕未！<$40$>"
}

sgs.ai_chat.Duel = {
"不服Sala！<$38$>",
"<$27$>这波服不服"
}

sgs.ai_chat.ExNihilo = {
"稳得不行<$26$>",
"美滋滋<$20$>"
}

sgs.ai_chat.Collateral_female = {
"别以为这样就算赢了！"
}

sgs.ai_chat.Collateral = {
"你妹啊，我的刀！"
}

--indulgence
sgs.ai_chat.indulgence = {
"乐你妹<$11$>",
"诶诶诶被乐了！<$43$>"
}

sgs.ai_chat.xiehang = {
"出大的！<$17$>",
"来来来拼点了<$22$>",
"<$57$>哟，拼点吧"
}

--salvageassault
sgs.ai_chat.daxiang = {
"<$46$>南蛮可怕",
"别放A了行吗<$45$>"
}

sgs.ai_chat.guojia_weak = {
"<$55$>再卖血会卖死的",
"不敢再卖了诶诶诶诶<$50$>"
}

sgs.ai_chat.yuanshao_fire = {
"谁去打119啊",
"别别别烧了别烧了。。。",
"又烧啊，饶了我吧。。。"
}

sgs.ai_chat.usepeach = {
"不好，这桃里有毒<$53$>"
}

sgs.ai_chat.LureTiger = function(self)
	if math.random() < 0.05 then
		local chat = {
			"爆裂吧！现实！粉碎吧！精神！放逐这个世界！",
		}
		self.player:speak(chat[math.random(1, #chat)])
	end
end

sgs.ai_chat.BurningCamps = function(self)
	local x = math.random()
	if x < 0.033 then
		self.player:speak("让火焰净化一切")
	elseif x < 0.067 then
		local t = sgs.GetConfig("OriginAIDelay", 0)
		self.player:speak("火元素之王啊")
		self.room:getThread():delay(t)
		self.player:speak("藉由您所有的力量")
		self.room:getThread():delay(t)
		self.player:speak("赐与我强大的烈焰之力吧！")
		self.room:getThread():delay(t)
		self.player:speak("火烧连营~")
	elseif x < 0.1 then
		local t = sgs.GetConfig("OriginAIDelay", 0)
		self.player:speak("狂暴的火之精灵哦")
		self.room:getThread():delay(t)
		self.player:speak("将您的力量暂时给予我")
		self.room:getThread():delay(t)
		self.player:speak("您的契约者在此呼唤")
		self.room:getThread():delay(t)
		self.player:speak("爆裂吾眼前所有之物")
	end
end
