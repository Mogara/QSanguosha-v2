local dingpin_skill = {}
dingpin_skill.name = "dingpin"
table.insert(sgs.ai_skills, dingpin_skill)
dingpin_skill.getTurnUseCard = function(self, inclusive)
	sgs.ai_use_priority.DingpinCard = 0
	if not self.player:canDiscard(self.player, "h") or self.player:getMark("dingpin") == 0xE then return false end
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if not p:hasFlag("dingpin") and p:isWounded() then
			if not self:toTurnOver(self.player) then sgs.ai_use_priority.DingpinCard = 8.9 end
			return sgs.Card_Parse("@DingpinCard=.")
		end
	end
end
sgs.ai_skill_use_func.DingpinCard = function(card, use, self)
	local cards = {}
	local cardType = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if bit32.band(self.player:getMark("dingpin"), bit32.lshift(1, card:getTypeId())) == 0 then
			table.insert(cards, card)
			if not table.contains(cardType, card:getTypeId()) then table.insert(cardType, card:getTypeId()) end
		end
	end
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		local card = sgs.Sanguosha:getCard(id)
		if bit32.band(self.player:getMark("dingpin"), bit32.lshift(1, card:getTypeId())) == 0 then
			table.insert(cards, card)
			if not table.contains(cardType, card:getTypeId()) then table.insert(cardType, card:getTypeId()) end
		end
	end
	if #cards == 0 then return end
	self:sortByUseValue(cards, true)
	if self:isValuableCard(cards[1]) then return end

	if #cardType > 1 or not self:toTurnOver(self.player) then
		self:sort(self.friends)
		for _, friend in ipairs(self.friends) do
			if not friend:hasFlag("dingpin") and friend:isWounded() then
				use.card = sgs.Card_Parse("@DingpinCard=" .. cards[1]:getEffectiveId())
				if use.to then use.to:append(friend) end
				return
			end
		end
	end

end

sgs.ai_use_priority.DingpinCard = 0
sgs.ai_card_intention.DingpinCard = -10

sgs.ai_skill_invoke.faen = function(self, data)
	local player = data:toPlayer()
	if self:needKongcheng(player, true) then return not self:isFriend(player) end
	return self:isFriend(player)
end

