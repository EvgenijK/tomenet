function get_speed()
	local spd
--  22->35skill+10, 19->39skill+10, 17->42skill+10
	spd = get_level(Ind, QUICKFEET, 17)
	if spd > 10 then
		spd = 10
	end
	return spd
end

-- The original 'healing cloud' for priests (which never happened)
-- Basically a nox that heals you. Not targettable; casts the cloud around the caster

HEALINGCLOUD = add_spell {
	["name"] = 	"Forest's Embrace",
	["school"] = 	{SCHOOL_DRUID_PHYSICAL},
	["spell_power"] = 0,
	["level"] = 	18,
	["mana"] = 	25,
	["mana_max"] = 	25,
	["fail"] = 	30,
	["stat"] = 	A_WIS,
	["direction"] = FALSE,
	["spell"] = 	function()
			fire_cloud(Ind, GF_HEALINGCLOUD, 0, (1 + get_level(Ind, HEALINGCLOUD, 25)), (1 + get_level(Ind, HEALINGCLOUD, 2)), (5 + get_level(Ind, HEALINGCLOUD, 5)), 10, " calls the spirits")
			end,
	["info"] = 	function()
			return "heals " .. (get_level(Ind, HEALINGCLOUD, 25) + 1) .. " rad " .. (1 + get_level(Ind,HEALINGCLOUD,2)) .. " dur " .. (5 + get_level(Ind, HEALINGCLOUD, 5))
			end,
	["desc"] = 	{ "Continuously heals you and those around you. (Auto-projecting)", }
}

-- Similar to Temporal's 'Essence of Speed'
-- This one adds {1 .. 10} to player's speed, and affects all in radius 2
QUICKFEET = add_spell {
	["name"] = 	"Quickfeet",
	["school"] = 	{SCHOOL_DRUID_PHYSICAL},
	["spell_power"] = 0,
	["level"] = 	13,
	["mana"] = 	25,
	["mana_max"] = 	25,
	["fail"] = 	40,
	["stat"] = 	A_WIS,
	["direction"] = FALSE,
	["spell"] = 	function()
			set_fast(Ind, 30 + randint(10) + get_level(Ind, QUICKFEET, 30), get_speed())
			fire_ball(Ind, GF_SPEED_PLAYER, 0, get_speed() * 2, 2, "")
			end,
	["info"] = 	function()
			return "dur " .. (30 + get_level(Ind, QUICKFEET, 30)) .. "+d10 speed +" .. get_speed()
			end,
	["desc"] = 	{ "Quicken your steps and those around you. (Auto-projecting)", }
}

-- A curing spell
HERBALTEA = add_spell {
	["name"] = 	"Herbal Tea",
	["school"] = 	{SCHOOL_DRUID_PHYSICAL},
	["spell_power"] = 0,
	["level"] = 	3,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	20,
	["stat"] = 	A_WIS,
	["direction"] = FALSE,
	["spell"] = 	function()
			local lvl
			lvl = get_level(Ind, HERBALTEA, 50)

			msg_print(Ind, "That tasted bitter sweet.")
			set_food(Ind, PY_FOOD_MAX - 1)

			if lvl >= 35 then
				restore_level(Ind)
				do_res_stat(Ind, A_STR)
				do_res_stat(Ind, A_CON)
				do_res_stat(Ind, A_DEX)
				do_res_stat(Ind, A_WIS)
				do_res_stat(Ind, A_INT)
				do_res_stat(Ind, A_CHR)
				if (player.black_breath == TRUE) then
					msg_print(Ind, "The hold of the Black Breath on you is broken!");
					player.black_breath = FALSE
				end
				fire_ball(Ind, GF_RESTORE_PLAYER, 0, 1 + 2 + 4 + 8, 1, " gives you something bitter to drink.")
			elseif lvl >= 25 then
				restore_level(Ind)
				do_res_stat(Ind, A_STR)
				do_res_stat(Ind, A_CON)
				do_res_stat(Ind, A_DEX)
				do_res_stat(Ind, A_WIS)
				do_res_stat(Ind, A_INT)
				do_res_stat(Ind, A_CHR)
				fire_ball(Ind, GF_RESTORE_PLAYER, 0, 1 + 2 + 4, 1, " gives you something bitter to drink.")
			elseif lvl >= 20 then
				restore_level(Ind)
				fire_ball(Ind, GF_RESTORE_PLAYER, 0, 2, 1 + 2, " gives you something bitter to drink.")
			else
				fire_ball(Ind, GF_SATHUNGER_PLAYER, 0, 1, 1, " gives you something bitter to drink.");
			end
			end,
	["info"] = 	function()
			return ""
			end,
	["desc"] = 	{ "It sustains you and those around you. (Auto-projecting)",
			  "At level 20 it brews a drink that restores your life level.",
			  "At level 25 it brews tea that restores your body status.",
			  "At level 35 it brews the strongest tea to cure even the Black Breath.", }
}

