-- The summoning school

SUMMON = add_spell {
	["name"] = 	"Summon",
	["name2"] = 	"Summon",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	1,
	["mana"] = 	4,
	["mana_max"] = 	4,
	["fail"] = 	10,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
		  summon_pet(Ind,  1073)
	end,
	["info"] = 	function()
			return "Summon friendly monster as your pet"
	end,
	["desc"] = 	{ "Summon friendly monster as your pet", }
}

UNSUMMON = add_spell {
	["name"] = 	"Unsummon pets",
	["name2"] = 	"Unsummon",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	0,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
			unsummon_pets(Ind)
	end,
	["info"] = 	function()
			return "Unsummon your pet"
	end,
	["desc"] = 	{ "Unsummon your pet", }
}