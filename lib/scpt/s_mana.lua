-- The mana school

function get_manathrust_dam(Ind, limit_lev)
	local lev, lev2

	lev = get_level(Ind, MANATHRUST_I, 50)
	lev2 = get_level(Ind, MANATHRUST_I, 20)
	if limit_lev ~= 0 and lev > limit_lev then lev = limit_lev + (lev - limit_lev) / 3 end

	return 3 + lev, 1 + lev2
end

MANATHRUST_I = add_spell {
	["name"] = 	"Manathrust I",
	["name2"] = 	"MThrust I",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	1,
	["mana"] = 	4,
	["mana_max"] = 	4,
	["fail"] = 	10,
	["direction"] = TRUE,
	["ftk"] = 1,
	["spell"] = 	function(args)
			fire_bolt(Ind, GF_MANA, args.dir, damroll(get_manathrust_dam(Ind, 1)), " casts a mana bolt for")
	end,
	["info"] = 	function()
			local x, y

			x, y = get_manathrust_dam(Ind, 1)
			return "dam "..x.."d"..y
	end,
	["desc"] = 	{ "Conjures up mana into a nearly irresistible bolt.", }
}
MANATHRUST_II = add_spell {
	["name"] = 	"Manathrust II",
	["name2"] = 	"MThrust II",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	20,
	["mana"] = 	10,
	["mana_max"] = 	10,
	["fail"] = 	-15,
	["direction"] = TRUE,
	["ftk"] = 1,
	["spell"] = 	function(args)
			fire_bolt(Ind, GF_MANA, args.dir, damroll(get_manathrust_dam(Ind, 20)), " casts a mana bolt for")
	end,
	["info"] = 	function()
			local x, y

			x, y = get_manathrust_dam(Ind, 20)
			return "dam "..x.."d"..y
	end,
	["desc"] = 	{ "Conjures up mana into a nearly irresistible bolt.", }
}
MANATHRUST_III = add_spell {
	["name"] = 	"Manathrust III",
	["name2"] = 	"MThrust III",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	40,
	["mana"] = 	25,
	["mana_max"] = 	25,
	["fail"] = 	-75,
	["direction"] = TRUE,
	["ftk"] = 1,
	["spell"] = 	function(args)
			fire_bolt(Ind, GF_MANA, args.dir, damroll(get_manathrust_dam(Ind, 0)), " casts a mana bolt for")
	end,
	["info"] = 	function()
			local x, y

			x, y = get_manathrust_dam(Ind, 0)
			return "dam "..x.."d"..y
	end,
	["desc"] = 	{ "Conjures up mana into a nearly irresistible bolt.", }
}

function get_recharge_pow(Ind, limit_lev)
	local lev

	lev = get_level(Ind, RECHARGE_I, 144)
	if limit_lev ~= 0 and lev > limit_lev then lev = limit_lev + (lev - limit_lev) / 3 end

	--just for visual niceness below actually learnable level
	if (lev < 2) then lev = 2 end

	return 8 + lev
end
RECHARGE_I = add_spell {
	["name"] = 	"Recharge I",
	["name2"] = 	"Rech I",
	["school"] = 	{SCHOOL_MANA},
	["level"] = 	5,
	["mana"] = 	10,
	["mana_max"] = 	10,
	["fail"] = 	20,
	["stat"] = 	A_INT,
	["spell_power"] = 0,
	["spell"] = 	function(args)
			if args.book < 0 then
				recharge(Ind, get_recharge_pow(Ind, 49), -1)
				return
			end
			recharge(Ind, get_recharge_pow(Ind, 49), args.book)
	end,
	["info"] = 	function()
			return "power "..get_recharge_pow(Ind, 49)
	end,
	["desc"] = 	{ "Taps on the ambient mana to recharge a wand or staff.", }
}
RECHARGE_II = add_spell {
	["name"] = 	"Recharge II",
	["name2"] = 	"Rech II",
	["school"] = 	{SCHOOL_MANA},
	["level"] = 	25,
	["mana"] = 	30,
	["mana_max"] = 	30,
	["fail"] = 	-25,
	["stat"] = 	A_INT,
	["spell_power"] = 0,
	["spell"] = 	function(args)
			if args.book < 0 then
				recharge(Ind, get_recharge_pow(Ind, 78), -1)
				return
			end
			recharge(Ind, get_recharge_pow(Ind, 78), args.book)
	end,
	["info"] = 	function()
			return "power "..get_recharge_pow(Ind, 78)
	end,
	["desc"] = 	{ "Taps on the ambient mana to recharge a magic device.", }
}
RECHARGE_III = add_spell {
	["name"] = 	"Recharge III",
	["name2"] = 	"Rech III",
	["school"] = 	{SCHOOL_MANA},
	["level"] = 	40,
	["mana"] = 	100,
	["mana_max"] = 	100,
	["fail"] = 	-65,
	["stat"] = 	A_INT,
	["spell_power"] = 0,
	["spell"] = 	function(args)
			if args.book < 0 then
				recharge(Ind, get_recharge_pow(Ind, 0), -1)
				return
			end
			recharge(Ind, get_recharge_pow(Ind, 0), args.book)
	end,
	["info"] = 	function()
			return "power "..get_recharge_pow(Ind, 0)
	end,
	["desc"] = 	{ "Taps on the ambient mana to recharge a magic device.", }
}

DISPERSEMAGIC = add_spell {
	["name"] = 	"Disperse Magic",
	["name2"] = 	"Disperse",
	["school"] = 	{SCHOOL_MANA},
	["level"] = 	15,
	["mana"] = 	30,
	["mana_max"] = 	30,
	["fail"] = 	10,
	["stat"] = 	A_INT,
	["blind"] = 	FALSE,
	["confusion"] = FALSE,
	["spell"] = 	function()
			set_blind(Ind, 0)
			set_confused(Ind, 0)
			if get_level(Ind, DISPERSEMAGIC, 50) >= 10 then
				set_image(Ind, 0)
			end
			if get_level(Ind, DISPERSEMAGIC, 50) >= 15 then
				set_slow(Ind, 0)
				set_fast(Ind, 0, 0)
				set_stun(Ind, 0)
			end
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] = 	{
			"Dispels a lot of magic that can affect you, be it good or bad.",
			"Level 1: blindness and confusion.",
			"Level 10: hallucination.",
			"Level 15: speed (both bad or good) and stun.",
	}
}

MANASHIELD = add_spell {
	["name"] = 	"Disruption Shield",
	["name2"] = 	"DShield",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	35,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	10,
	["spell"] = 	function()
			set_tim_manashield(Ind, randint(10) + 20 + get_level(Ind, MANASHIELD, 75))
	end,
	["info"] = 	function()
			return "dur "..(20 + get_level(Ind, MANASHIELD, 75)).."+d10"
	end,
	["desc"] = 	{
			"Redirects damage taken to your mana pool instead of reducing your hit points."
		}
}
