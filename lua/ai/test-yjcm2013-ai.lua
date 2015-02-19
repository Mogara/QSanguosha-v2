-- @todo: RenXin AI
sgs.ai_skill_cardask["@renxin-card"] = function(self, data, pattern)
	local dmg = data:toDamage()
	local invoke
	if self:isFriend(dmg.to) then
		if self:damageIsEffective_(dmg) and not self:getDamagedEffects(dmg.to, dmg.from, dmg.card and dmg.card:isKindOf("Slash"))
			and not self:needToLoseHp(dmg.to, dmg.from, dmg.card and dmg.card:isKindOf("Slash")) then
			invoke = true
		elseif not self:toTurnOver(self.player) then
			invoke = true
		end
	elseif self:objectiveLevel(dmg.to) == 0 and not self:toTurnOver(self.player) then
		invoke = true
	end
	if invoke then
		local equipCards = {}
		for _, c in sgs.qlist(self.player:getCards("he")) do
			if c:isKindOf("EquipCard") and self.player:canDiscard(self.player, c:getEffectiveId()) then
				table.insert(equipCards, c)
			end
		end
		if #equipCards > 0 then
			self:sortByKeepValue(equipCards)
			return equipCards[1]:getEffectiveId()
		end
	end
	return "."
end

-- @todo: JueCe AI
sgs.ai_skill_playerchosen.juece = function(self, targetlist)
	local targets = sgs.QList2Table(targetlist)
	self:sort(targets)
	local friends, enemies = {}, {}
	for _, target in ipairs(targets) do
		if self:cantbeHurt(target, self.player) or not self:damageIsEffective(target, nil, self.player) then continue end
		if self:isEnemy(target) then table.insert(enemies, target)
		elseif self:isFriend(target) then table.insert(friends, target) end
	end

	for _, enemy in ipairs(enemies) do
		if not self:getDamagedEffects(enemy, self.player) and not self:needToLoseHp(enemy, self.player) then return enemy end
	end
	for _, friend in ipairs(friends) do
		if self:getDamagedEffects(friend, self.player) or self:needToLoseHp(friend, self.player) then return friend end
	end
	return false
end
sgs.ai_playerchosen_intention.juece = function(self, from, to)
	if self:damageIsEffective(to, nil, from) and not self:getDamagedEffects(friend, self.player) and not self:needToLoseHp(friend, self.player) then
		sgs.updateIntention(from, to, 10)
	end
end

-- @todo: MieJi AI
local mieji_skill = {}
mieji_skill.name = "mieji"
table.insert(sgs.ai_skills, mieji_skill)
mieji_skill.getTurnUseCard = function(self)
	sgs.ai_use_priority.MiejiCard = 3.5
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isBlack() and c:getTypeId() == sgs.Card_TypeTrick then return sgs.Card_Parse("@MiejiCard=.") end
	end
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		local card = sgs.Sanguosha:getCard(id)
		if card:isBlack() and card:getTypeId() == sgs.Card_TypeTrick then return sgs.Card_Parse("@MiejiCard=.") end
	end
end

sgs.ai_skill_use_func.MiejiCard = function(card, use, self)
	local cards = {}
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isBlack() and c:getTypeId() == sgs.Card_TypeTrick then table.insert(cards, c) end
	end
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		cards:prepend(sgs.Sanguosha:getCard(id))
	end
	if #cards == 0 then return end
	self:sortByUseValue(cards, true)
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasSkills("kongcheng|beifa|zhiji") and enemy:getHandcardNum() <= 2 then continue end
		if not enemy:isKongcheng() and self:hasLoseHandcardEffective(enemy) and getKnownCard(enemy, self.player, "TrickCard") == 0 then
			use.card = sgs.Card_Parse("@MiejiCard=" .. cards[1]:getEffectiveId())
			if use.to then use.to:append(enemy) end
			return
		end
	end

	for _, friend in ipairs(self.friends) do
		if friend:isKongcheng() then continue end
		local known = getKnownCard(enemy, self.player, "TrickCard")
		if friend:hasSkill("zhiji") and friend:getMark("zhiji") == 0 and self:getEnemyNumBySeat(self.player, friend) <= 1
			and (known == friend:getHandcardNum() and known == 1 or known == 0 and friend:getHandcardNum() <= 2) then
			use.card = sgs.Card_Parse("@MiejiCard=" .. cards[1]:getEffectiveId())
			if use.to then use.to:append(friend) end
			sgs.ai_use_priority.MiejiCard = 0
			return
		elseif friend:hasSkills("tuntian+zaoxian") and known > 0 then
			use.card = sgs.Card_Parse("@MiejiCard=" .. cards[1]:getEffectiveId())
			if use.to then use.to:append(friend) end
			sgs.ai_use_priority.MiejiCard = 0
			return
		end
	end

