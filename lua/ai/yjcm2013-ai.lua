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

sgs.ai_skill_invoke.jingce = function(self, data)
	return not self:needKongcheng(self.player, true)
end

local junxing_skill = {}
junxing_skill.name = "junxing"
table.insert(sgs.ai_skills, junxing_skill)
junxing_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() or self.player:hasUsed("JunxingCard") then return nil end
	return sgs.Card_Parse("@JunxingCard=.")
end

sgs.ai_skill_use_func.JunxingCard = function(card, use, self)
	-- find enough cards
	local unpreferedCards = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	local use_slash_num = 0
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		if card:isKindOf("Slash") then
			local will_use = false
			if use_slash_num <= sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_Residue, self.player, card) then
				local dummy_use = { isDummy = true }
				self:useBasicCard(card, dummy_use)
				if dummy_use.card then
					will_use = true
					use_slash_num = use_slash_num + 1
				end
			end
			if not will_use then table.insert(unpreferedCards, card:getId()) end
		end
	end
	local num = self:getCardsNum("Jink") - 1
	if self.player:getArmor() then num = num + 1 end
	if num > 0 then
		for _, card in ipairs(cards) do
			if card:isKindOf("Jink") and num > 0 then
				table.insert(unpreferedCards, card:getId())
				num = num - 1
			end
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("EquipCard") then
			local dummy_use = { isDummy = true }
			self:useEquipCard(card, dummy_use)
			if not dummy_use.card then table.insert(unpreferedCards, card:getId()) end
		end
	end
	for _, card in ipairs(cards) do
		if card:isNDTrick() or card:isKindOf("Lightning") then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if not dummy_use.card then table.insert(unpreferedCards, card:getId()) end
		end
	end
	local use_cards = {}
	for index = #unpreferedCards, 1, -1 do
		if not self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.insert(use_cards, unpreferedCards[index]) end
	end
	if #use_cards == 0 then return end

	-- to friends
	self:sort(self.friends_noself, "defense")
	for _, friend in ipairs(self.friends_noself) do
		if not self:toTurnOver(friend, #use_cards) then
			use.card = sgs.Card_Parse("@JunxingCard=" .. table.concat(use_cards, "+"))
			if use.to then use.to:append(friend) end
			return
		end
	end
	if #use_cards >= 3 then
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHandcardNum() <= 1 and not self:needKongcheng(friend) then
				use.card = sgs.Card_Parse("@JunxingCard=" .. table.concat(use_cards, "+"))
				if use.to then use.to:append(friend) end
				return
			end
		end
	end

	-- to enemies
	local basic, trick, equip
	for _, id in ipairs(use_cards) do
		local typeid = sgs.Sanguosha:getEngineCard(id):getTypeId()
		if not basic and typeid == sgs.Card_TypeBasic then basic = id
		elseif not trick and typeid == sgs.Card_TypeTrick then trick = id
		elseif not equip and typeid == sgs.Card_TypeEquip then equip = id
		end
		if basic and trick and equip then break end
	end
	self:sort(self.enemies, "handcard")
	local other_enemy
	for _, enemy in ipairs(self.enemies) do
		local id = nil
		if self:toTurnOver(enemy, 1) then
			if getKnownCard(enemy, self.player, "BasicCard") == 0 then id = equip or trick end
			if not id and getKnownCard(enemy, self.player, "TrickCard") == 0 then id = equip or basic end
			if not id and getKnownCard(enemy, self.player, "EquipCard") == 0 then id = trick or basic end
			if id then
				use.card = sgs.Card_Parse("@JunxingCard=" .. id)
				if use.to then use.to:append(enemy) end
				return
			elseif not other_enemy then
				other_enemy = enemy
			end
		end
	end
	if other_enemy then
		use.card = sgs.Card_Parse("@JunxingCard=" .. use_cards[1])
		if use.to then use.to:append(other_enemy) end
		return
	end
end

sgs.ai_use_priority.JunxingCard = 1.2
sgs.ai_card_intention.JunxingCard = function(self, card, from, tos)
	local to = tos[1]
	if not to:faceUp() then
		sgs.updateIntention(from, to, -80)
	else
		if to:getHandcardNum() <= 1 and card:subcardsLength() >= 3 then
			sgs.updateIntention(from, to, -40)
		else
			sgs.updateIntention(from, to, 80)
		end
	end
end

sgs.ai_skill_cardask["@junxing-discard"] = function(self, data, pattern)
	local manchong = self.room:findPlayerBySkillName("junxing")
	if manchong and self:isFriend(manchong) then return "." end

	local types = pattern:split("|")[1]:split(",")
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		if not self:isValuableCard(card) then
			for _, classname in ipairs(types) do
				if card:isKindOf(classname) then return "$" .. card:getEffectiveId() end
			end
		end
	end
	return "."
end

sgs.ai_skill_cardask["@yuce-show"] = function(self, data)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if not damage.from or damage.from:isDead() then return "." end
	if self:isFriend(damage.from) then return "$" .. self.player:handCards():first() end
	local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), damage.from:objectName())
	local types = { sgs.Card_TypeBasic, sgs.Card_TypeEquip, sgs.Card_TypeTrick }
	for _, card in sgs.qlist(damage.from:getHandcards()) do
		if card:hasFlag("visible") or card:hasFlag(flag) then
			table.removeOne(types, card:getTypeId())
		end
		if #types == 0 then break end
	end
	if #types == 0 then types = { sgs.Card_TypeBasic } end
	for _, card in sgs.qlist(self.player:getHandcards()) do
		for _, cardtype in ipairs(types) do
			if card:getTypeId() == cardtype then return "$" .. card:getEffectiveId() end
		end
	end
	return "$" .. self.player:handCards():first()
