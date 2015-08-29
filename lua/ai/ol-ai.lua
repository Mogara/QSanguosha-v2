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

--[[
    技能：安恤（阶段技）
    描述：你可以选择两名手牌数不同的其他角色，令其中手牌多的角色将一张手牌交给手牌少的角色，然后若这两名角色手牌数相等，你选择一项：1．摸一张牌；2．回复1点体力。
]]--
--OlAnxuCard:Play
anxu_skill = {
    name = "olanxu",
    getTurnUseCard = function(self, inclusive)
        if self.player:hasUsed("OlAnxuCard") then
            return nil
        elseif self.room:alivePlayerCount() > 2 then
            return sgs.Card_Parse("@OlAnxuCard=.")
        end
    end,
}
table.insert(sgs.ai_skills, anxu_skill)
sgs.ai_skill_use_func["OlAnxuCard"] = function(card, use, self)
    --
end
--room->askForExchange(playerA, "olanxu", 1, 1, false, QString("@olanxu:%1:%2").arg(source->objectName()).arg(playerB->objectName()))
sgs.ai_skill_discard["olanxu"] = function(self, discard_num, min_num, optional, include_equip)
    local others = self.room:getOtherPlayers(self.player)
    local target = nil
    for _,p in sgs.qlist(others) do
        if p:hasFlag("olanxu_target") then
            target = nil
            break
        end
    end
    assert(target)
    local handcards = self.player:getHandcards()
    handcards = sgs.QList2Table(handcards)
    if self:isFriend(target) and not hasManjuanEffect(target) then
        self:sortByUseValue(handcards)
        return { handcards[1]:getEffectiveId() }
    end
    return self:askForDiscard("dummy", discard_num, min_num, optional, include_equip)
end
--room->askForChoice(source, "olanxu", choices)
sgs.ai_skill_choice["olanxu"] = function(self, choices, data)
    local items = choices:split("+")
    if #items == 1 then
        return items[1]
    end
    return "recover"
end
--[[
    技能：追忆
    描述：你死亡时，你可以令一名其他角色（除杀死你的角色）摸三张牌并回复1点体力。
]]--
--[[
    技能：陈情
    描述：每轮限一次，当一名角色处于濒死状态时，你可以令另一名其他角色摸四张牌，然后弃置四张牌。若其以此法弃置的四张牌花色各不相同，则视为该角色对濒死的角色使用一张【桃】
]]--
--room->askForPlayerChosen(source, targets, "olchenqing", QString("@olchenqing:%1").arg(victim->objectName()), false, true)
sgs.ai_skill_playerchosen["olchenqing"] = function(self, targets)
    local victim = self.room:getCurrentDyingPlayer()
    local help = false
    local careLord = false
    if victim then
        if self:isFriend(victim) then
            help = true
        elseif self.role == "renegade" and victim:isLord() and self.room:alivePlayerCount() > 2 then
            help = true
            careLord = true
        end
    end
    local friends, enemies = {}, {}
    for _,p in sgs.qlist(targets) do
        if self:isFriend(p) then
            table.insert(friends, p)
        else
            table.insert(enemies, p)
        end
    end
    local compare_func = function(a, b)
        local nA = a:getCardCount(true)
        local nB = b:getCardCount(true)
        if nA == nB then
            return a:getHandcardNum() > b:getHandcardNum()
        else
            return nA > nB
        end
    end
    if help and #friends > 0 then
        table.sort(friends, compare_func)
        for _,friend in ipairs(friends) do
            if not hasManjuanEffect(friend) then
                return friend
            end
        end
    end
    if careLord and #enemies > 0 then
        table.sort(enemies, compare_func)
        for _,enemy in ipairs(enemies) do
            if sgs.evaluatePlayerRole(enemy) == "loyalist" then
                return enemy
            end
        end
    end
    if #enemies > 0 then
        self:sort(enemies, "threat")
        for _,enemy in ipairs(enemies) do
            if hasManjuanEffect(enemy) then
                return enemy
            end
        end
    end
    if #friends > 0 then
        self:sort(friends, "defense")
        for _,friend ipairs(friends) do
            if not hasManjuanEffect(friend) then
                return friend
            end
        end
    end
