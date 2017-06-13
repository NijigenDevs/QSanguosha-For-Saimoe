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

-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	kingdoms = { "wei", "qun", "shu", "wu", "god" },
	kingdom_colors = {
		wei = "#547998",
		shu = "#D0796C",
		wu = "#4DB873",
		qun = "#8A807A",
		god = "#96943D",
	},

	skill_colors = {
		compulsory = "#0000FF",
		once_per_turn = "#008000",
		limited = "#FF0000",
		head = "#00FF00",
		deputy = "#00FFFF",
		array = "#800080",
		lord = "#FFA500",
	},

	-- Sci-fi style background
	--dialog_background_color = "#49C9F0";
	--dialog_background_alpha = 75;
	dialog_background_color = "#D6E7DB";
	dialog_background_alpha = 255;

	package_names = {
		"StandardCard",
		"FormationEquip",
		"MomentumEquip" ,
		"StrategicAdvantage",
		"TransformationEquip",

		"Moesen",
		"Test"
	},

	easy_text = {
		"太慢了，做两个俯卧撑吧！",
		"快点吧，我等的花儿都谢了！",
		"高，实在是高！",
		"好手段，可真不一般啊！",
		"哦，太菜了。水平有待提高。",
		"你会不会玩啊？！",
		"嘿，一般人，我不使这招。",
		"呵，好牌就是这么打地！",
		"杀！神挡杀神！佛挡杀佛！",
		"你也忒坏了吧？！"
	},

	robot_names = {
		"無に帰ろう", --Moligaloo
		"啦啦失恋过", --啦啦SLG
		"扶苏", --Fsu0413
		"Slob虫", --女王受·虫
		"黄焖鸡米饭", --hmqgg
		"爱撕衣", --Nagisa_Willing
		"钉子钉钉子", --钉子
		"抬翅膀的天使", --昂翼天使
		"好吃不过饺神", --水饺wch
		"企鹅企鹅企", --企鹅
		"兰博（基尼）", --兰博
		"肥羊勇决", --飞扬勇决
		"取个AI名好烦", --好烦
		"吃萝卜不如开萝卜", --果然萝卜斩
		"氯丙嗪", --阿米拉嗪
		"御坂2624", --御坂2623
		"糯米鸡" --萝莉姬
	},
}