-- +stats booster!
EXTRASTATS_I = add_spell {
	["name"] = 	"Extra Growth I",
	["school"] = 	{SCHOOL_DRUID_PHYSICAL},
	["spell_power"] = 0,
	["level"] = 	15,
	["mana"] = 	20,
	["mana_max"] = 	20,
	["fail"] = 	20,
	["stat"] = 	A_WIS,
	["direction"] = FALSE,
	["spell"] = 	function()
			local x
			x = get_level(Ind, EXTRASTATS_I, 50)
			if x > 9 then x = 9 end
			do_xtra_stats(Ind, 20 + get_level(Ind, EXTRASTATS_I, 50), x)
			end,
	["info"] = 	function()
			return "+" .. ((get_level(Ind, EXTRASTATS_I, 50) / 10) + 2) .. " dur " .. (20 + get_level(Ind, EXTRASTATS_I, 50))
			end,
	["desc"] = 	{ "At level 1 increases your strength.",
			  "At level 5 also increases your dexterity.",
			  "Not projectable.", }
}
EXTRASTATS_II = add_spell {
	["name"] = 	"Extra Growth II",
	["school"] = 	{SCHOOL_DRUID_PHYSICAL},
	["spell_power"] = 0,
	["level"] = 	25,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	-30,
	["stat"] = 	A_WIS,
	["direction"] = FALSE,
	["spell"] = 	function()
			do_xtra_stats(Ind, 20 + get_level(Ind, EXTRASTATS_I, 50), get_level(Ind, EXTRASTATS_I, 50))
			end,
	["info"] = 	function()
			return "+" .. ((get_level(Ind, EXTRASTATS_I, 50) / 10) + 2) .. " dur " .. (20 + get_level(Ind, EXTRASTATS_I, 50))
			end,
	["desc"] = 	{ "At level 1 increases strength, dexterity, constitution.",
			  "At level 11 also increases your intelligence.",
			  "Not projectable.", }
}

-- A shot that increases a players SPR (if wearing a shooter)
-- but also decreases his/her speed!
FOCUSSHOT = add_spell {
	["name"] = 	"Focus",
	["school"] = 	{SCHOOL_DRUID_PHYSICAL},
	["spell_power"] = 0,
	["level"] = 	1,
	["mana"] = 	35,
	["mana_max"] = 	35,
	["fail"] = 	30,
	["stat"] = 	A_WIS,
	["direction"] = FALSE,
	["spell"] = 	function()
			do_focus_shot(Ind, get_level(Ind, FOCUSSHOT, 100), get_level(Ind, FOCUSSHOT, 25))
			end,
	["info"] = 	function()
			return "+" .. get_level(Ind, FOCUSSHOT, 25) .. " dur " .. get_level(Ind, FOCUSSHOT, 100)
			end,
	["desc"] = 	{ "Increases your accuracy.", }
}
