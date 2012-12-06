-- The earth school

--[[
STONESKIN = add_spell
{
	["name"] = 	"Stone Skin",
        ["school"] = 	SCHOOL_EARTH,
        ["level"] = 	1,
        ["mana"] = 	1,
        ["mana_max"] = 	100,
        ["fail"] = 	10,
        ["spell"] = 	function()
        		local type
        		if get_level(Ind, STONESKIN, 50) >= 25 then
                        	type = SHIELD_COUNTER
                        else
                        	type = 0
                        end
               		set_shield(Ind, randint(10) + 10 + get_level(Ind, STONESKIN, 100), 5 + get_level(Ind, STONESKIN, 30), type, 2 + get_level(Ind, STONESKIN, 5), 3 + get_level(Ind, STONESKIN, 5))
	end,
	["info"] = 	function()
        		if get_level(Ind, STONESKIN, 50) >= 25 then
                                return "dam "..(2 + get_level(Ind, STONESKIN, 5)).."d"..(3 + get_level(Ind, STONESKIN, 5)).." dur "..(10 + get_level(Ind, STONESKIN, 100)).."+d10 AC "..(5 + get_level(Ind, STONESKIN, 30))
                        else
                                return "dur "..(10 + get_level(Ind, STONESKIN, 100)).."+d10 AC "..(5 + get_level(Ind, STONESKIN, 30))
                        end
	end,
        ["desc"] =	{
        		"Creates a shield of earth around you to protect you",
                        "At level 25 it starts dealing damage to attackers"
		}
}
]]
DIG = add_spell
{
	["name"] = 	"Dig",
        ["school"] = 	SCHOOL_EARTH,
        ["level"] = 	12,
        ["mana"] = 	14,
        ["mana_max"] = 	14,
        ["fail"] = 	10,
        ["direction"] = TRUE,
        ["spell"] = 	function(args)
			wall_to_mud(Ind, args.dir)
	end,
	["info"] = 	function()
        		return ""
	end,
        ["desc"] =	{
        		"Digs a hole in a wall much faster than any shovels",
		}
}

function get_acidbolt_dam()
        return 6 + get_level(Ind, ACIDBOLT, 25), 8 + get_level(Ind, ACIDBOLT, 25) + 1
end
ACIDBOLT = add_spell
{
        ["name"] =      "Acid Bolt",
        ["school"] =    SCHOOL_EARTH,
        ["level"] =     12,
        ["mana"] =      3,
        ["mana_max"] =  13,
        ["fail"] =      -10,
        ["direction"] = TRUE,
        ["ftk"] = 1,
        ["spell"] =     function(args)
                        fire_bolt(Ind, GF_COLD, args.dir, damroll(get_acidbolt_dam()), " casts a acid bolt for")
        end,
        ["info"] =      function()
                        local x, y

                        x, y = get_acidbolt_dam()
                        return "dam "..x.."d"..y
        end,
        ["desc"] =      {
                        "Conjures up corroding acid into a powerful bolt",
                }
}

STONEPRISON = add_spell
{
	["name"] = 	"Stone Prison",
        ["school"] = 	SCHOOL_EARTH,
        ["level"] = 	25,
        ["mana"] = 	40,
        ["mana_max"] = 	70,
        ["fail"] = 	10,
        ["spell"] = 	function()
			local ret, x, y
--        		if get_level(Ind, STONEPRISON, 50) >= 10 then
--                        	ret, x, y = tgt_pt()
--                        else
--                        	y = py
--                                x = px
--                        end
-- DGDGD we need client side tgt_pt			wall_stone(y, x)
                        fire_ball(Ind, GF_STONE_WALL, 0, 1, 1, "")
	end,
	["info"] = 	function()
        		return ""
	end,
        ["desc"] =	{
        		"Creates a prison of walls around you"
--                        ,"At level 10 it allows you to target a monster"
		}
}

STRIKE = add_spell
{
	["name"] = 	"Strike",
	["school"] = 	{SCHOOL_EARTH},
	["level"] = 	25,
	["mana"] = 	30,
	["mana_max"] = 	50,
	["fail"] = 	30,
	["direction"] = TRUE,
	["spell"] = 	function(args)
			if get_level(Ind, STRIKE, 50) >= 12 then
				fire_ball(Ind, GF_FORCE, args.dir, 50 + get_level(Ind, STRIKE, 50), 1, " casts a force ball of")
			else
				fire_ball(Ind, GF_FORCE, args.dir, 50 + get_level(Ind, STRIKE, 50), 0, " casts a force bolt of")
			end
	end,
	["info"] = 	function()
			if get_level(Ind, STRIKE, 50) >= 12 then
				return "dam "..(50 + get_level(Ind, STRIKE, 50)).." rad 1"
			else
				return "dam "..(50 + get_level(Ind, STRIKE, 50))
			end
	end,
	["desc"] =	{
			"Creates a force bolt that may stun enemies.",
			"At level 12 it turns into a ball of radius 1"
	}
}

SHAKE = add_spell
{
	["name"] = 	"Shake",
        ["school"] = 	{SCHOOL_EARTH},
        ["level"] = 	27,
        ["mana"] = 	25,
        ["mana_max"] = 	30,
        ["fail"] = 	30,
        ["spell"] = 	function()
--                        if get_level(Ind, SHAKE, 50) >= 10 then
--	                       	ret, x, y = tgt_pt()
--                                if ret == FALSE then return end
--                        else
--                        	x = player.px
--                                y = player.py
--                        end
			earthquake(player.wpos, player.py, player.px, 2 + get_level(Ind, SHAKE, 50));
	end,
	["info"] = 	function()
			return "rad "..(2 + get_level(Ind, SHAKE, 50))
	end,
        ["desc"] =	{
        		"Creates a localized earthquake"
--                        ,"At level 10 it can be targeted at any location"
        }
}
