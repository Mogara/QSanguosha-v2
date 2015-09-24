sgs.ai_skill_playerchosen.huituo = function(self, targets)
    self:sort(self.friends, "defense")
    return self.friends[1]
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
    local lord = self.room:getCurrentDyingPlayer()
    local save = false
    if lord and lord:getLostHp() > 0 then
        if self:isFriend(lord) then
            save = true
        elseif self.role == "renegade" and lord:isLord() and self.room:alivePlayerCount() > 2 then
            if lord:getHp() <= 0 then
                save = true
            end
        end
    end
    if save then
        local hp_max = lord:getHp() + self:getAllPeachNum(lord)
        local must_save = ( hp_max <= 0 )
        local others = self.room:getOtherPlayers(lord)
        others:removeOne(self.player)
        local count = 0
        for _,p in sgs.qlist(others) do
            if p:getKingdom() == "wei" and p:getMark("xingshuai_act") == 0 then
                if not self:isEnemy(p, lord) then
                    count = count + 1
                end
            end
        end
        local am_safe = false
        if self.player:getHp() > 1 then
            am_safe = true
        elseif self.player:hasSkill("buqu") then
            am_safe = true
        elseif self.player:hasSkill("nosbuqu") then
            am_safe = true
        elseif self.player:hasSkill("niepan") and self.player:getMark("@nirvana") > 0 then
            am_safe = true
        elseif self.player:hasSkill("fuli") and self.player:getMark("@laoji") > 0 then
            am_safe = true
        elseif self.player:hasSkill("zhichi") and self.player:getMark("@late") > 0 then
            am_safe = true
        elseif self.player:hasSkill("fenyong") and self.player:getMark("@fenyong") > 0 then
            am_safe = true
        elseif self.player:getMark("@fog") > 0 then
            am_safe = true
        elseif self.player:hasSkill("sizhan") then
            am_safe = true
        elseif self:getAllPeachNum() > 0 then
            am_safe = true
        end
        if self:hasSkills(sgs.masochism_skill) and am_safe then
            return true
        elseif must_save then
            if am_safe then
                return true
            elseif hp_max + count <= 0 then
                return true
            elseif self.role == "renegade" or self.role == "lord" then
                return false
            end
        end
        if am_safe then
            if getBestHp(self.player) > self.player:getHp() then
                return true
            end
        elseif self.player:hasSkill("wuhun") and self:needDeath(self.player) then
            if self.role ~= "renegade" and self.role ~= "lord" then
                return true
            end
        end
    end
    return false
end
sgs.ai_choicemade_filter["skillInvoke"]["_xingshuai"] = function(self, player, promptlist)
    if #promptlist == "yes" then
        local lord = self.room:getCurrentDyingPlayer()
        if lord and not self:hasSkills(sgs.masochism_skill, player) then
            sgs.updateIntention(player, lord, -60)
        end
    end
end

