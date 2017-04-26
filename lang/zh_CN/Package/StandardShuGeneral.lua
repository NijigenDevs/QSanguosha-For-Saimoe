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
	-- 蜀势力
	["#liubei"] = "乱世的枭雄",
	["liubei"] = "刘备",
	["rende"] = "仁德",
	[":rende"] = "出牌阶段，你可以将至少一张手牌交给一名角色，若你于此阶段内因发动\"仁德\"交给其他角色的手牌数不小于3，你回复1点体力（每阶段限一次）。",

	["#guanyu"] = "美髯公",
	["guanyu"] = "关羽",
	["wusheng"] = "武圣",
	[":wusheng"] = "你可以将一张红色牌当【杀】使用或打出。",

	["#zhangfei"] = "万夫不当",
	["zhangfei"] = "张飞",
	["paoxiao"] = "咆哮",
	[":paoxiao"] = "你使用【杀】无次数限制。",
	["#paoxiao-null"] = "咆哮（无视防具）",
	["paoxiao:armor_nullify"] = "场上有君刘备，你现在可以亮将令【杀】无视 %src 的防具效果。" ,
    ["paoxiaoVsCrossbow"] = "选择需要发动的技能" ,

	["#zhugeliang"] = "迟暮的丞相",
	["zhugeliang"] = "诸葛亮",
	["illustrator:zhugeliang"] = "木美人",
	["guanxing"] = "观星",
	[":guanxing"] = "准备阶段开始时，你可以观看牌堆顶的X张牌（X为全场角色数且至多为5）并改变其中任意数量的牌的顺序并将其余的牌置于牌堆底。",
	["GuanxingShowGeneral"] = "观星亮将" ,
	["kongcheng"] = "空城",
	[":kongcheng"] = "锁定技，每当你成为【杀】或【决斗】的目标时，若你没有手牌，你取消自己。",
	["#GuanxingResult"] = "%from 的“<font color=\"yellow\"><b>观星</b></font>”结果：%arg 上 %arg2 下",
	["$GuanxingTop"] = "置于牌堆顶的牌：%card",
	["$GuanxingBottom"] = "置于牌堆底的牌：%card",

	["#zhaoyun"] = "少年将军",
	["zhaoyun"] = "赵云",
	["longdan"] = "龙胆",
	[":longdan"] = "你可以将一张【杀】当【闪】使用或打出；你可以将一张【闪】当【杀】使用或打出。",

	["#machao"] = "一骑当千",
	["machao"] = "马超",
	["mashu_machao"] = "马术",
	[":mashu"] = "锁定技，你与其他角色的距离-1。",
	["tieqi"] = "铁骑",
	[":tieqi"] = "每当你使用【杀】指定一个目标后，你可以判定，若结果为红色，其不能使用【闪】响应此【杀】。",

	["#huangyueying"] = "归隐的杰女",
	["huangyueying"] = "黄月英",
	["illustrator:huangyueying"] = "木美人",
	["jizhi"] = "集智",
	[":jizhi"] = "每当你使用非转化的非延时类锦囊牌时，你可以摸一张牌。",
	["qicai"] = "奇才",
	[":qicai"] = "锁定技，你使用锦囊牌无距离限制。",

	["#huangzhong"] = "老当益壮",
	["huangzhong"] = "黄忠",
	["liegong"] = "烈弓",
	[":liegong"] = "每当你于出牌阶段内使用【杀】指定一个目标后，若其手牌数不小于你的体力值或不大于你的攻击范围，你可以令其不能使用【闪】响应此【杀】。",

	["#weiyan"] = "嗜血的独狼",
	["weiyan"] = "魏延",
	["illustrator:weiyan"] = "SoniaTang",
	["kuanggu"] = "狂骨",
	[":kuanggu"] = "每当你对一名角色造成1点伤害后，若你与其的距离于其因受到此伤害而扣减体力前不大于1，你回复1点体力。",

	["#pangtong"] = "凤雏",
	["pangtong"] = "庞统",
	["lianhuan"] = "连环",
	[":lianhuan"] = "你可以将一张梅花手牌当【铁索连环】使用；你能重铸梅花手牌。",
	["niepan"] = "涅槃",
	[":niepan"] = "限定技，当你处于濒死状态时，你可以弃置你区域里的所有牌，然后恢复至平置状态并重置，摸三张牌，将体力值回复至3点。",
	["@nirvana"] = "涅槃",

	["#wolong"] = "卧龙",
	["wolong"] = "卧龙诸葛亮",
	["&wolong"] = "诸葛亮",
	["illustrator:wolong"] = "北",
	["bazhen"] = "八阵",
	[":bazhen"] = "锁定技，若你的装备区里没有防具牌，你视为装备着【八卦阵】。",
	["huoji"] = "火计",
	[":huoji"] = "你可以将一张红色手牌当【火攻】使用。",
	["kanpo"] = "看破",
	[":kanpo"] = "你可以将一张黑色手牌当【无懈可击】使用。",

	["#liushan"] = "无为的真命主",
	["liushan"] = "刘禅",
	["illustrator:liushan"] = "LiuHeng",
	["xiangle"] = "享乐",
	[":xiangle"] = "锁定技，每当你成为其他角色使用【杀】的目标时，你令其选择是否弃置一张基本牌，若其选择否或其已死亡，此次对你结算的此【杀】对你无效。",
	["@xiangle-discard"] = "%src 触发【享乐】，请弃置一张基本牌，否则该【杀】对 %src 无效",
	["fangquan"] = "放权",
	[":fangquan"] = "你可以跳过出牌阶段，若如此做，此回合结束时，你可以弃置一张手牌并选择一名其他角色，若如此做，其获得一个额外的回合。",
	["@fangquan-discard"] = "你可以弃置一张手牌选择一名其他角色，该角色将获得一个额外的回合",
	["~fangquan"] = "选择一张手牌→选择一名其他角色→点击确定",
	["#Fangquan"] = "%to 将获得一个额外的回合",

	["#menghuo"] = "南蛮王",
	["menghuo"] = "孟获",
	["illustrator:menghuo"] = "废柴男",
	["huoshou"] = "祸首",
	[":huoshou"] = "锁定技，【南蛮入侵】对你无效；锁定技，每当其他角色使用【南蛮入侵】指定目标后，你代替其成为此【南蛮入侵】造成的伤害的来源。",
	["#sa_avoid_huoshou"] = "祸首（无效南蛮入侵）" ,
	["zaiqi"] = "再起",
	[":zaiqi"] = "摸牌阶段开始时，若你已受伤，你可以放弃摸牌，亮出牌堆顶的X张牌（X为你已损失的体力值），然后回复等同于其中红桃牌数的体力，将这些红桃牌置入弃牌堆，获得其余的牌。",
	["#HuoshouTransfer"] = "%from 的“%arg2”被触发，【<font color=\"yellow\"><b>南蛮入侵</b></font>】的伤害来源改为 %from",

	["#zhurong"] = "野性的女王",
	["zhurong"] = "祝融",
	["illustrator:zhurong"] = "废柴男",
	["juxiang"] = "巨象",
	[":juxiang"] = "锁定技，【南蛮入侵】对你无效；锁定技，每当其他角色使用的【南蛮入侵】因结算完毕而置入弃牌堆后，你获得之。",
	["#sa_avoid_juxiang"] = "巨象（无效南蛮入侵）" ,
	["lieren"] = "烈刃",
	[":lieren"] = "每当你使用【杀】对目标角色造成伤害后，你可以与其拼点，当你赢后，你获得其一张牌。",

	["#ganfuren"] = "昭烈皇后",
	["ganfuren"] = "甘夫人",
	["illustrator:ganfuren"] = "琛·美弟奇",
	["shushen"] = "淑慎",
	[":shushen"] = "每当你回复1点体力后，你可以令与你势力相同的一名其他角色摸一张牌。",
	["shushen-invoke"] = "你可以发动“淑慎”<br/> <b>操作提示</b>: 选择一名其他角色→点击确定<br/>",
	["shenzhi"] = "神智",
	[":shenzhi"] = "准备阶段开始时，你可以弃置所有手牌，然后若你以此法弃置的手牌数不小于X（X为你的体力值），你回复1点体力。",

}

