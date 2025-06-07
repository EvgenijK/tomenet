-- $Id: s_aux.lua,v 1.25 2007/12/28 16:44:42 cblue Exp $
-- Server side functions to help with spells, do not touch
--

__schools = {}
__schools_num = 0

__tmp_spells = {}
__tmp_spells_num = 0

-- for WIELD_BOOKS, bad hack :(
__cur_inven_slot = -1


-- define get_check2() for older clients
if rawget(globals(), "get_check2") == nil then
	function get_check2(prompt, defaultyes)
		return get_check(prompt)
	end
end


-- TomeNET additional
-- Get the amount of mana(or power) needed
function need_direction(s)
	return spell(s).direction
end

function need_item(s)
	return spell(s).item
end



function add_school(s)
	__schools[__schools_num] = s

	__schools_num = __schools_num + 1
	return (__schools_num - 1)
end

function finish_school(i)
	local s

	s = __schools[i]
	assert(s.name, "No school name!")
	assert(s.skill, "No school skill!")
	new_school(i, s.name, s.skill)
end

function add_spell(s)
	__tmp_spells[__tmp_spells_num] = s

	__tmp_spells_num = __tmp_spells_num + 1
	return (__tmp_spells_num - 1)
end

function finish_spell(must_i)
	local i, s

	s = __tmp_spells[must_i]

	assert(s.name, "No spell name!")
	assert(s.name2, "No short name!")
	assert(s.school, "No spell school!")
	assert(s.level, "No spell level!")
	assert(s.mana, "No spell mana!")
	if not s.mana_max then s.mana_max = s.mana end
	assert(s.fail, "No spell failure rate!")
	assert(s.spell, "No spell function!")
	assert(s.info, "No spell info!")
	assert(s.desc, "No spell desc!")
	if not s.direction then s.direction = FALSE end
	if not s.item then s.item = -1 end
	if not s.extrareq then s.extrareq = -1 end
	if not s.extralev then s.extralev = 0 end

	i = new_spell(must_i, s.name)
	assert(i == must_i, "ACK ! i != must_i ! please contact the maintainer")
	if type(s.school) == "number" then __spell_school[i] = {s.school}
	else __spell_school[i] = s.school end
	spell(i).mana = s.mana
	spell(i).mana_max = s.mana_max
	spell(i).fail = s.fail
	spell(i).skill_level = s.level
	if type(s.spell_power) == "number" then
		spell(i).spell_power = s.spell_power
	else
		spell(i).spell_power = 1
	end

	__spell_spell[i] = s.spell
	__spell_name2[i] = s.name2
	__spell_info[i] = s.info
	__spell_desc[i] = s.desc

	spell(i).extrareq = s.extrareq
	spell(i).extralev = s.extralev
	return i
end

-- Creates the school books array
__spell_spell = {}
__spell_name2 = {}
__spell_info = {}
__spell_desc = {}
__spell_school = {}
school_book = {}

-- Find a spell by name
function find_spell(name)
	local i

	i = 0
	while (i < __tmp_spells_num) do
		if __tmp_spells[i].name == name then return i end
		i = i + 1
	end
	return -1
end
-- Find a spell by item+position
function find_spell_from_item(inven_slot, s_ind, school_name)
	local s, book, i

	s = get_inven_pval(Ind, inven_slot)
	book = get_inven_sval(Ind, inven_slot)

	if book == 255 then
		-- spell scrolls only have 1 spell (index 1)
		if s_ind == 1 then
			return s
		else
			return -1
		end
	end

	-- custom tomes
	if book == 100 or book == 101 or book == 102 then
		if s_ind > 9 then
			return -1
		end

		s = get_inven_xtra(Ind, inven_slot, s_ind) - 1
		if s ~= -1 then
			return s
		end
		return -1
	end

	-- static books aka handbooks and tomes
	for i, s in school_book[book] do
		if i == s_ind then
			return s
		end
	end
	return -1
end

-- Find if the school is under the influence of a god, returns nil or the level
--
-- pgod? 100% sure it won't work
--[[
function get_god_level(i, sch)
--	local ply

	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
	else
		ply = player
	end

	if __schools[sch].gods[ply.pgod] then
		return (s_info[__schools[sch].gods[ply.pgod].skill + 1].value * __schools[sch].gods[ply.pgod].mul) / __schools[sch].gods[ply.pgod].div
	else
		return nil
	end
end
]]

-- Change this fct if I want to switch to learnable spells
function get_level_school(i, s, max, min, inven_slot)
	local lvl, sch, index, num, bonus, ply

	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
	else
		ply = player
	end

	lvl = 0
	num = 0
	bonus = 0

	-- No max specified ? assume 50
	if not max then
		max = 50
	end

	for index, sch in __spell_school[s] do
		local r, p, ok = 0, 0, 0, 0

		-- Take the basic skill value
		r = ply.s_info[(school(sch).skill) + 1].value

		-- Are we under sorcery effect ?
		p = 0
		if __schools[sch].sorcery then
			p = ply.s_info[SKILL_SORCERY + 1].value
		end

		-- Find the higher
		ok = r
		if r < p then ok = p end

		-- Do we need to add a special bonus ?
		if __schools[sch].bonus_level then
			bonus = bonus + (__schools[sch].bonus_level() * SKILL_STEP)
		end

		-- Apply it
		lvl = lvl + ok
		num = num + 1
	end

	-- / 10 because otherwise we can overflow a s32b and we cannot use a u32b because the value can be negative
	-- The loss of information should be negligible since 1 skill = 1000 internaly
	lvl = (lvl / num) / 10
	if not min then
		lvl = lua_get_level(i, s, lvl, max, 1, bonus)
	else
		lvl = lua_get_level(i, s, lvl, max, min, bonus)
	end

	-- for WIELD_BOOKS:
	if (inven_slot == INVEN_WIELD and lvl > 0) then lvl = lvl + 1 end

--	--Hack: Disruption Shield only for Istari. Not for Adventurer/Ranger.
--	if spell(s).name == "Disruption Shield" then
--		if ply.pclass == CLASS_RANGER then
--			lvl = 0
--		end
--		if ply.pclass == CLASS_ADVENTURER then
--			lvl = 0
--		end
--	end

	return lvl
end

-- The real get_level, works for schooled magic and for innate powers
function get_level(i, s, max, min)
	-- baaad hack, ugh - for WIELD_BOOKS - needed because get_level() is called from spell_spell and spell_info, which both do not have access to inven_slot!
	local inven_slot = __cur_inven_slot

	if type(s) == "number" then
		return get_level_school(i, s, max, min, inven_slot)
	else
-- this shouldn't happen for now
		return get_level_power(i, s, max, min)
	end
end

-- Can we cast the spell ?
function is_ok_spell(i, s)
	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
	else
		ply = player
	end

	if get_level(i, s, 50, 0) == 0 then return nil end

	if (s == FIREFLASH_I or s == FIREFLASH_II) and ply.prace == RACE_VAMPIRE then
		return nil
	end
	if ply.admin_wiz == 0 and ply.admin_dm == 0 then
		if s == BLOODSACRIFICE and ply.pclass ~= CLASS_HELLKNIGHT and ply.pclass ~= CLASS_CPRIEST then
			return nil
		end
		if (s == OREGEN or s == OUNLIFERES) and ply.prace ~= RACE_VAMPIRE then
			return nil
		end
	end

	if spell(s).extrareq ~= -1 then
		local r = ply.s_info[spell(s).extrareq + 1].value

		if r < spell(s).extralev then return nil end
	end

	return 1
end
--New: Allow spells of identical name; use 'priority' to figure out which one is better
function is_ok_spell2(i, s)
	local lev = get_level(i, s, 50, 0)

	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
	else
		ply = player
	end

	if lev == 0 then return nil end

	if (s == FIREFLASH_I or s == FIREFLASH_II) and ply.prace == RACE_VAMPIRE then
		return nil
	end
	if ply.admin_wiz == 0 and ply.admin_dm == 0 then
		if s == BLOODSACRIFICE and ply.pclass ~= CLASS_HELLKNIGHT and ply.pclass ~= CLASS_CPRIEST then
			return nil
		end
		if (s == OREGEN or s == OUNLIFERES) and ply.prace ~= RACE_VAMPIRE then
			return nil
		end
	end
	if s == OBLINK and ply.s_info[SKILL_CONVEYANCE + 1].value < 5000 then
		return nil
	end
	if s == SHADOWGATE and ply.s_info[SKILL_CONVEYANCE + 1].value < 10000 then
		return nil
	end

	if __tmp_spells[s].priority then lev = lev + __tmp_spells[s].priority end

	return lev
end

-- Get the amount of mana(or power) needed
function get_mana(i, s, inven_slot)
--	local mana, ply
--	mana = spell(s).mana + get_level(i, s, spell(s).mana_max - spell(s).mana, 0)

--	--under influence of Martyrdom, spells cost more mana:
--	if i ~= 0 then ply = players(i) else ply = player end
--	--exempt Martyrdom itself from its double mana cost
--	if ply.martyr > 0 and s ~= HMARTYR then mana = mana * 2 end

--	return mana

	-- for WIELD_BOOKS:
	local __prev_inven_slot = __cur_inven_slot
	__cur_inven_slot = inven_slot
	local lcost = spell(s).mana + get_level(i, s, spell(s).mana_max - spell(s).mana, 0)
	__cur_inven_slot = __prev_inven_slot

	--round up? - we need to use this, because this function is also called for spell information when browsing, so it must not be random
	if (inven_slot == INVEN_WIELD) then lcost = ((lcost * 80) + 99) / 100 end

	return lcost
end

-- Return the amount of power(mana, piety, whatever) for the spell
function get_power(i, s)
	local ply

	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
	else
		ply = player
	end

	if check_affect(s, "piety", FALSE) then
		return ply.grace
	else
	    if ply.cmp == nil then
		return ply.csp
	    else
		return ply.cmp
	    end
	end
end

-- Return the amount of power(mana, piety, whatever) for the spell
function get_power_name(s)
	if check_affect(s, "piety", FALSE) then
		return "piety"
	else
		return "mana"
	end
end

-- Changes the amount of power(mana, piety, whatever) for the spell
function adjust_power(i, s, x)
	if check_affect(s, "piety", FALSE) then
		inc_piety(i, GOD_ALL, x)
	else
		increase_mana(i, x)
	end
end

-- Print the book and the spells
-- XXX client only
function print_book2(i, inven_slot, sval, spl)
	local x, y, index, sch, size, s, book

	x = 0
	y = 2
	size = 0

--[[STOREBROWSE
	if inven_slot ~= -1 then
		--from inventory
		book = get_inven_sval(i, inven_slot)
	else
		--browsing within a store!
		book = sval
	end
]]
	if inven_slot >= 0 then
		--from inventory
		book = get_inven_sval(i, inven_slot)
	else
		--browsing within a store!
		book = sval
	end

	-- Hack for custom tomes
	if book == 100 or book == 101 or book == 102 then
		return print_custom_tome(i, inven_slot)
	end

	-- Hack if the book is 255 it is a random book
	if book == 255 then
		school_book[book] = {spl}
	end

	__cur_inven_slot = inven_slot

	-- Parse all spells
	for index, s in school_book[book] do
		local color = TERM_L_DARK
		local lvl = get_level(i, s, 50, -50)
		local xx, sch_str

		if is_ok_spell(i, s) then
			if get_mana(i, s, inven_slot) > get_power(i, s) then color = TERM_ORANGE
			else color = TERM_L_GREEN end
		end

		xx = nil
		sch_str = ""

		for index, sch in __spell_school[s] do
			if xx then
				sch_str = sch_str.."/"..school(sch).name
			else
				xx = 1
				sch_str = sch_str..school(sch).name
			end
		end
		sch_str = sch_str_lim(sch_str)

		c_prt(color, format("%c) %-22s%-16s %3d %4s %3d%s %s", size + strbyte("a"), spell(s).name, sch_str, lvl, get_mana(i, s, inven_slot), spell_chance(i, s, inven_slot), "%", __spell_info[s]()), y, x)
		y = y + 1
		size = size + 1
	end

	__cur_inven_slot = -1

	prt(format("   %-22s%-14s Level Cost Fail Info", "Name", "School"), 1, x)
	return y
end

function print_custom_tome(i, inven_slot)
	local x, y, index, sch, size, s, custom_book

	x = 0
	y = 2
	size = 0

--[[STOREBROWSE
--HACK/problem: can't browse custom tomes in shops!:
	if inven_slot == -1 then return 0 end
]]

	custom_book = {}

	if get_inven_xtra(Ind, inven_slot, 1) ~= 0 then
		custom_book[1] = get_inven_xtra(Ind, inven_slot, 1) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 2) ~= 0 then
		custom_book[2] = get_inven_xtra(Ind, inven_slot, 2) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 3) ~= 0 then
		custom_book[3] = get_inven_xtra(Ind, inven_slot, 3) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 4) ~= 0 then
		custom_book[4] = get_inven_xtra(Ind, inven_slot, 4) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 5) ~= 0 then
		custom_book[5] = get_inven_xtra(Ind, inven_slot, 5) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 6) ~= 0 then
		custom_book[6] = get_inven_xtra(Ind, inven_slot, 6) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 7) ~= 0 then
		custom_book[7] = get_inven_xtra(Ind, inven_slot, 7) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 8) ~= 0 then
		custom_book[8] = get_inven_xtra(Ind, inven_slot, 8) - 1
	end
	if get_inven_xtra(Ind, inven_slot, 9) ~= 0 then
		custom_book[9] = get_inven_xtra(Ind, inven_slot, 9) - 1
	end

	__cur_inven_slot = inven_slot

	-- Parse all spells
	for index, s in custom_book do
		local color = TERM_L_DARK
		local lvl = get_level(i, s, 50, -50)
		local xx, sch_str

		if is_ok_spell(i, s) then
			if get_mana(i, s, inven_slot) > get_power(i, s) then color = TERM_ORANGE
			else color = TERM_L_GREEN end
		end

		xx = nil
		sch_str = ""

		for index, sch in __spell_school[s] do
			if xx then
				sch_str = sch_str.."/"..school(sch).name
			else
				xx = 1
				sch_str = sch_str..school(sch).name
			end
		end
		sch_str = sch_str_lim(sch_str)

		c_prt(color, format("%c) %-22s%-16s %3d %4s %3d%s %s", size + strbyte("a"), spell(s).name, sch_str, lvl, get_mana(i, s, inven_slot), spell_chance(i, s, inven_slot), "%", __spell_info[s]()), y, x)
		y = y + 1
		size = size + 1
	end

	__cur_inven_slot = -1

	prt(format("   %-22s%-14s Level Cost Fail Info", "Name", "School"), 1, x)
	return y