end
--room->askForExchange(target, "olchenqing", 4, 4, true, QString("@olchenqing-exchange:%1:%2").arg(source->objectName()).arg(victim->objectName()), false)
sgs.ai_skill_discard["olchenqing"] = function(self, discard_num, min_num, optional, include_equip)
    local victim = self.room:getCurrentDyingPlayer()
    local help = false
    if victim then
        if self:isFriend(victim) then
            help = true
        elseif self.role == "renegade" and victim:isLord() and self.room:alivePlayerCount() > 2 then
            help = true
        end
    end
    local cards = self.room:getCards("he")
    cards = sgs.QList2Table(cards)
    self:sortByKeepValue(cards)
    if help then
        local peach_num = 0
        local spade, heart, club, diamond = nil, nil, nil, nil
        for _,c in ipairs(cards) do
            if isCard("Peach", c, self.player) then
                peach_num = peach_num + 1
            else
                local suit = c:getSuit()
                if not spade and suit == sgs.Card_Spade then
                    spade = c:getEffectiveId()
                elseif not heart and suit == sgs.Card_Heart then
                    heart = c:getEffectiveId()
                elseif not club and suit == sgs.Card_Club then
                    club = c:getEffectiveId()
                elseif not diamond and suit == sgs.Card_Diamond then
                    diamond = c:getEffectiveId()
                end
            end
        end
        if peach_num + victim:getHp() <= 0 then
            if spade and heart and club and diamond then
                return {spade, heart, club, diamond}
            end
        end
    end
    return self:askForDiscard("dummy", discard_num, min_num, optional, include_equip)
end
--[[
    技能：默识
    描述：结束阶段开始时，你可以将一张手牌当你本回合出牌阶段使用的第一张基本或非延时类锦囊牌使用。然后，你可以将一张手牌当你本回合出牌阶段使用的第二张基本或非延时类锦囊牌使用。
]]--
--[[
    技能：庸肆（锁定技）
    描述：摸牌阶段开始时，你改为摸X张牌。锁定技，弃牌阶段开始时，你选择一项：1．弃置一张牌；2．失去1点体力。（X为场上势力数） 
]]--
--room->askForDiscard(player, "olyongsi", 1, 1, true, true, "@olyongsi")
sgs.ai_skill_discard["olyongsi"] = function(self, discard_num, min_num, optional, include_equip)
    if self:needToLoseHp() or getBestHp(self.player) > self.player:getHp() then
        return "."
    end
    return self:askForDiscard("dummy", discard_num, min_num, optional, include_equip)
end
--[[
    技能：觊玺（觉醒技）
    描述：你的回合结束时，若你连续三回合没有失去过体力，则你加1点体力上限并回复1点体力，然后选择一项：1．获得技能“妄尊”；2．摸两张牌并获得当前主公的主公技。
]]--
--room->askForChoice(player, "oljixi", choices.join("+"))
sgs.ai_skill_choice["oljixi"] = function(self, choices, data)
    local items = choices:split("+")
    if #items == 1 then
        return items[1]
    end
    return "wangzun"
end
--[[
    技能：雷击
    描述：当你使用或打出【闪】时，你可以令一名其他角色进行判定，若结果为：♠，你对该角色造成2点雷电伤害；♣，你回复1点体力，然后对该角色造成1点雷电伤害。
]]--
--room->askForPlayerChosen(player, others, "olleiji", "@olleiji", true, true)
sgs.ai_skill_playerchosen["olleiji"] = function(self, targets)
    --
end
--[[
    技能：鬼道
    描述：每当一名角色的判定牌生效前，你可以打出一张黑色牌替换之。
]]--
--[[
    技能：黄天（主公技、阶段技）
    描述：其他群雄角色的出牌阶段，该角色可以交给你一张【闪】或【闪电】。
]]--