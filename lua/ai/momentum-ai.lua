sgs.ai_slash_prohibit.PeaceSpell = function(self, from, enemy, card)
	if from:hasShownSkill("zhiman") then return false end
	if enemy:hasArmorEffect("PeaceSpell") and card:isKindOf("NatureSlash") and not IgnoreArmor(from, enemy) and not from:hasWeapon("IceSword") then return true end
	return
end

function sgs.ai_armor_value.PeaceSpell(player, self)
	if player:hasShownSkills("hongfa+wendao") then return 1000 end
	if getCardsNum("Peach", player, player) + getCardsNum("Analeptic", player, player) == 0 and player:getHp() == 1 then
		if player:hasArmorEffect("PeaceSpell") then return 99
		else return -99
		end
	end
	return 3.5
end

sgs.ai_use_priority.PeaceSpell = 0.75