end

-- XXX client only
function print_spell_desc(s, y)
	local index, desc, x

	x = 0

	if type(__spell_desc[s]) == "string" then c_prt(TERM_L_BLUE, __spell_desc[s], y, x)
	else
		for index, desc in __spell_desc[s] do
			c_prt(TERM_L_BLUE, desc, y, x)
			y = y + 1
		end
	end
	if check_affect(s, "piety", FALSE) then
		c_prt(TERM_L_WHITE, "It uses piety to cast.", y, x)
		y = y + 1
	end
	if not check_affect(s, "blind") then
		c_prt(TERM_ORANGE, "It is castable even while blinded.", y, x)
		y = y + 1
	end
	if not check_affect(s, "confusion") then
		c_prt(TERM_ORANGE, "It is castable even while confused.", y, x)
		y = y + 1
	end

	return y
end


-- paste-spells-to-chat special functions (client side only) --
-- Note: inven_slot is just needed for the wield-quirk boni calculations
function print_spell_chat(s, inven_slot)
	local index, desc
	local i = 0

	local lvl = get_level(i, s, 50, -50)
	local xx = nil
	local sch_str = ""

	for index, sch in __spell_school[s] do
		if xx then
			sch_str = sch_str.."/"..school(sch).name
		else
			xx = 1
			sch_str = sch_str..school(sch).name
		end
	end
	sch_str = sch_str_lim(sch_str)

	--format("   %-22s%-14s Level Cost Fail Info", "Name", "School")
	--c_msg_print(format("\255G%-22s%-16s %3d %4s %3d%s %s", spell(s).name, sch_str, lvl, get_mana(i, s, inven_slot), spell_chance(i, s, inven_slot), "%", __spell_info[s]()))
	Send_msg(format("\255G%-22s%-16s %3d %4s %3d%s %s", spell(s).name, sch_str, lvl, get_mana(i, s, inven_slot), spell_chance(i, s, inven_slot), "%", __spell_info[s]()))
