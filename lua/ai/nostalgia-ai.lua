sgs.weapon_range.MoonSpear = 3
sgs.ai_use_priority.MoonSpear = 2.635

nosjujian_skill = {}
nosjujian_skill.name = "nosjujian"
table.insert(sgs.ai_skills, nosjujian_skill)
nosjujian_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	if not self.player:hasUsed("NosJujianCard") then return sgs.Card_Parse("@NosJujianCard=.") end
end

sgs.ai_skill_use_func.NosJujianCard = function(card, use, self)
	local abandon_card = {}
	local index = 0
	local hasPeach = (self:getCardsNum("Peach") > 0)
	local to
	local AssistTarget = self:AssistTarget()
	if AssistTarget and self:willSkipPlayPhase(AssistTarget) then AssistTarget = nil end

	local trick_num, basic_num, equip_num = 0, 0, 0
	if not hasPeach and self.player:isWounded() and self.player:getCards("he"):length() >=3 then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		for _, card in ipairs(cards) do
			if card:getTypeId() == sgs.Card_TypeTrick and not isCard("ExNihilo", card, self.player) then trick_num = trick_num + 1
			elseif card:getTypeId() == sgs.Card_TypeBasic then basic_num = basic_num + 1
			elseif card:getTypeId() == sgs.Card_TypeEquip then equip_num = equip_num + 1
			end
		end
		local result_class
		if trick_num >= 3 then result_class = "TrickCard"
		elseif equip_num >= 3 then result_class = "EquipCard"
		elseif basic_num >= 3 then result_class = "BasicCard"
		end

		for _, fcard in ipairs(cards) do
			if fcard:isKindOf(result_class) and not isCard("ExNihilo", fcard, self.player) then
				table.insert(abandon_card, fcard:getId())
				index = index + 1
				if index == 3 then break end
			end
		end

		if index == 3 then
			if AssistTarget and not AssistTarget:hasSkill("manjuan") then
				to = AssistTarget
			else
				to = self:findPlayerToDraw(false, 3)
			end
			if not to then return end
			if use.to then use.to:append(to) end
			use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(abandon_card, "+"))
			return
		end
	end

	abandon_card = {}
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local slash_num = self:getCardsNum("Slash")
	local jink_num = self:getCardsNum("Jink")
	index = 0
	for _, card in ipairs(cards) do
		if index >= 3 then break end
		if card:isKindOf("TrickCard") and not card:isKindOf("Nullification") then
			table.insert(abandon_card, card:getId())
			index = index + 1
		elseif card:isKindOf("EquipCard") then
			table.insert(abandon_card, card:getId())
			index = index + 1
		elseif card:isKindOf("Slash") then
			table.insert(abandon_card, card:getId())
			index = index + 1
			slash_num = slash_num - 1
		elseif card:isKindOf("Jink") and jink_num > 1 then
			table.insert(abandon_card, card:getId())
			index = index + 1
			jink_num = jink_num - 1
		end
	end

	if index == 3 then
		if AssistTarget and not AssistTarget:hasSkill("manjuan") then
			to = AssistTarget
		else
			to = self:findPlayerToDraw(false, 3)
		end
		if not to then return end
		if use.to then use.to:append(to) end
		use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(abandon_card, "+"))
		return
	end

	if self:getOverflow() > 0 then
		local getOverflow = math.max(self:getOverflow(), 0)
		local discard = self:askForDiscard("dummyreason", math.min(getOverflow, 3), nil, false, true)
		if AssistTarget and not AssistTarget:hasSkill("manjuan") and not self:needKongcheng(AssistTarget, true) then
			to = AssistTarget
		else
			to = self:findPlayerToDraw(false, math.min(getOverflow, 3))
		end
		if not to then return end
		use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(discard, "+"))
		if use.to then use.to:append(to) end
		return
	end

	if index > 0 then
		if AssistTarget and not AssistTarget:hasSkill("manjuan") and not self:needKongcheng(AssistTarget, true) then
			to = AssistTarget
		else
			to = self:findPlayerToDraw(false, index)
		end
		if not to then return end
		use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(abandon_card, "+"))
		if use.to then use.to:append(to) end
		return
	end
end

sgs.ai_use_priority.NosJujianCard = 0
sgs.ai_use_value.NosJujianCard = 6.7

sgs.ai_card_intention.NosJujianCard = -100

sgs.dynamic_value.benefit.NosJujianCard = true

sgs.ai_skill_cardask["@nosenyuan-heart"] = function(self, data)
	local damage = data:toDamage()
	if self:needToLoseHp(self.player, damage.to, nil, true) and not self:hasSkills(sgs.masochism_skill) then return "." end
	if self:isFriend(damage.to) then return end
	if self:needToLoseHp() and not self:hasSkills(sgs.masochism_skill) then return "." end

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Heart and not isCard("Peach", card, self.player) and not isCard("ExNihilo", card, self.player) then
			return card:getEffectiveId()
		end
	end
	return "."
end

function sgs.ai_slash_prohibit.nosenyuan(self, from, to, card)
	if from:hasSkill("jueqing") then return false end
	if from:hasSkill("nosqianxi") and from:distanceTo(to) == 1 then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	if self:needToLoseHp(from) and not self:hasSkills(sgs.masochism_skill, from) then return false end
	if from:getHp() > 3 then return false end

	local role = from:objectName() == self.player:objectName() and from:getRole() or sgs.ai_role[from:objectName()]
	if (role == "loyalist" or role == "lord") and sgs.current_mode_players.rebel + sgs.current_mode_players.renegade == 1
		and to:getHp() == 1 and getCardsNum("Peach", to, self.player) < 1 and getCardsNum("Analeptic", to, self.player) < 1
		and (from:getHp() > 1 or getCardsNum("Peach", from, self.player) >= 1 and getCardsNum("Analeptic", from, self.player) >= 1) then
		return false
	end
	if role == "rebel" and isLord(to) and self:getAllPeachNum(player) < 1 and to:getHp() == 1
		and (from:getHp() > 1 or getCardsNum("Peach", from, self.player) >= 1 and getCardsNum("Analeptic", from, self.player) >= 1) then
		return false
	end
	if role == "renegade" and from:aliveCount() == 2 and to:getHp() == 1 and getCardsNum("Peach", to, self.player) < 1 and getCardsNum("Analeptic", to, self.player) < 1
		and (from:getHp() > 1 or getCardsNum("Peach", from, self.player) >= 1 and getCardsNum("Analeptic", from, self.player) >= 1) then
		return false
	end

	local n = 0
	local cards = from:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Heart and not isCard("Peach", card, from) and not isCard("ExNihilo", card, from) then
			if not card:isKindOf("Slash") then return false end
			n = n + 1
		end
	end
	if n < 1 then return true end
	if n > 1 then return false end
	if n == 1 then return card:getSuit() == sgs.Card_Heart end
	return self:isWeak(from)
end

sgs.ai_need_damaged.nosenyuan = function (self, attacker, player)
	if player:hasSkill("nosenyuan") and attacker and self:isEnemy(attacker, player) and self:isWeak(attacker)
		and not (self:needToLoseHp(attacker) and not self:hasSkills(sgs.masochism_skill, attacker)) then
			return true
	end
	return false
end

nosxuanhuo_skill = {}
nosxuanhuo_skill.name = "nosxuanhuo"
table.insert(sgs.ai_skills, nosxuanhuo_skill)
nosxuanhuo_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("NosXuanhuoCard") then
		return sgs.Card_Parse("@NosXuanhuoCard=.")
	end
end

sgs.ai_skill_use_func.NosXuanhuoCard = function(card, use, self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)

	local target
	for _, friend in ipairs(self.friends_noself) do
		if self:hasSkills(sgs.lose_equip_skill, friend) and not friend:getEquips():isEmpty() and not friend:hasSkill("manjuan") then
			target = friend
			break
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if self:getDangerousCard(enemy) then
				target = enemy
				break
			end
		end
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if self:needToThrowArmor(friend) and not friend:hasSkill("manjuan") then
				target = friend
				break
			end
		end
	end
	if not target then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if self:getValuableCard(enemy) then
				target = enemy
				break
			end
			if target then break end

			local cards = sgs.QList2Table(enemy:getHandcards())
			local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
			if not enemy:isKongcheng() and not enemy:hasSkills("tuntian+zaoxian") then
				for _, cc in ipairs(cards) do
					if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) then
						target = enemy
						break
					end
				end
			end
			if target then break end

			if self:getValuableCard(enemy) then
				target = enemy
				break
			end
			if target then break end
		end
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkills("tuntian+zaoxian") and not friend:hasSkill("manjuan") then
				target = friend
				break
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude() and enemy:hasSkill("manjuan") then
				target = enemy
				break
			end
		end
	end

	if target then
		local willUse
		if self:isFriend(target) then
			for _, card in ipairs(cards) do
				if card:getSuit() == sgs.Card_Heart then
					willUse = card
					break
				end
			end
		else
			for _, card in ipairs(cards) do
				if card:getSuit() == sgs.Card_Heart and not isCard("Peach", card, target) and not isCard("Nullification", card, target) then
					willUse = card
					break
				end
			end
		end

		if willUse then
			target:setFlags("AI_NosXuanhuoTarget")
			use.card = sgs.Card_Parse("@NosXuanhuoCard=" .. willUse:getEffectiveId())
			if use.to then use.to:append(target) end
		end
	end
end

sgs.ai_skill_playerchosen.nosxuanhuo = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if (player:getHandcardNum() <= 2 or player:getHp() < 2) and self:isFriend(player)
			and not player:hasFlag("AI_NosXuanhuoTarget") and not self:needKongcheng(player, true) and not player:hasSkill("manjuan") then
			return player
		end
	end
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player)
			and not player:hasFlag("AI_NosXuanhuoTarget") and not self:needKongcheng(player, true) and not player:hasSkill("manjuan") then
			return player
		end
	end
	for _, player in sgs.qlist(targets) do
		if player == self.player then
			return player
		end
	end
end

sgs.nosxuanhuo_suit_value = {
	heart = 3.9
}



sgs.ai_cardneed.nosxuanhuo = function(to, card)
	return card:getSuit() == sgs.Card_Heart
end

sgs.ai_skill_choice.nosxuanfeng = function(self, choices)
	self:sort(self.enemies, "defenseSlash")
	local slash = sgs.Sanguosha:cloneCard("slash")
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then
			return "damage"
		elseif not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then
			return "slash"
		end
	end
	return "nothing"
end

sgs.ai_skill_playerchosen.nosxuanfeng_damage = sgs.ai_skill_playerchosen.damage
sgs.ai_skill_playerchosen.nosxuanfeng_slash = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_playerchosen_intention.nosxuanfeng_damage = 80
sgs.ai_playerchosen_intention.nosxuanfeng_slash = 80

sgs.nosxuanfeng_keep_value = sgs.xiaoji_keep_value

sgs.ai_cardneed.nosxuanfeng = sgs.ai_cardneed.equip

