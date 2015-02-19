--[[
魅步: 一名其他角色的出牌阶段开始时，若你不在其攻击范围内，你可以令该角色的锦囊牌均视为【杀】,直到回合结束：
若如此做，本回合你在其攻击范围内。

穆穆: 结束阶段开始时，若你未于本回合出牌阶段内造成伤害，你可以选择一项：弃置一名角色装备区的武器牌，然后摸一张牌；
或将一名其他角色装备区的防具牌移动至你的装备区（替换原装备）。
]]

--@todo: meibu
sgs.ai_skill_invoke.meibu = function(self, data)
	local current = self.room:getCurrent()
	if self:isFriend(current) then
		for _, enemy in ipairs(self.enemies) do
			if not self:slashProhibit(nil, current, enemy) and self:hasCrossbowEffect(current) then
				return true
			end
		end
	elseif not self:hasCrossbowEffect(current)
		and (self:slashProhibit(nil, current, enemy) or self:getDamagedEffects(self.player, current, true) or self:needToLoseHp(self.player, current, true)) then
		return true
	end
	return false
end

--@todo: mumu
sgs.ai_skill_choice.mumu = function(self, choices)
	self.playerchosen_mumu = nil
	for _, friend in ipairs(self.friends_noself) do
		if friend:getArmor() and self:askForCardChosen(friend, "e", "dummyreason") then
			self.playerchosen_mumu = friend
			return "armor"
		end
	end

	if self:needToThrowArmor() then
		for _, enemy in ipairs(self.enemies) do
			if enemy:getArmor() and self:evaluateArmor(enemy:getArmor()) > 0 and not self:needToThrowArmor(enemy) then
				self.playerchosen_mumu = enemy
				return "armor"
			end
		end
	end

	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if enemy:getArmor() or enemy:getWeapon() and self.player:canDiscard(enemy, enemy:getWeapon():getEffectiveId()) then
			self.playerchosen_mumu = enemy
			return enemy:getArmor() and "armor" or "weapon"
		end
	end
	return "cancel"
end

sgs.ai_skill_playerchosen.mumu = function(self, targetlist)
	if self.playerchosen_mumu then return self.playerchosen_mumu end
	local p = math.random(0, targetlist:length() - 1)
	return targetlist:at(p)
end


--[[协穆: 阶段技。你可以弃置一张【杀】并选择一个势力：
若如此做，直到你的回合开始时，每当你成为该势力的角色的黑色牌的目标后，你摸两张牌。

纳蛮: 每当其他角色打出的【杀】因打出而置入弃牌堆时，你可以获得之。
]]

--@tudo: xiemu

local xiemu_skill = {}
xiemu_skill.name = "xiemu"
table.insert(sgs.ai_skills, xiemu_skill)
xiemu_skill.getTurnUseCard = function(self, inclusive)
	if not self.player:hasUsed("XiemuCard") then
		return sgs.Card_Parse("@XiemuCard=.")
	end
end

sgs.ai_skill_use_func.XiemuCard = function(card, use, self)
	local p = self.room:getOtherPlayers(self.player):at(math.random(0, self.player:aliveCount() - 1))
	use.card = card
	if use.to then
		use.to:append(p)
		return
	end
end

sgs.ai_use_priority.XiemuCard = 9.5

--@tudo: naman
sgs.ai_skill_invoke.naman = function(self)
	return not self:needKongcheng(self.player, true)
end


--[[
散谣: 阶段技。你可以弃置一张牌并选择一名体力值为场上最多（或之一）的角色：若如此做，该角色受到1点伤害。

制蛮: 每当你对一名角色造成伤害时，你可以防止此伤害，然后获得其装备区或判定区的一张牌。 ]]

--@tudo: sanyao
local xiemu_skill = {}
xiemu_skill.name = "xiemu"
table.insert(sgs.ai_skills, xiemu_skill)
xiemu_skill.getTurnUseCard = function(self, inclusive)
	if not self.player:hasUsed("SanyaoCard") then
		return sgs.Card_Parse("@SanyaoCard=.")
	end
end

sgs.ai_skill_use_func.SanyaoCard = function(card, use, self)
	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(players, "hp")
	players = sgs.reverse(players)
	local hp = players[1]
	for _, p in ipairs(players) do
		if p:getHp() == hp and self:damageIsEffective(p, nil, self.player) then
			if self:isFriend(p) then
				if self:getDamagedEffects(p, self.player) then
					use.card = card
					if use.to then
						use.to:append(p)
						return
					end
				end
			else
				if not self:getDamagedEffects(p, self.player) and not self:needToLoseHp(p, self.player) then
					use.card = card
					if use.to then
						use.to:append(p)
						return
					end
				end
			end
		end
	end
end

sgs.ai_use_priority.SanyaoCard = 9.5
sgs.ai_card_intention.SanyaoCard = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if self:getDamagedEffects(to, from) and not self:getDamagedEffects(to, from) then
			sgs.updateIntention(from, to, 10)
		end
	end
end

--@tudo: zhiman
sgs.ai_skill_invoke.zhiman = function(self, data)
	local dmg = data:toDamage()
	return self:askForCardChosen(dmg.to, "ej", "dummyreason")
end