end

sgs.ai_skill_cardask["@yuce-discard"] = function(self, data, pattern, target)
	if target and self:isFriend(target) then return "." end
	local types = pattern:split("|")[1]:split(",")
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards)
	for _, card in ipairs(cards) do
		if not self:isValuableCard(card) then
			for _, classname in ipairs(types) do
				if card:isKindOf(classname) then return "$" .. card:getEffectiveId() end
			end
		end
	end
	return "."
end

sgs.ai_skill_use["@@xiansi"] = function(self, prompt)
	local crossbow_effect
	if not self.player:getTag("HuashenSkill"):toString() == "xiansi" then
		for _, enemy in ipairs(self.enemies) do
			if enemy:inMyAttackRange(self.player) and (self:hasCrossbowEffect(enemy) or getKnownCard(enemy, self.player, "Crossbow") > 0) then
				crossbow_effect = true
				break
			end
		end
	end
	local max_num = 999
	if crossbow_effect then max_num = 3
	elseif self:getCardsNum("Jink") < 1 or self:isWeak() then max_num = 5 end
	if self.player:getPile("counter"):length() >= max_num then return "." end
	local rest_num = math.min(2, max_num - self.player:getPile("counter"):length())
	local targets = {}

	local add_player = function(player, isfriend)
		if player:getHandcardNum() == 0 or player:objectName() == self.player:objectName() then return #targets end
		if self:objectiveLevel(player) == 0 and player:isLord() and sgs.current_mode_players["rebel"] > 1 then return #targets end
		if #targets == 0 then
			table.insert(targets, player:objectName())
		elseif #targets == 1 then
			if player:objectName() ~= targets[1] then
				table.insert(targets, player:objectName())
			end
		end
		if isfriend and isfriend == 1 then
			self.player:setFlags("AI_XiansiToFriend_" .. player:objectName())
		end
		return #targets
	end

	local player = self:findPlayerToDiscard("he", true, false)
	if player then
		if rest_num == 1 then return "@XiansiCard=.->" .. player:objectName() end
		add_player(player, self:isFriend(player) and 1 or nil)
		local another = self:findPlayerToDiscard("he", true, false, self.room:getOtherPlayers(player))
		if another then
			add_player(another, self:isFriend(another) and 1 or nil)
			return "@XiansiCard=.->" .. table.concat(targets, "+")
		end
	end

	local lord = self.room:getLord()
	if lord and self:isEnemy(lord) and sgs.turncount <= 1 and not lord:isNude() then
		if add_player(lord) == rest_num then return "@XiansiCard=.->" .. table.concat(targets, "+") end
	end

	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	local luxun = self.room:findPlayerBySkillName("lianying") or self.room:findPlayerBySkillName("noslianying")
	local dengai = self.room:findPlayerBySkillName("tuntian")
	local jiangwei = self.room:findPlayerBySkillName("zhiji")

	if jiangwei and self:isFriend(jiangwei) and jiangwei:getMark("zhiji") == 0 and jiangwei:getHandcardNum()== 1
		and self:getEnemyNumBySeat(self.player, jiangwei) <= (jiangwei:getHp() >= 3 and 1 or 0) then
		if add_player(jiangwei, 1) == rest_num then return "@XiansiCard=.->" .. table.concat(targets, "+") end
	end
	if dengai and dengai:hasSkill("zaoxian") and self:isFriend(dengai) and (not self:isWeak(dengai) or self:getEnemyNumBySeat(self.player, dengai) == 0)
		and dengai:getMark("zaoxian") == 0 and dengai:getPile("field"):length() == 2 and add_player(dengai, 1) == rest_num then
		return "@XiansiCard=.->" .. table.concat(targets, "+")
	end

	if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and self:getEnemyNumBySeat(self.player, zhugeliang) > 0 then
		if zhugeliang:getHp() <= 2 then
			if add_player(zhugeliang, 1) == rest_num then return "@XiansiCard=.->" .. table.concat(targets, "+") end
		else
			local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), zhugeliang:objectName())
			local cards = sgs.QList2Table(zhugeliang:getHandcards())
			if #cards == 1 and (cards[1]:hasFlag("visible") or cards[1]:hasFlag(flag)) then
				if cards[1]:isKindOf("TrickCard") or cards[1]:isKindOf("Slash") or cards[1]:isKindOf("EquipCard") then
					if add_player(zhugeliang, 1) == rest_num then return "@XiansiCard=.->" .. table.concat(targets, "+") end
				end
			end
		end
	end

	if luxun and self:isFriend(luxun) and luxun:getHandcardNum() == 1 and self:getEnemyNumBySeat(self.player, luxun) > 0 then
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), luxun:objectName())
		local cards = sgs.QList2Table(luxun:getHandcards())
		if #cards == 1 and (cards[1]:hasFlag("visible") or cards[1]:hasFlag(flag)) then
			if cards[1]:isKindOf("TrickCard") or cards[1]:isKindOf("Slash") or cards[1]:isKindOf("EquipCard") then
				if add_player(luxun, 1) == rest_num then return "@XiansiCard=.->" .. table.concat(targets, "+") end
			end
		end
	end

	if luxun and add_player(luxun, (self:isFriend(luxun) and 1 or nil)) == rest_num then
		return "@XiansiCard=.->" .. table.concat(targets, "+")
	end

	if dengai and self:isFriend(dengai) and (not self:isWeak(dengai) or self:getEnemyNumBySeat(self.player, dengai) == 0) and add_player(dengai, 1) == rest_num then
		return "@XiansiCard=.->" .. table.concat(targets, "+")
	end

	if #targets == 1 then
		local target = findPlayerByObjectName(self.room, targets[1])
		if target then
			local another
			if rest_num > 1 then another = self:findPlayerToDiscard("he", true, false, self.room:getOtherPlayers(target)) end
			if another then
				add_player(another, self:isFriend(another) and 1 or nil)
				return "@XiansiCard=.->" .. table.concat(targets, "+")
			else
				return "@XiansiCard=.->" .. targets[1]
			end
		end
	end
	return "."