sgs.ai_skill_invoke.nosshangshi = sgs.ai_skill_invoke.shangshi

sgs.ai_view_as.nosgongqi = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_PlaceSpecial and card:getTypeId() == sgs.Card_TypeEquip and not card:hasFlag("using") then
		return ("slash:nosgongqi[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local nosgongqi_skill = {}
nosgongqi_skill.name = "nosgongqi"
table.insert(sgs.ai_skills, nosgongqi_skill)
nosgongqi_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)

	local equip_card
	self:sortByUseValue(cards, true)

	for _, card in ipairs(cards) do
		if card:getTypeId() == sgs.Card_TypeEquip and (self:getUseValue(card) < sgs.ai_use_value.Slash or inclusive) then
			equip_card = card
			break
		end
	end

	if equip_card then
		local suit = equip_card:getSuitString()
		local number = equip_card:getNumberString()
		local card_id = equip_card:getEffectiveId()
		local card_str = ("slash:nosgongqi[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)

		return slash
	end
end


function sgs.ai_cardneed.nosgongqi(to, card, self)
	return card:getTypeId() == sgs.Card_TypeEquip and getKnownCard(to, self.player, "EquipCard", true) == 0
end

function sgs.ai_cardsview_valuable.nosjiefan(self, class_name, player)
	if class_name == "Peach" and not player:hasFlag("Global_NosJiefanFailed") then
		local dying = player:getRoom():getCurrentDyingPlayer()
		if not dying then return nil end
		local current = player:getRoom():getCurrent()
		if not current or current:isDead() or current:getPhase() == sgs.Player_NotActive
			or current:objectName() == player:objectName() or (current:hasSkill("wansha") and player:objectName() ~= dying:objectName())
			or (self:isEnemy(current) and self:findLeijiTarget(current, 50, player)) then return nil end
		return "@NosJiefanCard=."
	end
end

sgs.ai_card_intention.NosJiefanCard = sgs.ai_card_intention.Peach

sgs.ai_skill_cardask["nosjiefan-slash"] = function(self, data, pattern, target)
	if self:isEnemy(target) and self:findLeijiTarget(target, 50, self.player) then return "." end
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, target) then
			return slash:toString()
		end
	end
	return "."
end

function sgs.ai_cardneed.nosjiefan(to, card, self)
	return isCard("Slash", card, to) and getKnownCard(to, self.player,"Slash", true) == 0
end

sgs.ai_skill_invoke.nosfuhun = function(self, data)
	local target = 0
	for _, enemy in ipairs(self.enemies) do
		if (self.player:distanceTo(enemy) <= self.player:getAttackRange()) then target = target + 1 end
	end
	return target > 0 and not self.player:isSkipped(sgs.Player_Play)
end

sgs.ai_skill_invoke.noszhenlie = function(self, data)
	local judge = data:toJudge()
	if not judge:isGood() then
	return true end
	return false
end

sgs.ai_skill_playerchosen.nosmiji = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defense")
	local n = self.player:getLostHp()
	if self.player:getPhase() == sgs.Player_Start then
		if self.player:getHandcardNum() - n < 2 and not self:needKongcheng() and not self:willSkipPlayPhase() then return self.player end
	elseif self.player:getPhase() == sgs.Player_Finish then
		if self.player:getHandcardNum() - n < 2 and not self:needKongcheng() then return self.player end
	end
	local to = self:findPlayerToDraw(true, n)
	return to or self.player
end

sgs.ai_playerchosen_intention.nosmiji = function(self, from, to)
	if not (self:needKongcheng(to, true) and from:getLostHp() == 1)
		and not hasManjuanEffect(to) then
		sgs.updateIntention(from, to, -10)
	end
end

sgs.ai_skill_invoke.nosqianxi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return false end
	if target:getLostHp() >= 2 and target:getHp() <= 1 then return false end
	if target:hasSkills(sgs.masochism_skill .. "|" .. sgs.recover_skill .. "|longhun|buqu|nosbuqu") then return true end
	if self:hasHeavySlashDamage(self.player, damage.card, target) then return false end
	return (target:getMaxHp() - target:getHp()) < 2
end

function sgs.ai_cardneed.nosqianxi(to, card, self)
	return isCard("Slash", card, to) and getKnownCard(to, self.player, "Slash", true) == 0
end

local noslijian_skill = {}
noslijian_skill.name = "noslijian"
table.insert(sgs.ai_skills, noslijian_skill)
noslijian_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("NosLijianCard") or self.player:isNude() then
		return
	end
	local card_id = self:getLijianCard()
	if card_id then return sgs.Card_Parse("@NosLijianCard=" .. card_id) end
end

sgs.ai_skill_use_func.NosLijianCard = function(card, use, self)
	local first, second = self:findLijianTarget("NosLijianCard", use)
	if first and second then
		use.card = card
		if use.to then
			use.to:append(first)
			use.to:append(second)
		end
	end
end

sgs.ai_use_value.NosLijianCard = sgs.ai_use_value.LijianCard
sgs.ai_use_priority.NosLijianCard = sgs.ai_use_priority.LijianCard

noslijian_filter = function(self, player, carduse)
	if carduse.card:isKindOf("NosLijianCard") then
		sgs.ai_lijian_effect = true
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, noslijian_filter)

sgs.ai_card_intention.NosLijianCard = sgs.ai_card_intention.LijianCard

local nosrende_skill = {}
nosrende_skill.name = "nosrende"
table.insert(sgs.ai_skills, nosrende_skill)
nosrende_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return end
	local mode = string.lower(global_room:getMode())
	if self.player:getMark("nosrende") > 1 and mode:find("04_1v3") then return end

	if self:shouldUseRende() then
		return sgs.Card_Parse("@NosRendeCard=.")
	end
end

sgs.ai_skill_use_func.NosRendeCard = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	local notFound

	for i = 1, self.player:getHandcardNum() do
		local card, friend = self:getCardNeedPlayer(cards)
		if card and friend then
			cards = self:resetCards(cards, card)
		else
			notFound = true
			break
		end

		if friend:objectName() == self.player:objectName() or not self.player:getHandcards():contains(card) then continue end
		local canJijiang = self.player:hasLordSkill("jijiang") and friend:getKingdom() == "shu"
		if card:isAvailable(self.player) and ((card:isKindOf("Slash") and not canJijiang) or card:isKindOf("Duel") or card:isKindOf("Snatch") or card:isKindOf("Dismantlement")) then
			local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
			local cardtype = card:getTypeId()
			self["use" .. sgs.ai_type_name[cardtype + 1] .. "Card"](self, card, dummy_use)
			if dummy_use.card and dummy_use.to:length() > 0 then
				if card:isKindOf("Slash") or card:isKindOf("Duel") then
					local t1 = dummy_use.to:first()
					if dummy_use.to:length() > 1 then continue
					elseif t1:getHp() == 1 or sgs.card_lack[t1:objectName()]["Jink"] == 1
							or t1:isCardLimited(sgs.Sanguosha:cloneCard("jink"), sgs.Card_MethodResponse) then continue
					end
				elseif (card:isKindOf("Snatch") or card:isKindOf("Dismantlement")) and self:getEnemyNumBySeat(self.player, friend) > 0 then
					local hasDelayedTrick
					for _, p in sgs.qlist(dummy_use.to) do
						if self:isFriend(p) and (self:willSkipDrawPhase(p) or self:willSkipPlayPhase(p)) then hasDelayedTrick = true break end
					end
					if hasDelayedTrick then continue end
				end
			end
		elseif card:isAvailable(self.player) and self:getEnemyNumBySeat(self.player, friend) > 0 and (card:isKindOf("Indulgence") or card:isKindOf("SupplyShortage")) then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then continue end
		end

		if friend:hasSkill("enyuan") and #cards >= 1 and not (self.room:getMode() == "04_1v3" and self.player:getMark("nosrende") == 1) then
			use.card = sgs.Card_Parse("@NosRendeCard=" .. card:getId() .. "+" .. cards[1]:getId())
		else
			use.card = sgs.Card_Parse("@NosRendeCard=" .. card:getId())
		end
		if use.to then use.to:append(friend) end
		return
	end

	if notFound then
		local pangtong = self.room:findPlayerBySkillName("manjuan")
		if not pangtong then return end
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		if self.player:isWounded() and self.player:getHandcardNum() > 3 and self.player:getMark("nosrende") < 2 then
			self:sortByUseValue(cards, true)
			local to_give = {}
			for _, card in ipairs(cards) do
				if not isCard("Peach", card, self.player) and not isCard("ExNihilo", card, self.player) then table.insert(to_give, card:getId()) end
				if #to_give == 2 - self.player:getMark("nosrende") then break end
			end
			if #to_give > 0 then
				use.card = sgs.Card_Parse("@NosRendeCard=" .. table.concat(to_give, "+"))
				if use.to then use.to:append(pangtong) end
			end
		end
	end
end

sgs.ai_use_value.NosRendeCard = sgs.ai_use_value.RendeCard
sgs.ai_use_priority.NosRendeCard = sgs.ai_use_priority.RendeCard

sgs.ai_card_intention.NosRendeCard = sgs.ai_card_intention.RendeCard

sgs.dynamic_value.benefit.NosRendeCard = true

function sgs.ai_cardneed.nosjizhi(to, card)
	return card:isNDTrick()
end

sgs.nosjizhi_keep_value = sgs.jizhi_keep_value



function sgs.ai_skill_invoke.nosjushou(self, data)
	local sbdiaochan = self.room:findPlayerBySkillName("lihun")
	if sbdiaochan and sbdiaochan:faceUp() and not self:willSkipPlayPhase(sbdiaochan)
		and (self:isEnemy(sbdiaochan) or (sgs.turncount <= 1 and sgs.evaluatePlayerRole(sbdiaochan) == "neutral")) then return false end
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if self:hasSkills("fangzhu|jilve", friend) then return true end
		if friend:hasSkill("junxing") and friend:faceUp() and not self:willSkipPlayPhase(friend)
			and not (friend:isKongcheng() and self:willSkipDrawPhase(friend)) then
			return true
		end
	end
	return self:isWeak()
end

sgs.ai_skill_askforag.nosbuqu = function(self, card_ids)
	for i, card_id in ipairs(card_ids) do
		for j, card_id2 in ipairs(card_ids) do
			if i ~= j and sgs.Sanguosha:getCard(card_id):getNumber() == sgs.Sanguosha:getCard(card_id2):getNumber() then
				return card_id
			end
		end
	end

	return card_ids[1]
end

function sgs.ai_skill_invoke.nosbuqu(self, data)
	if #self.enemies == 1 and self.enemies[1]:hasSkill("nosguhuo") then
		return false
	else
		local damage = data:toDamage()
		if self.player:getHp() == 1 and damage.to and damage:getReason() == "duwu" and self:getSaveNum(true) >= 1 then return false end
		return true
	end
end



