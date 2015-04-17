
function sgs.ai_skill_invoke.hengjiang(self, data)
	local target = data:toPlayer()
	if self:isEnemy(target) then
		return true
	else
		if hasManjuanEffect(self.player) then return false end
		if target:getPhase() > sgs.Player_Discard then return true end
		if target:hasSkill("yongsi") then return false end
		if target:hasSkill("keji") and not target:hasFlag("KejiSlashInPlayPhase") then return true end
		return target:getHandcardNum() <= target:getMaxCards() - 2
	end
end

sgs.ai_choicemade_filter.skillInvoke.hengjiang = function(self, player, promptlist)
	if promptlist[3] == "yes" then
		local current = self.room:getCurrent()
		if current and current:getPhase() <= sgs.Player_Discard
			and not (current:hasSkill("keji") and not current:hasFlag("KejiSlashInPlayPhase")) and current:getHandcardNum() > current:getMaxCards() - 2 then
			sgs.updateIntention(player, current, 50)
		end
	end
end

sgs.ai_skill_invoke.guixiu = function(self, data)
	return self:isWeak() and not self:willSkipPlayPhase()
end

sgs.ai_skill_invoke.guixiu_rec = function()
	return true
end

local cunsi_skill = {}
cunsi_skill.name = "cunsi"
table.insert(sgs.ai_skills, cunsi_skill)
cunsi_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@CunsiCard=.")
end

sgs.ai_skill_use_func.CunsiCard = function(card, use, self)
	if sgs.turncount <= 2 and self.player:aliveCount() > 2 and #self.friends_noself == 0 then return end
	local to, manjuan
	for _, friend in ipairs(self.friends_noself) do
		if not hasManjuanEffect(friend) then
			to = friend
			break
		else
			manjuan = friend
		end
	end
	if not to and manjuan then to = manjuan end
	if not to then to = self.player end
	if self.player:getMark("guixiu") >= 1 then
		use.card = sgs.Card_Parse("@GuixiuCard=.")
		return
	else
		use.card = card
		if use.to then use.to:append(to) end
	end
end

sgs.ai_skill_use_func.GuixiuCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_invoke.yongjue = function(self, data)
	local player = data:toPlayer()
	return player and self:isFriend(player) and not (self:needKongcheng(player, true) and not self:hasCrossbowEffect(player))
end

sgs.ai_use_value.CunsiCard = 10
sgs.ai_use_priority.CunsiCard = 10.1
sgs.ai_use_priority.GuixiuCard = sgs.ai_use_priority.CunsiCard


sgs.ai_skill_invoke.fengshi = function(self, data)
	local target = data:toPlayer()
	if not target then return end
	if target:hasSkills(sgs.lose_equip_skill) then return self:isFriend(target) end
	return not self:isFriend(target)
end

sgs.ai_choicemade_filter.skillInvoke.fengshi = function(self, player, promptlist)
	if promptlist[3] == "yes" then
	end
end

sgs.ai_skill_invoke.chuanxin = function(self, data)
	local damage = data:toDamage()
	local invoke
	local to = damage.to
	if to:getMark("chuanxin_" .. self.player:objectName()) == 0 then
		for _, skill in sgs.qlist(to:getVisibleSkillList()) do
			if string.find("benghua|shiyong", skill:objectName()) then return self:isFriend(to) end
		end
		invoke = true
	end
	if to:getEquips():length() > 0 then
		if to:hasSkills(sgs.lose_equip_skill) then return self:isFriend(to) end
		invoke = true
	end
	return invoke and not self:isFriend(to)
end

sgs.ai_choicemade_filter.skillInvoke.chuanxin = function(self, player, promptlist)
	if promptlist[3] == "yes" then
	end
end

sgs.ai_skill_choice.chuanxin = function(self, choices, data)
	if self.player:hasSkills("benghuai|shiyong") then return "detach"
	elseif self.player:hasSkills(sgs.lose_equip_skill) then return "throw"
	else return ((not self:isWeak() or self:needToThrowArmor()) and "throw") or "detach"
	end

end