end

sgs.ai_card_intention.XiansiCard = function(self, card, from, tos)
	local lord = self.room:getLord()
	if sgs.evaluatePlayerRole(from) == "neutral" and sgs.evaluatePlayerRole(tos[1]) == "neutral"
		and (not tos[2] or sgs.evaluatePlayerRole(tos[2]) == "neutral") and lord and not lord:isNude()
		and self:doNotDiscard(lord, "he", true) and from:aliveCount() >= 4 then
		sgs.updateIntention(from, lord, -35)
		return
	end
	if from:getState() == "online" then
		for _, to in ipairs(tos) do
			if (self:hasSkills("kongcheng|zhiji|lianying|noslianying", to) and to:getHandcardNum() == 1) or to:hasSkills("tuntian+zaoxian") then
			else
				sgs.updateIntention(from, to, 80)
			end
		end
	else
		for _, to in ipairs(tos) do
			local intention = from:hasFlag("AI_XiansiToFriend_" .. to:objectName()) and -5 or 80
			sgs.updateIntention(from, to, intention)
		end
	end
end

local getXiansiCard = function(pile)
	if #pile > 1 then return pile[1], pile[2] end
	return nil
end

local xiansi_slash_skill = {}
xiansi_slash_skill.name = "xiansi_slash"
table.insert(sgs.ai_skills, xiansi_slash_skill)
xiansi_slash_skill.getTurnUseCard = function(self)
	if not self:slashIsAvailable() then return end
	local liufeng = self.room:findPlayerBySkillName("xiansi")
	if not liufeng or liufeng:getPile("counter"):length() <= 1 or not self.player:canSlash(liufeng) then return end
	local ints = sgs.QList2Table(liufeng:getPile("counter"))
	local a, b = getXiansiCard(ints)
	if a and b then
		return sgs.Card_Parse("@XiansiSlashCard=" .. tostring(a) .. "+" .. tostring(b))
	end
end

sgs.ai_skill_use_func.XiansiSlashCard = function(card, use, self)
	local liufeng = self.room:findPlayerBySkillName("xiansi")
	if not liufeng or liufeng:getPile("counter"):length() <= 1 or not self.player:canSlash(liufeng) then return "." end
	local slash = sgs.Sanguosha:cloneCard("slash")

	if self:slashIsAvailable() and not self:slashIsEffective(slash, liufeng, self.player) and self:isFriend(liufeng) then
		sgs.ai_use_priority.XiansiSlashCard = 0.1
		use.card = card
		if use.to then use.to:append(liufeng) end
	else
		sgs.ai_use_priority.XiansiSlashCard = 2.6
		local dummy_use = { to = sgs.SPlayerList() }
		self:useCardSlash(slash, dummy_use)
		if dummy_use.card then
			if (dummy_use.card:isKindOf("GodSalvation") or dummy_use.card:isKindOf("Analeptic") or dummy_use.card:isKindOf("Weapon"))
				and self:getCardsNum("Slash") > 0 then
				use.card = dummy_use.card
				if use.to then use.to:append(liufeng) end
			else
				if dummy_use.card:isKindOf("Slash") and dummy_use.to:length() > 0 then
					local lf
					for _, p in sgs.qlist(dummy_use.to) do
						if p:objectName() == liufeng:objectName() then
							lf = true
							break
						end
					end
					if lf then
						use.card = card
						if use.to then use.to:append(liufeng) end
					end
				end
			end
		end
	end
	if not use.card then
		sgs.ai_use_priority.XiansiSlashCard = 2.0
		if self:slashIsAvailable() and self:isEnemy(liufeng)
			and not self:slashProhibit(slash, liufeng) and self:slashIsEffective(slash, liufeng) and sgs.isGoodTarget(liufeng, self.enemies, self) then
			use.card = card
			if use.to then use.to:append(liufeng) end
		end
	end
end

sgs.ai_card_intention.XiansiSlashCard = function(self, card, from, tos)
	local slash = sgs.Sanguosha:cloneCard("slash")
	if not self:slashIsEffective(slash, tos[1], from) then
		sgs.updateIntention(from, tos[1], -30)
	else
		return sgs.ai_card_intention.Slash(self, slash, from, tos)
	end
end

