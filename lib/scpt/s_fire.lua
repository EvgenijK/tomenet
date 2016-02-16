-- handle the fire school

GLOBELIGHT = add_spell
{
	["name"] = 	"Globe of Light",
	["school"] = 	{SCHOOL_FIRE},
	["level"] = 	2,
	["mana"] = 	2,
	["mana_max"] = 	15,
	["fail"] = 	10,
	["spell"] = 	function()
		if get_level(Ind, GLOBELIGHT, 50) >= 25 then
			msg_print(Ind, "You are surrounded by a globe of light")
			lite_room(Ind, player.wpos, player.py, player.px)
			fire_ball(Ind, GF_LITE, 0, 10 + get_level(Ind, GLOBELIGHT, 100), 5 + get_level(Ind, GLOBELIGHT, 6), " calls a globe of light for")
		elseif get_level(Ind, GLOBELIGHT, 50) >= 10 then
			lite_area(Ind, 10, 4)
		else
			msg_print(Ind, "You are surrounded by a globe of light")
			lite_room(Ind, player.wpos, player.py, player.px)
		end
	end,
	["info"] = 	function()
		if get_level(Ind, GLOBELIGHT, 50) >= 10 then
			return "dam "..(10 + get_level(Ind, GLOBELIGHT, 100)).." rad "..(5 + get_level(Ind, GLOBELIGHT, 6))
		else
			return ""
		end
	end,
	["desc"] = 	{
			"Creates a globe of magical light",
			"At level 10 it damages monsters that are susceptible to light",
			"At level 25 it becomes more powerful and hurts all monsters",
	}
}

function get_firebolt_dam()
        return 5 + get_level(Ind, FIREBOLT, 25), 7 + get_level(Ind, FIREBOLT, 25) + 1
end
FIREBOLT = add_spell
{
        ["name"] =      "Fire Bolt",
        ["school"] =    SCHOOL_FIRE,
        ["level"] =     10,
        ["mana"] =      3,
        ["mana_max"] =  12,
        ["fail"] =      -10,
        ["direction"] = TRUE,
        ["ftk"] = 1,
        ["spell"] =     function(args)
                        fire_bolt(Ind, GF_FIRE, args.dir, damroll(get_firebolt_dam()), " casts a fire bolt for")
        end,
        ["info"] =      function()
                        local x, y

                        x, y = get_firebolt_dam()
                        return "dam "..x.."d"..y
        end,
        ["desc"] =      {
                        "Conjures up fire into a powerful bolt",
                }
}


FIREFLASH = add_spell
{
	["name"] = 	"Fireflash",
        ["school"] = 	{SCHOOL_FIRE},
        ["level"] = 	10,
        ["mana"] = 	5,
        ["mana_max"] = 	30,
        ["fail"] = 	30,
        ["direction"] = TRUE,
        ["ftk"] = 2,
        ["spell"] = 	function(args)
                local type
        	if (get_level(Ind, FIREFLASH, 50) >= 20) then
	        	type = GF_HOLY_FIRE
	    		fire_ball(Ind, type, args.dir, 20 + get_level(Ind, FIREFLASH, 500), 2 + get_level(Ind, FIREFLASH, 3), " casts a ball of holy fire for")
        	else
        		type = GF_FIRE
			fire_ball(Ind, type, args.dir, 20 + get_level(Ind, FIREFLASH, 500), 2 + get_level(Ind, FIREFLASH, 3), " casts a fire ball for")
	        end
	end,
	["info"] = 	function()
		return "dam "..(20 + get_level(Ind, FIREFLASH, 500)).." rad "..(2 + get_level(Ind, FIREFLASH, 3))
	end,
        ["desc"] = 	{
        		"Conjures a ball of fire to burn your foes to ashes",
                        "At level 20 it turns into a ball of holy fire"
        }
}

FIERYAURA = add_spell
{
	["name"] = 	"Fiery Shield",
        ["school"] = 	{SCHOOL_FIRE},
        ["level"] = 	16,
        ["mana"] = 	20,
        ["mana_max"] = 	60,
        ["fail"] = 	50,
        ["spell"] = 	function()
		local type
--        	if (get_level(Ind, FIERYAURA, 50) >= 8) then
--	        	type = SHIELD_GREAT_FIRE
--        	else
        		type = SHIELD_FIRE
--	        end
                set_shield(Ind, randint(20) + 10 + get_level(Ind, FIERYAURA, 70), 10, type, 5 + get_level(Ind, FIERYAURA, 10), 5 + get_level(Ind, FIERYAURA, 7))
	end,
	["info"] = 	function()
		return "dam "..(5 + get_level(Ind, FIERYAURA, 15)).."d"..(5 + get_level(Ind, FIERYAURA, 7)).." dur "..(10 + get_level(Ind, FIERYAURA, 70)).."+d20"
	end,
        ["desc"] = 	{
        		"Creates a shield of fierce flames around you"
--                      ,"At level 8 it turns into a greater kind of flame that can not be resisted"
        }
}

FIREWALL = add_spell
{
	["name"] = 	"Firewall",
        ["school"] = 	{SCHOOL_FIRE},
        ["level"] = 	20,
        ["mana"] = 	25,
        ["mana_max"] = 	100,
        ["fail"] = 	40,
        ["direction"] = TRUE,
        ["spell"] = 	function(args)
                local type
--        	if (get_level(Ind, FIREWALL, 50) >= 6) then
--	        	type = GF_HOLY_FIRE
--        	else
        		type = GF_FIRE
--	        end
--	        fire_wall(Ind, type, args.dir, 20 + get_level(Ind, FIREWALL, 150), 6 + get_level(Ind, FIREWALL, 14), 10, " summons a fire wall for")
--1.5	        fire_wall(Ind, type, args.dir, 20 + get_level(Ind, FIREWALL, 184), 6 + get_level(Ind, FIREWALL, 4), 8, " summons a fire wall for")
	        fire_wall(Ind, type, args.dir, 20 + get_level(Ind, FIREWALL, 252), 6 + get_level(Ind, FIREWALL, 4), 8, " summons a fire wall for")
	end,
	["info"] = 	function()
		return "dam "..(20 + get_level(Ind, FIREWALL, 184)).." dur "..(6 + get_level(Ind, FIREWALL, 4))
	end,
        ["desc"] = 	{
        		"Creates a fiery wall to incinerate monsters stupid enough to attack you"
--                      ,"At level 6 it turns into a ball of holy fire"
        }
}