-- taoxi buhui!!!
--player->askForSkillInvoke(objectName(), QVariant::fromValue(to))
sgs.ai_skill_invoke["taoxi"] = function(self, data)
    local to = data:toPlayer()
    if to and self:isEnemy(to) then

        if self.player:hasSkill("lihun") and to:isMale() and not self.player:hasUsed("LihunCard") then
            local callback = lihun_skill.getTurnUseCard
            if type(callback) == "function" then
                local skillcard = callback(self)
                if skillcard then
                    local dummy_use = {
                        isDummy = true,
                        to = sgs.SPlayerList(),
                    }
                    self:useSkillCard(skillcard, dummy_use)
                    if dummy_use.card then
                        for _,p in sgs.qlist(dummy_use.to) do
                            if p:objectName() == to:objectName() then
                                return true
                            end
                        end
                    end
                end
            end
        end

        if self.player:hasSkill("dimeng") and not self.player:hasUsed("DimengCard") then
            local callback = dimeng_skill.getTurnUseCard
            if type(callback) == "function" then
                local skillcard = callback(self)
                if skillcard then
                    local dummy_use = {
                        isDummy = true,
                        to = sgs.SPlayerList(),
                    }
                    self:useSkillCard(skillcard, dummy_use)
                    if dummy_use.card then
                        for _,p in sgs.qlist(dummy_use.to) do
                            if p:objectName() == to:objectName() then
                                return true
                            end
                        end
                    end
                end
            end
        end

        local dismantlement_count = 0
        if to:hasSkill("noswuyan") or to:getMark("@late") > 0 then
            if self.player:hasSkill("yinling") then
                local black = self:getSuitNum("spade|club", true)
                local num = 4 - self.player:getPile("brocade"):length()
                dismantlement_count = dismantlement_count + math.max(0, math.min(black, num))
            end
        else
            dismantlement_count = dismantlement_count + self:getCardsNum("Dismantlement")
            if self.player:distanceTo(to) == 1 or self:hasSkills("qicai|nosqicai") then
                dismantlement_count = dismantlement_count + self:getCardsNum("Snatch")
            end
        end

        local handcards = to:getHandcards()
        if dismantlement_count >= handcards:length() then
            return true
        end

        local can_use, cant_use = {}, {}
        for _,c in sgs.qlist(handcards) do
            if self.player:isCardLimited(c, sgs.Card_MethodUse, false) then
                table.insert(cant_use, c)
            else
                table.insert(can_use, c)
            end
        end

        if #can_use == 0 and dismantlement_count == 0 then
            return false
        end

        if self:needToLoseHp() then
            --Infact, needToLoseHp now doesn't mean self.player still needToLoseHp when this turn end.
            --So this part may make us upset sometimes... Waiting for more details.
            return true
        end

        local knowns, unknowns = {}, {}
        local flag = string.format("visible_%s_%s", self.player:objectName(), to:objectName())
        for _,c in sgs.qlist(handcards) do
            if c:hasFlag("visible") or c:hasFlag(flag) then
                table.insert(knowns, c)
            else
                table.insert(unknowns, c)
            end
        end

        if #knowns > 0 then --Now I begin to lose control... Need more help.
            local can_use_record = {}
            for _,c in ipairs(can_use) do
                can_use_record[c:getId()] = true
            end

            local can_use_count = 0
            local to_can_use_count = 0
            local function can_use_check(user, to_use)
                if to_use:isKindOf("EquipCard") then
                    return not user:isProhibited(user, to_use)
                elseif to_use:isKindOf("BasicCard") then
                    if to_use:isKindOf("Jink") then
                        return false
                    elseif to_use:isKindOf("Peach") then
                        if user:hasFlag("Global_PreventPeach") then
                            return false
                        elseif user:getLostHp() == 0 then
                            return false
                        elseif user:isProhibited(user, to_use) then
                            return false
                        end
                        return true
                    elseif to_use:isKindOf("Slash") then
                        if to_use:isAvailable(user) then
                            local others = self.room:getOtherPlayers(user)
                            for _,p in sgs.qlist(others) do
                                if user:canSlash(p, to_use) then
                                    return true
                                end
                            end
                        end
                        return false
                    elseif to_use:isKindOf("Analeptic") then
                        if to_use:isAvailable(user) then
                            return not user:isProhibited(user, to_use)
                        end
                    end
                elseif to_use:isKindOf("TrickCard") then
                    if to_use:isKindOf("Nullification") then
                        return false
                    elseif to_use:isKindOf("DelayedTrick") then
                        if user:containsTrick(to_use:objectName()) then
                            return false
                        elseif user:isProhibited(user, to_use) then
                            return false
                        end
                        return true
                    elseif to_use:isKindOf("Collateral") then
                        local others = self.room:getOtherPlayers(user)
                        local selected = sgs.PlayerList()
                        for _,p in sgs.qlist(others) do
                            if to_use:targetFilter(selected, p, user) then
                                local victims = self.room:getOtherPlayers(p)
                                for _,p2 in sgs.qlist(victims) do
                                    if p:canSlash(p2) then
                                        return true
                                    end
                                end
                            end
                        end
                    elseif to_use:isKindOf("ExNihilo") then
                        return not user:isProhibited(user, to_use)
                    else
                        local others = self.room:getOtherPlayers(user)
                        for _,p in sgs.qlist(others) do
                            if not user:isProhibited(p, to_use) then
                                return true
                            end
                        end
                        if to_use:isKindOf("GlobalEffect") and not user:isProhibited(user, to_use) then
                            return true
                        end
                    end
                end
                return false
            end
            for _,c in ipairs(knowns) do
                if can_use_record[c:getId()] and can_use_check(self.player, c) then
                    can_use_count = can_use_count + 1
                end
            end

            local to_friends = self:getFriends(to)
            local to_has_weak_friend = false
            local to_is_weak = self:isWeak(to)
            for _,friend in ipairs(to_friends) do
                if self:isEnemy(friend) and self:isWeak(friend) then
                    to_has_weak_friend = true
                    break
                end
            end

            local my_trick, my_slash, my_aa, my_duel, my_sa = nil, nil, nil, nil, nil
            local use = self.player:getTag("taoxi_carduse"):toCardUse()
            local ucard = use.card
            if ucard:isKindOf("TrickCard") then
                my_trick = 1
                if ucard:isKindOf("Duel") then
                    my_duel = 1
                elseif ucard:isKindOf("ArcheryAttack") then
                    my_aa = 1
                elseif ucard:isKindOf("SavageAssault") then
                    my_sa = 1
                end
            elseif ucard:isKindOf("Slash") then
                my_slash = 1
            end
            
            for _,c in ipairs(knowns) do
                if isCard("Nullification", c, to) then
                    my_trick = my_trick or ( self:getCardsNum("TrickCard") - self:getCardsNum("DelayedTrick") )
                    if my_trick > 0 then
                        to_can_use_count = to_can_use_count + 1
                        continue
                    end
                end
                if isCard("Jink", c, to) then
                    my_slash = my_slash or self:getCardsNum("Slash")
                    if my_slash > 0 then
                        to_can_use_count = to_can_use_count + 1
                        continue
                    end
                    my_aa = my_aa or self:getCardsNum("ArcheryAttack")
                    if my_aa > 0 then
                        to_can_use_count = to_can_use_count + 1
                        continue
                    end
                end
                if isCard("Peach", c, to) then
                    if to_has_weak_friend then
                        to_can_use_count = to_can_use_count + 1
                        continue
                    end
                end
                if isCard("Analeptic", c, to) then
                    if to:getHp() <= 1 and to_is_weak then
                        to_can_use_count = to_can_use_count + 1
                        continue
                    end
                end
                if isCard("Slash", c, to) then
                    my_duel = my_duel or self:getCardsNum("Duel")
                    if my_duel > 0 then
                        to_can_use_count = to_can_use_count + 1
                        continue
                    end
                    my_sa = my_sa or self:getCardsNum("SavageAssault")
                    if my_sa > 0 then
                        to_can_use_count = to_can_use_count + 1
                        continue
                    end
                end
            end

            if can_use_count >= to_can_use_count + #unknowns then
                return true
            elseif can_use_count > 0 and ( can_use_count + 0.01 ) / ( to_can_use_count + 0.01 ) >= 0.5 then
                return true
            end
        end

        if self:getCardsNum("Peach") > 0 then
            return true
        end
    end
    return false
