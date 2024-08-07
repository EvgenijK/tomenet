-- The summoning school

SUMMON_I = add_spell {
	["name"] = 	"Summon I",
	["name2"] = 	"Summon I",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	1,
	["mana"] = 	4,
	["mana_max"] = 	4,
	["fail"] = 	10,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
		  summon_pet_on_player(Ind,  33)
	end,
	["info"] = 	function()
		 	local lev
			lev = get_level(Ind, SUMMON_I, 30)
			return "Summon "..lev.."lvl weak monster as your pet"
	end,
	["desc"] = 	{ "Summon friendly monster as your pet", }
}

SUMMON_II = add_spell {
	["name"] = 	"Summon II",
	["name2"] = 	"Summon II",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	20,
	["mana"] = 	10,
	["mana_max"] = 	10,
	["fail"] = 	-20,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
		  summon_pet_on_player(Ind,  196)
	end,
	["info"] = 	function()
		 	local lev
			lev = get_level(Ind, SUMMON_II, 50)
			return "Summon "..lev.."lvl weak monster as your pet"
	end,
	["desc"] = 	{ "Summon friendly monster as your pet", }
}

SUMMON_III = add_spell {
	["name"] = 	"Summon III",
	["name2"] = 	"Summon III",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	40,
	["mana"] = 	25,
	["mana_max"] = 	25,
	["fail"] = 	-75,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
		  summon_pet_on_player(Ind,  1073)
	end,
	["info"] = 	function()
		 	local lev
			lev = get_level(Ind, SUMMON_III, 70)
			return "Summon "..lev.."lvl weak monster as your pet"
	end,
	["desc"] = 	{ "Summon friendly monster as your pet", }
}

SUMMON_GREAT = add_spell {
	["name"] = 	"Great Summon",
	["name2"] = 	"GreatSummon",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	45,
	["mana"] = 	40,
	["mana_max"] = 	40,
	["fail"] = 	-80,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
		  summon_pet_on_player(Ind,  1073)
	end,
	["info"] = 	function()
		 	local lev
			lev = get_level(Ind, SUMMON_GREAT, 100)
			return "Summon "..lev.."lvl weak monster as your pet"
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
	["info"] = 	{ "Unsummon your pet", },
	["desc"] = 	{ "Unsummon your pet", }
}

PET_MIND_ATTACK = add_spell {
	["name"] = 	"Pet attack",
	["name2"] = 	"PetAttack",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	0,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
			toggle_all_pets_mind(Ind, PET_ATTACK)
	end,
	["info"] = 	{ "", },
	["desc"] = 	{ "", }
}

PET_MIND_GUARD = add_spell {
	["name"] = 	"Pet guard",
	["name2"] = 	"PetGuard",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	0,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
			toggle_all_pets_mind(Ind, PET_GUARD)
	end,
	["info"] = 	{ "", },
	["desc"] = 	{ "", }
}

PET_MIND_FOLLOW = add_spell {
	["name"] = 	"Pet follow",
	["name2"] = 	"PetFollow",
	["school"] = 	SCHOOL_SUMMON,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	0,
	["direction"] = FALSE,
	["ftk"] = 0,
	["spell"] = 	function(args)
			toggle_all_pets_mind(Ind, PET_FOLLOW)
	end,
	["info"] = 	{ "", },
	["desc"] = 	{ "", }
}
