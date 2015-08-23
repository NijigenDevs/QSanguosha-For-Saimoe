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

-- translation for Standard General Package

return {

	-- 魏势力
	["#caocao"] = "魏武帝",
	["caocao"] = "曹操",
	["jianxiong"] = "奸雄",
	[":jianxiong"] = "每当你受到伤害后，你可以获得造成此伤害的牌。",

	["#simayi"] = "狼顾之鬼",
	["simayi"] = "司马懿",
	["fankui"] = "反馈",
	[":fankui"] = "每当你受到伤害后，你可以获得来源的一张牌。",
	["guicai"] = "鬼才",
	[":guicai"] = "每当一名角色的判定牌生效前，你可以打出手牌代替之。",
	["@guicai-card"] = CommonTranslationTable["@askforretrial"],
	["~guicai"] = "选择一张手牌→点击确定",

	["#xiahoudun"] = "独眼的罗刹",
	["xiahoudun"] = "夏侯惇",
	["ganglie"] = "刚烈",
	[":ganglie"] = "每当你受到伤害后，你可以判定，若结果不为红桃，来源选择一项：1.弃置两张手牌；2.受到你造成的1点伤害。",

	["#zhangliao"] = "前将军",
	["zhangliao"] = "张辽",
	["tuxi"] = "突袭",
	[":tuxi"] = "摸牌阶段开始时，你可以放弃摸牌并选择一至两名有手牌的其他角色，获得这些角色的各一张手牌。",
	["@tuxi-card"] = "你可以发动“突袭”",
	["~tuxi"] = "选择 1-2 名其他角色→点击确定",

	["#xuchu"] = "虎痴",
	["xuchu"] = "许褚",
	["luoyi"] = "裸衣",
	[":luoyi"] = "摸牌阶段，你可以少摸一张牌，若如此做，每当你于此回合内使用【杀】或【决斗】对目标角色造成伤害时，此伤害+1。",
	["#LuoyiBuff"] = "%from 的“<font color=\"yellow\"><b>裸衣</b></font>”效果被触发，伤害从 %arg 点增加至 %arg2 点",

	["#guojia"] = "早终的先知",
	["guojia"] = "郭嘉",
	["tiandu"] = "天妒",
	[":tiandu"] = "每当你的判定牌生效后，你可以获得之。",
	["yiji"] = "遗计",
	[":yiji"] = "每当你受到1点伤害后，你可以观看牌堆顶的两张牌，然后将其中的一张牌交给一名角色，将另一张牌交给一名角色。",

	["#zhenji"] = "薄幸的美人",
	["zhenji"] = "甄姬",
	["illustrator:zhenji"] = "DH",
	["luoshen"] = "洛神",
	[":luoshen"] = "准备阶段开始时，你可以判定，若结果为黑色，你可以重复此流程。最后你获得所有的黑色判定牌。",
	["#luoshen-move"] = "洛神（将此牌置于处理区）",
	["qingguo"] = "倾国",
	[":qingguo"] = "你可以将一张黑色手牌当【闪】使用或打出。",

	["#xiahouyuan"] = "疾行的猎豹",
	["xiahouyuan"] = "夏侯渊",
	["shensu"] = "神速",
	[":shensu"] = "你可以跳过判定阶段和摸牌阶段，视为使用【杀】；你可以跳过出牌阶段并弃置一张装备牌，视为使用【杀】。",
	["@shensu1"] = "你可以跳过判定阶段和摸牌阶段发动“神速”",
	["@shensu2"] = "你可以跳过出牌阶段并弃置一张装备牌发动“神速”",
	["~shensu1"] = "选择【杀】的目标角色→点击确定",
	["~shensu2"] = "选择一张装备牌→选择【杀】的目标角色→点击确定",

	["#zhanghe"] = "料敌机先",
	["zhanghe"] = "张郃",
	["illustrator:zhanghe"] = "张帅",
	["qiaobian"] = "巧变",
	[":qiaobian"] = "你可以弃置一张手牌，跳过一个阶段（准备阶段和结束阶段除外），" ..
					"然后若你以此法：跳过摸牌阶段，你可以选择有手牌的一至两名其他角色，然后获得这些角色的各一张手牌；" ..
					"跳过出牌阶段，你可以将一名角色判定区/装备区里的一张牌置入另一名角色的判定区/装备区。",
	["@qiaobian-2"] = "你可以依次获得一至两名其他角色的各一张手牌",
	["@qiaobian-3"] = "你可以将场上的一张牌移动至另一名角色相应的区域内",
	["#qiaobian"] = "你可以弃置 1 张手牌跳过 <font color='yellow'><b> %arg </b></font> 阶段",
	["~qiaobian2"] = "选择 1-2 名其他角色→点击确定",
	["~qiaobian3"] = "选择一名角色→点击确定",
	["@qiaobian-to"] = "请选择移动【%arg】的目标角色",

	["#xuhuang"] = "周亚夫之风",
	["xuhuang"] = "徐晃",
	["illustrator:xuhuang"] = "Tuu.",
	["duanliang"] = "断粮",
	[":duanliang"] = "你可以将一张不为锦囊牌的黑色牌当【兵粮寸断】使用；你能对距离为2的角色使用【兵粮寸断】。",

	["#caoren"] = "大将军",
	["caoren"] = "曹仁",
	["jushou"] = "据守",
	[":jushou"] = "结束阶段开始时，你可以摸三张牌，然后叠置。",

	["#dianwei"] = "古之恶来",
	["dianwei"] = "典韦",
	["illustrator:dianwei"] = "小冷",
	["qiangxi"] = "强袭",
	[":qiangxi"] = "出牌阶段限一次，你可以失去1点体力或弃置一张武器牌，并选择你攻击范围内的一名角色，对其造成1点伤害。",

	["#xunyu"] = "王佐之才",
	["xunyu"] = "荀彧",
	["illustrator:xunyu"] = "LiuHeng",
	["quhu"] = "驱虎",
	[":quhu"] = "出牌阶段限一次，你可以与一名体力值大于你的角色拼点：当你赢后，其对其攻击范围内你选择的一名角色造成1点伤害；当你没赢后，其对你造成1点伤害。",
	["@quhu-damage"] = "请选择 %src 攻击范围内的一名角色",
	["jieming"] = "节命",
	[":jieming"] = "每当你受到1点伤害后，你可以令一名角色将手牌补至X张（X为其体力上限且至多为5）。",
	["jieming-invoke"] = "你可以发动“节命”<br/> <b>操作提示</b>: 选择一名角色→点击确定<br/>",
	["#QuhuNoWolf"] = "%from “<font color=\"yellow\"><b>驱虎</b></font>”拼点赢，由于 %to 攻击范围内没有其他角色，结算中止",

	["#caopi"] = "霸业的继承者",
	["caopi"] = "曹丕",
	["illustrator:caopi"] = "DH",
	["xingshang"] = "行殇",
	[":xingshang"] = "每当其他角色死亡时，你可以获得其所有牌。",
	["fangzhu"] = "放逐",
	[":fangzhu"] = "每当你受到伤害后，你可以令一名其他角色摸X张牌（X为你已损失的体力值），然后其叠置。",
	["fangzhu-invoke"] = "你可以发动“放逐”<br/> <b>操作提示</b>: 选择一名其他角色→点击确定<br/>",

	["#yuejin"] = "奋强突固",
	["yuejin"] = "乐进",
	["illustrator:yuejin"] = "巴萨小马",
	["xiaoguo"] = "骁果",
	[":xiaoguo"] = "其他角色的结束阶段开始时，你可以弃置一张基本牌，令其选择一项：1.弃置一张装备牌；2.受到你造成的1点伤害。",
	["@xiaoguo"] = "你可以弃置一张基本牌发动“骁果”",
	["@xiaoguo-discard"] = "请弃置一张装备牌，否则受到 1 点伤害",

}

