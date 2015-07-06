sgs.ai_skill_choice.olmiji_draw = function(self, choices)
	return "" .. self.player:getLostHp()
end

sgs.ai_skill_invoke.olmiji = function(self, data)
	if #self.friends == 0 then return false end
	for _, friend in ipairs(self.friends) do
		if not friend:hasSkill("manjuan") and not self:isLihunTarget(friend) then return true end
	end
	return false
end

sgs.ai_skill_askforyiji.olmiji = function(self, card_ids)
	
	local available_friends = {}
	for _, friend in ipairs(self.friends) do
		if not friend:hasSkill("manjuan") and not self:isLihunTarget(friend) then table.insert(available_friends, friend) end
	end

	local toGive, allcards = {}, {}
	local keep
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if not keep and (isCard("Jink", card, self.player) or isCard("Analeptic", card, self.player)) then
			keep = true
		else
			table.insert(toGive, card)
		end
		table.insert(allcards, card)
	end

	
	
	local cards = #toGive > 0 and toGive or allcards
	self:sortByKeepValue(cards, true)
	local id = cards[1]:getId()

	local card, friend = self:getCardNeedPlayer(cards, true)
	if card and friend and table.contains(available_friends, friend) then 
		if friend:objectName() == self.player:objectName() then 
			return nil, -1
		else
			return friend, card:getId() 
		end
	end

	
	if #available_friends > 0 then
		self:sort(available_friends, "handcard")
		for _, afriend in ipairs(available_friends) do
			if not self:needKongcheng(afriend, true) then
				if afriend:objectName() == self.player:objectName() then 
					return nil, -1
				else
					return afriend, id
				end
			end
		end
		self:sort(available_friends, "defense")
		if available_friends[1]:objectName() == self.player:objectName() then 
			return nil, -1
		else
			return available_friends[1], id
		end
	end
	return nil, -1
end

sgs.ai_skill_use["@@bushi"] = function(self, prompt, method)
	local zhanglu = self.room:findPlayerBySkillName("bushi")
	if not zhanglu or zhanglu:getPile("rice"):length() < 1 then return "." end
	if self:isEnemy(zhanglu) and zhanglu:getPile("rice"):length() == 1 and zhanglu:isWounded() then return "." end
	if self:isFriend(zhanglu) and (not (zhanglu:getPile("rice"):length() == 1 and zhanglu:isWounded())) and self:getOverflow() > 1 then return "." end
	local cards = {}
	for _,id in sgs.qlist(zhanglu:getPile("rice")) do
		table.insert(cards,sgs.Sanguosha:getCard(id))
	end
	self:sortByUseValue(cards, true)
	return "@BushiCard="..cards[1]:getEffectiveId()	
end	

sgs.ai_skill_use["@@midao"] = function(self, prompt, method)
	local judge = self.player:getTag("judgeData"):toJudge()
	local ids = self.player:getPile("rice")
	if self.room:getMode():find("_mini_46") and not judge:isGood() then return "@MidaoCard=" .. ids:first() end
	if self:needRetrial(judge) then
		local cards = {}
		for _,id in sgs.qlist(ids) do
			table.insert(cards,sgs.Sanguosha:getCard(id))
		end
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@MidaoCard=" .. card_id
		end
	end
	return "."	
end