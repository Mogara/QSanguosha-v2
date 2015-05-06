--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Rara

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

  QSanguosha-Rara
*********************************************************************]]

sgs.ai_skill_invoke.jglingfeng = true

sgs.ai_skill_playerchosen.jglingfeng = function(self, targets)
	self:updatePlayers()
	self:sort(self.enemies, "hp")
	local target
	for _, enemy in ipairs(self.enemies) do
			target = enemy
			break
	end
	return target
end

sgs.ai_playerchosen_intention.jglingfeng = 80

sgs.ai_skill_invoke.jgbiantian = true

sgs.ai_slash_prohibit.jgbiantian = function(self, from, enemy, card)
	if enemy:getMark("@fog") > 0 and not card:isKindOf("ThunderSlash") then return false end
	return true
end

sgs.ai_skill_choice.jggongshen = function(self, choice)
	self:updatePlayers()
	self:sort(self.friends_noself)
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if string.find(enemy:getGeneral():objectName(), "machine") then
			if enemy:hasArmorEffect("Vine") or enemy:getMark("@gale") > 0 or enemy:getHp() == 1 then
				return "damage"
			end
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if string.find(friend:getGeneral():objectName(), "machine") and friend:getLostHp() > 0 then
			if self:isWeak(friend) then
				return "recover"
			end
		end
	end
	return "damage"
end

sgs.ai_skill_invoke.jgzhinang = true

sgs.ai_skill_playerchosen.jgzhinang = function(self, targets)
	for _, friend in ipairs(self.friends_noself) do
		if friend:faceUp() and not self:isWeak(friend) then
			if not friend:getWeapon() or friend:hasSkills("rende|jizhi") then
				return friend
			end
		end
	end
	return self.player
end

sgs.ai_playerchosen_intention.jgzhinang = function(self, from, to)
	if not self:needKongcheng(to, true)  then sgs.updateIntention(from, to, -50) end
end

sgs.ai_skill_invoke.jgjingmiao = true

sgs.ai_skill_invoke.jgqiwu = true

sgs.ai_skill_playerchosen.jgqiwu = function(self, targets)
	local target
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if  friend:getLostHp() > 0 then
			target = friend
		end
	end
	return target
end

sgs.ai_skill_playerchosen.jgtianyun = function(self, targets)
	local target = nil
	local chained = 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasArmorEffect("Vine") or enemy:getMark("@gale") > 0 or (enemy:getCards("e"):length() >= 2) or enemy:getHp() == 1 then
			target = enemy
			break
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if self:isGoodChainTarget(enemy) then
				target = enemy
				break
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if self:damageIsEffective(enemy, sgs.DamageStruct_Fire, self.player)  then
				target = enemy
				break
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgtianyun = 80

function sgs.ai_skill_invoke.jglingyu(self, data)
	local weak = 0
	for _, friend in ipairs(self.friends) do
		if friend:getLostHp() > 0 then
			weak = weak + 1
			if self:isWeak(friend) then
				weak = weak + 1
			end
		end
	end
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if friend:hasSkills("fangzhu") then return true end
	end
	return weak >= 2
end

sgs.ai_skill_playerchosen.jgleili = function(self, targets)
	self:updatePlayers()
	self:sort(self.enemies, "hp")
	local target
	for _, enemy in ipairs(self.enemies) do
		if self:isGoodChainTarget(enemy)then
			target = enemy
			break
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if self:damageIsEffective(enemy, sgs.DamageStruct_Thunder, self.player) then
				target = enemy
				break
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			target = enemy
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgleili = 80

sgs.ai_skill_playerchosen.jgchuanyun = function(self, targets)
	self:updatePlayers()
	local target
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if  enemy:getHp() > self.player:getHp() then
			target = enemy
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgchuanyun = 80

sgs.ai_skill_playerchosen.jgfengxing =  sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_playerchosen_intention.jgfengxing = 80

sgs.ai_skill_playerchosen.jghuodi = function(self, targets)
	local target
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasSkills("jgtianyu|jgtianyun") and not enemy:faceUp() then
			target = enemy
			break
		end
	end
	if not target then
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if self:toTurnOver(enemy) and enemy:hasSkills(sgs.priority_skill)
			and not (enemy:getMark("@fog") > 0 and enemy:hasSkill("jgbiantian")) then
				target = enemy
				break
			end
		end
		if not target then
			for _, enemy in ipairs(self.enemies) do
				if self:toTurnOver(enemy)
				and not (enemy:getMark("@fog") > 0 and enemy:hasSkill("jgbiantian")) then
					target = enemy
					break
				end
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jghuodi = 80

sgs.ai_skill_invoke.jgjueji = true

sgs.ai_skill_playerchosen.jgdidong = function(self, targets)
	self:updatePlayers()
	local target
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasSkills("jgtianyu|jgtianyun") and not self:isWeak(enemy) and not enemy:faceUp() then
			target = enemy
			break
		end
	end
	if not target then
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if self:toTurnOver(enemy) and enemy:hasSkills(sgs.priority_skill) and not (enemy:getMark("@fog") > 0 and enemy:hasSkill("jgbiantian")) then
				target = enemy
				break
			end
		end
		if not target then
			for _, enemy in ipairs(self.enemies) do
				if self:toTurnOver(enemy) and not (enemy:getMark("@fog") > 0 and enemy:hasSkill("jgbiantian")) then
					target = enemy
					break
				end
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgdidong = 80

sgs.ai_skill_invoke.jglianyu = true

function sgs.ai_skill_invoke.jgdixian(self, data)
	local throw, e= 0, 0
	for _, enemy in ipairs(self.enemies) do
			e = enemy:getCards("e"):length()
			throw = throw + e
	end
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if friend:hasSkills("fangzhu") then return true end
	end
	return throw >= 3
end

sgs.ai_skill_invoke.jgkonghun = true