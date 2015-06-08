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
			if lord and lord:objectName() ~= victim:objectName() and #enemies > 0 then
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
--[[
	卡牌：狂风甲
	技能：1、锁定技，每次受到火焰伤害时，该伤害+1；
		2、你可以将狂风甲装备和你距离为1以内的一名角色的装备区内
]]--
sgs.ai_card_intention.GaleShell = 80
sgs.ai_use_priority.GaleShell = 0.9
sgs.dynamic_value.control_card.GaleShell = true
sgs.ai_armor_value["gale_shell"] = function(player, self)
	return -10
end
function SmartAI:useCardGaleShell(card, use)
	self:sort(self.enemies, "threat")
	local targets = {}
	for _,enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) == 1 then
			table.insert(targets, enemy)
		end
	end
	if #targets > 0 then
		local function getArmorUseValue(target)
			local value = 0
			if target:getMark("@gale") > 0 then
				value = value + 2
			end
			local armor = target:getArmor()
			if armor then
				value = value + 10
				if target:hasArmorEffect("silver_lion") and target:isWounded() then
					value = value - 4
				end
				if self:hasSkills(sgs.lose_equip_skill, target) then
					value = value - 1.5
				end
				if target:hasSkill("tuntian") then
					value = value - 1
				end
			else
				value = value + 2
				if self:hasSkills("bazhen|yizhong|jiqiao|bossmanjia", target) then
					value = value + 8
				end
				if self:hasSkills(sgs.lose_equip_skill, target) then
					value = value - 2
				end
                if self:hasSkills(sgs.need_equip_skill, target) then
                    value = value - 2
                end
			end
			if self:hasSkills("jijiu|longhun", target) then
				value = value - 5
			end
			if self:hasSkills("wusheng|wushen", target) then
				value = value - 2
			end
			return value
		end
		local values = {}
		for _,enemy in ipairs(targets) do
			values[enemy:objectName()] = getArmorUseValue(enemy)
		end
		local compare_func = function(a, b)
			local valueA = values[a:objectName()] or 0
			local valueB = values[b:objectName()] or 0
			return valueA > valueB
		end
		table.sort(targets, compare_func)
		local target = targets[1]
		local value = values[target:objectName()] or 0
		if value > 0 then
			use.card = card
			if use.to then
				use.to:append(target)
			end
		end
	end
end
--[[
	卡牌：地震
	效果：将【地震】放置于你的判定区里，回合判定阶段进行判定：若判定结果为♣2~9之间，与当前角色距离为1以内的角色(无视+1马)弃置装备区里的所有牌，将【地震】置入弃牌堆。若判定结果不为♣2~9之间，将【地震】移动到当前角色下家的判定区里
]]--
function SmartAI:useCardEarthquake(card, use)
	if self.player:containsTrick("earthquake") then
		return
	elseif self.player:isProhibited(self.player, card) then
		return
	elseif self.player:containsTrick("YanxiaoCard") and self:getOverflow() > 0 then
		use.card = card
		return
	end
	local value = 0
	local finalRetrial, wizard = self:getFinalRetrial(self.player, "earthquake")
	if finalRetrial == 2 then
		return
	elseif finalRetrial == 1 then
		value = value + 12
	end
	local function getEquipsValue(player)
		local v = 0
		local danID = self:getDangerousCard(player)
		local weapon = player:getWeapon()
		if weapon then
			v = v + 5
			if danID and weapon:getEffectiveId() == danID then
				value = value + 2
			end
		end
		local armor = player:getArmor()
		if armor then
			v = v + 8
			if danID and armor:getEffectiveId() == danID then
				value = value + 2
			end
		end
		local dhorse = player:getDefensiveHorse()
		if dhorse then
			v = v + 7
		end
		local ohorse = player:getOffensiveHorse()
		if ohorse then
			v = v + 4
		end
		local treasure = player:getTreasure()
		if treasure then
			v = v + 2
		end
		return v
	end
	if #self.enemies > 0 then
		for _,enemy in ipairs(self.enemies) do
			if self:hasSkills("tiandu|luoying", enemy) then
				value = value - 10
			end
			if self:hasSkills("guanxing|super_guanxing", enemy) then
				value = value + 2
			end
			if enemy:hasSkill("xinzhan") then
				value = value - 1
			end
			local equips = enemy:getEquips()
			if not equips:isEmpty() then
				value = value + getEquipsValue(enemy)
				if self:hasSkills(sgs.lose_equip_skill, enemy) then
					value = value - equips:length() * 2
				end
				if enemy:getArmor() and self:needToThrowArmor(enemy) then
					value = value - 1.5
				end
				if enemy:hasSkill("tuntian") then
					value = value - 1
				end
			end
		end
	end
	if #self.friends > 0 then
		for _,friend in ipairs(self.friends) do
			if self:hasSkills("tiandu|luoying", friend) then
				value = value + 10
			end
			if self:hasSkills("guanxing|super_guanxing", friend) then
				value = value - 2
			end
			if friend:hasSkill("xinzhan") then
				value = value + 1
			end
			local equips = friend:getEquips()
			if not equips:isEmpty() then
				value = value - getEquipsValue(friend)
				if self:hasSkills(sgs.lose_equip_skill, friend) then
					value = value + equips:length() * 2
				end
				if friend:getArmor() and self:needToThrowArmor(friend) then
					value = value + 1.5
				end
				if friend:hasSkill("tuntian") then
					value = value + 1
				end
			end
		end
	end
	local HanHaoShiHuan = self.room:findPlayerBySkillName("yonglve")
	if HanHaoShiHuan then
		if self:isFriend(HanHaoShiHuan) then
			value = value + 10
		else
			value = value - 10
		end
	end
	if value > 0 then
		use.card = card
	end
