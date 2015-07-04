sgs.ai_skill_playerchosen.huituo = function(self, targets)
    self:sort(self.friends, "defense")
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

    self:sort(self.enemies, "threat")
    for _, p in ipairs(self.enemies) do
        if not p:isNude() then
            use.card = card
            if (use.to) then use.to:append(p) end
            -- use.from = self.player
            return 
        end
    end
    
    for _, p in ipairs(self.friends_noself) do
        if self.needToThrowArmor(p) and p:getArmor() and not p:isJilei(p:getArmor()) then
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

sgs.ai_skill_use["@@xingxue"] = function(self) 

    local n = self.player:hasSkill("yanzhu", true) and self.player:getHp() or self.player:getMaxHp()
    
    self:sort(self.friends, "defense")
    
    n = math.min(n, #self.friends)
    
    local l = sgs.SPlayerList()
    local s = {}
    for i = 1, n, 1 do
        l:append(self.friends[i])
        table.insert(s, self.friends[i]:objectName())
    end
    
    if #s == 0 then return "." end
    
    sgs.xingxuelist = l
    
    return "@XingxueCard=.->" .. (table.concat(s, "+"))
end

-- 兴学剩下的部分大家想象吧，我懒得想
-- sgs.ai_skill_discard.xingxue = function(self) end

sgs.ai_skill_invoke.qiaoshi = true

yanyu_skill = {name = "yjyanyu"}
table.insert(sgs.ai_skills, yanyu_skill)
yanyu_skill.getTurnUseCard = function(self)
    return sgs.Card_Parse("@YjYanyuCard=.")
end

sgs.ai_skill_use_func.YjYanyuCard = function(card, use, self)
    local n = self.player:getMark("yjyanyu")
    
    if n >= 2 then return nil end
    
    local ns, fs, ts = {}, {}, {}
    for _, c in sgs.qlist(self:getHandcards()) do
        if (c:isKindOf("Slash")) then
            n = n + 1
            if (c:isKindOf("FireSlash")) then
                table.insert(fs, c)
            elseif (c:isKindOf("ThunderSlash")) then
                table.insert(ts, c)
            else
                table.insert(ns, c)
            end
        end
    end
    
    if n < 2 then return nil end
    
    local hasmale = false
    for _, p in ipairs(self.friends_noself) do
        if (p:isMale()) then
            hasmale = true
            break
        end
    end
    
    if not hasmale then return nil end
    
    local id = nil
    if #ns > 0 then
        id = ns[1]:getEffectiveId()
    elseif #ts > 0 then
        id = ts[1]:getEffectiveId()
    elseif #fs > 0 then
        id  = fs[1]:getEffectiveId()
    end
    
    if not id then return nil end
    
    use.card = "@YjYanyuCard=" + tostring(id)
end

sgs.ai_skill_playerchosen.yjyanyu = function(self)

    self.sort(self.friends_noself, "handcard")
    
    for i = #self.friends_noself, 1, -1 do
        if self.friends_noself[i]:isMale() then
            return self.friends_noself[i]
        end
    end

    return nil
end

-- furong 懒得想
--[[
furong_skill = {name = "furong"}
table.insert(sgs.ai_skills, furong_skill)
furong_skill.getTurnUseCard = function(self)
    return nil
end

sgs.ai_skill_use_func.FurongCard = function(card, use, self)
    
end
]]
-- sgs.ai_skill_discard.furong = function(self) end

-- huomo buhui !!!
-- sgs.ai_cardsview_valuable.huomo = function() end
--[[
huomo_skill = {name = "huomo"}
table.insert(sgs.ai_skills, huomo_skill)
huomo_skill.getTurnUseCard = function(self)
    return nil
end

sgs.ai_skill_use_func.HuomoCard = function(card, use, self) end
]]


sgs.ai_skill_playerchosen.zuoding = function(self, targets)
    local l = {}
    for _, p in sgs.qlist(targets) do
        if table.contains(self.friends, p, true) then
            table.insert(l, p)
        end
    end
    
    if #l == 0 then return nil end
    
    self:sort(l, "defense")
    return l[#l]
end

anguo_skill = {name = "anguo"}
table.insert(sgs.ai_skills, anguo_skill)
anguo_skill.getTurnUseCard = function(self)
    if (not self.player:hasUsed("AnguoCard")) then
        return sgs.Card_Parse("@AnguoCard=.")
    end
    
    return nil
end

sgs.ai_skill_use_func.AnguoCard = function(card, use, self)
    local l = {}
    function calculateMinus(player, range)
        if range <= 0 then return 0 end
        local n = 0
        for _, p in sgs.qlist(self.room:getAlivePlayers()) do
            if player:inMyAttackRange(p) and not player:inMyAttackRange(p, -range) then n = n + 1 end
        end
        return n
    end
    
    function filluse(to)
        use.card = card
        if (use.to) then use.to:append(to) end
    end
    
    for _, p in ipairs(self.enemies) do
        if (p:getWeapon()) then
            local weaponrange = p:getWeapon():getRealCard():toWeapon():getRange()
            local n = calculateMinus(p, weaponrange - 1)
            table.insert(l, {player = p, id = p:getWeapon():getEffectiveId(), minus = n})
        end
        if (p:getOffensiveHorse()) then
            local n = calculateMinus(p, 1)
            table.insert(l, {player = p, id = p:getOffensiveHorse():getEffectiveId(), minus = n})
        end
    end
    
    if #l > 0 then
        function sortByMinus(a, b)
            return a.minus > b.minus
        end
        
        table.sort(l, sortByMinus)
        if l[1].minus > 0 then
            sgs.anguoid = l[1].id
            filluse(l[1].player)
            return
        end
    end
    
    for _, p in ipairs(self.enemies) do
        if (p:getTreasure()) then
            sgs.anguoid = p:getTreasure():getEffectiveId()
            filluse(p)
            return 
        end
    end
    
    for _, p in ipairs(self.friends_noself) do
        if (self:needToThrowArmor(p) and p:getArmor()) then
            sgs.anguoid = p:getArmor():getEffectiveId()
            filluse(p)
            return 
        end
    end
    
    self:sort(self.enemies, "threat")
    for _, p in ipairs(self.enemies) do
        if (p:getArmor() and not p:getArmor():isKindOf("GaleShell")) then
            sgs.anguoid = p:getArmor():getEffectiveId()
            filluse(p)
            return 
        end
    end
    
    for _, p in ipairs(self.enemies) do
        if (p:getDefensiveHorse()) then
            sgs.anguoid = p:getDefensiveHorse():getEffectiveId()
            filluse(p)
            return 
        end
    end
end

sgs.ai_use_priority.AnguoCard = sgs.ai_use_priority.ExNihilo + 0.01

sgs.ai_skill_cardchosen.AnguoCard = function()
    return sgs.anguoid -- 为什么不选我设置好的anguoid。。为什么乱选。。。
end