sgs.ai_skill_choice.chuanxin_lose = function(self, choices, data)
	if self.player:hasSkill("benghuai") then return "benghuai"
	elseif self.player:hasSkill("shiyong") then return "shiyong"
	else
		choices = choices:split("+")
		return choices[math.random(1, #choices)]
	end
end

sgs.ai_skill_invoke.hengzheng = function(self, data)
	local value = 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		value = value + getGuixinValue(self, player)
	end
	return value >= 1.3
end

local duanxie_skill = {}
duanxie_skill.name = "duanxie"
table.insert(sgs.ai_skills, duanxie_skill)
duanxie_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("DuanxieCard") then return end
	return sgs.Card_Parse("@DuanxieCard=.")
end
sgs.ai_skill_use_func.DuanxieCard = function(card, use, self)

	self:sort(self.enemies, "handcard")

	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and enemy:isKongcheng() then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end

end

sgs.ai_card_intention.DuanxieCard = 50
sgs.ai_use_priority.DuanxieCard = 0

sgs.ai_skill_invoke.fenming = function(self, data)
	local value = 0
	local players = self.room:getAlivePlayers()
	for _, p in sgs.qlist(players) do
		if not p:isChained() or p:isKongcheng() then continue end
		local HandcardNum = p:getHandcardNum()
		local v = 1 / HandcardNum
		local v1, v2 = 0, 0
		if HandcardNum == 1 then
			if p:hasSkill("kongcheng") then v1 = v1 - 1 end
			if p:hasSkill("zhiji") and p:getMark("zhiji") == 0 then v1 = v1 - 1 end
			if p:hasSkill("beifa") then
				local canSlash
				for _, pp in ipairs(self:getEnemies(p)) do
					if pp:canSlash(friend, slash, true) then canSlash = true break end
				end
				v1 = canSlash and - 1 or 1
			end
			if p:hasSkill("hengzheng") and #self:getEnemies(p) > 2 then v1 = v1 - 1 end
		end
		local v2 = (self:getLeastHandcardNum(p) - HandcardNum - 1) or 0
		v = v + math.max(v1, v2)

		if self:isFriend(p) then value = value - v
		else value = value + v
		end
	end

	return value > 0
end

sgs.ai_skill_choice.yingyang = function(self, choices, data)
	local pindian = data:toPindian()
	local reason = pindian.reason
	local from, to = pindian.from, pindian.to
	local f_num, t_num = pindian.from_number, pindian.to_number
	local amFrom = self.player:objectName() == from:objectName()

	if math.abs(f_num - t_num) > 3 and not self.room:findPlayerBySkillName("fuzhuo") then return "cancel" end

	if reason == "mizhao" then
		if amFrom then
			if self:isFriend(to) then
				if self:getCardsNum("Jink") > 0 then return "down"
				elseif getCardsNum("Jink", to, self.player) >= 1 then return "up"
				else return self.player:getHp() >= to:getHp() and "down" or "up" end
			else
				return "up"
			end
		else
			if self:isFriend(from) then
				if self:getCardsNum("Jink") > 0 then return "down"
				elseif getCardsNum("Jink", from, self.player) >= 1 then return "up"
				else return self.player:getHp() >= to:getHp() and "down" or "up"
				end
			else return "up" end
		end
	elseif reason == "quhu" then
		if amFrom and self.player:hasSkill("jieming") then
			if f_num > 8 then return "up"
			elseif self:getJiemingChaofeng(player) <= -6 then return "down"
			end
		end
		return "up"
	elseif reason == "xiechan" then
		return not amFrom and self:getCardsNum("Slash") > getCardsNum("Slash", from, self.player) and "down" or "up"
	elseif reason == "zhiba_pindian" then
		return amFrom and self:isFriend(to) and "down" or "up"
	elseif string.find("tianyi,shuangren,qiaoshui", reason) then
		return not amFrom and self:isFriend(from) and "down" or "up"
	elseif string.find("dahe,tanhu,lieren,tanlan,jueji,xianzhen,zhuikong", reason) then
		return "up"
	else
		return "cancel"
	end
end