end
function print_spell_desc_chat(s)
	local index, desc
	local msg = ""

	if type(__spell_desc[s]) == "string" then Send_msg("\255W"..__spell_desc[s])
	else
		for index, desc in __spell_desc[s] do
			msg = msg..desc.." "
		end
		Send_msg("\255W"..msg)
	end
	if check_affect(s, "piety", FALSE) then
		Send_msg("\255wIt uses piety to cast.")
	end
	if not check_affect(s, "blind") then
		Send_msg("\255oIt is castable even while blinded.")
	end
	if not check_affect(s, "confusion") then
		Send_msg("\255oIt is castable even while confused.")
	end
end


function book_spells_num2(inven_slot, sval)
	local size, index, sch, book

	size = 0

--[[STOREBROWSE
	if inven_slot ~= -1 then
		book = get_inven_sval(Ind, inven_slot)
	else
		book = sval
	end
]]
	if inven_slot >= 0 then
		book = get_inven_sval(Ind, inven_slot)
	else
		book = sval
	end

	-- Hack: if the book sval is 255 it is a spell scroll, which always has exactly 1 spell
	if book == 255 then
		return 1
	end

	-- Hacks for custom tomes
	if book == 100 or book == 101 or book == 102 then
--[[STOREBROWSE
--HACK/problem: can't browse custom tomes in shops!:
		if inven_slot == -1 then return 0 end
]]
		if get_inven_xtra(Ind, inven_slot, 9) ~= 0 then
			return 9
		end
		if get_inven_xtra(Ind, inven_slot, 8) ~= 0 then
			return 8
		end
		if get_inven_xtra(Ind, inven_slot, 7) ~= 0 then
			return 7
		end
		if get_inven_xtra(Ind, inven_slot, 6) ~= 0 then
			return 6
		end
		if get_inven_xtra(Ind, inven_slot, 5) ~= 0 then
			return 5
		end
		if get_inven_xtra(Ind, inven_slot, 4) ~= 0 then
			return 4
		end
		if get_inven_xtra(Ind, inven_slot, 3) ~= 0 then
			return 3
		end
		if get_inven_xtra(Ind, inven_slot, 2) ~= 0 then
			return 2
		end
		if get_inven_xtra(Ind, inven_slot, 1) ~= 0 then
			return 1
		end
		--no spells in this custom tome yet
		return 0
	end

	-- Parse all spells
	for index, s in school_book[book] do
		size = size + 1
	end

	return size