sgs.ai_skill_playerchosen.nosleiji = function(self, targets)
	local mode = self.room:getMode()
	if mode:find("_mini_17") or mode:find("_mini_19") or mode:find("_mini_20") or mode:find("_mini_26") then
		local players = self.room:getAllPlayers()
		for _, aplayer in sgs.qlist(players) do
			if aplayer:getState() ~= "robot" then
				return aplayer
			end
		end
	end

	self:updatePlayers()
	return self:findLeijiTarget(self.player, 100, nil, -1)
end

sgs.ai_playerchosen_intention.nosleiji = sgs.ai_playerchosen_intention.leiji

function sgs.ai_slash_prohibit.nosleiji(self, from, to, card)
	if self:isFriend(to, from) then return false end
	if to:hasFlag("QianxiTarget") and (not self:hasEightDiagramEffect(to) or self.player:hasWeapon("qinggang_sword")) then return false end
	local hcard = to:getHandcardNum()
	if from:hasSkill("liegong") and (hcard >= from:getHp() or hcard <= from:getAttackRange()) then return false end
	if from:hasSkill("kofliegong") and hcard >= from:getHp() then return false end
	if from:getRole() == "rebel" and to:isLord() then
		local other_rebel
		for _, player in sgs.qlist(self.room:getOtherPlayers(from)) do
			if sgs.evaluatePlayerRole(player) == "rebel" or sgs.compareRoleEvaluation(player, "rebel", "loyalist") == "rebel" then
				other_rebel = player
				break
			end
		end
		if not other_rebel and ((from:getHp() >= 4 and (getCardsNum("Peach", from, self.player) > 0 or from:hasSkills("nosganglie|vsnosganglie"))) or from:hasSkill("hongyan")) then
			return false
		end
	end

	if sgs.card_lack[to:objectName()]["Jink"] == 2 then return true end
	if getKnownCard(to, from, "Jink", true) >= 1 or (self:hasSuit("spade", true, to) and hcard >= 2) or hcard >= 4 then return true end
	if self:hasEightDiagramEffect(to) then return true end
end

sgs.ai_cardneed.nosleiji = sgs.ai_cardneed.leiji

table.insert(sgs.ai_global_flags, "questioner")

