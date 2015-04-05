sgs.ai_skill_playerchosen.bossdidong = function(self, targets)
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if enemy:faceUp() then return enemy end
	end
end

sgs.ai_skill_playerchosen.bossluolei = function(self, targets)
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if self:canAttack(enemy, self.player, sgs.DamageStruct_Thunder) then
			return enemy
		end
	end
end

sgs.ai_skill_playerchosen.bossguihuo = function(self, targets)
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if self:canAttack(enemy, self.player, sgs.DamageStruct_Fire)
			and (enemy:hasArmorEffect("vine") or enemy:getMark("@gale") > 0) then
			return enemy
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if self:canAttack(enemy, self.player, sgs.DamageStruct_Fire) then
			return enemy
		end
	end
end

sgs.ai_skill_playerchosen.bossxiaoshou = function(self, targets)
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > self.player:getHp() and self:canAttack(enemy, self.player) then
			return enemy
		end
	end
end

sgs.ai_armor_value.bossmanjia = function(card, player, self)
	if not card then return sgs.ai_armor_value.vine(player, self, true) end
end

sgs.ai_skill_invoke.bosslianyu = function(self, data)
	local value, avail = 0, 0
	for _, enemy in ipairs(self.enemies) do
		if not self:damageIsEffective(enemy, sgs.DamageStruct_Fire, self.player) then continue end
		avail = avail + 1
		if self:canAttack(enemy, self.player, sgs.DamageStruct_Fire) then
			value = value + 1
			if enemy:hasArmorEffect("vine") or enemy:getMark("@gale") > 0 then
				value = value + 1
			end
		end
	end
	return avail > 0 and value / avail >= 2 / 3
end

sgs.ai_skill_invoke.bosssuoming = function(self, data)
	local value = 0
	for _, enemy in ipairs(self.enemies) do
		if sgs.isGoodTarget(enemy, self.enemies, self) then
			value = value + 1
		end
	end
	return value / #self.enemies >= 2 / 3
end

sgs.ai_skill_playerchosen.bossxixing = function(self, targets)
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() and self:canAttack(enemy, self.player, sgs.DamageStruct_Thunder) then
			return enemy
		end
	end
end

sgs.ai_skill_invoke.bossqiangzheng = function(self, data)
	local value = 0
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() == 1 and (enemy:hasSkill("kongcheng") or (enemy:hasSkill("zhiji") and enemy:getMark("zhiji") == 0)) then
			value = value + 1
		end
	end
	return value / #self.enemies < 2 / 3
end

sgs.ai_skill_invoke.bossqushou = function(self, data)
	local sa = sgs.Sanguosha:cloneCard("savage_assault")
	local dummy_use = { isDummy = true }
	self:useTrickCard(sa, dummy_use)
	return (dummy_use.card ~= nil)
end

sgs.ai_skill_invoke.bossmojian = function(self, data)
	local aa = sgs.Sanguosha:cloneCard("archery_attack")
	local dummy_use = { isDummy = true }
	self:useTrickCard(aa, dummy_use)
	return (dummy_use.card ~= nil)
end

sgs.ai_skill_invoke.bossdanshu = function(self, data)
	if not self.player:isWounded() then return false end
	local zj = self.room:findPlayerBySkillName("guidao")
	if self.player:getHp() / self.player:getMaxHp() >= 0.5 and zj and self:isEnemy(zj) and self:canRetrial(zj) then return false end
	return true
end