end

-- huaiyi buhui!!!
--HuaiyiCard:Play
huaiyi_skill = {
    name = "huaiyi",
    getTurnUseCard = function(self, inclusive)
        if self.player:hasUsed("HuaiyiCard") then
            return nil
        elseif self.player:isKongcheng() then
            return nil
        end
        local handcards = self.player:getHandcards()
        local red, black = false, false
        for _,c in sgs.qlist(handcards) do
            if c:isRed() and not red then
                red = true
                if black then
                    break
                end
            elseif c:isBlack() and not black then
                black = true
                if red then
                    break
                end
            end
        end
        if red and black then
            return sgs.Card_Parse("@HuaiyiCard=.")
        end
    end,
}
table.insert(sgs.ai_skills, huaiyi_skill)
sgs.ai_skill_use_func["HuaiyiCard"] = function(card, use, self)
    local handcards = self.player:getHandcards()
    local reds, blacks = {}, {}
    for _,c in sgs.qlist(handcards) do
        local dummy_use = {
            isDummy = true,
        }
        if c:isKindOf("BasicCard") then
            self:useBasicCard(c, dummy_use)
        elseif c:isKindOf("EquipCard") then
            self:useEquipCard(c, dummy_use)
        elseif c:isKindOf("TrickCard") then
            self:useTrickCard(c, dummy_use)
        end
        if dummy_use.card then
            return --It seems that self.player should use this card first.
        end
        if c:isRed() then
            table.insert(reds, c)
        else
            table.insert(blacks, c)
        end
    end
    
    local targets = self:findPlayerToDiscard("he", false, false, nil, true)
    local n_reds, n_blacks, n_targets = #reds, #blacks, #targets
    if n_targets == 0 then
        return 
    elseif n_reds - n_targets >= 2 and n_blacks - n_targets >= 2 and handcards:length() - n_targets >= 5 then
        return 
    end
    --[[------------------
        Haven't finished.
    ]]--------------------
    use.card = card
