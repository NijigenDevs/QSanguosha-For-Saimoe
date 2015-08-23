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

-- translation for Hegemony Formation Package

return {
	["formation"] = "君临天下·阵",
	["formation_equip"] = "君临天下·阵",

	["#dengai"] = "矫然的壮士",
	["dengai"] = "邓艾",
	["illustrator:dengai"] = "Amo",
	["tuntian"] = "屯田",
	[":tuntian"] = "每当你于回合外失去牌后，你可以判定，当非红桃的判定牌生效后，你可以将之置于武将牌上，称为“田”；你与其他角色的距离-X（X为\"田\"数）。",
	["#tuntian-gotofield"] = "屯田（移出游戏）",
	["#tuntian"] = "屯田" ,
	["_tuntian:gotofield"] = "你可以将判定牌移出游戏称为“田”" ,
	["field"] = "田",
	["#tuntian-dist"] = "屯田" ,
	["jixi"] = "急袭",
	[":jixi"] = "主将技，此武将牌上单独的阴阳鱼个数-1；主将技，你可以将一张\"田\"当【顺手牵羊】使用。",
	["ziliang"] = "资粮",
	[":ziliang"] = "副将技，每当与你势力相同的一名角色受到伤害后，你可以将一张\"田\"交给该角色。",
	["@ziliang-give"] = "你可以发动“资粮”，将一张“田”交给受到伤害的角色",
	["~ziliang"] = "选择一张\"田\"→点击确定",

	["#caohong"] = "魏之福将",
	["caohong"] = "曹洪",
	["illustrator:caohong"] = "YellowKiss",
	["huyuan"] = "护援",
	[":huyuan"] = "结束阶段开始时，你可以将一张装备牌置入一名角色的装备区，若如此做，你可以弃置其距离为1的一名角色的一张牌。",
	["@huyuan-equip"] = "你可以发动“护援”",
	["@huyuan-discard"] = "请选择 %src 距离1的一名角色",
	["~huyuan"] = "选择一张装备牌→选择一名角色→点击确定",
	["heyi"] = "鹤翼",
	[":heyi"] = "阵法技，与你处于同一队列的其他角色视为拥有\"飞影\"。",
	["HeyiSummon"] = "鹤翼",
	["#heyi_feiying"] = "飞影",
	["feiying"] = "飞影",
	[":feiying"] = "锁定技，其他角色与你的距离+1。",

	["#jiangwei"] = "龙的衣钵",
	["jiangwei"] = "姜维",
	["illustrator:jiangwei"] = "木美人",
	["tiaoxin"] = "挑衅",
	[":tiaoxin"] = "出牌阶段限一次，你可以令攻击范围内含有你的一名角色选择是否对你使用【杀】，若其选择否，你弃置其一张牌。",
	["@tiaoxin-slash"] = "%src 对你发动“挑衅”，请对其使用一张【杀】",
	["yizhi"] = "遗志",
	[":yizhi"] = "副将技，此武将牌上单独的阴阳鱼个数-1；副将技，若你的主将有\"观星\"，此\"观星\"描述中的X视为5，否则你视为拥有\"观星\"。",
	["tianfu"] = "天覆",
	[":tianfu"] = "主将技，阵法技，若当前回合角色为你所在队列里的角色，你视为拥有\"看破\"。",
	["TianfuSummon"] = "天覆",

	["#jiangwanfeiyi"] = "社稷股肱",
	["jiangwanfeiyi"] = "蒋琬＆费祎",
	["&jiangwanfeiyi"] = "蒋琬费祎",
	["illustrator:jiangwanfeiyi"] = "cometrue",
	["shengxi"] = "生息",
	[":shengxi"] = "出牌阶段结束时，若你未于此阶段内造成过伤害，你可以摸两张牌。",
	["shoucheng"] = "守成",
	[":shoucheng"] = "每当与你势力相同的一名角色于其回合外失去所有手牌后，你可以令其摸一张牌。",

	["#jiangqin"] = "祁奚之器",
	["jiangqin"] = "蒋钦",
	["illustrator:jiangqin"] = "天空之城",
	["shangyi"] = "尚义",
	[":shangyi"] = "出牌阶段限一次，你可以令一名其他角色观看你的所有手牌，你选择一项：1.观看其手牌并可以弃置其中的一张黑色牌；2.观看其所有暗置的武将牌。",
	["niaoxiang"] = "鸟翔",
	[":niaoxiang"] = "阵法技，在你为围攻角色的围攻关系中，每当围攻角色使用【杀】指定一个被围攻的目标后，该围攻角色将该被围攻角色抵消此【杀】的方式改为依次使用两张【闪】。",
	["NiaoxiangSummon"] = "鸟翔",

	["#xusheng"] = "江东的铁壁",
	["xusheng"] = "徐盛",
	["illustrator:xusheng"] = "天信",
	["yicheng"] = "疑城",
	[":yicheng"] = "每当与你势力相同的一名角色成为【杀】的目标后，你可以令其摸一张牌，然后其弃置一张牌。",

	["#yuji"] = "魂绕左右",
	["yuji"] = "于吉",
	["illustrator:yuji"] = "G.G.G.",
	["qianhuan"] = "千幻",
	[":qianhuan"] = "每当与你势力相同的一名角色受到伤害后，若其存活，你可以将牌堆顶的一张牌置于武将牌上，称为\"幻\"，若此\"幻\"与另一张\"幻\"花色相同，你将此\"幻\"置入弃牌堆；" ..
					"每当与你势力相同的一名角色成为基本牌或锦囊牌的目标时，若目标数为1，你可以将一张\"幻\"置入弃牌堆，取消该角色。",
	["qianhuan:gethuan"] = "是否发动技能“千幻”将牌堆顶一张牌移出游戏成为“幻”？" ,
	--["qianhuan:canceltarget"] = "是否发动技能“千幻”取消 %arg 的目标 %dest？" ,
	["sorcery"] = "幻",
	["@qianhuan-cancel"] = "你可以发动“千幻”，取消【%arg】的目标 %dest",
	["~qianhuan"] = "选择一张\"幻\"→点击确定",

	["#hetaihou"] = "弄权之蛇蝎",
	["hetaihou"] = "何太后",
	["illustrator:hetaihou"] = "KayaK, 木美人",
	["zhendu"] = "鸩毒",
	[":zhendu"] = "其他角色的出牌阶段开始时，你可以弃置一张手牌，令其视为以方法Ⅰ使用【酒】，然后你对其造成1点伤害。",
	["@zhendu-discard"] = "你可以弃置一张手牌发动“鸩毒”",
	["qiluan"] = "戚乱",
	[":qiluan"] = "一名角色的回合结束后，若你于此回合内杀死过角色，你可以摸三张牌。",

	["#lord_liubei"] = "龙横蜀汉",
	["lord_liubei"] = "刘备-君",
	["&lord_liubei"] = "刘备" ,
	["illustrator:lord_liubei"] = "LiuHeng",
	["zhangwu"] = "章武",
	[":zhangwu"] = "锁定技，每当【飞龙夺凤】置入弃牌堆或其他角色的装备区后，你获得之；" ..
				   "锁定技，每当你失去【飞龙夺凤】前，你展示之，然后将此牌移动的目标区域改为牌堆底，若如此做，当此牌置于牌堆底后，你摸两张牌。",
	["shouyue"] = "授钺",
	[":shouyue"] = "君主技，锁定技，你拥有\"五虎将大旗\"。\n\n#\"五虎将大旗\"\n" ..
					"存活的蜀势力角色拥有的下列五个技能分别调整为：\n" ..
					"武圣——你可以将一张牌当【杀】使用或打出。\n" ..
					"咆哮——你使用【杀】无次数限制；每当你使用【杀】指定一个目标后，你无视其防具。\n" ..  --remove the detail of Qinggang-like effect
					"龙胆——你可以将一张【杀】当【闪】，或一张【闪】当【杀】使用或打出；每当你发动【龙胆①】使用或打出牌时，你摸一张牌。\n" ..
					"烈弓——每当你于出牌阶段内使用【杀】指定一个目标后，若其手牌数不小于你的体力值或不大于你的攻击范围，你可以令其不能使用【闪】响应此【杀】；你的攻击范围+1。\n" ..
					"铁骑——每当你使用【杀】指定一个目标后，你可以判定，若结果不为黑桃，其不能使用【闪】响应此【杀】。",
	["jizhao"] = "激诏",
	[":jizhao"] = "限定技，当你处于濒死状态时，你可以将手牌补至X张（X为你的体力上限），然后将体力值回复至2点，失去\"授钺\"并获得\"仁德\"。",
	["@jizhao"] = "激诏",

	["DragonPhoenix"] = "飞龙夺凤",
	[":DragonPhoenix"] = "装备牌·武器\n\n攻击范围：2\n技能：每当你使用【杀】指定一个目标后，你可以令其弃置一张牌；每当被你使用【杀】杀死的角色死亡后，若你的势力是角色最少的势力，你可以令扮演其的玩家选择是否从未使用的武将牌中选择与你势力相同的一张武将牌重新加入游戏。",
	["@dragonphoenix-discard"] = "受到【飞龙夺凤】效果影响，请弃置一张牌",
	["DragonPhoenix:revive"] = "你可以点击下方确定按钮重新复活加入战斗",

}