sgs.ai_skill_invoke.grab_peach = function(self, data) 
    local from = data:toCardUse().from
    return self:isEnemy(from)
end
--[[
    卡牌：杨修剑
    技能：当你的【杀】造成伤害时，可以指定攻击范围内的一名其他角色为伤害来源，杨修剑归该角色所有
]]--
sgs.weapon_range.YxSword = 3
--room->askForPlayerChosen(player, players, objectName(), "@yxsword-select", true, true)
sgs.ai_skill_playerchosen["yx_sword"] = function(self, targets)
    local data = self.room:getTag("YxSwordData")
    local damage = data:toDamage()
    local victim = damage.to
    local willKillVictim = ( victim:getHp() + self:getAllPeachNum(victim) <= damage.damage )
    local friends, enemies = {}, {}
    for _,p in sgs.qlist(targets) do
        if self:isFriend(p) then
            table.insert(friends, p)
        else
            table.insert(enemies, p)
        end
    end
    if willKillVictim then
        if victim:hasSkill("yuwen") then
            return nil
        elseif victim:hasSkill("duanchang") then
            local bad_skills = "benghuai|wumou|shiyong|yaowu|zaoyao|chanyuan|chouhai"
            local function hasOtherSkills(player)
                local skills = player:getVisibleSkillList()
                for _,skill in sgs.qlist(skills) do
                    if skill:inherits("SPConvertSkill") then
                    elseif skill:isAttachedLordSkill() then
                    elseif skill:isLordSkill() then
                        if player:hasLordSkill(skill:objectName()) then
                            return true
                        end
                    elseif not string.find(bad_skills, skill:objectName()) then
                        return true
                    end
                end
                return false
            end
            if #friends > 0 then
                self:sort(friends, "defense")
                for _,friend in ipairs(friends) do
                    if self:hasSkills(bad_skills, friend) then
                        if not hasOtherSkills(friend) then
                            return friend
                        end
                    end
                end
            end
            if #enemies > 0 then
                self:sort(enemies, "threat")
                for _,enemy in ipairs(enemies) do
                    if hasOtherSkills(enemy) then
                        return enemy
                    end
                end
            end
        end
        local role = sgs.evaluatePlayerRole(victim)
        if role == "rebel" then
            if #friends > 0 then
                local drawTargets = self:findPlayerToDraw(true, 3, true)
                for _,target in ipairs(drawTargets) do
                    for _,friend in ipairs(friends) do
                        if target:objectName() == friend:objectName() then
                            return friend
                        end
                    end
                end
            end
        elseif role == "loyalist" then
            local lord = getLord(victim)
            if lord and #enemies > 0 then
                for _,enemy in ipairs(enemies) do
                    if lord:objectName() == enemy:objectName() then
                        return enemy
                    end
                end
            end
        end
    end
    if self:cantbeHurt(victim, self.player, damage.damage) then
        if #friends > 0 then
            for _,friend in ipairs(friends) do
                if not self:cantbeHurt(victim, friend, damage.damage) then
                    return friend
                end
            end
        end
        if #enemies > 0 then
            for _,enemy in ipairs(enemies) do
                if self:cantbeHurt(victim, enemy, damage.damage) then
                    return enemy
                end
            end
        end
    end
    if not willKillVictim then
        if self:getDamagedEffects(victim, self.player, true) then
            if #friends > 0 then
                for _,friend in ipairs(friends) do
                    if not self:getDamagedEffects(victim, friend, true) then
                        return friend
                    end
                end
            end
            if #enemies > 0 then
                self:sort(enemies, "defense")
                return enemies[1]
            end
        end
    end
    if self:hasSkills(sgs.lose_equip_skill) and #friends > 0 then
        local cards = { self.player:getWeapon() }
        local weapon, priorTarget = self:getCardNeedPlayer(cards, false)
        if weapon and priorTarget then
            for _,friend in ipairs(friends) do
                if priorTarget:objectName() == friend:objectName() then
                    return friend
                end
            end
        end
        self:sort(friends, "threat")
        friends = sgs.reverse(friends)
        return friends[1]
    end
    return nil
end