sgs.ai_choicemade_filter.skillInvoke.faen = function(self, player, promptlist)
	local target = findPlayerByObjectName(self.room, promptlist[#promptlist - 1])
	if not target then return end
	local yes = promptlist[#promptlist] == "yes"
	if self:needKongcheng(target, true) then
		sgs.updateIntention(player, target, yes and 10 or -10)
	else
		sgs.updateIntention(player, target, yes and -10 or 10)
	end
end

sgs.ai_skill_invoke.sidi = true

sgs.ai_skill_use["@@sidi"] = function(self)
	local current = self.room:getCurrent()
	local slash = sgs.Sanguosha:cloneCard("slash")
	if self:isEnemy(current) then
		if (getCardsNum("Slash", current, self.player) >= 1 or self.player:getPile("sidi"):length() > 2)
		and not (current:hasWeapon("crossbow") or current:hasSkill("paoxiao")) then
			for _, player in sgs.qlist(self.room:getOtherPlayers(current)) do
				if self:isFriend(player) and player:distanceTo(current) <= current:getAttackRange()
				and self:slashIsEffective(slash, player) and (self:isWeak(player) or self.player:getPile("sidi"):length() > 1) then
					return "@SidiCard=" .. self.player:getPile("sidi"):first()
				end
			end
		end
	end
	return "."
end

sgs.ai_skill_use["@@shenduan"] = function(self)
	local ids = self.player:property("shenduan"):toString():split("+")
	for _, id in ipairs(ids) do
		local card = sgs.Sanguosha:getCard(id)
		if self.player:isCardLimited(card, sgs.Card_MethodUse) then continue end
		local card_str = ("supply_shortage:shenduan[%s:%s]=%d"):format(card:getSuitString(), card:getNumberString(), id)
		local ss = sgs.Card_Parse(card_str)
		local dummy_use = { isDummy = true , to = sgs.SPlayerList() }
		self:useCardSupplyShortage(ss, dummy_use)
		if dummy_use.card and not dummy_use.to:isEmpty() then
			return card_str .. "->" .. dummy_use.to:first():objectName()
		end
	end
	return "."
end

sgs.ai_skill_invoke.yonglve = function(self)
	local current = self.room:getCurrent()
	if self:isFriend(current) and self:askForCardChosen(current, "h", "dummyReason", sgs.Card_MethodDiscard) then
		if not self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), current, self.player) then return true end
		if not self:isWeak(current) or getKnownCard(current, self.player, "Jink") > 0 then return true end
	elseif self:isEnemy(current) then
		if self:askForCardChosen(current, "h", "dummyReason", sgs.Card_MethodDiscard) then return true end
		for _, card in sgs.qlist(current:getJudgingArea()) do
			if card:isKindOf("SupplyShortage") and (current:getHandcardNum() > 4 or current:containsTrick("indulgence")) then
				sgs.ai_skill_cardchosen.yonglve = card:getEffectiveId()
				return true
			elseif card:isKindOf("Indulgence") and current:getHandcardNum() + self:ImitateResult_DrawNCards(current) <= self:getOverflow(current, true) then
				sgs.ai_skill_cardchosen.yonglve = card:getEffectiveId()
				return true
			end
		end
		if self:isWeak(current) and current:getHp() == 1 and (sgs.card_lack[current:objectName()]["Jink"] == 1 or getCardsNum("Jink", current, self.player) == 0)
			and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), current, self.player) then
			sgs.ai_skill_cardchosen.yonglve = self:getCardRandomly(current, "j")
			return true
		end
	end
	return false
end

sgs.ai_skill_invoke.qiangzhi = function(self)
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_playerchosen.qiangzhi = function(self, targetlist)
	local slash = self:getCard("Slash")
	if slash then
		local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
		self:useCardSlash(slash, dummy_use)
		if dummy_use.card and not dummy_use.to:isEmpty() then
			local target = dummy_use.to:first()
			if targetlist:contains(target) and target:getHandcardNum() - getKnownNum(target, self.player) <= 2 and target:getHandcardNum() <= 2 then
				return target
			end
		end
	end

	local cardType = { trick = 0, basic = 0, equip = 0 }
	local turnUse = self:getTurnUse()
	for _, card in ipairs(turnUse) do
		if card:getType() ~= "skill_card" then cardType[card:getType()] = cardType[card:getType()] + 1 end
	end

	local target = {}
	local max_trick, max_basic, max_equip = 0, 0, 0
	for _, player in sgs.qlist(targetlist) do
		local known = getKnownCard(player, self.player, "TrickCard")
		if cardType.trick > 0 and known / player:getHandcardNum() > max_trick then
			max_trick = known
			target.trick = player
		end

		known = getKnownCard(player, self.player, "BasicCard")
		if cardType.basic > 0 and known / player:getHandcardNum() > max_basic then
			max_basic = known
			target.basic = player
		end

		known = getKnownCard(player, self.player, "EquipCard")
		if cardType.equip > 0 and known / player:getHandcardNum() > max_equip then
			max_equip = known
			target.equip = player
		end
	end

	local max_value = math.max(cardType.trick * max_trick, cardType.basic * max_basic, cardType.equip * max_equip)
	if max_value > 0 then
		for cardype, value in pairs(cardType) do
			if max_value == value then return target[cardype] end
		end
	end

	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if targetlist:contains(enemy) then return enemy end
	end

	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(players)
	for _, p in ipairs(players) do
		if not self:isFriend(p) and targetlist:contains(p) then return p end
	end

	self:sort(self.friends_noself, "handcard")
	self.friends_noself = sgs.reverse(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if targetlist:contains(friend) then return friend end
	end

	return fasle
end

sgs.ai_skill_invoke.xiantu = function(self)
	local current = self.room:getCurrent()
	if self:isFriend(current) then
		if current:isLord() and sgs.isLordInDanger() then return true end
		if self.role == "renegade" and not self:needToThrowArmor() then return false end
		if sgs.evaluatePlayerRole(current, self.player) == "renegade" and not self:needToThrowArmor() then return false end
		for _, enemy in ipairs(self.enemies) do
			if self:isWeak(enemy) and enemy:getHp() == 1 and not self:slashProhibit(nil, enemy, current)
				and (not sgs.isJinkAvailable(current, enemy) or getCardsNum("Jink", enemy, self.player) == 0 or sgs.card_lack[enemy:objectName()] == 1)
				and (getCardsNum("Slash", current, self.player) >= 1 or self:getCardsNum("Slash") > 0) then
				return true
			end
		end
		if not self.player:isWounded() and self:isWeak(current) then return true end
	end
end

sgs.ai_skill_discard.xiantu = function(self, discard_num, min_num, optional, include_equip)
	local to_exchange = {}
	local current = self.room:getCurrent()
	if self.player:isWounded() and self.player:hasArmorEffect("silver_lion") then table.insert(to_exchange, self.player:getArmor():getEffectiveId()) end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards)
	if getCardsNum("Slash", current, self.player) < 1 then
		for _, card in ipairs(cards) do
			if not isCard("Peach", card, self.player) and isCard("Slash", card, current) then
				table.insert(to_exchange, card:getEffectiveId())
			end
		end
	end

	if #to_exchange == 2 then return to_exchange end

	for _, card in ipairs(cards) do
		if self.player:hasEquip(card) and self.player:hasSkills(sgs.lose_equip_skill) then
			table.insert(to_exchange, card:getEffectiveId())
			if #to_exchange == 2 then return to_exchange end
			break
		end
	end
	for _, card in ipairs(cards) do
		table.insert(to_exchange, card:getEffectiveId())
		if #to_exchange == 2 then return to_exchange end
	end