end
--[[
	卡牌：台风
	效果：将【台风】放置于你的判定区里，回合判定阶段进行判定：若判定结果为♦2~9之间，与当前角色距离为1的角色弃置6张手牌，将【台风】置入弃牌堆。若判定结果不为♦2~9之间，将【台风】移动到当前角色下家的判定区里
]]--
function SmartAI:useCardTyphoon(card, use)
	if self.player:containsTrick("typhoon") then
		return 
	elseif self.player:isProhibited(self.player, card) then
		return
	elseif self.player:containsTrick("YanxiaoCard") and self:getOverflow() > 0 then
		use.card = card
		return
	end
	local finalRetrial, wizard = self:getFinalRetrial(self.player, "typhoon")
	if finalRetrial == 2 then
		return
	elseif finalRetrial == 1 then
		use.card = card
		return 
	end
	local alives = self.room:getAlivePlayers()
	local value = 0
	for _,p in sgs.qlist(alives) do
		local v = 0
		local num = p:getHandcardNum()
		local discard = math.min(6, num)
		if discard > 0 then
			local keep = num - discard
			v = v + discard * 1.5
			if keep == 0 then
				if self:needKongcheng(p) then
					v = v - 4
				end
				v = v + 10
			else
				v = v + 1.5 ^ keep
			end
		end
		if self:hasSkills("tiandu|luoying", p) then
			v = v - 10
		end
		if self:hasSkills("guanxing|super_guanxing", p) then
			v = v + 2
		end
		if self:isFriend(p) then
			v = - v
		end
		value = value + v
	end
	local HanHaoShiHuan = self.room:findPlayerBySkillName("yonglve")
	if HanHaoShiHuan then
		if self:isFriend(HanHaoShiHuan) then
			value = value + 10
		else
			value = value - 10
		end
	end
	if value > 0 then
		if self:getOverflow() > 0 or value > 6 then
			use.card = card
		end
	end
end
--相关信息：判断是否需要改判
sgs.ai_need_retrial_func["typhoon"] = function(self, judge, isGood, who, isFriend, lord)
	local others = self.room:getOtherPlayers(who)
	local friends, enemies = {}, {}
	for _,p in sgs.qlist(others) do
		if who:distanceTo(p) == 1 then
			if self:isFriend(p) then
				table.insert(friends, p)
			else
				table.insert(enemies, p)
			end
		end
	end
	local friend_discard_num, enemy_discard_num = 0, 0
	for _,friend in ipairs(friends) do
		local num = friend:getHandcardNum()
		num = math.min(6, num)
		friend_discard_num = friend_discard_num + num
	end
	for _,enemy in ipairs(enemies) do
		local num = enemy:getHandcardNum()
		num = math.min(6, num)
		enemy_discard_num = enemy_discard_num + num
	end
	--如果没中奖
	if isGood then
		if friend_discard_num == 0 and enemy_discard_num > 0 then
			return true
		end
		return false
	end
	--如果中奖
	if enemy_discard_num == 0 and friend_discard_num > 0 then
		return true
	elseif friend_discard_num > enemy_discard_num + 1 then
		return true
	end
	return false
end
--相关信息：改判动机值
sgs.ai_retrial_intention["typhoon"] = function(self, player, who, judge, last_judge)
	return 0
end