end
--room->askForChoice(source, "huaiyi", "black+red")
sgs.ai_skill_choice["huaiyi"] = function(self, choices, data)
    local choice = sgs.huaiyi_choice
    if choice then
        sgs.huaiyi_choice = nil
        return choice
    end
    local handcards = self.player:getHandcards()
    local reds, blacks = {}, {}
    for _,c in sgs.qlist(handcards) do
        if c:isRed() then
            table.insert(reds, c)
        else
            table.insert(blacks, c)
        end
    end
    if #reds < #blacks then
        return "red"
    else
        return "black"
    end
end
--room->askForUseCard(source, "@@huaiyi", "@huaiyi:::" + QString::number(n), -1, Card::MethodNone);
sgs.ai_skill_use["@@huaiyi"] = function(self, prompt, method)
    local n = self.player:getMark("huaiyi_num")
    if n >= 2 then
        if self:needToLoseHp() then
        elseif self:isWeak() then
            n = 1
        end
    end
    if n == 0 then
        return "."
    end
    local targets = self:findPlayerToDiscard("he", false, false, nil, true)
    local names = {}
    for index, target in ipairs(targets) do
        if index <= n then
            table.insert(names, target:objectName())
        else
            break
        end
    end
    return "@HuaiyiSnatchCard=.->"..table.concat(names, "+")
end

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
    if (self.player:getMark("zhanjuedraw") >= 2) then return nil end

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

    local n = (self.player:getMark("yanzhu_lost") == 0) and self.player:getHp() or self.player:getMaxHp()

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
    for _, c in sgs.qlist(self.player:getHandcards()) do
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

    use.card = sgs.Card_Parse("@YjYanyuCard=" .. tostring(id))
end

sgs.ai_skill_playerchosen.yjyanyu = function(self)

    self:sort(self.friends_noself, "handcard")

    for i = #self.friends_noself, 1, -1 do
        if self.friends_noself[i]:isMale() then
            return self.friends_noself[i]
        end
    end

    return nil
end

sgs.ai_use_priority.YjYanyuCard = sgs.ai_use_priority.Slash - 0.01

-- furong 懒得想
-- 出牌阶段限一次，你可以令一名其他角色与你同时展示一张手牌：
-- 若你展示的是【杀】且该角色展示的不是【闪】，则你弃置此【杀】并对其造成1点伤害；
-- 若你展示的不是【杀】且该角色展示的是【闪】，则你弃置你展示的牌并获得其一张牌。 

function getKnownHandcards(player, target)
    local handcards = target:getHandcards()
    
    if player:objectName() == target:objectName() then
        return sgs.QList2Table(handcards), {}
    end
    
    local dongchaee = global_room:getTag("Dongchaee"):toString() or ""
    if dongchaee == target:objectName() then
        local dongchaer = global_room:getTag("Dongchaer"):toString() or ""
        if dongchaer == player:objectName() then
            return sgs.QList2Table(handcards), {}
        end
    end
    
    local knowns, unknowns = {}, {}
    local flag = string.format("visible_%s_%s", player:objectName(), target:objectName())
    for _,card in sgs.qlist(handcards) do
        if card:hasFlag("visible") or card:hasFlag(flag) then
            table.insert(knowns, card)
        else
            table.insert(unknowns, card)
        end
    end
    return knowns, unknowns
end

furong_skill = {name = "furong"}
table.insert(sgs.ai_skills, furong_skill)
furong_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("FurongCard") then
        return nil
    elseif self.player:isKongcheng() then
        return nil
    end
    return sgs.Card_Parse("@FurongCard=.")
end