end

sgs.ai_skill_playerchosen.zhongyong = function(self, targetlist)
	self:sort(self.friends)
	if self:getCardsNum("Slash") > 0 then
		for _, friend in ipairs(self.friends) do
			if not targetlist:contains(friend) or friend:objectName() == self.player:objectName() then continue end
			if getCardsNum("Jink", friend, self.player) < 1 or sgs.card_lack[friend:objectName()]["Jink"] == 1 then
				return friend
			end
		end
		if self:getCardsNum("Jink") == 0 and targetlist:contains(self.player) then return self.player end
	end
	local lord = self.room:getLord()
	if self:isFriend(lord) and sgs.isLordInDanger() and targetlist:contains(lord) and getCardsNum("Jink", lord, self.player) < 2 then return lord end
	if self.role == "renegade" and targetlist:contains(self.player) then return self.player end
	return self:findPlayerToDraw(true, 1) or self.friends[1]
end

local shenxing_skill = {}
shenxing_skill.name = "shenxing"
table.insert(sgs.ai_skills, shenxing_skill)
shenxing_skill.getTurnUseCard = function(self)
	sgs.ai_use_priority.ShenxingCard = 3
	if self.player:getCardCount(true) < 2 then return false end
	if self:getOverflow() <= 0 then return false end
	if self:isWeak() and self:getOverflow() <= 1 then return false end
	return sgs.Card_Parse("@ShenxingCard=.")