end

function spell_x2(inven_slot, sval, spl, s)
	local book

--[[STOREBROWSE
	if inven_slot ~= -1 then
		book = get_inven_sval(Ind, inven_slot)
	else
		book = sval
	end
]]
	if inven_slot >= 0 then
		book = get_inven_sval(Ind, inven_slot)
	else
		book = sval
	end

	if book == 255 then
		return spl
	elseif book == 100 or book == 101 or book == 102 then
--[[STOREBROWSE
--HACK/problem: can't browse custom tomes in shops!:
		if inven_slot == -1 then return 0 end
]]
		if s == 0 then return get_inven_xtra(Ind, inven_slot, 1) - 1 end
		if s == 1 then return get_inven_xtra(Ind, inven_slot, 2) - 1 end
		if s == 2 then return get_inven_xtra(Ind, inven_slot, 3) - 1 end
		if s == 3 then return get_inven_xtra(Ind, inven_slot, 4) - 1 end
		if s == 4 then return get_inven_xtra(Ind, inven_slot, 5) - 1 end
		if s == 5 then return get_inven_xtra(Ind, inven_slot, 6) - 1 end
		if s == 6 then return get_inven_xtra(Ind, inven_slot, 7) - 1 end
		if s == 7 then return get_inven_xtra(Ind, inven_slot, 8) - 1 end
		if s == 8 then return get_inven_xtra(Ind, inven_slot, 9) - 1 end
		--failure?
		return 0
	else
		local i, x, val

		i, val = next(school_book[book], nil)
		x = 0
		while x < s do
			i, val = next(school_book[book], i)
			x = x + 1
		end
		return val
	end