sgs.ai_skill_choice.nosguhuo = function(self, choices)
	local yuji = self.room:findPlayerBySkillName("nosguhuo")
	local nosguhuoname = self.room:getTag("NosGuhuoType"):toString()
	if nosguhuoname == "peach+analeptic" then nosguhuoname = "peach" end
	if nosguhuoname == "normal_slash" then nosguhuoname = "slash" end
	local nosguhuocard = sgs.Sanguosha:cloneCard(nosguhuoname)
	local nosguhuotype = nosguhuocard:getClassName()
	if nosguhuotype and self:getRestCardsNum(nosguhuotype, yuji) == 0 and self.player:getHp() > 0 then return "question" end
	if nosguhuotype and nosguhuotype == "AmazingGrace" then return "noquestion" end
	if nosguhuotype:match("Slash") then
		if yuji:getState() ~= "robot" and math.random(1, 4) == 1 and not sgs.questioner and self:isEnemy(yuji) then return "question" end
		if not self:hasCrossbowEffect(yuji) then return "noquestion" end
	end
	if yuji:hasFlag("NosGuhuoFailed") and math.random(1, 6) == 1 and self:isEnemy(yuji) and self.player:getHp() >= 3
		and self.player:getHp() > self.player:getLostHp() then return "question" end
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local x = math.random(1, 5)

	self:sort(self.friends, "hp")
	if self.player:getHp() < 2 and self:getCardsNum("Peach") < 1 and self.room:alivePlayerCount() > 2 then return "noquestion" end
	if self:isFriend(yuji) then return "noquestion"
	elseif sgs.questioner then return "noquestion"
	elseif self.player:getHp() < self.friends[#self.friends]:getHp() then return "noquestion"
	end
	if self:needToLoseHp(self.player) and not self.player:hasSkills(sgs.masochism_skill) and x ~= 1 and self:isEnemy(yuji) then return "question" end

	local questioner
	for _, friend in ipairs(self.friends) do
		if friend:getHp() == self.friends[#self.friends]:getHp() then
			if friend:hasSkills("nosrende|rende|kuanggu|kofkuanggu|zaiqi|buqu|nosbuqu|yinghun|longhun|xueji|baobian") then
				questioner = friend
				break
			end
		end
	end
	if not questioner then questioner = self.friends[#self.friends] end
	return self.player:objectName() == questioner:objectName() and x ~= 1 and "question" or "noquestion"
end

sgs.ai_choicemade_filter.skillChoice.nosguhuo = function(self, player, promptlist)
	if promptlist[#promptlist] == "question" then
		sgs.questioner = player
		local yuji = self.room:findPlayerBySkillName("nosguhuo")
		if not yuji then return end
		local nosguhuoname = self.room:getTag("NosGuhuoType"):toString()
		if nosguhuoname == "peach+analeptic" or nosguhuoname == "peach" then
			sgs.updateIntention(player, yuji, 80)
			return
		end
		if nosguhuoname == "normal_slash" then nosguhuoname = "slash" end
		local nosguhuocard = sgs.Sanguosha:cloneCard(nosguhuoname)
		if nosguhuocard then
			local nosguhuotype = nosguhuocard:getClassName()
			if nosguhuotype and self:getRestCardsNum(nosguhuotype, yuji) > 0 then
				sgs.updateIntention(player, yuji, 80)
				return
			end
		end
	end
end
local nosguhuo_skill = {}
nosguhuo_skill.name = "nosguhuo"
table.insert(sgs.ai_skills, nosguhuo_skill)
nosguhuo_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return end

	local cards = sgs.QList2Table(self.player:getHandcards())
	local otherSuit_str, NosGuhuoCard_str = {}, {}

	for _, card in ipairs(cards) do
		if card:isNDTrick() then
			local dummyuse = { isDummy = true }
			self:useTrickCard(card, dummyuse)
			if dummyuse.card then
				local cardstr = "@NosGuhuoCard=" .. card:getId() .. ":" .. card:objectName()
				if card:getSuit() == sgs.Card_Heart then
					table.insert(NosGuhuoCard_str, cardstr)
				else
					table.insert(otherSuit_str, cardstr)
				end
			end
		end
	end

	local other_suit, enemy_is_weak, zgl_kongcheng = true
	local can_fake_nosguhuo = sgs.turncount > 1
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > 2 then
			other_suit = false
		end
		if enemy:getHp() > 1 then
			can_fake_nosguhuo = false
		end
		if self:isWeak(enemy) then
			enemy_is_weak = true
		end
		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
			zgl_kongcheng = true
		end
	end

	if #otherSuit_str > 0 and other_suit then
		table.insertTable(NosGuhuoCard_str, otherSuit_str)
	end

	local peach_str = self:getGuhuoCard("Peach", true, -1)
	if peach_str then table.insert(NosGuhuoCard_str, peach_str) end

	local fakeCards = {}

	for _, card in sgs.qlist(self.player:getHandcards()) do
		if (card:isKindOf("Slash") and self:getCardsNum("Slash", "h") >= 2 and not self:hasCrossbowEffect())
			or (card:isKindOf("Jink") and self:getCardsNum("Jink", "h") >= 3)
			or (card:isKindOf("EquipCard") and self:getSameEquip(card))
			or card:isKindOf("Disaster") then
			table.insert(fakeCards, card)
		end
	end
	self:sortByUseValue(fakeCards, true)

	local function fake_nosguhuo(objectName, can_fake_nosguhuo)
		if #fakeCards == 0 then return end

		local fakeCard
		local nosguhuo = "peach|ex_nihilo|snatch|dismantlement|amazing_grace|archery_attack|savage_assault|god_salvation"
		local ban = table.concat(sgs.Sanguosha:getBanPackages(), "|")
		if not ban:match("maneuvering") then nosguhuo = nosguhuo .. "|fire_attack" end
		local nosguhuos = nosguhuo:split("|")
		for i = 1, #nosguhuos do
			local forbidden = nosguhuos[i]
			local forbid = sgs.Sanguosha:cloneCard(forbidden)
			if self.player:isLocked(forbid) then
				table.remove(nosguhuos, i)
				i = i - 1
			end
		end
		if can_fake_nosguhuo then
			for i = 1, #nosguhuos do
				if nosguhuos[i] == "god_salvation" then table.remove(nosguhuos, i) break end
			end
		end
		for i = 1, 10 do
			local card = fakeCards[math.random(1, #fakeCards)]
			local newnosguhuo = objectName or nosguhuos[math.random(1, #nosguhuos)]
			local nosguhuocard = sgs.Sanguosha:cloneCard(newnosguhuo, card:getSuit(), card:getNumber())
			if self:getRestCardsNum(nosguhuocard:getClassName()) > 0 then
				local dummyuse = { isDummy = true }
				if newnosguhuo == "peach" then self:useBasicCard(nosguhuocard, dummyuse) else self:useTrickCard(nosguhuocard, dummyuse) end
				if dummyuse.card then
					fakeCard = sgs.Card_Parse("@NosGuhuoCard=" .. card:getId() .. ":" .. newnosguhuo)
					break
				end
			end
		end
		return fakeCard
	end

	if #NosGuhuoCard_str > 0 then
		local nosguhuo_str = NosGuhuoCard_str[math.random(1, #NosGuhuoCard_str)]

		local str = nosguhuo_str:split("=")
		str = str[2]:split(":")
		local cardid, cardname = str[1], str[2]
		if sgs.Sanguosha:getCard(cardid):objectName() == cardname and cardname == "ex_nihilo" then
			if math.random(1, 3) == 1 then
				local fake_exnihilo = fake_nosguhuo(cardname)
				if fake_exnihilo then return fake_exnihilo end
			end
			return sgs.Card_Parse(nosguhuo_str)
		elseif math.random(1, 5) == 1 then
			local fake_NosGuhuoCard = fake_nosguhuo()
			if fake_NosGuhuoCard then return fake_NosGuhuoCard end
		else
			return sgs.Card_Parse(nosguhuo_str)
		end
	elseif can_fake_nosguhuo and math.random(1, 4) ~= 1 then
		local fake_NosGuhuoCard = fake_nosguhuo(nil, can_fake_nosguhuo)
		if fake_NosGuhuoCard then return fake_NosGuhuoCard end
	elseif zgl_kongcheng and #fakeCards > 0 then
		return sgs.Card_Parse("@NosGuhuoCard=" .. fakeCards[1]:getEffectiveId() .. ":amazing_grace")
	else
		local lord = self.room:getLord()
		local drawcard = false
		if lord and self:isFriend(lord) and self:isWeak(lord) and not self.player:isLord() then
			drawcard = true
		elseif not enemy_is_weak then
			if sgs.current_mode_players["loyalist"] > sgs.current_mode_players["renegade"] + sgs.current_mode_players["rebel"]
				and self.role == "loyalist" and sgs.current_mode_players["rebel"] > 0 then
				drawcard = true
			elseif sgs.current_mode_players["rebel"] > sgs.current_mode_players["loyalist"] + sgs.current_mode_players["renegade"] + 2
				and self.role == "rebel" then
				drawcard = true
			end
		end

		if drawcard and #fakeCards > 0 then
			local card_objectname
			local objectNames = { "ex_nihilo", "snatch", "dismantlement", "amazing_grace", "archery_attack", "savage_assault", "god_salvation", "duel" }
			for _, objectName in ipairs(objectNames) do
				local acard = sgs.Sanguosha:cloneCard(objectName)
				if self:getRestCardsNum(acard:getClassName()) == 0 then
					card_objectname = objectName
					break
				end
			end
			if card_objectname then
				return sgs.Card_Parse("@NosGuhuoCard=" .. fakeCards[1]:getEffectiveId() .. ":" .. card_objectname)
			end
		end
	end

	if self:isWeak() then
		local peach_str = self:getGuhuoCard("Peach", true, -1)
		if peach_str then
			local card = sgs.Card_Parse(peach_str)
			local peach = sgs.Sanguosha:cloneCard("peach", card:getSuit(), card:getNumber())
			local dummy_use = { isDummy = true }
			self:useBasicCard(peach, dummy_use)
			if dummy_use.card then return card end
		end
	end
	local slash_str = self:getGuhuoCard("Slash", true, -1)
	if slash_str and self:slashIsAvailable() then
		local card = sgs.Card_Parse(slash_str)
		local slash = sgs.Sanguosha:cloneCard("slash", card:getSuit(), card:getNumber())
		local dummy_use = { isDummy = true }
		self:useBasicCard(slash, dummy_use)
		if dummy_use.card then return card end
	end
end

sgs.ai_skill_use_func.NosGuhuoCard = function(card, use, self)
	local userstring = card:toString()
	userstring = (userstring:split(":"))[3]
	local nosguhuocard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	nosguhuocard:setSkillName("nosguhuo")
	if nosguhuocard:getTypeId() == sgs.Card_TypeBasic then
		if not use.isDummy and use.card and nosguhuocard:isKindOf("Slash") and (not use.to or use.to:isEmpty()) then return end
		self:useBasicCard(nosguhuocard, use)
	else
		assert(nosguhuocard)
		self:useTrickCard(nosguhuocard, use)
	end
	if not use.card then return end
	use.card = card
end

sgs.ai_use_priority.NosGuhuoCard = 10

sgs.nosguhuo_suit_value = {
	heart = 5,
}

sgs.ai_skill_choice.nosguhuo_saveself = sgs.ai_skill_choice.guhuo_saveself
sgs.ai_skill_choice.nosguhuo_slash = sgs.ai_skill_choice.guhuo_slash

function sgs.ai_cardneed.nosguhuo(to, card)
	return card:getSuit() == sgs.Card_Heart and (card:isKindOf("BasicCard") or card:isNDTrick())
end


sgs.ai_skill_invoke.nosdanshou = function(self, data)
	local damage = data:toDamage()
	local phase = self.player:getPhase()
	if phase < sgs.Player_Play then
		return self:willSkipPlayPhase()
	elseif phase == sgs.Player_Play then
		if damage.chain or self.room:getTag("is_chained"):toInt() > 0 then
			for _, ap in sgs.qlist(self.room:getAllPlayers()) do
				if ap:isChained() and self:isGoodChainTarget(ap, self.player, damage.nature, damage.damage, damage.card) then return false end
			end
			return true
		elseif self:getOverflow() >= 2 then
			return true
		else
			if damage.chain or self.room:getTag("is_chained"):toInt() > 0 then
				local nextp
				for _, p in sgs.qlist(self.room:getAllPlayers()) do
					if p:isChained() and self:damageIsEffective(p, damage.nature, self.player) then
						nextp = p
						break
					end
				end
				return not nextp or self:isFriend(nextp)
			end
			if damage.card and damage.card:isKindOf("Slash") and self:getCardsNum("Slash") >= 1 and self:slashIsAvailable() then
				return false
			end
			if (damage.card and damage.card:isKindOf("AOE")) or (self.player:hasFlag("ShenfenUsing") and self.player:faceUp()) then
				if damage.to:getNextAlive():objectName() == self.player:objectName() then return true
				else
					local dmg_val = 0
					local p = damage.to
					repeat
						if self:damageIsEffective(p, damage.nature, self.player) then
							if self:isFriend(p) then
								dmg_val = dmg_val + 1
							else
								if self:cantbeHurt(p, self.player, damage.damage) then dmg_val = dmg_val + 1 end
								if self:getDamagedEffects(p, self.player) then dmg_val = dmg_val + 0.5 end
								if self:isEnemy(p) then dmg_val = dmg_val - 1 end
							end
						end
						p = p:getNextAlive()
					until p:objectName() == self.player:objectName()
					return dmg_val >= 1.5
				end
			end
			if damage.to:hasSkills(sgs.masochism_skill .. "|zhichi|zhiyu|fenyong") then return self:isEnemy(damage.to) end
			return true
		end
	elseif phase > sgs.Player_Play and phase ~= sgs.Player_NotActive then
		return true
	elseif phase == sgs.Player_NotActive then
		local current = self.room:getCurrent()
		if not current or not current:isAlive() or current:getPhase() == sgs.Player_NotActive then return true end
		if self:isFriend(current) then
			return self:getOverflow(current) >= 2
		else
			if self:getOverflow(current) <= 2 then
				return true
			else
				local threat = getCardsNum("Duel", current, self.player) + getCardsNum("AOE", current, self.player)
				if self:slashIsAvailable(current) and getCardsNum("Slash", current, self.player) > 0 then threat = threat + math.min(1, getCardsNum("Slash", current, self.player)) end
				return threat >= 1
			end
		end
	end
	return false
end

sgs.ai_skill_invoke.noszhuikong = function(self, data)
	if self.player:getHandcardNum() <= (self:isWeak() and 2 or 1) then return false end
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

sgs.ai_skill_playerchosen.nosqiuyuan = function(self, targets)
	local targetlist = sgs.QList2Table(targets)
	self:sort(targetlist, "handcard")
	local enemy
	for _, p in ipairs(targetlist) do
		if self:isEnemy(p) and not (p:getHandcardNum() == 1 and (p:hasSkill("kongcheng") or (p:hasSkill("zhiji") and p:getMark("zhiji") == 0))) then
			if p:hasSkills(sgs.cardneed_skill) then return p
			elseif not enemy and not self:canLiuli(p, self.friends_noself) then enemy = p end
		end
	end
	if enemy then return enemy end
	targetlist = sgs.reverse(targetlist)
	local friend
	for _, p in ipairs(targetlist) do
		if self:isFriend(p) then
			if (p:hasSkill("kongcheng") and p:getHandcardNum() == 1) or (p:getCardCount() >= 2 and self:canLiuli(p, self.enemies)) then return p
			elseif not friend and getCardsNum("Jink", p, self.player) >= 1 then friend = p end
		end
	end
	return friend
end

sgs.ai_skill_cardask["@nosqiuyuan-give"] = function(self, data, pattern, target)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		local e_card = sgs.Sanguosha:getEngineCard(card:getEffectiveId())
		if e_card:isKindOf("Jink")
			and not (target and target:isAlive() and target:hasSkill("wushen") and (e_card:getSuit() == sgs.Card_Heart or (target:hasSkill("hongyan") and e_card:getSuit() == sgs.Card_Spade))) then
			return "$" .. card:getEffectiveId()
		end
	end
	if (pattern == "Jink") then return "." end
	for _, card in ipairs(cards) do
		if not self:isValuableCard(card) and self:getKeepValue(card) < 5 then return "$" .. card:getEffectiveId() end
	end
	return "$" .. cards[1]:getEffectiveId()
end

function sgs.ai_slash_prohibit.nosqiuyuan(self, from, to)
	if self:isFriend(to, from) then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	for _, friend in ipairs(self:getFriendsNoself(from)) do
		if not to:isKongcheng() and not (to:getHandcardNum() == 1 and (to:hasSkill("kongcheng") or (to:hasSkill("zhiji") and to:getMark("zhiji") == 0))) then return true end
	end
end

function SmartAI:hasNosQiuyuanEffect(from, to)
	if not from or not to:hasSkill("nosqiuyuan") then return false end
	if getKnownCard(to, self.player, "Jink", true, "he") >= 1 then
		for _, target in ipairs(self:getEnemies(to)) do
			if not target:isKongcheng() and not (target:getHandcardNum() == 1 and self:needKongcheng(target, true)) then
				return true
			end
		end
		for _, friend in ipairs(self:getFriends(to)) do
			if (friend:getHandcardNum() == 1 and self:needKongcheng(friend, true))
					and not (friend:isKongcheng() and to and from) then
				return true
			end
		end
	end
	return
end

sgs.ai_skill_invoke.nosjuece = function(self, data)
	local move = data:toMoveOneTime()
	if not move.from then return false end
	local from = findPlayerByObjectName(self.room, move.from:objectName())
	return from and ((self:isFriend(from) and self:getDamagedEffects(from, self.player)) or (not self:isFriend(from) and self:canAttack(from)))
end

sgs.ai_skill_playerchosen.nosmieji = function(self, targets) -- extra target for Ex Nihilo
	return self:findPlayerToDraw(false, 2)
end

sgs.ai_playerchosen_intention.nosmieji = -50

sgs.ai_skill_use["@@nosmieji"] = function(self, prompt) -- extra target for Collateral
	local collateral = sgs.Sanguosha:cloneCard("collateral", sgs.Card_NoSuitBlack)
	local dummy_use = { isDummy = true, to = sgs.SPlayerList(), current_targets = {} }
	dummy_use.current_targets = self.player:property("extra_collateral_current_targets"):toString():split("+")
	self:useCardCollateral(collateral, dummy_use)
	if dummy_use.card and dummy_use.to:length() == 2 then
		local first = dummy_use.to:at(0):objectName()
		local second = dummy_use.to:at(1):objectName()
		return "@ExtraCollateralCard=.->" .. first .. "+" .. second
	end
end

sgs.ai_card_intention.ExtraCollateralCard = 0

local function getNosFenchengValue(self, player)
	if not self:damageIsEffective(player, sgs.DamageStruct_Fire, self.player) then return 0 end
	if not player:canDiscard(player, "he") then return self:isWeak(player) and 1.5 or 1 end
	if self.player:hasSkill("juece|nosjuece") and self:isEnemy(player)
		and player:getEquips():isEmpty() and player:getHandcardNum() == 1 and not self:needKongcheng(player)
		and not (player:isChained() or not self:isGoodChainTarget(player, self.player)) then return self:isWeak(player) and 1.5 or 1 end
	if self:isGoodChainTarget(player, self.player) or self:getDamagedEffects(player, self.player) or self:needToLoseHp(player, self.player) then return -0.1 end

	local num = player:getEquips():length() - player:getHandcardNum()
	if num < 0 then
		if self:needToThrowArmor(player) then num = 1 else num = 0 end
	elseif num == 0 then
		num = 1
	end
	local equip_table = {}
	local needToTA = self:needToThrowArmor(player)
	if needToTA then table.insert(equip_table, 1) end
	if player:getOffensiveHorse() then table.insert(equip_table, 3) end
	if player:getWeapon() then table.insert(equip_table, 0) end
	if player:getDefensiveHorse() then table.insert(equip_table, 2) end
	if player:getArmor() and not needToTA then table.insert(equip_table, 1) end

	local value = 0
	for i = 1, num, 1 do
		local index = equip_table[i]
		if index == 0 then value = value + 0.4
		elseif index == 1 then
			value = value + (needToTA and -0.5 or 0.8)
		elseif index == 2 then value = value + 0.7
		elseif index == 3 then value = value + 0.3 end
	end
	if player:hasSkills("kofxiaoji|xiaoji") then value = value - 0.8 * num end
	if player:hasSkills("xuanfeng|nosxuanfeng") and num > 0 then value = value - 0.8 end

	local handcard = player:getHandcardNum() - num
	value = value + 0.1 * handcard
	if self:needKongcheng(player) or self:getLeastHandcardNum(player) > num then value = value - 0.15
	elseif num == 0 then value = value + 0.1 end
	return value
end

nosfencheng_skill = {}
nosfencheng_skill.name = "nosfencheng"
table.insert(sgs.ai_skills, nosfencheng_skill)
nosfencheng_skill.getTurnUseCard = function(self)
	if self.player:getMark("@burn") <= 0 or sgs.turncount <= 1 then return end
	local lord = self.room:getLord()
	if (self.role == "loyalist" or self.role == "renegade") and (sgs.isLordInDanger() or (lord and self:isWeak(lord))) then return end
	local value = 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isFriend(player) then value = value - getNosFenchengValue(self, player)
		elseif self:isEnemy(player) then value = value + getNosFenchengValue(self, player) end
	end
	if #self.friends_noself >= #self.enemies and value > 0 then return sgs.Card_Parse("@NosFenchengCard=.") end
	local ratio = value / (#self.enemies - #self.friends_noself)
	if ratio >= 0.4 then return sgs.Card_Parse("@NosFenchengCard=.") end
end

sgs.ai_skill_use_func.FenchengCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_discard.nosfencheng = function(self, discard_num, min_num, optional, include_equip)
	if discard_num == 1 and self:needToThrowArmor() then return { self.player:getArmor():getEffectiveId() } end
	local liru = self.room:getCurrent()
	local juece_effect
	if liru and liru:isAlive() and liru:hasSkill("juece|nosjuece") then juece_effect = true end
	if not self:damageIsEffective(self.player, sgs.DamageStruct_Fire, liru) then return {} end
	if juece_effect and self:isEnemy(liru) and self.player:getEquips():isEmpty() and self.player:getHandcardNum() == 1 and not self:needKongcheng()
		and not (self.player:isChained() or not self:isGoodChainTarget(self.player, liru)) then return {} end
	if self:isGoodChainTarget(self.player, liru) or self:getDamagedEffects(self.player, liru) or self:needToLoseHp(self.player, liru) then return {} end
	local to_discard = self:askForDiscard("dummyreason", discard_num, min_num, false, include_equip)
	if #to_discard < discard_num then return {} end
	for _, id in ipairs(to_discard) do
		if isCard("Peach", sgs.Sanguosha:getCard(id), self.player) then return {}
		elseif 1 == self.player:getHp() and isCard("Analeptic", sgs.Sanguosha:getCard(id), self.player) then return {}
		end
	end
	if not juece_effect then return to_discard
	else
		if self.player:isKongcheng() then return to_discard end
		for _, id in sgs.qlist(self.player:handCards()) do
			if not table.contains(to_discard, id) then return to_discard end
		end
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards, true)
		local wep_id, arm_id, def_id, off_id
		if self.player:getWeapon() then wep_id = self.player:getWeapon():getEffectiveId() end
		if self.player:getArmor() then arm_id = self.player:getArmor():getEffectiveId() end
		if self.player:getDefensiveHorse() then def_id = self.player:getDefensiveHorse():getEffectiveId() end
		if self.player:getOffensiveHorse() then off_id = self.player:getOffensiveHorse():getEffectiveId() end
		if self:needToThrowArmor() and not table.contains(to_discard, arm_id) then table.insert(to_discard, arm_id)
		else
			if self.player:getOffensiveHorse() and not table.contains(to_discard, off_id) then table.insert(to_discard, off_id)
			elseif self.player:getWeapon() and not table.contains(to_discard, wep_id) then table.insert(to_discard, wep_id)
			elseif self.player:getDefensiveHorse() and not table.contains(to_discard, def_id) then
				if self:isWeak() then table.insert(to_discard, def_id)
				else return {} end
			elseif self.player:getArmor() and not table.contains(to_discard, arm_id) then
				if self:isWeak() or (not liru:hasSkill("jueqing") and self.player:hasArmorEffect("vine")) then table.insert(to_discard, arm_id)
				else return {} end
			end
			if #to_discard == discard_num + 1 then table.removeOne(to_discard, cards[1]:getEffectiveId()) end
			return to_discard
		end
	end
end

sgs.ai_skill_invoke.noschengxiang = function(self, data)
	return not hasManjuanEffect(self.player)
end

sgs.ai_skill_askforag.noschengxiang = function(self, card_ids)
	return self:askForAG(card_ids, false, "dummyreason")
end

function sgs.ai_cardsview_valuable.nosrenxin(self, class_name, player)
	if class_name == "Peach" and not player:isKongcheng() then
		local dying = player:getRoom():getCurrentDyingPlayer()
		if not dying or self:isEnemy(dying, player) or dying:objectName() == player:objectName() then return nil end
		if dying:isLord() and self:isFriend(dying, player) then return "@NosRenxinCard=." end
		if hasManjuanEffect(dying) then
			local peach_num = 0
			if player:getMark("Global_PreventPeach") == 0 then
				for _, c in sgs.qlist(player:getCards("he")) do
					if isCard("Peach", c, player) then peach_num = peach_num + 1 end
					if peach_num > 1 then return nil end
				end
			end
		end
		if self:playerGetRound(dying) < self:playerGetRound(self.player) and dying:getHp() < 0 then return nil end
		if not player:faceUp() then
			if player:getHp() < 2 and (getCardsNum("Jink", player, self.player) > 0 or getCardsNum("Analeptic", player, self.player) > 0) then return nil end
			return "@NosRenxinCard=."
		else
			if dying:getMark("Global_PreventPeach") == 0 then
				for _, c in sgs.qlist(player:getHandcards()) do
					if not isCard("Peach", c, player) then return nil end
				end
			end
			return "@NosRenxinCard=."
		end
		return nil
	end
end

function sgs.ai_cardsview.nosrenxin(self, class_name, player)
	if class_name == "Peach" and not player:isKongcheng() then
		local dying = player:getRoom():getCurrentDyingPlayer()
		if not dying or self:isEnemy(dying, player) or dying:objectName() == player:objectName() then return nil end
		if dying:isLord() and self:isFriend(lord, player) then return "@NosRenxinCard=." end
		if player:getHp() < 2 and (getCardsNum("Jink", player, self.player) > 0 or getCardsNum("Analeptic", player, self.player) > 0) then return nil end
		if not self:isWeak(player) then return "@NosRenxinCard=." end
		return nil
	end
end

sgs.ai_card_intention.NosRenxinCard = sgs.ai_card_intention.Peach

--standard package
sgs.ai_skill_invoke.nosjianxiong = function(self, data)
	if self.nosjianxiong then self.nosjianxiong = nil return true end
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_invoke.nosfankui = function(self, data)
	local target = data:toPlayer()
	if sgs.ai_need_damaged.nosfankui(self, target, self.player) then return true end

	if self:isFriend(target) then
		if self:getOverflow(target) > 2 then return true end
		if self:doNotDiscard(target) then return true end
		return (self:hasSkills(sgs.lose_equip_skill, target) and not target:getEquips():isEmpty())
		  or (self:needToThrowArmor(target) and target:getArmor()) or self:doNotDiscard(target)
	end
	if self:isEnemy(target) then
		if self:doNotDiscard(target) then return false end
		return true
	end
	--self:updateLoyalty(-0.8*sgs.ai_loyalty[target:objectName()],self.player:objectName())
	return true
end

sgs.ai_choicemade_filter.cardChosen.nosfankui = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if damage.from then
		local intention = 10
		local id = promptlist[3]
		local card = sgs.Sanguosha:getCard(id)
		local target = damage.from
		if self:needToThrowArmor(target) and self.room:getCardPlace(id) == sgs.Player_PlaceEquip and card:isKindOf("Armor") then
			intention = -intention
		elseif self:doNotDiscard(target) then intention = -intention
		elseif self:hasSkills(sgs.lose_equip_skill, target) and not target:getEquips():isEmpty() and
			self.room:getCardPlace(id) == sgs.Player_PlaceEquip and card:isKindOf("EquipCard") then
				intention = -intention
		elseif sgs.ai_need_damaged.nosfankui(self, target, player) then intention = 0
		elseif self:getOverflow(target) > 2 then intention = 0
		end
		sgs.updateIntention(player, target, intention)
	end
end

sgs.ai_skill_cardchosen.nosfankui = function(self, who, flags)
	local suit = sgs.ai_need_damaged.nosfankui(self, who, self.player)
	if not suit then return nil end

	local cards = sgs.QList2Table(who:getEquips())
	local handcards = sgs.QList2Table(who:getHandcards())
	if #handcards==1 and handcards[1]:hasFlag("visible") then table.insert(cards,handcards[1]) end

	for i=1,#cards,1 do
		if (cards[i]:getSuit() == suit and suit ~= sgs.Card_Spade) or
			(cards[i]:getSuit() == suit and suit == sgs.Card_Spade and cards[i]:getNumber() >= 2 and cards[i]:getNumber()<=9) then
			return cards[i]
		end
	end
	return nil
end


sgs.ai_need_damaged.nosfankui = function (self, attacker, player)
	if not player:hasSkill("nosguicai+nosfankui") then return false end
	if not attacker then return end
	local need_retrial = function(target)
		local alive_num = self.room:alivePlayerCount()
		return alive_num + target:getSeat() % alive_num > self.room:getCurrent():getSeat()
				and target:getSeat() < alive_num + player:getSeat() % alive_num
	end
	local retrial_card ={["spade"]=nil,["heart"]=nil,["club"]=nil}
	local attacker_card ={["spade"]=nil,["heart"]=nil,["club"]=nil}

	local handcards = sgs.QList2Table(player:getHandcards())
	for i=1,#handcards,1 do
		if handcards[i]:getSuit() == sgs.Card_Spade and handcards[i]:getNumber()>=2 and handcards[i]:getNumber()<=9 then
			retrial_card.spade = true
		end
		if handcards[i]:getSuit() == sgs.Card_Heart then
			retrial_card.heart = true
		end
		if handcards[i]:getSuit() == sgs.Card_Club then
			retrial_card.club = true
		end
	end

	local cards = sgs.QList2Table(attacker:getEquips())
	local handcards = sgs.QList2Table(attacker:getHandcards())
	if #handcards==1 and handcards[1]:hasFlag("visible") then table.insert(cards,handcards[1]) end

	for i=1,#cards,1 do
		if cards[i]:getSuit() == sgs.Card_Spade and cards[i]:getNumber()>=2 and cards[i]:getNumber()<=9 then
			attacker_card.spade = sgs.Card_Spade
		end
		if cards[i]:getSuit() == sgs.Card_Heart then
			attacker_card.heart = sgs.Card_Heart
		end
		if cards[i]:getSuit() == sgs.Card_Club then
			attacker_card.club = sgs.Card_Club
		end
	end

	local players = self.room:getOtherPlayers(player)
	for _, aplayer in sgs.qlist(players) do
		if aplayer:containsTrick("lightning") and self:getFinalRetrial(aplayer) ==1 and need_retrial(aplayer) then
			if not retrial_card.spade and attacker_card.spade then return attacker_card.spade end
		end

		if self:isFriend(aplayer, player) and not aplayer:containsTrick("YanxiaoCard") and not aplayer:hasSkill("qiaobian") then

			if aplayer:containsTrick("indulgence") and self:getFinalRetrial(aplayer) ==1 and need_retrial(aplayer) and aplayer:getHandcardNum()>=aplayer:getHp() then
				if not retrial_card.heart and attacker_card.heart then return attacker_card.heart end
			end

			if aplayer:containsTrick("supply_shortage") and self:getFinalRetrial(aplayer) ==1 and need_retrial(aplayer) and self:hasSkills("yongshi",aplayer) then
				if not retrial_card.club and attacker_card.club then return attacker_card.club end
			end
		end
	end
	return false
end

sgs.ai_skill_cardask["@nosguicai-card"]=function(self, data)
	local judge = data:toJudge()

	if self.room:getMode():find("_mini_46") and not judge:isGood() then return "$" .. self.player:handCards():first() end
	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "$" .. card_id
		end
	end

	return "."
end

function sgs.ai_cardneed.nosguicai(to, card, self)
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if self:getFinalRetrial(to) == 1 then
			if player:containsTrick("lightning") and not player:containsTrick("YanxiaoCard") then
				return card:getSuit() == sgs.Card_Spade and card:getNumber() >= 2 and card:getNumber() <= 9 and not self:hasSkills("hongyan|wuyan")
			end
			if self:isFriend(player) and self:willSkipDrawPhase(player) then
				return card:getSuit() == sgs.Card_Club
			end
			if self:isFriend(player) and self:willSkipPlayPhase(player) then
				return card:getSuit() == sgs.Card_Heart
			end
		end
	end
end

sgs.nosguicai_suit_value = {
	heart = 3.9,
	club = 3.9,
	spade = 3.5
}

sgs.ai_skill_invoke.nosganglie = function(self, data)
	local mode = self.room:getMode()
	if mode:find("_mini_41") or mode:find("_mini_46") then return true end
	local damage = data:toDamage()
	if not damage.from then
		local zhangjiao = self.room:findPlayerBySkillName("guidao")
		return zhangjiao and self:isFriend(zhangjiao) and not zhangjiao:isNude()
	end
	if self:getDamagedEffects(damage.from, self.player) then
		if self:isFriend(damage.from) then
			sgs.ai_nosganglie_effect = string.format("%s_%s_%d", self.player:objectName(), damage.from:objectName(), sgs.turncount)
			return true
		end
		return false
	end
	return not self:isFriend(damage.from) and self:canAttack(damage.from)
end

sgs.ai_need_damaged.nosganglie = function(self, attacker, player)
	if not attacker then return end
	if not attacker:hasSkill("nosganglie") and self:getDamagedEffects(attacker, player) then return self:isFriend(attacker, player) end
	if self:isEnemy(attacker) and attacker:getHp() + attacker:getHandcardNum() <= 3
		and not (self:hasSkills(sgs.need_kongcheng .. "|buqu", attacker) and attacker:getHandcardNum() > 1) and sgs.isGoodTarget(attacker, self:getEnemies(attacker), self) then
		return true
	end
	return false
end

function nosganglie_discard(self, discard_num, min_num, optional, include_equip, skillName)
	local xiahou = self.room:findPlayerBySkillName(skillName)
	if xiahou and (not self:damageIsEffective(self.player, sgs.DamageStruct_Normal, xiahou) or self:getDamagedEffects(self.player, xiahou)) then return {} end
	if xiahou and self:needToLoseHp(self.player, xiahou) then return {} end
	local to_discard = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	local index = 0
	local all_peaches = 0
	for _, card in ipairs(cards) do
		if isCard("Peach", card, self.player) then
			all_peaches = all_peaches + 1
		end
	end
	if all_peaches >= 2 and self:getOverflow() <= 0 then return {} end
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)

	for i = #cards, 1, -1 do
		local card = cards[i]
		if not isCard("Peach", card, self.player) and not self.player:isJilei(card) then
			table.insert(to_discard, card:getEffectiveId())
			table.remove(cards, i)
			index = index + 1
			if index == 2 then break end
		end
	end
	if #to_discard < 2 then return {}
	else
		return to_discard
	end
end

sgs.ai_skill_discard.nosganglie = function(self, discard_num, min_num, optional, include_equip)
	return nosganglie_discard(self, discard_num, min_num, optional, include_equip, "nosganglie")
end

function sgs.ai_slash_prohibit.nosganglie(self, from, to)
	if self:isFriend(from, to) then return false end
	if from:hasSkill("jueqing") or (from:hasSkill("nosqianxi") and from:distanceTo(to) == 1) then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	return from:getHandcardNum() + from:getHp() < 4
end

sgs.ai_choicemade_filter.skillInvoke.nosganglie = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if damage.from and damage.to then
		if promptlist[#promptlist] == "yes" then
			if not self:getDamagedEffects(damage.from, player) and not self:needToLoseHp(damage.from, player) then
				sgs.updateIntention(damage.to, damage.from, 40)
			end
		elseif self:canAttack(damage.from) then
			sgs.updateIntention(damage.to, damage.from, -40)
		end
	end
end

sgs.ai_skill_use["@@nostuxi"] = function(self, prompt)
	self:sort(self.enemies, "handcard_defense")
	local targets = {}

	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	local luxun = self.room:findPlayerBySkillName("noslianying")
	local dengai = self.room:findPlayerBySkillName("tuntian")
	local jiangwei = self.room:findPlayerBySkillName("zhiji")
	local zhijiangwei = self.room:findPlayerBySkillName("beifa")

	local add_player = function (player,isfriend)
		if player:getHandcardNum() ==0 or player:objectName() == self.player:objectName() then return #targets end
		if self:objectiveLevel(player) == 0 and player:isLord() and sgs.current_mode_players["rebel"] > 1 then return #targets end
		if #targets == 0 then
			table.insert(targets, player:objectName())
		elseif #targets== 1 then
			if player:objectName()~=targets[1] then
				table.insert(targets, player:objectName())
			end
		end
		if isfriend and isfriend == 1 then
			self.player:setFlags("nostuxi_isfriend_"..player:objectName())
		end
		return #targets
	end

	local lord = self.room:getLord()
	if lord and self:isEnemy(lord) and sgs.turncount <= 1 and not lord:isKongcheng() then
		add_player(lord)
	end

	if jiangwei and self:isFriend(jiangwei) and jiangwei:getMark("zhiji") == 0 and jiangwei:getHandcardNum()== 1
			and self:getEnemyNumBySeat(self.player,jiangwei) <= (jiangwei:getHp() >= 3 and 1 or 0) then
		if add_player(jiangwei,1) == 2  then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
	end

	if dengai and self:isFriend(dengai) and (not self:isWeak(dengai) or self:getEnemyNumBySeat(self.player,dengai) == 0 )
			and dengai:hasSkill("zaoxian") and dengai:getMark("zaoxian") == 0 and dengai:getPile("field"):length() == 2 and add_player(dengai, 1) == 2 then
		return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2])
	end

	if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and self:getEnemyNumBySeat(self.player,zhugeliang) > 0 then
		if zhugeliang:getHp() <= 2 then
			if add_player(zhugeliang,1) == 2 then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
		else
			local flag = string.format("%s_%s_%s","visible",self.player:objectName(),zhugeliang:objectName())
			local cards = sgs.QList2Table(zhugeliang:getHandcards())
			if #cards == 1 and (cards[1]:hasFlag("visible") or cards[1]:hasFlag(flag)) then
				if cards[1]:isKindOf("TrickCard") or cards[1]:isKindOf("Slash") or cards[1]:isKindOf("EquipCard") then
					if add_player(zhugeliang,1) == 2 then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
				end
			end
		end
	end

	if luxun and self:isFriend(luxun) and luxun:getHandcardNum() == 1 and self:getEnemyNumBySeat(self.player,luxun) > 0 then
		local flag = string.format("%s_%s_%s","visible",self.player:objectName(),luxun:objectName())
		local cards = sgs.QList2Table(luxun:getHandcards())
		if #cards==1 and (cards[1]:hasFlag("visible") or cards[1]:hasFlag(flag)) then
			if cards[1]:isKindOf("TrickCard") or cards[1]:isKindOf("Slash") or cards[1]:isKindOf("EquipCard") then
				if add_player(luxun,1)==2  then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
			end
		end
	end

	if zhijiangwei and self:isFriend(zhijiangwei) and zhijiangwei:getHandcardNum()== 1 and
		self:getEnemyNumBySeat(self.player, zhijiangwei) <= (zhijiangwei:getHp() >= 3 and 1 or 0) then
		local isGood
		for _, enemy in ipairs(self.enemies) do
			local def = sgs.getDefenseSlash(enemy)
			local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
			local eff = self:slashIsEffective(slash, enemy, zhijiangwei) and sgs.isGoodTarget(enemy, self.enemies, self)
			if zhijiangwei:canSlash(enemy, slash) and not self:slashProhibit(slash, enemy, zhijiangwei) and eff and def < 4 then
				isGood = true
			end
		end
		if isGood and add_player(zhijiangwei, 1) == 2  then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
	end

	for i = 1, #self.enemies, 1 do
		local p = self.enemies[i]
		local cards = sgs.QList2Table(p:getHandcards())
		local flag = string.format("%s_%s_%s","visible",self.player:objectName(),p:objectName())
		for _, card in ipairs(cards) do
			if (card:hasFlag("visible") or card:hasFlag(flag)) and (card:isKindOf("Peach") or card:isKindOf("Nullification") or card:isKindOf("Analeptic") ) then
				if add_player(p)==2  then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
			end
		end
	end

	for i = 1, #self.enemies, 1 do
		local p = self.enemies[i]
		if p:hasSkills("jijiu|qingnang|xinzhan|leiji|nosleiji|jieyin|beige|kanpo|liuli|qiaobian|zhiheng|guidao|longhun|xuanfeng|tianxiang|noslijian|lijian") then
			if add_player(p) == 2  then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
		end
	end

	for i = 1, #self.enemies, 1 do
		local p = self.enemies[i]
		local x = p:getHandcardNum()
		local good_target = true
		if x == 1 and self:needKongcheng(p) then good_target = false end
		if x >= 2 and p:hasSkill("tuntian") and p:hasSkill("zaoxian") then good_target = false end
		if good_target and add_player(p)==2 then return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2]) end
	end


	if luxun and add_player(luxun,(self:isFriend(luxun) and 1 or nil)) == 2 then
		return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2])
	end

	if dengai and self:isFriend(dengai) and dengai:hasSkill("zaoxian") and (not self:isWeak(dengai) or self:getEnemyNumBySeat(self.player,dengai) == 0 ) and add_player(dengai,1) == 2 then
		return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2])
	end

	local others = self.room:getOtherPlayers(self.player)
	for _, other in sgs.qlist(others) do
		if self:objectiveLevel(other) >= 0 and not (other:hasSkill("tuntian") and other:hasSkill("zaoxian")) and add_player(other) == 2 then
			return ("@NosTuxiCard=.->%s+%s"):format(targets[1], targets[2])
		end
	end

	for _, other in sgs.qlist(others) do
		if self:objectiveLevel(other) >= 0 and not (other:hasSkill("tuntian") and other:hasSkill("zaoxian")) and add_player(other) == 1 and math.random(0, 5) <= 1 and not self:hasSkills("qiaobian") then
			return ("@NosTuxiCard=.->%s"):format(targets[1])
		end
	end

	return "."
end

sgs.ai_card_intention.NosTuxiCard = function(self, card, from, tos)
	local lord = getLord(self.player)
	local nostuxi_lord = false
	if sgs.evaluatePlayerRole(from) == "neutral" and sgs.evaluatePlayerRole(tos[1]) == "neutral" and
		(not tos[2] or sgs.evaluatePlayerRole(tos[2]) == "neutral") and lord and not lord:isKongcheng() and
		not (self:needKongcheng(lord) and lord:getHandcardNum() == 1 ) and
		self:hasLoseHandcardEffective(lord) and not (lord:hasSkill("tuntian") and lord:hasSkill("zaoxian")) and from:aliveCount() >= 4 then
			sgs.updateIntention(from, lord, -80)
		return
	end
	if from:getState() == "online" then
		for _, to in ipairs(tos) do
			if to:hasSkill("kongcheng") or to:hasSkill("noslianying") or to:hasSkill("zhiji")
				or (to:hasSkill("tuntian") and to:hasSkill("zaoxian")) then
			else
				sgs.updateIntention(from, to, 80)
			end
		end
	else
		for _, to in ipairs(tos) do
			if lord and to:objectName() == lord:objectName() then nostuxi_lord = true end
			local intention = from:hasFlag("nostuxi_isfriend_"..to:objectName()) and -5 or 80
			sgs.updateIntention(from, to, intention)
		end
		if sgs.turncount ==1 and not nostuxi_lord and lord and not lord:isKongcheng() and from:getRoom():alivePlayerCount() > 2 then
			sgs.updateIntention(from, lord, -80)
		end
	end
end

sgs.ai_skill_invoke.nosluoyi = function(self,data)
	if self.player:isSkipped(sgs.Player_Play) then return false end
	if self:needBear() then return false end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	local slashtarget = 0
	local dueltarget = 0
	self:sort(self.enemies,"hp")
	for _,card in ipairs(cards) do
		if card:isKindOf("Slash") then
			for _,enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, card, true) and self:slashIsEffective(card, enemy) and self:objectiveLevel(enemy) > 3 and sgs.isGoodTarget(enemy, self.enemies, self) then
					if getCardsNum("Jink", enemy) < 1 or (self.player:hasWeapon("axe") and self.player:getCards("he"):length() > 4) then
						slashtarget = slashtarget + 1
					end
				end
			end
		end
		if card:isKindOf("Duel") then
			for _, enemy in ipairs(self.enemies) do
				if self:getCardsNum("Slash") >= getCardsNum("Slash", enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
				and self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy, self.player, 2)
				and self:damageIsEffective(enemy) and enemy:getMark("@late") < 1 then
					dueltarget = dueltarget + 1
				end
			end
		end
	end
	if (slashtarget+dueltarget) > 0 then
		self:speak("luoyi")
		return true
	end
	return false
end

function sgs.ai_cardneed.nosluoyi(to, card, self)
	local slash_num = 0
	local target
	local slash = sgs.Sanguosha:cloneCard("slash")

	local cards = to:getHandcards()
	local need_slash = true
	for _, c in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s","visible",self.room:getCurrent():objectName(),to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if isCard("Slash", c, to) then
				need_slash = false
				break
			end
		end
	end

	self:sort(self.enemies, "defenseSlash")
	for _, enemy in ipairs(self.enemies) do
		if to:canSlash(enemy) and not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) and sgs.getDefenseSlash(enemy, self) <= 2 then
			target = enemy
			break
		end
	end

	if need_slash and target and isCard("Slash", card, to) then return true end
	return isCard("Duel",card, to)