end

sgs.ai_use_priority.MiejiCard = 3.5

sgs.ai_card_intention.MieJiCard = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if to:getHandcardNum() <= 2 and (to:hasSkill("zhiji") and to:getMark("zhiji") == 0 or to:hasSkills("kongcheng|beifa")) then continue end
		sgs.updateIntention(from, to, 10)
	end
end

-- @todo: FenCheng AI
local fencheng_skill = {}
fencheng_skill.name = "fencheng"
table.insert(sgs.ai_skills, fencheng_skill)
fencheng_skill.getTurnUseCard = function(self)
	if self.player:getMark("@burn") == 0 then return false end
	return sgs.Card_Parse("@FenchengCard=.")
end

sgs.ai_skill_use_func.FenchengCard = function(card, use, self)
	local value = 0
	local neutral = 0
	local damage = { from = self.player, damage = 2, nature = sgs.DamageStruct_Fire }
	local lastPlayer = self.player
	for i, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		damage.to = p
		if self:damageIsEffective_(damage) then
			if sgs.evaluatePlayerRole(p, self.player) == "neutral" then neutral = neutral + 1 end
			local v = -4
			if (self:getDamagedEffects(p, self.player) or self:needToLoseHp(p, self.player)) and getCardsNum("Peach", p, self.player) + p:getHp() > 2 then
				v = v + 6
			elseif lastPlayer:objectName() ~= self.player:objectName() and lastPlayer:getCardCount(true) < p:getCardCount(true) then
				v = v + 4
			elseif lastPlayer:objectName() == self.player:objectName() and not p:isNude() then
				v = v + 4
			end
			if self:isFriend(p) then
				value = value + v
			elseif self:isEnemy(p) then
				value = value - v
			end
			if p:isLord() and p:getHp() <= 2
				and (self:isEnemy(p, lastPlayer) and p:getCardCount(true) <= lastPlayer:getCardCount(true)
					or lastPlayer:objectName() == self.player:objectName() and (not p:canDiscard(p, "he") or p:isNude())) then
				if not self:isEnemy(p) then
					if self:getCardsNum("Peach") + getCardsNum("Peach", p, self.player) + p:getHp() <= 2 then return end
				else
					use.card = card
					return
				end
			end
		end
	end

	if neutral > self.player:aliveCount() / 2 then return end
	if value > 0 then
		use.card = card
	end
end

sgs.ai_use_priority.FenchengCard = 9.1

sgs.ai_skill_discard.fencheng = function(self, discard_num, min_num, optional, include_equip)
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	local current = self.room:getCurrent()
	local damage = { from = current, damage = 2, nature = sgs.DamageStruct_Fire }
	local to_discard = {}
	local peaches = 0
	for _, c in ipairs(cards) do
		if self.player:canDiscard(self.player, c:getEffectiveId()) then table.insert(to_discard, c:getEffectiveId()) end
		if isCard("Peach", c, self.player) then peaches = peaches + 1 end
		if #to_discard == min_num then break end
	end

	if peaches > 2 then
		return {}
	elseif peaches == 2 and self.player:getHp() > 1 then
		for _, friend in ipairs(self.friends_noself) do
			damage.to = friend
			if friend:getHp() <= 2 and self:damageIsEffective_(damage) then return {} end
		end
	end

	local nextPlayer = self.player:getNextAlive()
	if nextPlayer:isLord() and self.role ~= "rebel" and nextPlayer:getHandcardNum() < min_num
		and not self:getDamagedEffects(nextPlayer, current) and not self:needToLoseHp(nextPlayer, current) then
		if nextPlayer:getHp() + getCardsNum("Peach", nextPlayer, self.player) + self:getCardsNum("Peach") <= 2 then return {} end
		if self.player:getHp() > nextPlayer:getHp() and self.player:getHp() > 2 then return {} end
	end

