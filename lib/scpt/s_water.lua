-- handle the water school

function get_frostbolt_dam()
	return 4 + get_level(Ind, FROSTBOLT, 25), 6 + get_level(Ind, FROSTBOLT, 25) + 0
end
FROSTBOLT = add_spell {
	["name"] = 	"Frost Bolt",
	["school"] = 	SCHOOL_WATER,
	["level"] = 	8,
	["mana"] = 	2,
	["mana_max"] = 	11,
	["fail"] = 	-10,
	["direction"] = TRUE,
	["ftk"] = 	1,
	["spell"] = 	function(args)
			fire_bolt(Ind, GF_COLD, args.dir, damroll(get_frostbolt_dam()), " casts a frost bolt for")
		end,
	["info"] = 	function()
			local x, y

			x, y = get_frostbolt_dam()
			return "dam "..x.."d"..y
		end,
	["desc"] = 	{ "Conjures up icy moisture into a powerful frost bolt", }
}

function get_waterbolt_dam()
	return 4 + get_level(Ind, WATERBOLT, 25), 6 + get_level(Ind, WATERBOLT, 25) + 0
end
WATERBOLT = add_spell {
	["name"] = 	"Water Bolt",
	["school"] = 	SCHOOL_WATER,
	["level"] = 	14,
	["mana"] = 	3,
	["mana_max"] = 	17,
	["fail"] = 	-10,
	["direction"] = TRUE,
	["ftk"] = 	1,
	["spell"] = 	function(args)
			fire_bolt(Ind, GF_WATER, args.dir, damroll(get_waterbolt_dam()), " casts a water bolt for")
		end,
	["info"] = 	function()
			local x, y

			x, y = get_waterbolt_dam()
			return "dam "..x.."d"..y
		end,
	["desc"] = 	{ "Conjures up water into a powerful bolt", }
}

TIDALWAVE = add_spell {
	["name"] = 	"Tidal Wave",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	16,
	["mana"] = 	16,
	["mana_max"] = 	40,
	["fail"] = 	20,
	["spell"] = 	function()
			fire_wave(Ind, GF_WAVE, 0, 40 + get_level(Ind, TIDALWAVE, 200), 1, 6 + get_level(Ind, TIDALWAVE, 6), 5, EFF_WAVE, " casts a tidal wave for")
	end,
	["info"] = 	function()
			return "dam "..(40 + get_level(Ind, TIDALWAVE,  200)).." rad "..(6 + get_level(Ind, TIDALWAVE, 6))
	end,
	["desc"] = 	{
			"Summons a monstruous tidal wave that will expand and crush the",
			"monsters under it's mighty waves"
	}
}

ICESTORM = add_spell {
	["name"] = 	"Frost Barrier",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	22,
	["mana"] = 	30,
	["mana_max"] = 	60,
	["fail"] = 	20,
	["spell"] = 	function()
			local type

			if get_level(Ind, ICESTORM, 50) >= 15 then type = GF_ICE
			else type = GF_COLD end
			fire_wave(Ind, type, 0, 80 + get_level(Ind, ICESTORM, 200), 1, 20 + get_level(Ind, ICESTORM, 47), 5, EFF_STORM, " summons an icy  for")
	end,
	["info"] = 	function()
			return "dam "..(80 + get_level(Ind, ICESTORM, 200)).." rad 1 dur "..(20 + get_level(Ind, ICESTORM, 47))
	end,
	["desc"] = 	{
			"Engulfs you in a whirl of roaring cold that strikes all foes at close range",
			"At level 15 it turns into shards of ice"
	}
}

ENTPOTION = add_spell {
	["name"] = 	"Ent's Potion",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	6,
	["mana"] = 	7,
	["mana_max"] = 	15,
	["fail"] = 	20,
	["spell"] = 	function()
				--if player.suscep_life == false then
				if player.prace ~= RACE_VAMPIRE then
					set_food(Ind, PY_FOOD_MAX - 1)
					msg_print(Ind, "The Ent's Potion fills your stomach.")
				end
				if player.spell_project > 0 then
					fire_ball(Ind, GF_SATHUNGER_PLAYER, 0, 1, player.spell_project, "")
				end
			if get_level(Ind, ENTPOTION, 50) >= 5 then
				set_afraid(Ind, 0)
				if player.spell_project > 0 then
					fire_ball(Ind, GF_REMFEAR_PLAYER, 0, 1, player.spell_project, "")
				end
			end
			if get_level(Ind, ENTPOTION, 50) >= 12 then
				set_hero(Ind, randint(25) + 25 + get_level(Ind, ENTPOTION, 40))
				if player.spell_project > 0 then
					fire_ball(Ind, GF_HERO_PLAYER, 0, randint(25) + 25 + get_level(Ind, ENTPOTION, 40), player.spell_project, "")
				end
			end
	end,
	["info"] = 	function()
			if get_level(Ind, ENTPOTION, 50) >= 12 then
				return "dur "..(25 + get_level(Ind, ENTPOTION, 40)).."+d25"
			else
				return ""
			end
	end,
	["desc"] = 	{
			"Fills up your stomach",
			"At level 5 it boldens your heart",
			"At level 12 it make you heroic",
			"***Affected by the Meta spell: Project Spell***",
	}
}

VAPOR = add_spell {
	["name"] = 	"Vapor",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	2,
	["mana"] = 	2,
	["mana_max"] = 	12,
	["fail"] = 	20,
	["spell"] = 	function()
			fire_cloud(Ind, GF_VAPOUR, 0, 3 + get_level(Ind, VAPOR, 60), 3 + get_level(Ind, VAPOR, 4, 0), 5, 8, " fires a cloud of vapor for")
	end,
	["info"] = 	function()
			return "dam "..(3 + get_level(Ind, VAPOR, 60)).." rad "..(3 + get_level(Ind, VAPOR, 4, 0)).." dur 5"
	end,
	["desc"] = 	{
			"Fills the air with toxic moisture to eradicate annoying critters"
	}
}
