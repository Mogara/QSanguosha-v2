sgs.ai_skill_playerchosen.huituo = function(self, targets)
    self:sort(self.friends, "defence")
    return self.friends[#self.friends]
end

sgs.ai_playerchosen_intention.huituo = -80

sgs.ai_skill_playerchosen.mingjian = function(self, targets)
    if (sgs.ai_skill_invoke.fangquan(self) or self:needKongcheng(self.player)) then
        local cards = sgs.QList2Table(self.player:getHandcards())
        self:sortByKeepValue(cards)
        if sgs.current_mode_players.rebel == 0 then
            local lord = self.room:getLord()
            if lord and self:isFriend(lord) and lord:objectName() ~= self.player:objectName() then
                return lord
            end
        end

        local AssistTarget = self:AssistTarget()
        if AssistTarget and not self:willSkipPlayPhase(AssistTarget) then
            return AssistTarget
        end

        self:sort(self.friends_noself, "chaofeng")
        return self.friends_noself[1]
    end
    return nil
end

sgs.ai_playerchosen_intention.mingjian = -80


sgs.ai_skill_invoke.xingshuai = sgs.ai_skill_invoke.niepan

sgs.ai_skill_invoke._xingshuai = function(self)
    return self:hasSkills(sgs.masochism_skill) or self.player:getHp() > 1
end


-- taoxi buhui!!!

-- huaiyi buhui!!!

sgs.ai_skill_invoke.jigong = function(self)
    if self.player:isKongcheng() then return true end
    for _, c in sgs.qlist(self.player:getHandcards()) do
        local x = nil
        if isCard("ArcheryAttack", c, self.player) then
            x = sgs.Sanguosha:cloneCard("ArcheryAttack")
        elseif isCard("SavageAssault", c, self.player) then
            x = sgs.Sanguosha:cloneCard("SavageAssault")
        else continue end
        
        local du = { isDummy = true }
        self:useTrickCard(x, du)
        if (du.card) then return true end
    end
    
    return false
end

sgs.ai_skill_invoke.shifei = function(self) 
    local l = {}
    for _, p in sgs.qlist(self.room:getAlivePlayers()) do
        l[p:objectName()] = p:getHandcardNum()
        if (p:objectName() == self.room:getCurrent():objectName()) then
            l[p:objectName()] = p:getHandcardNum() + 1
        end
    end
        
    local most = {}
    for k, t in pairs(l) do
        if #most == 0 then
            table.insert(most, k)
            continue
        end
        
        if (t > l[most[1]]) then
            most = {}
        end
        
        table.insert(most, k)
    end
    
    if (table.contains(most, self.room:getCurrent():objectName())) then
        return table.contains(self.friends, self.room:getCurrent(), true)
    end
    
    for _, p in ipairs(most) do
        if (table.contains(self.enemies, p, true)) then return true end
    end
    return false
end

sgs.ai_skill_playerchosen.shifei = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, target in ipairs(targets) do
		if self:isEnemy(target) and target:isAlive() then
			return target
		end
	end
end


zhanjue_skill = {name = "zhanjue"}
table.insert(sgs.ai_skills, zhanjue_skill)
zhanjue_skill.getTurnUseCard = function(self)
    if (self.player.getMark("zhanjuedraw") >= 2) then return nil end
    
    if (self.player:isKongcheng()) then return nil end
    
    local duel = sgs.Sanguosha:cloneCard("duel", sgs.Card_SuitToBeDecided, -1)
    duel:addSubcards(self.player:getHandcards())
    duel:setSkillName("zhanjue")
    
    return duel
end


-- qinwang buhui!!!

-- zhenshan buhui!!!


yanzhu_skill = {name = "yanzhu"}
table.insert(sgs.ai_skills, yanzhu_skill)
yanzhu_skill.getTurnUseCard = function(self) 

    if (self.player:hasUsed("YanzhuCard")) then return nil end
    
    return sgs.Card_Parse("@YanzhuCard=.")

end


sgs.ai_skill_use_func.YanzhuCard = function(card, use, self)

    if (#self.enemies == 0) then return nil end

    self:sort(self.enemies, "threat")
    for _, p in ipairs(self.enemies) do
        if not p:isNude() then
            use.card = card
            if (use.to) then use.to:append(p) end
            -- use.from = self.player
            return 
        end
    end

    return nil
end

sgs.ai_use_priority.YanzhuCard = sgs.ai_use_priority.Dismantlement - 0.1

sgs.ai_skill_discard.yanzhu = function(self, _, __, optional)
    if not optional then return self:askForDiscard("dummyreason", 1, 1, false, true) end
    
    if self:needToThrowArmor() and self.player:getArmor() and not self.player:isJilei(self.player:getArmor()) then return self.player:getArmor():getEffectiveId() end
    
    if self.player:getTreasure() then
        if (self.player:getCardCount() == 1) then return self.player:getTreasure():getEffectiveId()
        elseif not self.player:isKongcheng() then return self:askForDiscard("dummyreason", 1, 1, false, false) end
    end
    
    if self.player:getEquips():length() > 2 and not self.player:isKongcheng() then return self:askForDiscard("dummyreason", 1, 1, false, false) end
    
    if self.player:getEquips():length() == 1 then return {} end
    
     return self:askForDiscard("dummyreason", 1, 1, false, true)
end