sgs.ai_skill_cardask["@longyin"] = function(self, data)
	local function getLeastValueCard(isRed)
		local offhorse_avail, weapon_avail
		for _, enemy in ipairs(self.enemies) do
			if self:canAttack(enemy, self.player) then
				if not offhorse_avail and self.player:getOffensiveHorse() and self.player:distanceTo(enemy, 1) <= self.player:getAttackRange() then
					offhorse_avail = true
				end
				if not weapon_avail and self.player:getWeapon() and self.player:distanceTo(enemy) == 1 then
					weapon_avail = true
				end
			end
			if offhorse_avail and weapon_avail then break end
		end
		if self:needToThrowArmor() then return "$" .. self.player:getArmor():getEffectiveId() end
		if self.player:getPhase() > sgs.Player_Play then
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if self:getKeepValue(c) < 8 and not self.player:isJilei(c) and not self:isValuableCard(c) then return "$" .. c:getEffectiveId() end
			end
			if offhorse_avail and not self.player:isJilei(self.player:getOffensiveHorse()) then return "$" .. self.player:getOffensiveHorse():getEffectiveId() end
			if weapon_avail and not self.player:isJilei(self.player:getWeapon()) and self:evaluateWeapon(self.player:getWeapon()) < 5 then return "$" .. self.player:getWeapon():getEffectiveId() end
		else
			local slashc
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByUseValue(cards)
			for _, c in ipairs(cards) do
				if self:getUseValue(c) < 6 and not self:isValuableCard(c) and not self.player:isJilei(c) then
					if isCard("Slash", c, self.player) then
						if not slashc then slashc = c end
					else
						return "$" .. c:getEffectiveId()
					end
				end
			end
			if offhorse_avail and not self.player:isJilei(self.player:getOffensiveHorse()) then return "$" .. self.player:getOffensiveHorse():getEffectiveId() end
			if isRed and slashc then return "$" .. slashc:getEffectiveId() end
		end
	end
	local use = data:toCardUse()
	local slash = use.card
	local slash_num = 0
	if use.from:objectName() == self.player:objectName() then slash_num = self:getCardsNum("Slash") else slash_num = getCardsNum("Slash", use.from, self.player) end
	if self:isEnemy(use.from) and use.m_addHistory and not self:hasCrossbowEffect(use.from) and slash_num > 0 then return "." end
	if (slash:isRed() and not hasManjuanEffect(self.player))
		or (use.m_reason == sgs.CardUseStruct_CARD_USE_REASON_PLAY and use.m_addHistory and self:isFriend(use.from) and slash_num >= 1
			and (not self:hasCrossbowEffect(use.from) or slash:isRed())) then
		local str = getLeastValueCard(slash:isRed())
		if str then return str end
	end
	return "."
end

sgs.ai_skill_use["@@qiaoshui"] = function(self, prompt)
	local trick_num = 0
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isNDTrick() and not card:isKindOf("Nullification") then trick_num = trick_num + 1 end
	end
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()

	for _, enemy in ipairs(self.enemies) do
		if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
			local enemy_max_card = self:getMaxCard(enemy)
			local enemy_max_point = enemy_max_card and enemy_max_card:getNumber() or 100
			if max_point > enemy_max_point then
				self.qiaoshui_card = max_card:getEffectiveId()
				return "@QiaoshuiCard=.->" .. enemy:objectName()
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
			if max_point >= 10 then
				self.qiaoshui_card = max_card:getEffectiveId()
				return "@QiaoshuiCard=.->" .. enemy:objectName()
			end
		end
	end

	self:sort(self.friends_noself, "handcard")
	for index = #self.friends_noself, 1, -1 do
		local friend = self.friends_noself[index]
		if not friend:isKongcheng() then
			local friend_min_card = self:getMinCard(friend)
			local friend_min_point = friend_min_card and friend_min_card:getNumber() or 100
			if max_point > friend_min_point then
				self.qiaoshui_card = max_card:getEffectiveId()
				return "@QiaoshuiCard=.->" .. friend:objectName()
			end
		end
	end

	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and zhugeliang:objectName() ~= self.player:objectName() then
		if max_point >= 7 then
			self.qiaoshui_card = max_card:getEffectiveId()
			return "@QiaoshuiCard=.->" .. zhugeliang:objectName()
		end
	end

	for index = #self.friends_noself, 1, -1 do
		local friend = self.friends_noself[index]
		if not friend:isKongcheng() then
			if max_point >= 7 then
				self.qiaoshui_card = max_card:getEffectiveId()
				return "@QiaoshuiCard=.->" .. friend:objectName()
			end
		end
	end

	if trick_num == 0 or (trick_num <= 2 and self.player:hasSkill("zongshih")) and not self:isValuableCard(max_card) then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() and self:hasLoseHandcardEffective(enemy) then
				self.qiaoshui_card = max_card:getEffectiveId()
				return "@QiaoshuiCard=.->" .. enemy:objectName()
			end
		end
	end
	return "."
end

sgs.ai_card_intention.QiaoshuiCard = 0