end

sgs.nosluoyi_keep_value = {
	Peach           = 6,
	Analeptic       = 5.8,
	Jink            = 5.2,
	Duel            = 5.5,
	FireSlash       = 5.6,
	Slash           = 5.4,
	ThunderSlash    = 5.5,
	Axe             = 5,
	Blade           = 4.9,
	spear           = 4.9,
	fan             = 4.8,
	KylinBow        = 4.7,
	Halberd         = 4.6,
	MoonSpear       = 4.5,
	SPMoonSpear = 4.5,
	DefensiveHorse  = 4
}

sgs.ai_skill_invoke.nosyiji = function(self)
	local Shenfen_user
	for _, player in sgs.qlist(self.room:getAlivePlayers()) do
		if player:hasFlag("ShenfenUsing") then
			Shenfen_user = player
			break
		end
	end
	if self.player:getHandcardNum() < 2 then return true end
	local invoke
	for _, friend in ipairs(self.friends) do
		if not (friend:hasSkill("manjuan") and friend:getPhase() == sgs.Player_NotActive) and
			not self:needKongcheng(friend, true) and not self:isLihunTarget(friend) and
			(not Shenfen_user or Shenfen_user:objectName() == friend:objectName() or friend:getHandcardNum() >= 4) then
				invoke = true
			break
		end
	end
	return invoke