end
sgs.ai_skill_use_func.ShenxingCard = function(card, use, self)
	local unpreferedCards = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)

	local red_num, black_num = 0, 0
	if self.player:getHp() < 3 then
		local zcards = self.player:getCards("he")
		local use_slash, keep_jink, keep_analeptic, keep_weapon = false, false, false
		local keep_slash = self.player:getTag("JilveWansha"):toBool()
		for _, zcard in sgs.qlist(zcards) do
			if self.player:isCardLimited(zcard, sgs.Card_MethodDiscard) then continue end
			if not isCard("Peach", zcard, self.player) and not isCard("ExNihilo", zcard, self.player) then
				local shouldUse = true
				if isCard("Slash", zcard, self.player) and not use_slash then
					local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
					self:useBasicCard(zcard, dummy_use)
					if dummy_use.card then
						if keep_slash then shouldUse = false end
						if dummy_use.to then
							for _, p in sgs.qlist(dummy_use.to) do
								if p:getHp() <= 1 then
									shouldUse = false
									if self.player:distanceTo(p) > 1 then keep_weapon = self.player:getWeapon() end
									break
								end
							end
							if dummy_use.to:length() > 1 then shouldUse = false end
						end
						if not self:isWeak() then shouldUse = false end
						if not shouldUse then use_slash = true end
					end
				end
				if zcard:getTypeId() == sgs.Card_TypeTrick then
					local dummy_use = { isDummy = true }
					self:useTrickCard(zcard, dummy_use)
					if dummy_use.card then shouldUse = false end
				end
				if zcard:getTypeId() == sgs.Card_TypeEquip and not self.player:hasEquip(zcard) then
					local dummy_use = { isDummy = true }
					self:useEquipCard(zcard, dummy_use)
					if dummy_use.card then shouldUse = false end
					if keep_weapon and zcard:getEffectiveId() == keep_weapon:getEffectiveId() then shouldUse = false end
				end
				if self.player:hasEquip(zcard) and zcard:isKindOf("Armor") and not self:needToThrowArmor() then shouldUse = false end
				if self.player:hasTreasure("wooden_ox") then shouldUse = false end
				if self.player:hasEquip(zcard) and zcard:isKindOf("DefensiveHorse") and not self:needToThrowArmor() then shouldUse = false end
				if isCard("Jink", zcard, self.player) and not keep_jink then
					keep_jink = true
					shouldUse = false
				end
				if self.player:getHp() == 1 and isCard("Analeptic", zcard, self.player) and not keep_analeptic then
					keep_analeptic = true
					shouldUse = false
				end
				if shouldUse then
					if (table.contains(unpreferedCards, zcard:getId())) then continue end
					table.insert(unpreferedCards, zcard:getId())
					if self.room:getCardPlace(zcard:getId()) == sgs.Player_PlaceHand then
						if zcard:isRed() then red_num = red_num + 1
						else black_num = black_num + 1 end
					end
				end
				if #unpreferedCards == 2 then
					use.card = sgs.Card_Parse("@ShenxingCard=" .. table.concat(unpreferedCards, "+"))
					return
				end
			end
		end
	end

	local red = self:getSuitNum("red")
	local black = self:getSuitNum("black")
	if red - red_num <= 2 - #unpreferedCards then
		for _, c in ipairs(cards) do
			if c:isRed() and (not isCard("Peach", c, self.player) or not self:findFriendsByType(sgs.Friend_Weak) and #cards > 1) then
				if self.player:isCardLimited(c, sgs.Card_MethodDiscard) then continue end
				if table.contains(unpreferedCards, c:getId()) then continue end
				table.insert(unpreferedCards, c:getId())
			end
		end
	elseif black - black_num <= 2 - #unpreferedCards then
		for _, c in ipairs(cards) do
			if c:isBlack() and (not isCard("Peach", c, self.player) or not self:findFriendsByType(sgs.Friend_Weak) and #cards > 1) then
				if self.player:isCardLimited(c, sgs.Card_MethodDiscard) then continue end
				if table.contains(unpreferedCards, c:getId()) then continue end
				table.insert(unpreferedCards, c:getId())
			end
		end
	end

	if #unpreferedCards < 2 then
		for _, c in ipairs(cards) do
			if not self.player:isCardLimited(c, sgs.Card_MethodDiscard) then
				if table.contains(unpreferedCards, c:getId()) then continue end
				table.insert(unpreferedCards, c:getId())
			end
			if #unpreferedCards == 2 then break end
		end
	end

	if #unpreferedCards == 2 then
		use.card = sgs.Card_Parse("@ShenxingCard=" .. table.concat(unpreferedCards, "+"))
		sgs.ai_use_priority.ShenxingCard = 0
		return
	end

end

sgs.ai_use_priority.ShenxingCard = 3

sgs.ai_skill_use["@@bingyi"] = function(self)

	local cards = self.player:getHandcards()
	if cards:length() == 0 then return "." end

	if cards:first():isBlack() then
		for _, c in sgs.qlist(cards) do
			if c:isRed() then return "." end
		end
	elseif cards:first():isRed() then
		for _, c in sgs.qlist(cards) do
			if c:isBlack() then return "." end
		end
	end

	self:sort(self.friends)
	local targets = {}
	for _, friend in ipairs(self.friends) do
		if not hasManjuanEffect(friend) and not self:needKongcheng(friend, true) then
			table.insert(targets, friend:objectName())
		end
		if #targets == self.player:getHandcardNum() then break end
	end

	if #targets < self.player:getHandcardNum() then
		for _, enemy in ipairs(self.enemies) do
			if self:needKongcheng(enemy, true) then
				table.insert(targets, enemy:objectName())
			end
		end
	end

	if #targets > 0 then
		return "@BingyiCard=.->" .. table.concat(targets, "+")
	end
end

sgs.ai_card_intention.BingyiCard = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if self:needKongcheng(to, true) then sgs.updateIntention(from, to, 10)
		elseif hasManjuanEffect(to) then continue
		else sgs.updateIntention(from, to, -10) end
	end
end

sgs.ai_skill_playerchosen.zenhui = function(self, targetlist)
	self.zenhui_collateral = nil
	local use = self.player:getTag("zenhui"):toCardUse()
	local dummy_use = { isDummy = true, to = sgs.SPlayerList(), current_targets = {}, extra_target = 99 }
	local target = use.to:first()
	if not target then return end
	table.insert(dummy_use.current_targets, target:objectName())
	self:useCardByClassName(use.card, dummy_use)
	if dummy_use.card and use.card:getClassName() == dummy_use.card:getClassName() and not dummy_use.to:isEmpty() then
		if use.card:isKindOf("Collateral") then
			assert(dummy_use.to:length() == 2)
			local player = dummy_use.to:first()
			if targetlist:contains(player) and (self:isFriend(player) or self:hasTrickEffective(use.card, target, player)) then
				self.zenhui_collateral = dummy_use.to:at(1)
				return player
			end
			return false
		elseif use.card:isKindOf("Slash") then
			for _, player in sgs.qlist(dummy_use.to) do
				if targetlist:contains(player) and (self:isFriend(player) or not self:slashProhibit(use.card, target, player)) then
					return player
				end
			end
		elseif use.card:isKindOf("FireAttack") then
			for _, player in sgs.qlist(dummy_use.to) do
				if targetlist:contains(player) and not self:isFriend(player, target) and not self:isFriend(player) and self:hasTrickEffective(use.card, target, player) then
					return player
				end
			end
			dummy_use.to = sgs.QList2Table(dummy_use.to)
			self:sort(dummy_use.to, "handcard")
			dummy_use.to = sgs.reverse(dummy_use.to)
			local suits = {}
			for _, c in sgs.qlist(self.player:getHandcards()) do
				if c:getSuit() <= 3 and not table.contains(suits, c:getSuitString()) then table.insert(suits, c:getSuitString()) end
			end
			if #suits <= 2 or self:getSuitNum("heart", false, target) > 0 and self:getSuitNum("heart") == 0 then
				for _, player in ipairs(dummy_use.to) do
					if self:isFriend(player) and targetlist:contains(player) then return player end
				end
			end
		elseif use.card:isKindOf("Duel") then
			for _, player in sgs.qlist(dummy_use.to) do
				if targetlist:contains(player) and (self:isFriend(player) or self:hasTrickEffective(use.card, target, player)) then
					return player
				end
			end
		elseif use.card:isKindOf("Drowning") then
			for _, player in sgs.qlist(dummy_use.to) do
				if targetlist:contains(player) and (self:isFriend(player) or self:hasTrickEffective(use.card, target, player)) then
					return player
				end
			end
		elseif use.card:isKindOf("Dismantlement") then
			for _, player in sgs.qlist(dummy_use.to) do
				if targetlist:contains(player) and self:isFriend(player) then
					return player
				end
			end
			for _, player in sgs.qlist(dummy_use.to) do
				if targetlist:contains(player) then
					if not self:isFriend(player, target) then
						return player
					elseif not self:needToThrowArmor(target)
						and (target:getJudgingArea():isEmpty()
							or (not target:containsTrick("indulgence")
								and not target:containsTrick("supply_shortage")
								and not (target:containsTrick("lightning") and self:getFinalRetrial(target, "lightning") == 1))) then
						return player
					end
				end
			end
		elseif use.card:isKindOf("Snatch") then
			for _, player in sgs.qlist(dummy_use.to) do
				if targetlist:contains(player) and self:isFriend(player) then
					return player
				end
			end
			local friend = self:findPlayerToDraw(false)
			if friend and targetlist:contains(friend) and self:hasTrickEffective(use.card, target, friend) then
				return friend
			end
		else
			self.room:writeToConsole("playerchosen.zenhui->" .. use.card:getClassName() .. "?")
		end
	end
	return false
end

sgs.ai_skill_playerchosen.zenhui_collateral = function(self, targetlist)
	if self.zenhui_collateral then return self.zenhui_collateral end
	self.room:writeToConsole(debug.traceback())
	return targetlist:at(math.random(0, targetlist:length() - 1))
end

sgs.ai_skill_cardask["@zenhui-give"] = function(self, data)
	local use = data:toCardUse()
	local target = use.to:first()
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	local id = cards[1]:getEffectiveId()

	if use.card:isKindOf("Snatch") then
		if self:isFriend(use.from) then
			if self:askForCardChosen(self.player, "ej", "dummyReason") or not use.from:getAI() then
				return "."
			else
				self:sortByUseValue(cards)
				return cards[1]:getEffectiveId()
			end
		elseif not self:hasTrickEffective(use.card, self.player, use.from) then
			return "."
		end
		return id
	elseif use.card:isKindOf("Slash") then
		if self:slashProhibit(use.card, self.player, use.from)
			or not self:hasHeavySlashDamage(use.from, use.card, self.player) and (self:getDamagedEffects(self.player, use.from, true) or self:needToLoseHp(self.player, use.from)) then
			return "."
		elseif self:isFriend(target) then
			if not self:slashIsEffective(use.card, target, self.player) then
				return id
			end
		elseif not self:isValuableCard(cards[1]) or self:isWeak() or self:getCardsNum("Jink") == 0 or not sgs.isJinkAvailable(use.from, self.player, use.card) then
			return id
		end
		return "."
	elseif use.card:isKindOf("Dismantlement") then
		if not self:hasTrickEffective(use.card, self.player, use.from) then
			return "."
		elseif self:isFriend(use.from) and self:askForCardChosen(self.player, "ej", "dummyReason") then
			return "."
		end
		return id
	elseif use.card:isKindOf("Duel") then
		if self:getDamagedEffects(self.player, use.from) or self:needToLoseHp(self.player, use.from) then
			return "."
		elseif not self:hasTrickEffective(use.card, self.player, use.from) then
			return "."
		elseif self:isFriend(use.from) then
			if (self:getDamagedEffects(use.from, self.player) or self:needToLoseHp(use.from, self.player))
				and self:getCardsNum("Slash") - (cards[1]:isKindOf("Slash") and 1 or 0) > 0 then
				return "."
			end
		else
			if self:getCardsNum("Slash") - (cards[1]:isKindOf("Slash") and 1 or 0) > getCardsNum("Slash", use.from, self.player) then
				return "."
			end
		end
		return id
	elseif use.card:isKindOf("FireAttack") then
		if self:getDamagedEffects(self.player, use.from) or self:needToLoseHp(self.player, use.from) then
			return "."
		end
		return id
	elseif use.card:isKindOf("Collateral") then
		local victim = self.player:getTag("collateralVictim"):toPlayer()
		if sgs.ai_skill_cardask["collateral-slash"](self, nil, nil, victim, use.from) ~= "."
			and self:isFriend(use.from) or not self:isValuableCard(cards[1]) then
			return "."
		end
		return id
	elseif use.card:isKindOf("Drowning") then
		if self:getDamagedEffects(self.player, use.from) or self:needToLoseHp(self.player, use.from) or self:needToThrowArmor() then
			return "."
		elseif self:isValuableCard(cards[1]) and self:isEnemy(use.from) then
			return "."
		end
		return id
	else
		self.room:writeToConsole("@zenhui-give->" .. use.card:getClassName() .. "?")
	end

	return "."
end

sgs.ai_skill_cardask["@jiaojin"] = function(self, data)
	local damage = data:toDamage()
	if self:damageIsEffective_(damage) and not self:getDamagedEffects(damage.to, damage.from, damage.card and damage.card:isKindOf("Slash"))
		and not self:needToLoseHp(damage.to, damage.from, damage.card and damage.card:isKindOf("Slash")) then
		local cards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByKeepValue(cards)
		for _, c in ipairs(cards) do
			if c:getTypeId() == sgs.Card_TypeEquip then return c:getEffectiveId() end
		end
	end
	return "."
end

sgs.ai_skill_choice.qieting = function(self, choices)
	local target = self.room:getCurrent()
	local id = self:askForCardChosen(target, "e", "dummyReason")
	if id then
		for i = 0, 4 do
			if target:getEquip(i) and target:getEquip(i):getEffectiveId() == id and string.find(choices, i) then
				return i
			end
		end
	end
	return "draw"
end


local xianzhou_skill = {}
xianzhou_skill.name = "xianzhou"
table.insert(sgs.ai_skills, xianzhou_skill)
xianzhou_skill.getTurnUseCard = function(self)
	if self.player:getMark("@handover") <= 0 then return end
	if self.player:getEquips():isEmpty() then return end
	return sgs.Card_Parse("@XianzhouCard=.")
end
sgs.ai_skill_use_func.XianzhouCard = function(card, use, self)
	if self:isWeak() then
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkills(sgs.need_equip_skill) then
				use.card = card
				if use.to then use.to:append(friend) end
				return
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if not hasManjuanEffect(friend) then
				use.card = card
				if use.to then use.to:append(friend) end
				return
			end
		end
		self:sort(self.friends)
		for _, target in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			local canUse = true
			for _, friend in ipairs(self.friends) do
				if target:inMyAttackRange(friend) and self:damageIsEffective(friend, nil, target)
					and not self:getDamagedEffects(friend, target) and not self:needToLoseHp(friend, target) then
					canUse = false
					break
				end
			end
			if canUse then
				use.card = card
				if use.to then use.to:append(target) end
				return
			end
		end
	end
	if not self.player:isWounded() then
		local killer
		self:sort(self.friends_noself)
		for _, target in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			local canUse = false
			for _, friend in ipairs(self.friends_noself) do
				if friend:inMyAttackRange(target) and self:damageIsEffective(target, nil, friend)
					and not self:needToLoseHp(target, friend) and self:isWeak(target) then
					canUse = true
					killer = friend
					break
				end
			end
			if canUse then
				use.card = card
				if use.to then use.to:append(killer) end
				return
			end
		end
	end

	if #self.friends_noself == 0 then return end
	if self.player:getEquips():length() > 2 or self.player:getEquips():length() > #self.enemies and sgs.turncount > 2 then
		local function cmp_AttackRange(a, b)
			local ar_a = a:getAttackRange()
			local ar_b = b:getAttackRange()
			if ar_a == ar_b then
				return sgs.getDefense(a) > sgs.getDefense(b)
			else
				return ar_a > ar_b
			end
		end
		table.sort(self.friends_noself, cmp_AttackRange)
		use.card = card
		if use.to then use.to:append(self.friends_noself[1]) end
	end
end

sgs.ai_use_priority.XianzhouCard = 4.9
sgs.ai_card_intention.XianzhouCard = function(self, card, from, tos)
	if not from:isWounded() then sgs.updateIntentions(from, tos, -10) end
end

sgs.ai_skill_use["@xianzhou"] = function(self, prompt)
	local prompt = prompt:split(":")
	local num = prompt[#prompt]
	local current = self.room:getCurrent()
	if self:isWeak(current) and self:isFriend(current) then return "." end
	local targets = {}
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if self.player:inMyAttackRange(enemy) and self:damageIsEffective(enemy, nil, self.player)
			and not self:getDamagedEffects(enemy, self.player) and not self:needToLoseHp(enemy, self.player) then
			table.insert(targets, enemy:objectName())
			if #targets == tonumber(num) then break end
		end
	end
	if #targets < tonumber(num) then
		self:sort(self.friends_noself)
		self.friends_noself = sgs.reverse(self.friends_noself)
		for _, friend in ipairs(self.friends_noself) do
			if self.player:inMyAttackRange(friend) and self:damageIsEffective(friend, nil, self.player)
				and not self:getDamagedEffects(friend, self.player) and not self:needToLoseHp(friend, self.player) then
				table.insert(targets, friend:objectName())
				if #targets == tonumber(num) then break end
			end
		end
	end
	if #targets < tonumber(num) then
		for _, target in sgs.qlist(self.room:getAlivePlayers()) do
			if not self:isFriend(target) and self:isWeak(target) then
				table.insert(targets, target:objectName())
			end
		end
	end

	if #targets > 0 and #targets == tonumber(num) then
		return "@XianzhouDamageCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

sgs.ai_card_intention.XianzhouDamageCard = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if self:damageIsEffective(to, nil, from) and not self:getDamagedEffects(to, from) and not self:needToLoseHp(to, from) then
			sgs.updateIntention(from, to, 10)
		end
	end
end

sgs.ai_skill_invoke.jianying = function(self)
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_playerchosen.youdi = function(self, targets)
	self.youdi_obtain_to_friend = false
	local throw_armor = self:needToThrowArmor()
	if throw_armor and #self.friends_noself > 0 and self.player:getCardCount(true) > 1 then
		for _, friend in ipairs(self.friends_noself) do
			if friend:canDiscard(self.player, self.player:getArmor():getEffectiveId())
				and (self:needToThrowArmor(friend) or (self:needKongcheng(friend) and friend:getHandcardNum() == 1)
					or friend:getHandcardNum() <= self:getLeastHandcardNum(friend)) then
				return friend
			end
		end
	end

	local valuable, dangerous = self:getValuableCard(self.player), self:getDangerousCard(self.player)
	local slash_ratio = 0
	if not self.player:isKongcheng() then
		local slash_count = 0
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if c:isKindOf("Slash") then slash_count = slash_count + 1 end
		end
		slash_ratio = slash_count / self.player:getHandcardNum()
	end
	if not valuable and not dangerous and slash_ratio > 0.45 then return nil end

	self:sort(self.enemies, "defense")
	self.enemies = sgs.reverse(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if enemy:canDiscard(self.player, "he") and not self:doNotDiscard(enemy, "he") then
			if (valuable and enemy:canDiscard(self.player, valuable)) or (dangerous and enemy:canDiscard(self.player, dangerous)) then
				if (self:getValuableCard(enemy) or self:getDangerousCard(enemy)) and sgs.getDefense(enemy) > 8 then return enemy end
			elseif not enemy:isNude() then return enemy
			end
		end
	end
end

sgs.ai_choicemade_filter.cardChosen.youdi_obtain = sgs.ai_choicemade_filter.cardChosen.snatch