sgs.ai_skill_choice.qiaoshui = function(self, choices, data)
	local use = data:toCardUse()
	if use.card:isKindOf("Collateral") then
		local dummy_use = { isDummy = true, to = sgs.SPlayerList(), current_targets = {} }
		for _, p in sgs.qlist(use.to) do
			table.insert(dummy_use.current_targets, p:objectName())
		end
		self:useCardCollateral(use.card, dummy_use)
		if dummy_use.card and dummy_use.to:length() == 2 then
			local first = dummy_use.to:at(0):objectName()
			local second = dummy_use.to:at(1):objectName()
			self.qiaoshui_collateral = { first, second }
			return "add"
		else
			self.qiaoshui_collateral = nil
		end
	elseif use.card:isKindOf("Analeptic") then
	elseif use.card:isKindOf("Peach") then
		self:sort(self.friends_noself, "hp")
		for _, friend in ipairs(self.friends_noself) do
			if friend:isWounded() and friend:getHp() < getBestHp(friend) then
				self.qiaoshui_extra_target = friend
				return "add"
			end
		end
	elseif use.card:isKindOf("ExNihilo") then
		local friend = self:findPlayerToDraw(false, 2)
		if friend then
			self.qiaoshui_extra_target = friend
			return "add"
		end
	elseif use.card:isKindOf("GodSalvation") then
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if enemy:isWounded() and self:hasTrickEffective(use.card, enemy, self.player) then
				self.qiaoshui_remove_target = enemy
				return "remove"
			end
		end
	elseif use.card:isKindOf("AmazingGrace") then
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if self:hasTrickEffective(use.card, enemy, self.player) and not hasManjuanEffect(enemy)
				and not self:needKongcheng(enemy, true) then
				self.qiaoshui_remove_target = enemy
				return "remove"
			end
		end
	elseif use.card:isKindOf("AOE") then
		self:sort(self.friends_noself)
		local lord = self.room:getLord()
		if lord and lord:objectName() ~= self.player:objectName() and self:isFriend(lord) and self:isWeak(lord) then
			self.qiaoshui_remove_target = lord
			return "remove"
		end
		for _, friend in ipairs(self.friends_noself) do
			if self:hasTrickEffective(use.card, friend, self.player) then
				self.qiaoshui_remove_target = friend
				return "remove"
			end
		end
	elseif use.card:isKindOf("Snatch") or use.card:isKindOf("Dismantlement") then
		local trick = sgs.Sanguosha:cloneCard(use.card:objectName(), use.card:getSuit(), use.card:getNumber())
		trick:setSkillName("qiaoshui")
		local dummy_use = { isDummy = true, to = sgs.SPlayerList(), current_targets = {} }
		for _, p in sgs.qlist(use.to) do
			table.insert(dummy_use.current_targets, p:objectName())
		end
		self:useCardSnatchOrDismantlement(trick, dummy_use)
		if dummy_use.card and dummy_use.to:length() > 0 then
			self.qiaoshui_extra_target = dummy_use.to:first()
			return "add"
		end
	elseif use.card:isKindOf("Slash") then
		local slash = sgs.Sanguosha:cloneCard(use.card:objectName(), use.card:getSuit(), use.card:getNumber())
		slash:setSkillName("qiaoshui")
		local dummy_use = { isDummy = true, to = sgs.SPlayerList(), current_targets = {} }
		for _, p in sgs.qlist(use.to) do
			table.insert(dummy_use.current_targets, p:objectName())
		end
		self:useCardSlash(slash, dummy_use)
		if dummy_use.card and dummy_use.to:length() > 0 then
			self.qiaoshui_extra_target = dummy_use.to:first()
			return "add"
		end
	else
		local dummy_use = { isDummy = true, to = sgs.SPlayerList(), current_targets = {} }
		for _, p in sgs.qlist(use.to) do
			table.insert(dummy_use.current_targets, p:objectName())
		end
		self:useCardByClassName(use.card, dummy_use)
		if dummy_use.card and dummy_use.to:length() > 0 then
			self.qiaoshui_extra_target = dummy_use.to:first()
			return "add"
		end
	end
	self.qiaoshui_extra_target = nil
	self.qiaoshui_remove_target = nil
	return "cancel"
end

sgs.ai_skill_playerchosen.qiaoshui = function(self, targets)
	if not self.qiaoshui_extra_target and not self.qiaoshui_remove_target then self.room:writeToConsole("Qiaoshui player chosen error!!") end
	return self.qiaoshui_extra_target or self.qiaoshui_remove_target
end

sgs.ai_skill_use["@@qiaoshui!"] = function(self, prompt) -- extra target for Collateral
	if not self.qiaoshui_collateral then self.room:writeToConsole("Qiaoshui player chosen error!!") end
	return "@ExtraCollateralCard=.->" .. self.qiaoshui_collateral[1] .. "+" .. self.qiaoshui_collateral[2]
end