-- @todo: DanShou AI
local danshou_skill = {}
danshou_skill.name = "danshou"
table.insert(sgs.ai_skills, danshou_skill)
danshou_skill.getTurnUseCard = function(self)
	local times = self.player:getMark("danshou") + 1
	if times > self.player:getCardCount(true) then return false end
	local cards = self.player:getCards("he")
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		cards:append(sgs.Sanguosha:getCard(id))
	end
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	if times == 1 then
		local has_weapon = false

		for _, card in ipairs(cards) do
			if card:isKindOf("Weapon") and card:isBlack() then has_weapon = true end
		end

		for _, card in ipairs(cards) do
			if self.player:canDiscard(self.player, card:getEffectiveId()) and ((self:getUseValue(card) < sgs.ai_use_value.Dismantlement + 1) or self:getOverflow() > 0) then
				local shouldUse = true

				if card:isKindOf("Armor") then
					if not self.player:getArmor() then shouldUse = false
					elseif self.player:hasEquip(card) and not (card:isKindOf("SilverLion") and self.player:isWounded()) then shouldUse = false
					end
				end

				if card:isKindOf("Weapon") then
					if not self.player:getWeapon() then shouldUse = false
					elseif self.player:hasEquip(card) and not has_weapon then shouldUse = false
					end
				end

				if card:isKindOf("Slash") then
					local dummy_use = { isDummy = true }
					if self:getCardsNum("Slash") == 1 then
						self:useBasicCard(card, dummy_use)
						if dummy_use.card then shouldUse = false end
					end
				end

				if self:getUseValue(card) > sgs.ai_use_value.Dismantlement + 1 and card:isKindOf("TrickCard") then
					local dummy_use = { isDummy = true }
					self:useTrickCard(card, dummy_use)
					if dummy_use.card then shouldUse = false end
				end

				if shouldUse then
					local card_id = card:getEffectiveId()
					local card_str = ("@DanshouCard=" .. card_id)
					local danshou = sgs.Card_Parse(card_str)
					assert(danshou)
					return danshou
				end
			end
		end
	elseif times == 3 then
		return sgs.Card_Parse("@DanshouCard=.")
	elseif times == 4 or times == 2 then
		local to_discard = {}
		local jink_num = self:getCardsNum("Jink")
		for _, c in ipairs(cards) do
			if not isCard("Peach", c, self.player) and self.player:canDiscard(self.player, c:getEffectiveId()) then
				if isCard("Jink", c, self.player) then
					if jink_num <= 1 then continue end
					jink_num = jink_num - 1
				end
				table.insert(to_discard, c:getEffectiveId())
				if #to_discard == times then break end
			end
		end
		if #to_discard == times then
			return sgs.Card_Parse("@DanshouCard=" .. table.concat(to_discard, "+"))
		end
	end
	return false
end

sgs.ai_skill_use_func.DanshouCard = function(card, use, self)
	local times = self.player:getMark("danshou") + 1
	if times == 1 then
		self:useCardSnatchOrDismantlement(card, use)
		return
	elseif times == 2 then
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if not self:needToThrowArmor(enemy) then
				use.card = card
				if use.to then use.to:append(enemy) end
				return
			end
		end
	elseif times == 3 then
		local cards = self.player:getCards("he")
		for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
			cards:append(sgs.Sanguosha:getCard(id))
		end
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local to_discard = {}
		local jink_num = self:getCardsNum("Jink")
		for _, c in ipairs(cards) do
			if not isCard("Peach", c, self.player) and self.player:canDiscard(self.player, c:getEffectiveId()) then
				if isCard("Jink", c, self.player) then
					if jink_num <= 1 then continue end
					jink_num = jink_num - 1
				end
				table.insert(to_discard, c:getEffectiveId())
				if #to_discard == 3 then break end
			end
		end
		if #to_discard == 3 then
			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if self:damageIsEffective(enemy, nil, self.player) and not self:getDamagedEffects(enemy, self.player) and not self:needToLoseHp(enemy, self.player)
					and (enemy:getHp() == 1 or self:getOverflow() > 1) then
					use.card = sgs.Card_Parse("@DanshouCard=" .. table.concat(to_discard, "+"))
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
	elseif times == 4 then
		local friend = self:findPlayerToDraw(false, 2)
		if friend then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