end

sgs.ai_skill_askforyiji.nosyiji = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end

	local Shenfen_user
	for _, player in sgs.qlist(self.room:getAlivePlayers()) do
		if player:hasFlag("ShenfenUsing") then
			Shenfen_user = player
			break
		end
	end

	if Shenfen_user then
		if self:isFriend(Shenfen_user) then
			if Shenfen_user:objectName() ~= self.player:objectName() then
				for _, id in ipairs(card_ids) do
					return Shenfen_user, id
				end
			else
				return nil, -1
			end
		else
			if self.player:getHandcardNum() < self:getOverflow(false, true) then
				return nil, -1
			end
			local card, friend = self:getCardNeedPlayer(cards)
			if card and friend and friend:getHandcardNum() >= 4 then
				return friend, card:getId()
			end
		end
	end

	if self.player:getHandcardNum() <= 2 and not Shenfen_user then
		return nil, -1
	end

	local new_friends = {}
	local CanKeep
	for _, friend in ipairs(self.friends) do
		if not (friend:hasSkill("manjuan") and friend:getPhase() == sgs.Player_NotActive) and
		not self:needKongcheng(friend, true) and not self:isLihunTarget(friend) and
		(not Shenfen_user or friend:objectName() == Shenfen_user:objectName() or friend:getHandcardNum() >= 4) then
			if friend:objectName() == self.player:objectName() then CanKeep = true
			else
				table.insert(new_friends, friend)
			end
		end
	end

	if #new_friends > 0 then
		local card, target = self:getCardNeedPlayer(cards)
		if card and target then
			for _, friend in ipairs(new_friends) do
				if target:objectName() == friend:objectName() then
					return friend, card:getEffectiveId()
				end
			end
		end
		if Shenfen_user and self:isFriend(Shenfen_user) then
			return Shenfen_user, cards[1]:getEffectiveId()
		end
		self:sort(new_friends, "defense")
		self:sortByKeepValue(cards, true)
		return new_friends[1], cards[1]:getEffectiveId()
	elseif CanKeep then
		return nil, -1
	else
		local other = {}
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if not (self:isLihunTarget(player) and self:isFriend(player)) and (self:isFriend(player) or not player:hasSkill("lihun")) then
				table.insert(other, player)
			end
		end
		return other[math.random(1, #other)], card_ids[math.random(1, #card_ids)]
	end

end

sgs.ai_need_damaged.nosyiji = function (self, attacker, player)
	if not player:hasSkill("nosyiji") then return end
	local need_card = false
	local current = self.room:getCurrent()
	if self:hasCrossbowEffect(current) or current:hasSkill("paoxiao") or current:hasFlag("shuangxiong") then need_card = true end
	if self:hasSkills("jieyin|jijiu",current) and self:getOverflow(current) <= 0 then need_card = true end
	if self:isFriend(current, player) and need_card then return true end

	local friends = {}
	for _, ap in sgs.qlist(self.room:getAlivePlayers()) do
		if self:isFriend(ap, player) then
			table.insert(friends, ap)
		end
	end
	self:sort(friends, "hp")

	if #friends > 0 and friends[1]:objectName() == player:objectName() and self:isWeak(player) and getCardsNum("Peach", player, (attacker or self.player)) == 0 then return false end
	if #friends > 1 and self:isWeak(friends[2]) then return true end

	return player:getHp() > 2 and sgs.turncount > 2 and #friends > 1
end

sgs.ai_skill_invoke.nostieji = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then return false end

	local zj = self.room:findPlayerBySkillName("guidao")
	if zj and self:isEnemy(zj) and self:canRetrial(zj) then return false end

	--[[
	if target:hasArmorEffect("eight_diagram") and not IgnoreArmor(self.player, target) then return true end
	if target:hasLordSkill("hujia") then
		for _, p in ipairs(self.enemies) do
			if p:getKingdom() == "wei" and (p:hasArmorEffect("eight_diagram") or p:getHandcardNum() > 0) then return true end
		end
	end
	if target:hasSkill("longhun") and target:getHp() == 1 and self:hasSuit("club", true, target) then return true end

	if target:isKongcheng() or (self:getKnownNum(target) == target:getHandcardNum() and getKnownCard(target, self.player, "Jink", true) == 0) then return false end
	]]
	return true
end

local noskurou_skill={}
noskurou_skill.name="noskurou"
table.insert(sgs.ai_skills,noskurou_skill)
noskurou_skill.getTurnUseCard=function(self,inclusive)
	--特殊场景
	local func = Tactic("noskurou", self, nil)
	if func then return func(self, nil) end
	--一般场景
	sgs.ai_use_priority.NosKurouCard = 6.8
	local losthp = isLord(self.player) and 0 or 1
	if ((self.player:getHp() > 3 and self.player:getLostHp() <= losthp and self.player:getHandcardNum() > self.player:getHp())
		or (self.player:getHp() - self.player:getHandcardNum() >= 2)) and not (isLord(self.player) and sgs.turncount <= 1) then
		return sgs.Card_Parse("@NosKurouCard=.")
	end
	local slash = sgs.Sanguosha:cloneCard("slash")
	if (self.player:getWeapon() and self.player:getWeapon():isKindOf("Crossbow")) or self.player:hasSkill("paoxiao") then
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy, nil, true) and self:slashIsEffective(slash, enemy)
				and not (enemy:hasSkill("kongcheng") and enemy:isKongcheng())
				and not (enemy:hasSkills("fankui|guixin") and not self.player:hasSkill("paoxiao"))
				and not enemy:hasSkills("fenyong|jilei|zhichi")
				and sgs.isGoodTarget(enemy, self.enemies, self) and not self:slashProhibit(slash, enemy) and self.player:getHp() > 1 then
				return sgs.Card_Parse("@NosKurouCard=.")
			end
		end
	end
	if self.player:getHp() == 1 and self:getCardsNum("Analeptic") >= 1 then
		return sgs.Card_Parse("@NosKurouCard=.")
	end

	--Suicide by NosKurou
	local nextplayer = self.player:getNextAlive()
	if self.player:getHp() == 1 and self.player:getRole() ~= "lord" and self.player:getRole() ~= "renegade" then
		local to_death = false
		if self:isFriend(nextplayer) then
			for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if p:hasSkill("xiaoguo") and not self:isFriend(p) and not p:isKongcheng()
					and self.role == "rebel" and self.player:getEquips():isEmpty() then
					to_death = true
					break
				end
			end
			if not to_death and not self:willSkipPlayPhase(nextplayer) then
				if nextplayer:hasSkill("jieyin") and self.player:isMale() then return end
				if nextplayer:hasSkill("qingnang") then return end
			end
		end
		if self.player:getRole() == "rebel" and not self:isFriend(nextplayer) then
			if not self:willSkipPlayPhase(nextplayer) or nextplayer:hasSkill("shensu") then
				to_death = true
			end
		end
		local lord = getLord(self.player)
		if self.player:getRole()=="loyalist" then
			if lord and lord:getCards("he"):isEmpty() then return end
			if self:isEnemy(nextplayer) and not self:willSkipPlayPhase(nextplayer) then
				if nextplayer:hasSkills("noslijian|lijian") and self.player:isMale() and lord and lord:isMale() then
					to_death = true
				elseif nextplayer:hasSkill("quhu") and lord and lord:getHp() > nextplayer:getHp() and not lord:isKongcheng()
					and lord:inMyAttackRange(self.player) then
					to_death = true
				end
			end
		end
		if to_death then
			local caopi = self.room:findPlayerBySkillName("xingshang")
			if caopi and self:isEnemy(caopi) then
				if self.player:getRole() == "rebel" and self.player:getHandcardNum() > 3 then to_death = false end
				if self.player:getRole() == "loyalist" and lord and lord:getCardCount(true) + 2 <= self.player:getHandcardNum() then
					to_death = false
				end
			end
			if #self.friends == 1 and #self.enemies == 1 and self.player:aliveCount() == 2 then to_death = false end
		end
		if to_death then
			self.player:setFlags("NosKurou_toDie")
			sgs.ai_use_priority.NosKurouCard = 0
			return sgs.Card_Parse("@NosKurouCard=.")
		end
		self.player:setFlags("-NosKurou_toDie")
	end
end

sgs.ai_skill_use_func.NosKurouCard=function(card,use,self)
	if not use.isDummy then self:speak("noskurou") end
	use.card=card
end

sgs.ai_use_priority.NosKurouCard = 6.8

sgs.ai_skill_invoke.nosyingzi = function(self, data)
	if self.player:hasSkill("haoshi") then
		local num = self.player:getHandcardNum()
		local skills = self.player:getVisibleSkillList(true)
		local count = self:ImitateResult_DrawNCards(self.player, skills)
		if num + count > 5 then
			local others = self.room:getOtherPlayers(self.player)
			local least = 999
			local target = nil
			for _,p in sgs.qlist(others) do
				local handcardnum = p:getHandcardNum()
				if handcardnum < least then
					least = handcardnum
					target = p
				end
			end
			if target then
				if self:isFriend(target) then
					return not target:hasSkill("manjuan")
				end
			end
		end
	end
	return true
end

local nosfanjian_skill = {}
nosfanjian_skill.name = "nosfanjian"
table.insert(sgs.ai_skills, nosfanjian_skill)
nosfanjian_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return nil end
	if self.player:hasUsed("NosFanjianCard") then return nil end
	return sgs.Card_Parse("@NosFanjianCard=.")
end

sgs.ai_skill_use_func.NosFanjianCard=function(card,use,self)

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	if #cards == 1 and cards[1]:getSuit() == sgs.Card_Diamond then return end
	if #cards <= 4 and (self:getCardsNum("Peach") > 0 or self:getCardsNum("Analeptic") > 0) then return end
	self:sort(self.enemies, "hp")

	local suits = {}
	local suits_num = 0
	for _, c in ipairs(cards) do
		if not suits[c:getSuitString()] then
			suits[c:getSuitString()] = true
			suits_num = suits_num + 1
		end
	end

	local wgt = self.room:findPlayerBySkillName("buyi")
	if wgt and self:isFriend(wgt) then wgt = nil end

	for _, enemy in ipairs(self.enemies) do
		local visible = 0
		for _, card in ipairs(cards) do
			local flag = string.format("%s_%s_%s", "visible", enemy:objectName(), self.player:objectName())
			if card:hasFlag("visible") or card:hasFlag(flag) then visible = visible + 1 end
		end
		if visible > 0 and (#cards <= 2 or suits_num <= 2) then continue end
		if self:canAttack(enemy) and not enemy:hasSkills("qingnang|jijiu|tianxiang")
			and not (wgt and card:getTypeId() ~= sgs.Card_Basic and (enemy:isKongcheng() or enemy:objectName() == wgt:objectName())) then
			use.card = sgs.Card_Parse("@NosFanjianCard=.")
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_card_intention.NosFanjianCard = 70

function sgs.ai_skill_suit.nosfanjian(self)
	local map = {0, 0, 1, 2, 2, 3, 3, 3}
	local suit = map[math.random(1, 8)]
	local tg = self.room:getCurrent()
	local suits = {}
	local maxnum, maxsuit = 0
	for _, c in sgs.qlist(tg:getHandcards()) do
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), tg:objectName())
		if c:hasFlag(flag) or c:hasFlag("visible") then
			if not suits[c:getSuitString()] then suits[c:getSuitString()] = 1 else suits[c:getSuitString()] = suits[c:getSuitString()] + 1 end
			if suits[c:getSuitString()] > maxnum then
				maxnum = suits[c:getSuitString()]
				maxsuit = c:getSuit()
			end
		end
	end
	if self.player:hasSkill("hongyan") and (maxsuit == sgs.Card_Spade or suit == sgs.Card_Spade) then
		return sgs.Card_Heart
	end
	if maxsuit then
		if self.player:hasSkill("hongyan") and maxsuit == sgs.Card_Spade then return sgs.Card_Heart end
		return maxsuit
	else
		if self.player:hasSkill("hongyan") and suit == sgs.Card_Spade then return sgs.Card_Heart end
		return suit
	end