sgs.ai_skill_use_func.FurongCard = function(card, use, self)
    local handcards = self.player:getHandcards()
    local my_slashes, my_cards = {}, {}
    for _,c in sgs.qlist(handcards) do
        if c:isKindOf("Slash") then
            table.insert(my_slashes, c)
        else
            table.insert(my_cards, c)
        end
    end
    local no_slash = ( #my_slashes == 0 ) 
    local all_slash = ( #my_cards == 0 ) 
    local need_slash, target = nil, nil
    
    --自己展示的一定不是【杀】，目标展示的必须是【闪】，方可获得目标一张牌
    if no_slash then
        local others = self.room:getOtherPlayers(self.player)
        local targets = self:findPlayerToDiscard("he", false, false, others, true)
        for _,p in ipairs(targets) do
            if not p:isKongcheng() then
                local knowns, unknowns = getKnownHandcards(self.player, p)
                if #unknowns == 0 then
                    local all_jink = true
                    for _,jink in ipairs(knowns) do
                        if not jink:isKindOf("Jink") then
                            all_jink = false
                            break
                        end
                    end
                    if all_jink then
                        need_slash, target = false, p
                        break
                    end
                end
            end
        end
    end
    
    --自己展示的一定是【杀】，目标展示的不是【闪】时，可对目标造成1点伤害
    if all_slash and not target then
        local targets = self:findPlayerToDamage(1, self.player, nil, nil, false, 5, true)
        for _,p in ipairs(targets) do
            if not p:isKongcheng() then
                local knowns, unknowns = getKnownHandcards(self.player, p)
                if self:isFriend(p) then
                    local all_jink = true
                    if #unknowns == 0 then
                        for _,c in ipairs(knowns) do
                            if not c:isKindOf("Jink") then
                                all_jink = false
                                break
                            end
                        end
                    else
                        all_jink = false --队友会配合不展示【闪】的
                    end
                    if not all_jink then
                        need_slash, target = true, p
                        break
                    end
                else
                    local all_jink = false
                    if #unknowns == 0 then
                        for _,c in ipairs(knowns) do
                            if c:isKindOf("Jink") then
                                all_jink = true
                                break
                            end
                        end
                    end
                    if not all_jink then
                        need_slash, target = true, p
                        break
                    end
                end
            end
        end
    end
    
    --自己展示的不一定是【杀】，可根据目标情况决定展示的牌
    if not target then
        local friends, enemies, others = {}, {}, {}
        local other_players = self.room:getOtherPlayers(self.player)
        for _,p in sgs.qlist(other_players) do
            if not p:isKongcheng() then
                if self:isFriend(p) then
                    table.insert(friends, p)
                elseif self:isEnemy(p) then
                    table.insert(enemies, p)
                else
                    table.insert(others, p)
                end
            end
        end
        
        local to_damage = self:findPlayerToDamage(1, self.player, nil, enemies, false, 5, true)
        for _,enemy in ipairs(to_damage) do
            local knowns, unknowns = getKnownHandcards(self.player, enemy)
            local no_jink = true
            if #unknowns == 0 then
                for _,jink in ipairs(knowns) do
                    if jink:isKindOf("Jink") then
                        no_jink = false
                        break
                    end
                end
            else
                no_jink = false
            end
            if no_jink then
                need_slash, target = true, enemy
                break
            end
        end
        
        if not target then
            local other_players = self.room:getOtherPlayers(self.player)
            local to_obtain = self:findPlayerToDiscard("he", false, false, other_players, true)
            for _,p in ipairs(to_obtain) do
                if not p:isKongcheng() then
                    local knowns, unknowns = getKnownHandcards(self.player, p)
                    if self:isFriend(p) then
                        local has_jink = false
                        for _,jink in ipairs(knowns) do
                            if jink:isKindOf("Jink") then
                                has_jink = true
                                break
                            end
                        end
                        if has_jink then
                            need_slash, target = false, p
                            break
                        end
                    else
                        local all_jink = true
                        if #unknowns == 0 then
                            for _,c in ipairs(knowns) do
                                if not c:isKindOf("Jink") then
                                    all_jink = false
                                    break
                                end
                            end
                        else
                            all_jink = false
                        end
                        if all_jink then
                            need_slash, target = false, p
                            break
                        end
                    end
                end
            end
        end
        
        if not target then
            to_damage = self:findPlayerToDamage(1, self.player, nil, friends, false, 25, true)
            for _,friend in ipairs(to_damage) do
                local knowns, unknowns = getKnownHandcards(self.player, friend)
                local all_jink = true
                for _,c in ipairs(knowns) do
                    if not c:isKindOf("Jink") then
                        all_jink = false
                        break
                    end
                end
                if not all_jink then
                    need_slash, target = true, friend
                    break
                end
            end
        end
        
        if not target then
            local victim = self:findPlayerToDamage(1, self.player, nil, others, false, 5)
            if victim then
                need_slash, target = true, victim
            end
        end
        
        --只是为了看牌……
        if not target and #my_cards > 0 and self:getOverflow() > 0 then
            if #enemies > 0 then
                self:sort(enemies, "handcard")
                need_slash, target = false, enemies[1]
            elseif #others > 0 then
                self:sort(others, "threat")
                need_slash, target = false, others[1]
            end
        end
        
        if not target and #enemies > 0 then
            self:sort(enemies, "defense")
            need_slash, target = (math.random(0, 1) == 0), enemies[1]
        end
    end
    
    if target and not target:isKongcheng() then
        local use_cards = need_slash and my_slashes or my_cards
        if #use_cards > 0 then
            self:sortByUseValue(use_cards, true)
            local card_str = "@FurongCard="..use_cards[1]:getEffectiveId()
            local acard = sgs.Card_Parse(card_str)
            use.card = acard
            if use.to then
                use.to:append(target)
            end
        end
    end
end
sgs.ai_use_priority["FurongCard"] = 3.1
sgs.ai_use_value["FurongCard"] = 4.5
--room->askForExchange(effect.to, "furong", 1, 1, false, "@furong-show")

-- sgs.ai_skill_discard.furong = function(self) end

-- huomo buhui !!!
-- 每当你需要使用一张你于此回合内未使用过的基本牌时，你可以将一张黑色非基本牌置于牌堆顶，然后视为你使用了此基本牌。
sgs.ai_cardsview_valuable.huomo = function(self, class_name, player) 
    local pattern = nil
    if class_name == "Slash" and player:getMark("Huomo_Slash") == 0 then
        pattern = "slash"
    elseif class_name == "Jink" and player:getMark("Huomo_Jink") == 0 then
        pattern = "jink"
    elseif class_name == "Peach" and player:getMark("Huomo_Peach") == 0 then
        pattern = "peach"
    elseif class_name == "Analeptic" and player:getMark("Huomo_Analeptic") == 0 then
        pattern = "analeptic"
    end
    if pattern then
        local cards = player:getCards("he")
        local blacks = {}
        for _,c in sgs.qlist(cards) do
            if c:isKindOf("BasicCard") then
            elseif c:isBlack() then
                table.insert(blacks, c)
            end
        end
        if #blacks > 0 then
            local card = blacks[1]
            if card:isKindOf(class_name) then
            else
                local card_str = "@HuomoCard="..card:getEffectiveId()..":"..pattern
                return card_str
            end
        end
    end
end

huomo_skill = {name = "huomo"}
table.insert(sgs.ai_skills, huomo_skill)
huomo_skill.getTurnUseCard = function(self)
    if self.player:isNude() then
        return nil
    elseif self.player:getMark("Huomo_Peach") == 0 and self.player:getLostHp() > 0 then
        return sgs.Card_Parse("@HuomoCard=.:peach")
    elseif self.player:getMark("Huomo_Analpetic") == 0 and sgs.Analeptic_IsAvailable(self.player) then
        return sgs.Card_Parse("@HuomoCard=.:analeptic")
    elseif self.player:getMark("Huomo_Slash") == 0 and sgs.Slash_IsAvailable(self.player) then
        return sgs.Card_Parse("@HuomoCard=.:slash")
    end
end

sgs.ai_skill_use_func.HuomoCard = function(card, use, self) 
    local handcards = self.player:getHandcards()
    local can_use = {}
    for _,black in sgs.qlist(handcards) do
        if black:isBlack() and not black:isKindOf("BasicCard") then
            table.insert(can_use, black)
        end
    end
    if #can_use == 0 or self:hasSkills(sgs.lose_equip_skill) then
        local equips = self.player:getEquips()
        for _,equip in sgs.qlist(equips) do
            if equip:isBlack() then
                table.insert(can_use, equip)
            end
        end
    end
    if #can_use == 0 then
        return 
    end
    
    self:sortByKeepValue(can_use)
    local to_use, pattern = nil, nil
    local use_peach, use_anal, use_slash = false, false, false
    
    if self.player:getMark("Huomo_Peach") == 0 then
        if self.player:getLostHp() > 0 and self:isWeak() then
            local peach = sgs.Sanguosha:cloneCard("peach")
            peach:deleteLater()
            local dummy_use = {
                isDummy = true,
            }
            self:useBasicCard(peach, dummy_use)
            if dummy_use.card then
                use_peach = true
                local value = sgs.ai_use_value["Peach"] or 0
                for _,c in ipairs(can_use) do
                    if self:getUseValue(c) <= value then
                        to_use, pattern = c, "peach"
                        break
                    end
                end
            end
        end
    end
    
    if not to_use and self.player:getMark("Huomo_Analeptic") == 0 then
        if sgs.Analeptic_IsAvailable(self.player) then
            local anal = sgs.Sanguosha:cloneCard("analeptic")
            anal:deleteLater()
            local dummy_use = {
                isDummy = true,
            }
            self:useBasicCard(anal, dummy_use)
            if dummy_use.card then
                use_anal = true
                local value = sgs.ai_use_value["Analeptic"] or 0
                for _,c in ipairs(can_use) do
                    if self:getUseValue(c) <= value then
                        to_use, pattern = c, "analeptic"
                        break
                    end
                end
            end
        end
    end
    
    if not to_use and self.player:getMark("Huomo_Slash") == 0 then
        if sgs.Slash_IsAvailable(self.player) then
            local slash = sgs.Sanguosha:cloneCard("slash")
            slash:deleteLater()
            local dummy_use = {
                isDummy = true,
            }
            self:useBasicCard(slash, dummy_use)
            if dummy_use.card then
                use_slash = true
                local value = sgs.ai_use_value["Slash"] or 0
                for _,c in ipairs(can_use) do
                    if self:getUseValue(c) <= value then
                        to_use, pattern = c, "slash"
                        break
                    end
                end
            end
        end
    end
    
    if not to_use and self:getOverflow() > 0 then
        if use_anal then
            to_use, pattern = can_use[1], "analeptic"
        elseif use_slash then
            to_use, pattern = can_use[1], "slash"
        end
    end
    
    if not to_use and self.player:getMark("Huomo_Peach") == 0 then
        if not use_peach and self.player:getLostHp() > 0 then
            local peach = sgs.Sanguosha:cloneCard("Peach")
            peach:deleteLater()
            local dummy_use = {
                isDummy = true,
            }
            self:useBasicCard(peach, dummy_use)
            if dummy_use.card then
                use_peach = true
                local value = sgs.ai_use_value["Peach"] or 0
                for _,c in ipairs(can_use) do
                    if self:getUseValue(c) <= value then
                        to_use, pattern = c, "peach"
                        break
                    end
                end
            end
        end
        if use_peach then
            to_use = to_use or can_use[1]
            pattern = "peach"
        end
    end
    
    if to_use and pattern then
        local card_str = "@HuomoCard="..to_use:getEffectiveId()..":"..pattern
        local acard = sgs.Card_Parse(card_str)
        use.card = acard
    end
end
sgs.ai_use_value["HuomoCard"] = 2.7
sgs.ai_use_priority["HuomoCard"] = 3.5

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
            if player:inMyAttackRange(p) and not player:inMyAttackRange(p, range) then n = n + 1 end
        end
        return n
    end

    function filluse(to, id)
        use.card = card
        if (use.to) then use.to:append(to) end
        self.anguoid = id
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
            filluse(l[1].player, l[1].id)
            return
        end
    end

    for _, p in ipairs(self.enemies) do
        if (p:getTreasure()) then
            filluse(p, p:getTreasure():getEffectiveId())
            return
        end
    end

    for _, p in ipairs(self.friends_noself) do
        if (self:needToThrowArmor(p) and p:getArmor()) then
            filluse(p, p:getArmor():getEffectiveId())
            return
        end
    end

    self:sort(self.enemies, "threat")
    for _, p in ipairs(self.enemies) do
        if (p:getArmor() and not p:getArmor():isKindOf("GaleShell")) then
            filluse(p, p:getArmor():getEffectiveId())
            return
        end
    end

    for _, p in ipairs(self.enemies) do
        if (p:getDefensiveHorse()) then
            filluse(p, p:getDefensiveHorse():getEffectiveId())
            return
        end
    end
end

sgs.ai_use_priority.AnguoCard = sgs.ai_use_priority.ExNihilo + 0.01

sgs.ai_skill_cardchosen.anguo = function(self)
    return self.anguoid
end