sgs.ai_skill_invoke.zongshih = function(self, data)
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_cardask["@duodao-get"] = function(self, data)
	local function getLeastValueCard(from)
		if self:needToThrowArmor() then return "$" .. self.player:getArmor():getEffectiveId() end
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		for _, c in ipairs(cards) do
			if self:getKeepValue(c) < 8 and not self.player:isJilei(c) and not self:isValuableCard(c) then return "$" .. c:getEffectiveId() end
		end
		local offhorse_avail, weapon_avail
		for _, enemy in ipairs(self.enemies) do
			if self:canAttack(enemy, self.player) then
				if not offhorse_avail and self.player:getOffensiveHorse() and self.player:distanceTo(enemy, 1) <= self.player:getAttackRange() then
					offhorse_avail = true
				end
				if not weapon_avail and self.player:getWeapon() and self.player:distanceTo(enemy) == 1 then
					weapon_avail = true
				end
			end
			if offhorse_avail and weapon_avail then break end
		end
		if offhorse_avail and not self.player:isJilei(self.player:getOffensiveHorse()) then return "$" .. self.player:getOffensiveHorse():getEffectiveId() end
		if weapon_avail and not self.player:isJilei(self.player:getWeapon()) and self:evaluateWeapon(self.player:getWeapon()) < self:evaluateWeapon(from:getWeapon()) then
			return "$" .. self.player:getWeapon():getEffectiveId()
		end
	end
	local damage = data:toDamage()
	if not damage.from or not damage.from:getWeapon() then
		if self:needToThrowArmor() then
			return "$" .. self.player:getArmor():getEffectiveId()
		elseif self.player:getHandcardNum() == 1 and (self.player:hasSkill("kongcheng") or (self.player:hasSkill("zhiji") and self.player:getMark("zhiji") == 0)) then
			return "$" .. self.player:handCards():first()
		end
	else
		if self:isFriend(damage.from) then
			if damage.from:hasSkills("kofxiaoji|xiaoji") and self:isWeak(damage.from) then
				local str = getLeastValueCard(damage.from)
				if str then return str end
			else
				if self:getCardsNum("Slash") == 0 or self:willSkipPlayPhase() then return "." end
				local invoke = false
				local range = sgs.weapon_range[damage.from:getWeapon():getClassName()] or 0
				if self.player:hasSkill("anjian") then
					for _, enemy in ipairs(self.enemies) do
						if not enemy:inMyAttackRange(self.player) and not self.player:inMyAttackRange(enemy) and self.player:distanceTo(enemy) <= range then
							invoke = true
							break
						end
					end
				end
				if not invoke and self:evaluateWeapon(damage.from:getWeapon()) > 8 then invoke = true end
				if invoke then
					local str = getLeastValueCard(damage.from)
					if str then return str end
				end
			end
		else
			if damage.from:hasSkill("nosxuanfeng") then
				for _, friend in ipairs(self.friends) do
					if self:isWeak(friend) then return "." end
				end
			else
				if hasManjuanEffect(self.player) then
					if self:needToThrowArmor() and not self.player:isJilei(self.player:getArmor()) then
						return "$" .. self.player:getArmor():getEffectiveId()
					elseif self.player:getHandcardNum() == 1
							and (self.player:hasSkill("kongcheng") or (self.player:hasSkill("zhiji") and self.player:getMark("zhiji") == 0))
							and not self.player:isJilei(self.player:getHandcards():first()) then
						return "$" .. self.player:handCards():first()
					end
				else
					local str = getLeastValueCard(damage.from)
					if str then return str end
				end
			end
		end
	end
	return "."
end

local danshou_skill = {}
danshou_skill.name = "danshou"
table.insert(sgs.ai_skills, danshou_skill)
danshou_skill.getTurnUseCard = function(self)
	local times = self.player:getMark("danshou") + 1
	if times < self.player:getCardCount(true) then
		return sgs.Card_Parse("@DanshouCard=.")
	end
end

sgs.ai_skill_use_func.DanshouCard = function(card, use, self)
	local times = self.player:getMark("danshou") + 1
	local cards = self.player:getCards("he")
	local jink_num = self:getCardsNum("Jink")
	local to_discard = {}
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		cards:append(sgs.Sanguosha:getCard(id))
	end
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local has_weapon = false
	local DisWeapon = false
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
			if card:isKindOf("Weapon") and self.player:getHandcardNum() > 2 then
				if not self.player:getWeapon() then shouldUse = false
				elseif self.player:hasEquip(card) and not has_weapon then shouldUse = false
				else DisWeapon = true
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
			if isCard("Peach", card, self.player) then
				if times ~= 3 then shouldUse = false end
			end
			if isCard("Jink", card, self.player) then
				if jink_num <= 1 and times == 1 then shouldUse = false
				else jink_num = jink_num - 1 end
			end
			if shouldUse then
				table.insert(to_discard, card:getEffectiveId())
				if #to_discard == times then break end
			end
		end
	end

	local range =   self.player:getAttackRange()
	if DisWeapon then range = 1 end
	local target
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if self.player:distanceTo(p) <= range then
			if times == 1 or times == 2 then
				if self.player:canDiscard(p, "he") and not p:isNude() then
					if self:isFriend(p) then
						if(self:hasSkills(sgs.lose_equip_skill, p) and not p:getEquips():isEmpty())
						or (self:needToThrowArmor(p) and p:getArmor()) or self:doNotDiscard(p) then
							target = p  break end
					elseif self:isEnemy(p) then
						if times == 2 and self:needToThrowArmor(p) then continue
						elseif (not self:doNotDiscard(p) or self:getDangerousCard(p) or self:getValuableCard(p)) then
							target = p  break end
					end
				end
			elseif times == 3 then
				if self:isEnemy(p) then
					if self:damageIsEffective(p, nil, self.player) and not self:getDamagedEffects(p, self.player)
					and not self:needToLoseHp(p, self.player) and ((self:isWeak(p) and p:getHp() < 3) or self:getOverflow() > 3)  then
						target = p  break end
				end
			elseif times == 4 then
				if self:isFriend(p) and p:isWounded() then
					target = p  break end
			end
		end
	end
	if target and #to_discard == times then
		use.card = sgs.Card_Parse("@DanshouCard=" .. table.concat(to_discard, "+"))
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_priority.DanshouCard = sgs.ai_use_priority.Dismantlement + 2
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