end

sgs.dynamic_value.damage_card.NosFanjianCard = true

sgs.ai_skill_invoke.noslianying = function(self, data)
	if self:needKongcheng(self.player, true) then
		return self.player:getPhase() == sgs.Player_Play
	end
	return true
end

local nosguose_skill={}
nosguose_skill.name="nosguose"
table.insert(sgs.ai_skills,nosguose_skill)
nosguose_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	local has_weapon, has_armor = false, false

	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Weapon") and not (acard:getSuit() == sgs.Card_Diamond) then has_weapon=true end
	end

	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Armor") and not (acard:getSuit() == sgs.Card_Diamond) then has_armor=true end
	end

	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Diamond) and ((self:getUseValue(acard)<sgs.ai_use_value.Indulgence) or inclusive) then
			local shouldUse=true

			if acard:isKindOf("Armor") then
				if not self.player:getArmor() then shouldUse = false
				elseif self.player:hasEquip(acard) and not has_armor and self:evaluateArmor() > 0 then shouldUse = false
				end
			end

			if acard:isKindOf("Weapon") then
				if not self.player:getWeapon() then shouldUse = false
				elseif self.player:hasEquip(acard) and not has_weapon then shouldUse = false
				end
			end

			if shouldUse then
				card = acard
				break
			end
		end
	end

	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("indulgence:nosguose[diamond:%s]=%d"):format(number, card_id)
	local indulgence = sgs.Card_Parse(card_str)
	assert(indulgence)
	return indulgence
