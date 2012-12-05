-- handle the nature school

function get_healing_power()
	local pow
	pow = player.mhp * (25 + get_level(Ind, HEALING, 31)) / 100
	if pow > 400 then
		pow = 400
	end
	return pow
end

GROWTREE = add_spell
{
	["name"] = 	"Grow Trees",
--      ["school"] = 	{SCHOOL_NATURE, SCHOOL_TEMPORAL},
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
        ["desc"] =	{
        		"Makes trees grow extremely quickly around you",
        }
}

HEALING = add_spell
{
	["name"] = 	"Healing",
        ["school"] = 	{SCHOOL_NATURE},
        ["level"] = 	10,
        ["mana"] = 	15,
        ["mana_max"] = 	180,
        ["fail"] = 	30,
        ["spell"] = 	function()
                        if player.spell_project > 0 then
                                fire_ball(Ind, GF_HEAL_PLAYER, 0, get_healing_power(), player.spell_project, "")
                        end
	end,
	["info"] = 	function()
			return "heal "..(15 + get_level(Ind, HEALING, 43)).."%="..get_healing_power().."/"..(get_healing_power() / 2).." hp"
	end,
        ["desc"] =	{
        		"Heals a percent of hitpoints up to a maximum of 400 points healed",
			"Projecting it will heal up to half that amount on nearby players",
                        "***Affected by the Meta spell: Project Spell***",
        }
}

RECOVERY = add_spell
{
	["name"] = 	"Recovery",
        ["school"] = 	{SCHOOL_NATURE},
        ["level"] = 	15,
        ["mana"] = 	10,
        ["mana_max"] = 	25,
        ["fail"] = 	20,
        ["spell"] = 	function()
        		set_poisoned(Ind, player.poisoned / 2, player.poisoned_attacker)
                        if get_level(Ind, RECOVERY, 50) >= 10 then
                        	set_poisoned(Ind, 0, 0)
                                set_cut(Ind, 0, 0)
	                        if player.spell_project > 0 then
        	                        fire_ball(Ind, GF_CURECUT_PLAYER, 0, 1, player.spell_project, "")
        	                        fire_ball(Ind, GF_CUREPOISON_PLAYER, 0, 1, player.spell_project, "")
                	        end
                        end
                        if get_level(Ind, RECOVERY, 50) >= 20 then
				do_res_stat(Ind, A_STR)
				do_res_stat(Ind, A_CON)
				do_res_stat(Ind, A_DEX)
				do_res_stat(Ind, A_WIS)
				do_res_stat(Ind, A_INT)
				do_res_stat(Ind, A_CHR)
                        end
                        if get_level(Ind, RECOVERY, 50) >= 25 then
                        	restore_level(Ind)
                        end
	end,
	["info"] = 	function()
			return ""
	end,
        ["desc"] =	{
        		"Reduces the length of time that you are poisoned",
                        "At level 10 it cures poison and cuts (projectable)",
                        "At level 20 it restores drained stats",
                        "At level 25 it restores lost experience",
                        "***Affected by the Meta spell: Project Spell***",
        }
}

REGENERATION = add_spell
{
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
        ["desc"] =	{
        		"Increases your body's regeneration rate",
        }
}

VERMINCONTROL = add_spell
{
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
        ["desc"] =	{
        		"Prevents any vermin from breeding.",
        }
}

--[[
SUMMONANIMAL = add_spell
{
        ["name"] =      "Summon Animal",
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
        ["desc"] =	{
        		"Summons a leveled animal to your aid",
        }
}
]]