sgs.ai_use_priority.DanshouCard = sgs.ai_use_priority.Dismantlement + 1
sgs.ai_use_value.DanshouCard = sgs.ai_use_value.Dismantlement + 1
sgs.ai_card_intention.DanshouCard = function(self, card, from, tos)
	local num = card:subcardsLength()
	if num == 2 or num == 3 then
		sgs.updateIntentions(from, tos, 10)
	elseif num == 4 then
		sgs.updateIntentions(from, tos, -10)
	end
end

sgs.ai_choicemade_filter.cardChosen.danshou = sgs.ai_choicemade_filter.cardChosen.snatch

	if self.role ~= "renegade" then
		if self:isEnemy(nextPlayer) and not self:getDamagedEffects(nextPlayer, current) and not self:needToLoseHp(nextPlayer, current) then
			if self.player:getCardCount(true) >= nextPlayer:getCardCount(true)
				and (self:isFriend(current) and sgs.evaluatePlayerRole(nextPlayer) == "rebel" or sgs.evaluatePlayerRole(nextPlayer) ~= "rebel")
				and getCardsNum("Peach", nextPlayer, self.player) + nextPlayer:getHp() <= 2 then
				local discard = {}
				for _, c in ipairs(cards) do
					if self.player:canDiscard(self.player, c:getEffectiveId()) then table.insert(discard, c:getEffectiveId()) end
					if isCard("Peach", c, self.player) then peaches = peaches + 1 end
					if #discard == nextPlayer:getCardCount(true) then return discard end
				end
			end
		elseif self:isFriend(nextPlayer) then
			if sgs.evaluatePlayerRole(nextPlayer, self.player) ~= "renegade" and self.player:getHp() > nextPlayer:getHp()
				and not self:getDamagedEffects(nextPlayer, current) and not self:needToLoseHp(nextPlayer, current)
				and self.player:getHp() > 2 and nextPlayer:getCardCount(true) <= min_num then
				return {}
			end
		end
	end

	if self:getDamagedEffects(self.player, current) and not self:needToLoseHp(self.player, current)
		and self.player:getHp() + self:getCardsNum("Peach") <= 2 then return {} end

	if #to_discard == min_num then return to_discard end
	return {}
end

-- @todo: ZhuiKong AI
sgs.ai_skill_invoke.zhuikong = function(self, data)
	if self.player:getHandcardNum() <= (self:isWeak() and 3 or 1) then return false end
	local current = self.room:getCurrent()
	if not current or self:isFriend(current) then return false end

	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	if self.player:hasSkill("yingyang") then max_point = math.min(max_point + 3, 13) end
	if not (current:hasSkill("zhiji") and current:getMark("zhiji") == 0 and current:getHandcardNum() == 1) then
		local enemy_max_card = self:getMaxCard(current)
		local enemy_max_point = enemy_max_card and enemy_max_card:getNumber() or 100
		if enemy_max_card and current:hasSkill("yingyang") then enemy_max_point = math.min(enemy_max_point + 3, 13) end
		if max_point > enemy_max_point or max_point > 10 then
			self.zhuikong_card = max_card:getEffectiveId()
			return true
		end
	end
	if current:distanceTo(self.player) == 1 and not self:isValuableCard(max_card) then
		self.zhuikong_card = max_card:getEffectiveId()
		return true
	end
	return false
end

-- @todo: QiuYuan AI
sgs.ai_skill_playerchosen.qiuyuan = sgs.ai_skill_playerchosen.nosqiuyuan

sgs.ai_skill_cardask["@qiuyuan-give"] = sgs.ai_skill_cardask["@nosqiuyuan-give"]

function sgs.ai_slash_prohibit.qiuyuan(self, from, to)
	if self:isFriend(to, from) then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	for _, friend in ipairs(self:getFriendsNoself(from)) do
		if not (to:getHandcardNum() == 1 and (to:hasSkill("kongcheng") or (to:hasSkill("zhiji") and to:getMark("zhiji") == 0))) then return true end
	end
end

function SmartAI:hasQiuyuanEffect(from, to)
	if not from or not to or not to:hasSkill("qiuyuan") then return false end
	if getKnownCard(to, self.player, "Jink", true, "he") >= 1 then
		for _, target in ipairs(self:getEnemies(to)) do
			if target:getHandcardNum() ~= 1 or not self:needKongcheng(target, true) then
				return true
			end
		end
		for _, friend in ipairs(self:getFriends(to)) do
			if friend:getHandcardNum() == 1 and self:needKongcheng(friend, true) and not friend:isKongcheng() then
				return true
			end
		end
	end
end