sgs.ai_skill_use["@@zongxuan"] = function(self, prompt)
	if self.top_draw_pile_id or self.player:getPhase() >= sgs.Player_Finish then return "." end
	local list = self.player:property("zongxuan"):toString():split("+")
	local valuable
	for _, id in ipairs(list) do
		local card_id = tonumber(id)
		local card = sgs.Sanguosha:getCard(card_id)
		if card:isKindOf("EquipCard") then
			for _, friend in ipairs(self.friends) do
				if not (card:isKindOf("Armor") and not friend:getArmor() and friend:hasSkills("bazhen|yizhong"))
					and (not self:getSameEquip(card, friend) or card:isKindOf("DefensiveHorse") or card:isKindOf("OffensiveHorse")
						or (card:isKindOf("Weapon") and self:evaluateWeapon(card) > self:evaluateWeapon(friend:getWeapon()) - 1)) then
					self.top_draw_pile_id = card_id
					return "@ZongxuanCard=" .. card_id
				end
			end
		elseif self:isValuableCard(card) and not valuable then
			valuable = card_id
		end
	end
	if valuable then
		self.top_draw_pile_id = valuable
		return "@ZongxuanCard=" .. valuable
	end
	return "."
end

sgs.ai_skill_playerchosen.zhiyan = function(self, targets)
	if self.top_draw_pile_id then
		local card = sgs.Sanguosha:getCard(self.top_draw_pile_id)
		if card:isKindOf("EquipCard") then
			self:sort(self.friends, "hp")
			for _, friend in ipairs(self.friends) do
				if (not self:getSameEquip(card, friend) or card:isKindOf("DefensiveHorse") or card:isKindOf("OffensiveHorse"))
					and not (card:isKindOf("Armor") and (friend:hasSkills("bazhen|yizhong") or self:evaluateArmor(card, friend) < 0)) then
					return friend
				end
			end
			if not (card:isKindOf("Armor") and (self.player:hasSkills("bazhen|yizhong") or self:evaluateArmor(card) < 0))
				and not (card:isKindOf("Weapon") and self.player:getWeapon() and self:evaluateWeapon(card) < self:evaluateWeapon(self.player:getWeapon()) - 1) then
				return self.player
			end
		else
			local cards = { card }
			local card, player = self:getCardNeedPlayer(cards)
			if player then
				return player
			else
				self:sort(self.friends)
				for _, friend in ipairs(self.friends) do
					if not self:needKongcheng(friend, true) and not hasManjuanEffect(friend) then return friend end
				end
			end
		end
	else
		self:sort(self.friends)
		for _, friend in ipairs(self.friends) do
			if not self:needKongcheng(friend, true) and not hasManjuanEffect(friend) then return friend end
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.zhiyan = -60

sgs.ai_skill_invoke.zhuikong = sgs.ai_skill_invoke.noszhuikong

sgs.ai_skill_playerchosen.qiuyuan = function(self, targets)
	local targetlist = sgs.QList2Table(targets)
	self:sort(targetlist, "handcard")
	local enemy
	for _, p in ipairs(targetlist) do
		local jink = getKnownCard(p, self.player, "Jink", true, "he")
		if self:isEnemy(p) and (jink == 0 or (self:isWeak(p) and jink < 2)) then
			enemy = p
			break
		end
	end
	if enemy then return enemy end
	targetlist = sgs.reverse(targetlist)
	local friend
	for _, p in ipairs(targetlist) do
		local jink = getKnownCard(p, self.player, "Jink", true, "he")
		if self:isFriend(p) then
			if (self:needKongcheng(p) and p:getHandcardNum() == 1 and jink == 1)
				or (p:getCardCount() >= 2 and self:canLiuli(p, self.enemies))
				or self:needLeiji(p) or p:getHandcardNum() > 3 or jink >= 1 then
				friend = p
				break
			end
		end
	end
	if friend then return friend end
return nil
end

sgs.ai_skill_cardask["@qiuyuan-give"] = function(self, data, pattern, target)
	local give = true
	local huanghou = self.room:findPlayerBySkillName("qiuyuan")
	if self:isEnemy(huanghou) then
		if not (self:needKongcheng() and self.player:getHandcardNum() == 1) then
			give = false
		end
	elseif self:isFriend(huanghou) then
		if not self:isWeak(huanghou) and self:hasSkills("leiji|nosleiji") then
			give = false
		end
	end
	if give == true then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if card:isKindOf("Jink") then
				return "$" .. card:getEffectiveId()
			end
		end
	end
	return "."
end

function SmartAI:hasQiuyuanEffect(from, to)
	if not from or not to:hasSkill("qiuyuan") then return false end
	for _, target in ipairs(self:getEnemies(to)) do
		if self:isFriend(target) then
			if (target:isKongcheng() and not (target:getHandcardNum() == 1 and self:needKongcheng(target, true))) 
			or self:isWeak(target) then
				return true
			end
		end
	end
	return
end

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
		if self:getDamagedEffects(friend, self.player) and self:needToLoseHp(friend, self.player) then return friend end
	end
return nil
end

sgs.ai_playerchosen_intention.juece = function(self, from, to)
	if self:damageIsEffective(to, nil, from) and not self:getDamagedEffects(friend, self.player) and not self:needToLoseHp(friend, self.player) then
		sgs.updateIntention(from, to, 10)
	end