end

function spell_in_book2(inven_slot, book, spell)
	local i, s

	--treat exception
	if book == 255 then
		return FALSE
	end

	--hack of custom tomes
	if book == 100 or book == 101 or book == 102 then
		return spell_in_custom_tome(inven_slot, spell)
	end

	for i, s in school_book[book] do
		if s == spell then return TRUE end
	end
	return FALSE
end

function spell_in_custom_tome(inven_slot, spell)
	local i

	for i = 1, 9 do
		if get_inven_xtra(Ind, inven_slot, i) - 1 == spell then
			return TRUE
		end
	end

	return FALSE
end

-- Returns spell chance of failure for spell
function spell_chance(i, s, inven_slot)
	local chance, s_ptr
	local ply, ls

	s_ptr = spell(s)

	--hack: LIMIT_SPELLS uses top-level failrate here instead of worse one we had at lower level!
	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
		ls = ply.limit_spells
		ply.limit_spells = 0
	--client version recent enough to even know 'hack_force_spell_level'? (otherwise we'd get a lua error)
	elseif (def_hack("hack_force_spell_level", nil)) then
		ls = hack_force_spell_level
		hack_force_spell_level = 0
	end

	local __prev_inven_slot = __cur_inven_slot
	__cur_inven_slot = -1
	local lev = get_level(i, s, 50)
	__cur_inven_slot = inven_slot

	-- Hack: "101" means 100% chance to succeed ('fail' is unsigned byte, so it'll be 157) - C. Blue
	if (s_ptr.fail == 101) then
		chance = 0
	-- A new hack: "102" means greatly reduced fail chance (from 0 base fail chance) - C. Blue
	elseif (s_ptr.fail == 102) then
		chance = (lua_spell_chance(i, 0, lev, s_ptr.skill_level, get_mana(i, s, inven_slot), get_power(i, s), get_spell_stat(s)) + 5) / 6
	else
		-- Extract the base spell failure rate
		chance = lua_spell_chance(i, s_ptr.fail, lev, s_ptr.skill_level, get_mana(i, s, inven_slot), get_power(i, s), get_spell_stat(s))
	end

	__cur_inven_slot = __prev_inven_slot

	--unhack: LIMIT_SPELLS
	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
		ply.limit_spells = ls
	--client version recent enough to even know 'hack_force_spell_level'? (otherwise we'd get a lua error)
	elseif (def_hack("hack_force_spell_level", nil)) then
		hack_force_spell_level = ls
	end

	--for WIELD_BOOKS:
	if (inven_slot == INVEN_WIELD) then
		chance = chance - 5 -- note: we allow going below the usual minimum chance (derived from adj_mag_fail) here
		if (chance < 0) then chance = 0 end
	end

	-- Return the chance
	return chance
end

function check_affect(s, name, default)
	local s_ptr = __tmp_spells[s]
	local a

	if type(s_ptr[name]) == "number" then
		a = s_ptr[name]
	else
		a = default
	end

	if a == FALSE then
		return nil
	else
		return TRUE
	end
end

-- Returns the stat to use for the spell, INT by default
function get_spell_stat(s)
	if not __tmp_spells[s].stat then return A_INT
	else return __tmp_spells[s].stat end
end

-- XXX server only
-- one question.. why this should be LUA anyway?
-- because accessing lua table is so badly easier in lua
function cast_school_spell(i, s, s_ptr, no_cost, other)
	local ply

	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
	else
		ply = player
	end

	if not is_ok_spell(i, s) then
		local energy = level_speed(ply.wpos);
		ply.energy = ply.energy - energy
		msg_print(i, "\255oYou cannot cast this spell!")
		return 0
	end

	local use = FALSE

	-- No magic
	if check_antimagic(i, get_spell_am(s)) == TRUE then
--Next line is already in the server sources.
--		msg_print(i, "Your anti-magic field disrupts any magic attempts.")
		local energy = level_speed(ply.wpos);
		ply.energy = ply.energy - energy
		return 1 --continue ftk
	end

	-- TODO: check the ownership
	-- uh?

	-- if it costs something then some condition must be met
	if not no_cost then
		-- Require lite
		if (check_affect(s, "blind")) and ((ply.blind > 0) or (no_lite(i) == TRUE)) then
			local energy = level_speed(ply.wpos);
			ply.energy = ply.energy - energy
			msg_print(i, "\255oYou cannot see!")
			return 0
		end

		-- Not when confused
		if (check_affect(s, "confusion")) and (ply.confused > 0) then
			local energy = level_speed(ply.wpos);
			ply.energy = ply.energy - energy
			msg_print(i, "\255oYou are too confused!")
			return 0
		end

		-- Level requirements met?
		if (get_level(i, s, 50, -50) < 1) then
			msg_print(i, "\255sYour skill is not high enough!")
			lua_intrusion(i, "bad spell level")
			return 0
		end

		-- Enough mana
		if (get_mana(i, s, other.book) > get_power(i, s)) then
			local energy = level_speed(ply.wpos);
			--withdraw a little bit of energy just to prevent command-spam
			ply.energy = ply.energy - energy / 3
			--if (get_check2("You do not have enough "..get_power_name(s)..", do you want to try anyway?", FALSE) == FALSE) then return end
			msg_print(i, "\255oYou do not have enough mana to cast "..spell(s).name..".")
			__cur_inven_slot = -1
			return 0
		end

--[[		-- Sanity check for direction
		if (need_direction(s) && other.dir == -1) then
			msg_print(i, "Spell needs a direction.")
			return
		end
]]

		-- Invoke the spell effect
		if (magik(spell_chance(i, s, other.book)) == FALSE) then
			local mp_cost = get_mana(i, s, -1) --actually DON'T apply WIELD_BOOKS bonus here yet, we do it below, for fractional chance..
			--allow fractions of MP, conferred as +1 being applied at appropriate chance. Never go below 1 MP cost though.
			if (other.book == INVEN_WIELD and mp_cost > 1) then
				local mp_costr = mod(mp_cost, 5)
				mp_cost = (mp_cost * 80) / 100 --LUA will round down (int)
				if (mp_costr ~= 0 and randint(5) > mp_costr) then mp_cost = mp_cost + 1 end
			end

			msg_print(i, "You successfully cast the spell "..spell(s).name..".")

			-- Reduce mana BEFORE casting the spell, for Necromancy to work effectively:
			-- If the monster dies, MP should not get refunded before the spell cost was actually deducted.
			adjust_power(i, s, -mp_cost)
			use = TRUE

			__cur_inven_slot = other.book
			if (__spell_spell[s](other) ~= nil) then
				--correct the situation - we have to do it this way round,
				--so we were able to deduct MP before actually casting the spell above
				use = FALSE
				--and refund the mana
				adjust_power(i, s, mp_cost)
			end
			__cur_inven_slot = -1
		else
			local index, sch

--[[
			-- added because this is *extremely* important --pelpel
			if (flush_failure) then flush() end
]]

			msg_print(i, "\255yYou failed to get the spell "..spell(s).name.." off!")

			-- Reduce mana
			adjust_power(i, s, -get_mana(i, s, other.book))

			for index, sch in __spell_school[s] do
				if __schools[sch].fail then
					__schools[sch].fail(spell_chance(i, s, other.book))
				end
			end
			use  = TRUE
		end
	else
		__spell_spell[s](other)
	end
	__cur_inven_slot = -1

	if use == TRUE then
		-- Take a turn
		local energy = level_speed(ply.wpos);
		ply.energy = ply.energy - energy
	end

	ply.redraw = bor(ply.redraw, PR_MANA)
	--ply.window = bor(ply.window, PW_PLAYER)
	--ply.window = bor(ply.window, PW_SPELL)

	return 1
end


function test_school_spell(i, s, inven_slot)
	local ply

	-- client-side (0) or server-side (>=1) ?
	if i ~= 0 then
		ply = players(i)
	else
		ply = player
	end

	local use = FALSE

	-- Require lite
	if (check_affect(s, "blind")) and ((ply.blind > 0) or (no_lite(i) == TRUE)) then
		return 1
	end

	-- Not when confused
	if (check_affect(s, "confusion")) and (ply.confused > 0) then
		return 2
	end

	-- Enough mana
	if (get_mana(i, s, inven_slot) > get_power(i, s)) then
		return 3
	end

	-- Level requirements met?
	if (get_level(i, s, 50, -50) < 1) then
		return 4
	end

	return 0
end

--WARNING: Don't call this via exec_lua(0,..) from within a function that uses 'player' LUA variable!
--There is also a safe C version of this function in object1.c.
function get_spellbook_name_colour(i)
	local s

	s = __spell_school[i][1]
	-- green for priests
	if (s >= SCHOOL_HOFFENSE and s <= SCHOOL_HSUPPORT) then return TERM_GREEN end
	-- light green for druids
	if (s == SCHOOL_DRUID_ARCANE or s == SCHOOL_DRUID_PHYSICAL) then return TERM_L_GREEN end
	-- orange for astral tome
	if (s == SCHOOL_ASTRAL) then return TERM_ORANGE end
	-- yellow for mindcrafters
	if (s >= SCHOOL_PPOWER and s <= SCHOOL_MINTRUSION) then return TERM_YELLOW end
	-- blue for occult
	if (s >= SCHOOL_OSHADOW and s <= SCHOOL_OUNLIFE) then return TERM_BLUE end
	-- light blue for the rest (istari schools)
	return TERM_L_BLUE
end


-- Helper functions
HAVE_ARTIFACT = 0
HAVE_OBJECT = 1
HAVE_EGO = 2
function have_object(mode, type, find, find2)
	local o, i, min, max

	max = 0
	min = 0
	if band(mode, USE_EQUIP) == USE_EQUIP then
		min = INVEN_WIELD
		max = INVEN_TOTAL
	end
	if band(mode, USE_INVEN) == USE_INVEN then
		min = 0
		if max == 0 then max = INVEN_WIELD end
	end

	i = min
	while i < max do
		o = get_object(i)
		if o.k_idx ~= 0 then
			if type == HAVE_ARTIFACT then
				if find == o.name1 then return i end
			end
			if type == HAVE_OBJECT then
				if find2 == nil then
					if find == o.k_idx then return i end
				else
					if (find == o.tval) and (find2 == o.sval) then return i end
				end
			end
			if type == HAVE_EGO then
				if (find == o.name2) or (find == o.name2b) then return i end
			end
		end
		i = i + 1
	end
	return -1
end

function pre_exec_spell_dir(s)
	if __tmp_spells[s].direction then
		if type(__tmp_spells[s].direction) == "function" then
			return __tmp_spells[s].direction()
		else
			return __tmp_spells[s].direction
		end
	end
end

function pre_exec_spell_extra(s)
	if __tmp_spells[s].extra then
		__pre_exec_extra = __tmp_spells[s].extra()
		return TRUE
	end
end

function pre_exec_spell_item(s)
	if __tmp_spells[s].get_item then
		if type(__tmp_spells[s].get_item.get) == "function" then
			__get_item_hook_default = __tmp_spells[s].get_item.get
			lua_set_item_tester(0, "__get_item_hook_default")
		else
			lua_set_item_tester(lua__tmp_spells[s].get_item.get, "")
		end
		if not __tmp_spells[s].get_item.inven then __tmp_spells[s].get_item.inven = FALSE end
		if not __tmp_spells[s].get_item.equip then __tmp_spells[s].get_item.equip = FALSE end
		if not __tmp_spells[s].get_item.floor then __tmp_spells[s].get_item.floor = FALSE end

		local ret

		ret, __pre_exec_item = get_item_aux(0, __tmp_spells[s].get_item.prompt, __tmp_spells[s].get_item.equip, __tmp_spells[s].get_item.inven, __tmp_spells[s].get_item.floor)
		return ret
	end
end

-- Returns the percentage of AM (Antimagic field) efficiency vs this spell
function get_spell_am(s)
	if not __tmp_spells[s].am then return 100
	else return __tmp_spells[s].am end
end

-- Returns if a spell is an attack spell and therefore to be repeated in FTK mode - C. Blue
-- note: 0 means no FTK, 1 means 'do FTK', 2 means 'do FTK && no need for monster-free LOS'
function get_spell_ftk(s)
	if not __tmp_spells[s].ftk then return 0
	else return __tmp_spells[s].ftk end
end

-- Returns if a spell is used on players instead of monsters (friendly spell)
function get_spell_friendly(s)
	if not __tmp_spells[s].friendly then return 0
	else return __tmp_spells[s].friendly end
end

-- Reduce length of multi-school spells' "School" entry string
function sch_str_lim(s)
	local i

	-- nothing to do?
	if strlen(s) <= 16 then
		return s
	end

	-- shorten common words
	while 1 do
		i = strfind(s, "Holy ")
		if i ~= nil then
			s = strsub(s, 1, i).."."..strsub(s, i + 5)
		else
			break
		end
	end

	while 1 do
		i = strfind(s, "Lore")
		if i ~= nil then
			s = strsub(s, 1, i).."."..strsub(s, i + 4)
		else
			break
		end
	end

	while 1 do
		i = strfind(s, "Thought ")
		if i ~= nil then
			s = strsub(s, 1, i).."."..strsub(s, i + 8)
		else
			break
		end
	end
	while 1 do
		i = strfind(s, "Mental ")
		if i ~= nil then
			s = strsub(s, 1, i).."."..strsub(s, i + 7)
		else
			break
		end
	end
	while 1 do
		i = strfind(s, "Psycho-")
		if i ~= nil then
			s = strsub(s, 1, i).."."..strsub(s, i + 7)
		else
			break
		end
	end

	-- limit length if still not enough
	s = strsub(s, 1, 16)

	return s
end


-- ===================================================================
-- for new spell system rework, with I,II,III,IV,V discrete stages etc

-- Return the name of the spell at position #s_ind
-- in the spell book in inventory slot inven_slot - C. Blue
function get_spellname_in_book(inven_slot, s_ind)
	local s, book, i

	s = get_inven_pval(Ind, inven_slot)
	book = get_inven_sval(Ind, inven_slot)

	if book == 255 then
		-- spell scrolls only have 1 spell (index 1)
		if s_ind == 1 then
			return spell(s).name
		else
			return ""
		end
	end

	-- custom tomes
	if book == 100 or book == 101 or book == 102 then
		if s_ind > 9 then
			return ""
		end

		s = get_inven_xtra(Ind, inven_slot, s_ind) - 1
		if s ~= -1 then
			return spell(s).name
		end
		return ""
	end

	-- static books aka handbooks and tomes
	for i, s in school_book[book] do
		if i == s_ind then
			return spell(s).name
		end
	end
	return ""
end