end

function sgs.ai_cardneed.nosguose(to, card)
	return card:getSuit() == sgs.Card_Diamond
end

local qingnang_skill = {}
qingnang_skill.name = "qingnang"
table.insert(sgs.ai_skills, qingnang_skill)
qingnang_skill.getTurnUseCard = function(self)
	if self.player:getHandcardNum() < 1 then return nil end
	if self.player:usedTimes("QingnangCard") > 0 then return nil end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	local compare_func = function(a, b)
		local v1 = self:getKeepValue(a) + ( a:isRed() and 50 or 0 ) + ( a:isKindOf("Peach") and 50 or 0 )
		local v2 = self:getKeepValue(b) + ( b:isRed() and 50 or 0 ) + ( b:isKindOf("Peach") and 50 or 0 )
		return v1 < v2
	end
	table.sort(cards, compare_func)

	local card_str = ("@QingnangCard=%d"):format(cards[1]:getId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.QingnangCard = function(card, use, self)
	local arr1, arr2 = self:getWoundedFriend(false, true)
	local target = nil

	if #arr1 > 0 and (self:isWeak(arr1[1]) or self:getOverflow() >= 1) and arr1[1]:getHp() < getBestHp(arr1[1]) then target = arr1[1] end
	if target then
		use.card = card
		if use.to then use.to:append(target) end
		return
	end
	if self:getOverflow() > 0 and #arr2 > 0 then
		for _, friend in ipairs(arr2) do
			if not friend:hasSkills("hunzi|longhun") then
				use.card = card
				if use.to then use.to:append(friend) end
				return
			end
		end
	end
end

sgs.ai_use_priority.QingnangCard = 4.2
sgs.ai_card_intention.QingnangCard = -100

sgs.dynamic_value.benefit.QingnangCard = true