-- handle the nature school

function get_healing_percents(limit_lev)
	local perc
	perc = get_level(Ind, HEALING, 31)
	if limit_lev ~= 0 then
		if perc > (limit_lev * 3) / 5 then
			perc = (limit_lev * 3) / 5 + (perc - (limit_lev * 3) / 5) / 3
		end
	end
	return 25 + perc
end
function get_healing_cap(limit_lev)
	local pow
	pow = get_level(Ind, HEALING, 417)
	if limit_lev ~= 0 then
		if pow > limit_lev * 8 then
			pow = limit_lev * 8 + (pow - limit_lev * 8) / 3
		end
	end
	if pow > 400 then
		pow = 400
	end
	return pow
end
function get_healing_power(limit_lev)
	local pow, cap
	pow = player.mhp * get_healing_percents(limit_lev) / 100
	cap = get_healing_cap(limit_lev)
	if pow > cap then
		pow = cap
	end
	return pow
end

GROWTREE = add_spell {
	["name"] = 	"Grow Trees",
--	["school"] = 	{SCHOOL_NATURE, SCHOOL_TEMPORAL},
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	6,
	["mana"] = 	6,
	["mana_max"] = 	30,
	["fail"] = 	20,
	["spell"] = 	function()
			grow_trees(Ind, 1 + get_level(Ind, GROWTREE, 5))
	end,
	["info"] = 	function()
			return "rad "..(1 + get_level(Ind, GROWTREE, 5))
	end,
	["desc"] = 	{
			"Makes trees grow extremely quickly around you",
	}
}

HEALING = add_spell {
	["name"] = 	"Healing",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	10,
	["mana"] = 	15,
	["mana_max"] = 	120,
	["fail"] = 	30,
	["spell"] = 	function()
			local status_ailments
			status_ailments = 0
			--hacks to cure effects same as potions would
			if get_level(Ind, HEALING, 50) >= 24 then
				status_ailments = status_ailments + 8192 + 4096 + 2048
			elseif get_level(Ind, HEALING, 50) >= 10 then
				status_ailments = status_ailments + 4096 + 2048
			elseif get_level(Ind, HEALING, 50) >= 4 then
				status_ailments = status_ailments + 2048
			end

			if player.spell_project > 0 then
				fire_ball(Ind, GF_HEAL_PLAYER, 0, status_ailments + get_healing_power(0), player.spell_project, "")
			else
				fire_ball(Ind, GF_HEAL_PLAYER, 0, status_ailments + get_healing_power(0), 0, "")
--				hp_player(Ind, get_healing_power(0)) <- doesn't give a neat msg with numbers
	                end
	end,
	["info"] = 	function()
			return "heal "..get_healing_percents(0).."% (max "..get_healing_cap(0)..") = "..get_healing_power(0)
	end,
	["desc"] = 	{
			"Heals a percent of hitpoints up to a maximum of 400 points healed",
			"Projecting it will heal up to half that amount on nearby players",
			"Also cures blindness and cuts at level 4",
			"Also cures confusion at level 10",
			"Also cures stun at level 24",
			"***Affected by the Meta spell: Project Spell***",
	}
}

RECOVERY = add_spell	{
	["name"] = 	"Recovery",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	15,
	["mana"] = 	10,
	["mana_max"] = 	25,
	["fail"] = 	20,
	["spell"] = 	function()
			if get_level(Ind, RECOVERY, 50) >= 25 then
				set_poisoned(Ind, 0, 0)
				set_cut(Ind, 0, 0)
				set_confused(Ind, 0)
				set_blind(Ind, 0)
				set_stun(Ind, 0)
				do_res_stat(Ind, A_STR)
				do_res_stat(Ind, A_CON)
				do_res_stat(Ind, A_DEX)
				do_res_stat(Ind, A_WIS)
				do_res_stat(Ind, A_INT)
				do_res_stat(Ind, A_CHR)
				restore_level(Ind)
				if player.spell_project > 0 then
					fire_ball(Ind, GF_CURE_PLAYER, 0, 4 + 8 + 10 + 40 + 80, player.spell_project, "")
				end
			elseif get_level(Ind, RECOVERY, 50) >= 20 then
				set_poisoned(Ind, 0, 0)
				set_cut(Ind, 0, 0)
				set_confused(Ind, 0)
				set_blind(Ind, 0)
				set_stun(Ind, 0)
				do_res_stat(Ind, A_STR)
				do_res_stat(Ind, A_CON)
				do_res_stat(Ind, A_DEX)
				do_res_stat(Ind, A_WIS)
				do_res_stat(Ind, A_INT)
				do_res_stat(Ind, A_CHR)
				if player.spell_project > 0 then
					fire_ball(Ind, GF_CURE_PLAYER, 0, 4 + 8 + 10 + 40, player.spell_project, "")
				end
			elseif get_level(Ind, RECOVERY, 50) >= 15 then
				set_poisoned(Ind, 0, 0)
				set_cut(Ind, 0, 0)
				set_confused(Ind, 0)
				set_blind(Ind, 0)
				set_stun(Ind, 0)
				if player.spell_project > 0 then
					fire_ball(Ind, GF_CURE_PLAYER, 0, 4 + 8 + 10, player.spell_project, "")
				end
			elseif get_level(Ind, RECOVERY, 50) >= 5 then
				set_poisoned(Ind, 0, 0)
				if player.spell_project > 0 then
					fire_ball(Ind, GF_CUREPOISON_PLAYER, 0, 1, player.spell_project, "")
				end
			else
				if (player.poisoned ~= 0 and player.slow_poison == 0) then player.slow_poison = 1 end
				if player.spell_project > 0 then
					fire_ball(Ind, GF_SLOWPOISON_PLAYER, 0, 1, player.spell_project, "")
				end
			end
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] = 	{
			"Slows down the effect of poison",
			"At level 5 it neutralizes poison,",
			"At level 15 it heals cuts and cures confusion, blindness and stun",
			"At level 20 it restores drained stats",
			"At level 25 it restores lost experience",
			"***Affected by the Meta spell: Project Spell***",
	}
}

REGENERATION = add_spell {
	["name"] = 	"Regeneration",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	20,
	["mana"] = 	30,
	["mana_max"] = 	55,
	["fail"] = 	20,
	["spell"] = 	function()
			set_tim_regen(Ind, randint(10) + 5 + get_level(Ind, REGENERATION, 50), 300 + get_level(Ind, REGENERATION, 700))
	end,
	["info"] = 	function()
			return "dur "..(5 + get_level(Ind, REGENERATION, 50)).."+d10 power "..(300 + get_level(Ind, REGENERATION, 700))
	end,
	["desc"] = 	{
			"Increases your body's regeneration rate",
	}
}

VERMINCONTROL = add_spell {
	["name"] = 	"Vermin Control",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	15,
	["mana"] = 	30,
	["mana_max"] = 	30,
	["fail"] = 	30,
	["spell"] = 	function()
			do_vermin_control(Ind)
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] = 	{
			"Prevents any vermin from breeding.",
	}
}

--[[
SUMMONANIMAL = add_spell {
	["name"] = 	"Summon Animal",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	25,
	["mana"] = 	25,
	["mana_max"] = 	50,
	["fail"] = 	20,
	["spell"] = 	function()
			summon_specific_level = 25 + get_level(SUMMONANIMAL, 50)
			summon_monster(py, px, dun_level, TRUE, SUMMON_ANIMAL)
	end,
	["info"] = 	function()
			return "level "..(25 + get_level(SUMMONANIMAL, 50))
	end,
	["desc"] = 	{
			"Summons a leveled animal to your aid",
	}
}
]]