end

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
			local v = 4
			if (self:getDamagedEffects(p, self.player) or self:needToLoseHp(p, self.player)) and getCardsNum("Peach", p, self.player) + p:getHp() > 2 then
				v = v - 6
			elseif lastPlayer:objectName() ~= self.player:objectName() and lastPlayer:getCardCount(true) < p:getCardCount(true) then
				v = v - 4
			elseif lastPlayer:objectName() == self.player:objectName() and not p:isNude() then
				v = v - 4
			end
			if self:isFriend(p) then
				value = value - v - p:getHp() + 2
			elseif self:isEnemy(p) then
				value = value + v + p:getLostHp() - 1
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
	local length = min_num
	local peaches = 0

	local nextPlayer = self.player:getNextAlive()
	if self:isEnemy(nextPlayer) and self.player:getCardCount(true) > nextPlayer:getCardCount(true) and self.player:getCardCount(true) > length then
		length = tonumber(nextPlayer:getCardCount(true))
	end

	for _, c in ipairs(cards) do
		if self.player:canDiscard(self.player, c:getEffectiveId()) then
			table.insert(to_discard, c:getEffectiveId())
			if isCard("Peach", c, self.player) then peaches = peaches + 1 end
			if #to_discard == length then break end
		end
	end

	if peaches > 2 then
		return {}
	elseif peaches == 2 and self.player:getHp() > 1 and length == min_num then
		for _, friend in ipairs(self.friends_noself) do
			damage.to = friend
			if friend:getHp() <= 2 and self:damageIsEffective_(damage) then return {} end
		end
	end

	if nextPlayer:isLord() and self.role ~= "rebel" and nextPlayer:getHandcardNum() < min_num
		and not self:getDamagedEffects(nextPlayer, current) and not self:needToLoseHp(nextPlayer, current) then
		if nextPlayer:getHp() + getCardsNum("Peach", nextPlayer, self.player) + self:getCardsNum("Peach") <= 2 then return {} end
		if self.player:getHp() > nextPlayer:getHp() and self.player:getHp() > 2 then return {} end
	end
	return to_discard
end

local mieji_skill = {}
mieji_skill.name = "mieji"
table.insert(sgs.ai_skills, mieji_skill)
mieji_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("MiejiCard") or self.player:isKongcheng() then return end
	return sgs.Card_Parse("@MiejiCard=.")
end

sgs.ai_skill_use_func.MiejiCard = function(card, use, self)
	local nextAlive = self.player:getNextAlive()
	local hasLightning, hasIndulgence, hasSupplyShortage
	local tricks = nextAlive:getJudgingArea()
	if not tricks:isEmpty() and not nextAlive:containsTrick("YanxiaoCard") and not nextAlive:hasSkill("qianxi") then
		local trick = tricks:at(tricks:length() - 1)
		if self:hasTrickEffective(trick, nextAlive) then
			if trick:isKindOf("Lightning") then hasLightning = true
			elseif trick:isKindOf("Indulgence") then hasIndulgence = true
			elseif trick:isKindOf("SupplyShortage") then hasSupplyShortage = true
			end
		end
	end

	local putcard
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:isBlack() and card:isKindOf("TrickCard") then
			if hasLightning and card:getSuit() == sgs.Card_Spade and card:getNumber() >= 2 and card:getNumber() <= 9 then
				if self:isEnemy(nextAlive) then
					putcard = card break
				else continue
				end
			end
			if hasSupplyShortage and card:getSuit() == sgs.Card_Club then
				if self:isFriend(nextAlive) then
					putcard = card break
				else continue
				end
			end
			if not putcard then
				putcard = card break
			end
		end
	end

	local target
	for _, enemy in ipairs(self.enemies) do
		if self:needKongcheng(enemy) and enemy:getHandcardNum() <= 2 then continue end
		if not enemy:isNude()  then
			target = enemy break
		end
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if self:needKongcheng(friend) and friend:getHandcardNum() < 2 and not friend:isKongcheng() then
				target = friend break
			end
		end
	end

	if putcard and target then
		use.card = sgs.Card_Parse("@MiejiCard="..putcard:getEffectiveId())
		if use.to then use.to:append(target) end
		return
	end

end

sgs.ai_use_priority.MiejiCard = sgs.ai_use_priority.Dismantlement + 1

sgs.ai_card_intention.MiejiCard = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if self:needKongcheng(to) and to:getHandcardNum() <= 2 then continue end
		sgs.updateIntention(from, to, 10)
	end
end

sgs.ai_skill_cardask["@@miejidiscard!"] = function(self, prompt)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local trick = {}
	local nontrick = {}
	local discard = {}
	for _,card in ipairs(cards) do
		if card:isKindOf("TrickCard") then
			table.insert(trick, card)
		else
			table.insert(nontrick, card)
		end
	end
	if #cards <= 2 then return "." end
	if self:needToThrowArmor() and #nontrick >= 2 then
		table.insert(discard, self.player:getArmor())
		if nontrick[1] ~= discard[1] then
			table.insert(discard, nontrick[1])
		else
			table.insert(discard, nontrick[2])
		end
	end
	if #trick == 0 then
		for _,card in ipairs(nontrick) do
			table.insert(discard, card)
			if #discard == 2 or #discard == #nontrick then
				break
			end
		end
	end
	if #nontrick == 0 and #trick >= 1 then
		table.insert(discard, trick[1])
	end
	if #discard > 0 then
		return "$"..table.concat(discard:getEffectiveId(), "+")
	end
return "."
end


