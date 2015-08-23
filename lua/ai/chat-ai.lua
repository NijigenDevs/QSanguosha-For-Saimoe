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

	local chat = {"队长，是我，别开枪，自己人.",
				"尼玛你杀我，你真是夏侯惇啊",
				"再杀我一下，老子和你拼命了",
				"信不信等下我砍死你"
				}

	if self:hasCrossbowEffect(effect.from) then
		table.insert(chat, "杀得我也是醉了。。。")
		table.insert(chat, "果然是连弩降智商呀。")
		table.insert(chat, "杀死我也没牌拿，真2")
	end

	if effect.from:getMark("drank") > 0 then
		table.insert(chat, "喝醉了吧，乱砍人？")
	end

	if sgs.isAnjiang(effect.to) then
		table.insert(chat, "尼玛眼瞎了，老子是和你一伙的")
		table.insert(chat, "老大别打我，我现在不方便暴露")
		table.insert(chat, "别再杀我，你会裸")
		table.insert(chat, "盲狙一时爽啊, 我泪奔啊")
		table.insert(chat, "我次奥，哥们，盲狙能不能轻点？")
		if not sgs.isAnjiang(effect.from) and effect.from:getRole() ~= "careerist" then
			table.insert(chat, "杀你妹啊，我也是" .. sgs.Sanguosha:translate(effect.from:getKingdom()))
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
	local chat = {"2B了吧，老子和你是一伙的还杀我",
				"你这个逼装得太厉害了",
				"房主下盘T了这个2货，拉黑不解释",
				"还有更2的吗",
				"真的很无语",
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
				"现在" .. kingdom .. "国比较猖狂，我们应该联合起来",
				"不要乱砍了，砍" .. kingdom .. "的"
			}
			if os.time() % 10 < 1 then
				player:speak(chat[math.random(1, #chat)])
			end
		end
	end
end

sgs.ai_chat_func[sgs.EventPhaseStart].comeon = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local chat = {"有货，可以来搞一下",
				"看我眼色行事",
				"没闪, 不要乱来",
				"不爽，来啊！砍我啊",
				"求杀求砍求蹂躏",
				}
	if player:getPhase() == sgs.Player_Finish and not player:isKongcheng() and player:hasShownSkill("leiji") and os.time() % 10 < 4 then
		local index = 1 + (os.time() % #chat)
		player:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.EventPhaseStart].beset = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local chat = {
		"大家一起围观一下",
		"不要一下弄死了，慢慢来",
		"速度，一人一下，弄死",
		"你投降吧，免受皮肉之苦啊，投降给全尸",
	}
	if #self.enemies == 1 and player:getPhase() == sgs.Player_Start and self:getKingdomCount() == 2 and player:getPlayerNumWithSameKingdom("AI") >= 3
		and self.enemies[1]:getPlayerNumWithSameKingdom("AI") == 1 and os.time() % 10 < 4 then
		local index = 1 + (os.time() % #chat)
		player:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.CardUsed].qinshouzhang = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local use = data:toCardUse()
	if use.card:isKindOf("Blade") and use.from and use.from:objectName() == player:objectName() then
		if player:screenName() == "SB-禽受装逼佬张" then
			player:speak("这把刀就是我爷爷传下来的，上斩逗比，下斩傻逼！")
		else
			for _, p in ipairs(sgs.robot) do
				if p:screenName() == "SB-禽受装逼佬张" then
					p:speak("敢抢我爷爷传下来的的刀，约吗？")
				end
			end
		end
	end
end

sgs.ai_chat_func[sgs.CardFinished].yaoseng = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local use = data:toCardUse()
	if use.card:isKindOf("OffensiveHorse") and use.from:objectName() == player:objectName() then
		for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
			if self:isEnemy(player, p) and player:distanceTo(p) == 1 and player:distanceTo(p, 1) == 2 and math.random() < 0.2 then
				player:speak("妖僧" .. p:screenName() .. "你往哪里跑")
				return
			end
		end
	end
end

sgs.ai_chat_func[sgs.TargetConfirmed].gounannv = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local use = data:toCardUse()
	if use.card:isKindOf("Peach") then
		local to = use.to:first()
		if to:objectName() ~= use.from:objectName() and use.from:isFemale() and to:isMale() and math.random() < 0.1
			and to:getState() == "robot" and use.from:getState() == "robot" then
			use.from:speak("复活吧，我的勇士")
			to:speak("为你而战，我的女王")
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
			"呵呵",
			"喜闻乐见",
			"前排围观，出售爆米花，矿泉水，花生，瓜子...",
			"不要砍我，我有" .. "<b><font color = 'yellow'>" .. sgs.Sanguosha:translate("jink")
				.. string.format("[<img src='image/system/log/%s.png' height = 12/>", suit) .. math.random(1, 10) .. "] </font></b>",
			"我菊花一紧"
		}
		for _, p in ipairs(sgs.robot) do
			if p:objectName() ~= to:objectName() and not p:isFriendWith(to) and math.random() < 0.2 then
				if not p:isWounded() then
					table.insert(chat, "我满血，不慌")
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
				"首先声明，谁砍我我砍谁",
			}
			player:speak(chat[math.random(1, #chat)])
			return
		end
	end
end

sgs.ai_chat_func[sgs.TargetConfirmed].UnlimitedBladeWorks = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local use = data:toCardUse()
	if use.card:isKindOf("ArcheryAttack") and player:hasSkill("luanji") and use.from and use.from:objectName() == player:objectName() and sgs.ai_yuanshao_ArcheryAttack then
		if #sgs.ai_yuanshao_ArcheryAttack == 0 then
			sgs.ai_yuanshao_ArcheryAttack = {
				"此身，为剑所成",
				"血如钢铁，心似琉璃",
				"跨越无数战场而不败",
				"未尝一度被理解",
				"亦未尝一度有所得",
				"剑之丘上，剑手孤单一人，沉醉于辉煌的胜利",
				"铁匠孑然一身，执著于悠远的锻造",
				"因此，此生没有任何意义",
				"那么，此生无需任何意义",
				"这身体，注定由剑而成"
			}
		end
		player:speak(sgs.ai_yuanshao_ArcheryAttack[1])
		table.remove(sgs.ai_yuanshao_ArcheryAttack, 1)
	end
end

sgs.ai_chat_func[sgs.TargetConfirmed].imperial_order = function(self, player, data)
	if player:getState() ~= "robot" then return end
	local use = data:toCardUse()
	if use.card:isKindOf("ImperialOrder") and use.from and use.from:objectName() == player:objectName() then
			local chat = {
				"开门！查水表！",
			}
			if player:hasSkills("xiongyi|luanwu") then
				table.insert(chat, "都亮出来我好放大招")
			end
			if player:getKingdom() == "wei" then
				table.insert(chat, "我就看看是不是大魏")
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

sgs.ai_chat_func[sgs.CardFinished].duoshi = function(self, player, data)
	local use = data:toCardUse()
	if use.card:isKindOf("AwaitExhausted") and use.card:getSkillName() == "duoshi" and use.from:usedTimes("ViewAsSkill_duoshiCard") >= 2 then
		local chat = {
			"又刷牌了",
			"快点吧",
			"陆逊不要拖时间"
		}
		for _, p in ipairs(sgs.robot) do
			if p:objectName() ~= use.from:objectName() and math.random() < 0.8 then
				if p:hasSkill("xiaoji") then
					p:speak("继续继续")
				end
				p:speak(chat[math.random(1, #chat)])
				return
			end
		end
	end
end

sgs.ai_chat_func[sgs.GeneralShown].show = function(self, player, data)
	if sgs.isRoleExpose() then return end
	local name1 =  sgs.Sanguosha:translate(self.player:getGeneralName())
	local name2 =  sgs.Sanguosha:translate(self.player:getGeneral2Name())
	local kingdom = sgs.Sanguosha:translate(self.player:getKingdom())
	local chat = {
		"好牛逼的武将呀",
		"干脆全都亮了吧"
	}
	local chat1 = {
		"亮一个",
		"没想到吧"
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
			table.insert(chat1,"我来摸两张")
		end
		if not self.player:hasShownSkill("luanji") then
			table.insert(chat1,"你们懂不懂，渣将首亮防袁绍")
		else table.insert(chat,"大嘴你妹")
		end
	end
	if shown < 3 then
		table.insert(chat,"亮这么早，小心被打")
		table.insert(chat,"这么快就亮了？")
	end
	if notshown < 3 then
		table.insert(chat,"终于亮了")
		table.insert(chat,"竟然憋到现在")
	end
	if self.player:getRole() == "careerist" then
		table.insert(chat,"野了")
		table.insert(chat,"喜闻乐见野心家")
		table.insert(chat1,"竟然野了")
	end
	if not self.player:hasShownAllGenerals() then
		table.insert(chat,self.player:screenName() .."原来是"..kingdom)
		table.insert(chat,"看来这是大"..kingdom.."的节奏")
	elseif self.player:hasShownAllGenerals() then
		table.insert(chat, "我就说".. self.player:screenName() .."是"..name1..name2.."吧")
		table.insert(chat,"卧槽,"..name1..name2.."!")
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
			"看看局势再说",
			"都不亮吗？",
			}
	local chat1= {
			"不亮就打到亮",
			"你敢说你不是郭嘉？"
			}
	local chat2= {
			"我说了我不卖",
			"别打我，打明的",
			}
	if damage and not damage.to:hasShownOneGeneral() then
		if damage.to:getMaxHp() == 3 then
			table.insert(chat, "3血不卖不是魏")
		end
		for _, p in ipairs(sgs.robot) do
			if not p:hasShownOneGeneral() then
				table.insert(chat, "你们不亮，我也不亮")
				table.insert(chat, "国战就是要猥琐")
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

sgs.ai_chat_func[sgs.EventPhaseStart].luanwu = function(self, player, data)
	if player:getPhase() == sgs.Player_Play then
		local chat = {
			"乱一个，乱一个",
			"要乱了",
			"完了，没杀"
		}
		local chat1 = {
			"不要紧张",
			"准备好了吗？",
			"我凭什么听你的"
		}
		if self.player:hasShownSkill("luanwu") and self.player:getMark("@chaos") > 0 then
			for _, p in ipairs(sgs.robot) do
				if p:objectName() ~= player:objectName() and math.random() < 0.2 then
					p:speak(chat[math.random(1, #chat)])
				elseif p:objectName() == player:objectName() and math.random() < 0.1 then
					p:speak(chat1[math.random(1, #chat1)])
				end
			end
		end
	end
end

sgs.ai_chat_func[sgs.GeneralShown].show_jiange = function(self, player, data)
	if self.room:getMode() ~= "jiange_defense" then return end
	local kingdom = self.player:getKingdom()
	local chat1 = {
		"无知小儿，报上名来，饶你不死！",
		"剑阁乃险要之地，诸位将军须得谨慎行事。"
		}
	local chat2 = {
		"嗷~！",
		"呜~！",
		"发动机已启动，随时可以出发。。。"
		}
	if kingdom == "shu" then
		table.insert(chat1, "人在塔在！")
		table.insert(chat1, "汉室存亡，在此一战！")
		table.insert(chat2, "红色！")
	elseif kingdom == "wei" then
		table.insert(chat1, "众将官，剑阁去者！")
		table.insert(chat1, "此战若胜，大业必成！")
		table.insert(chat2, "蓝色！")
	end
	if string.find(self.player:getGeneral():objectName(), "baihu") then
		table.insert(chat2, "喵~！")
	end
	if string.find(self.player:getGeneral():objectName(), "jiangwei") then
		table.insert(chat1, "白水地狭路多，非征战之所，不如且退，去救剑阁。若剑阁一失，是绝路也。")
		table.insert(chat1, "今四面受敌，粮道不同，不如退守剑阁，再作良图。")
	elseif string.find(self.player:getGeneral():objectName(), "dengai") then
		table.insert(chat1, "剑阁之守必还赴涪，则会方轨而进；剑阁之军不还，则应涪之兵寡矣。")
		table.insert(chat1, "以愚意度之，可引一军从阴平小路出汉中德阳亭，用奇兵径取成都，姜维必撤兵来救，将军乘虚就取剑阁，可获全功。")
	end
	for _, p in ipairs(sgs.robot) do
		if p:objectName() == self.player:objectName() and (string.find(self.player:getGeneral():objectName(), "jg_") or math.random() < 0.1) then
			if string.find(self.player:getGeneral():objectName(), "machine") then
			p:speak(chat2[math.random(1, #chat2)])
			else
			p:speak(chat1[math.random(1, #chat1)])
			end
		end
	end
end

sgs.ai_chat.yiji = {
"再用力一点",
"要死了啊!"
}

sgs.ai_chat.Snatch_female = {
"啧啧啧，来帮你解决点手牌吧",
"叫你欺负人!" ,
"手牌什么的最讨厌了"
}

sgs.ai_chat.Snatch = {
"yoooo少年，不来一发么",
"果然还是看你不爽",
"我看你霸气外露，不可不防啊"
}

sgs.ai_chat.Dismantlement_female = sgs.ai_chat.Snatch_female

sgs.ai_chat.Dismantlement = sgs.ai_chat.Snatch

sgs.ai_chat.respond_hostile = {
"擦，小心菊花不保",
"内牛满面了", "哎哟我去"
}

sgs.ai_chat.friendly = {
"。。。"
}

sgs.ai_chat.respond_friendly = {
 "谢了。。。"
 }

sgs.ai_chat.Duel_female = {
"哼哼哼，怕了吧"
}

sgs.ai_chat.Duel = {
"来吧！像男人一样决斗吧！",
"我只用一张牌就能搞死你"
}

sgs.ai_chat.ExNihilo = {
"哎哟运气好",
"哈哈哈哈哈"
}

sgs.ai_chat.Collateral_female = {
"别以为这样就算赢了！"
}

sgs.ai_chat.Collateral = {
"你妹啊，我的刀！"
}

sgs.ai_chat.jijiang_female = {
"别指望下次我会帮你哦"
}

sgs.ai_chat.jijiang = {
"主公，我来啦"
}

--huanggai
sgs.ai_chat.KurouCard = {
"有桃么!有桃么？",
"教练，我想要摸桃",
"桃桃桃我的桃呢",
"求桃求连弩各种求"
}

--indulgence
sgs.ai_chat.indulgence = {
"乐，乐你妹啊乐",
"擦，乐我",
"诶诶诶被乐了！"
}

--leiji
sgs.ai_chat.leiji_jink = {
"我有闪我会到处乱说么？",
"你觉得我有木有闪啊",
"哈我有闪"
}

--quhu
sgs.ai_chat.quhu = {
"出大的！",
"来来来拼点了",
"哟，拼点吧"
}

--salvageassault
sgs.ai_chat.daxiang = {
"好多大象啊！",
"擦，孟获你的宠物又调皮了",
"内牛满面啊敢不敢少来点AOE"
}

--xiahoudun
sgs.ai_chat.ganglie_death = {
"菊花残，满地伤。。。"
}

sgs.ai_chat.guojia_weak = {
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}

sgs.ai_chat.yuanshao_fire = {
"谁去打119啊",
"别别别烧了别烧了。。。",
"又烧啊，饶了我吧。。。"
}

--xuchu
sgs.ai_chat.luoyi = {
"不脱光衣服干不过你"
}

sgs.ai_chat.usepeach = {
"不好，这桃里有屎"
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
