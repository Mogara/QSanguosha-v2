-- This is the start script of QSanguosha

package.path = package.path .. ";./lua/lib/?.lua"

dofile "lua/utilities.lua"
dofile "lua/sgs_ex.lua"

function load_translation(file)
	local t = dofile(file)
	if type(t) ~= "table" then
		error(("file %s is should return a table!"):format(file))
	end

	sgs.LoadTranslationTable(t)
end

function load_translations()
	local lang = sgs.GetConfig("Language", "zh_CN")
	local subdir = { "", "Audio", "Package" }
	for _, dir in ipairs(subdir) do
		local lang_dir = "lang/" .. lang .. "/" .. dir
		local lang_files = sgs.GetFileNames(lang_dir)
		for _, file in ipairs(lang_files) do
			load_translation(("%s/%s"):format(lang_dir, file))
		end
	end
end

function load_extensions()
	local scripts = sgs.GetFileNames("extensions")
	local package_names = {}
	for _, script in ipairs(scripts) do
		if script:match(".+%.lua$") then
			local name = script:sub(script:find("%w+"))
			local module_name = "extensions." .. name
			local loaded = require(module_name)
			if loaded and type(loaded) == "table" and loaded.hidden ~= true then -- need to consider the compatibility of 'module'
				if #loaded > 0 then
					for _, extension in ipairs(loaded) do
						if type(extension) == "userdata" and extension:inherits("Package") then
							table.insert(package_names, extension:objectName())
							sgs.Sanguosha:addPackage(extension)
						end
					end
				else
					table.insert(package_names, loaded.extension:objectName())
					sgs.Sanguosha:addPackage(loaded.extension)
				end
			elseif type(loaded) == "userdata" and loaded:inherits("Package") then
				table.insert(package_names, loaded:objectName())
				sgs.Sanguosha:addPackage(loaded)
			end
		end
	end
	local lua_packages = ""
	if #package_names > 0 then lua_packages = table.concat(package_names, "+") end
	sgs.SetConfig("LuaPackages", lua_packages)
end

if not sgs.GetConfig("DisableLua", false) then
	load_extensions()
end

bossModeExpMult = function(level)
	return math.floor(math.log(level) / math.log(2)) + 1
end

BossModeExperience = sgs.CreateTriggerSkill {
	name = "#bossModeExperience",
	events = { sgs.PreCardUsed, sgs.CardResponded, sgs.CardsMoveOneTime, sgs.PreDamageDone,
				sgs.HpLost, sgs.GameOverJudge },
	global = true,
	priority = 15,
	can_trigger = function(self, target)
		if target == nil then return false end
		local room = target:getRoom()
		return room:getMode() == "04_boss" and sgs.GetConfig("BossModeExp", false)
	end,
	on_trigger = function(self, triggerEvent, player, data)
		local room = player:getRoom()
		local level = room:getTag("BossModeLevel"):toInt() + 1
		local x = bossModeExpMult(level)
		if triggerEvent == sgs.PreCardUsed or triggerEvent == sgs.CardResponded then
			if player:isLord() then return false end
			local card = nil
			if triggerEvent == sgs.PreCardUsed then
				card = data:toCardUse().card
			else
				card = data:toCardResponse().m_card
			end
			local typeid = card:getTypeId()
			if typeid == sgs.Card_TypeBasic then
				room:addPlayerMark(player, "@bossExp", x)
			elseif typeid == sgs.Card_TypeTrick then
				room:addPlayerMark(player, "@bossExp", 3 * x)
			elseif typeid == sgs.Card_TypeEquip then
				room:addPlayerMark(player, "@bossExp", 2 * x)
			end
		elseif triggerEvent == sgs.CardsMoveOneTime then
			if player:isLord() then return false end
			local move = data:toMoveOneTime()
			if not move.to or player:objectName() ~= move.to:objectName()
				or (move.from and move.from:objectName() == move.to:objectName())
				or (move.to_place ~= sgs.Player_PlaceHand and move.to_place ~= sgs.Player_PlaceEquip)
				or room:getTag("FirstRound"):toBool() then
				return false
			end
			room:addPlayerMark(player, "@bossExp", move.card_ids:length() * x)
		elseif triggerEvent == sgs.PreDamageDone then
			local damage = data:toDamage()
			if damage.from and not damage.from:isLord() then
				room:addPlayerMark(damage.from, "@bossExp", damage.damage * 5 * x)
			end
			if not damage.to:isLord() then
				room:addPlayerMark(damage.to, "@bossExp", damage.damage * 2 * x)
			end
		elseif triggerEvent == sgs.HpLost then
			if player:isLord() then return false end
			local lose = data:toInt()
			room:addPlayerMark(player, "@bossExp", lose * x)
		elseif triggerEvent == sgs.GameOverJudge then
			local death = data:toDeath()
			if not death.who:isLord() then
				room:removePlayerMark(death.who, "@bossExp", 100)
			else
				for _, p in sgs.qlist(room:getOtherPlayers(death.who)) do
					room:addPlayerMark(p, "@bossExp", 10 * x)
				end
				local damage = death.damage
				if damage and damage.from and damage.from:isAlive() and not damage.from:isLord() then
					room:addPlayerMark(damage.from, "@bossExp", 5 * x)
				end
			end
		end
		return false
	end
}

BossModeMaxCards = sgs.CreateMaxCardsSkill {
	name = "#bossMaxCards",
	fixed_func = function(self, target)
		if target:isLord() and target:getMark("BossMode_Boss") == 1
			and (target:getGeneralName() == "sujiang" or target:getGeneralName() == "sujiangf") then
			return 20
		else
			return -1
		end
	end
}

local skillList = sgs.SkillList()
if not sgs.Sanguosha:getSkill("#bossModeExperience") then skillList:append(BossModeExperience) end
if not sgs.Sanguosha:getSkill("#bossMaxCards") then skillList:append(BossModeMaxCards) end
if not skillList:isEmpty() then sgs.Sanguosha:addSkills(skillList) end

local done_loading = sgs.Sanguosha:property("DoneLoading"):toBool()
if not done_loading then
	load_translations()
	done_loading = sgs.QVariant(true)
	sgs.Sanguosha:setProperty("DoneLoading", done_loading)
end
