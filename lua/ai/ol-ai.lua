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
				if friend:objectName() == self.player:objectName() then 
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