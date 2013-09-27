/* $Id$ */
/* File: cmd2.c */

/* Purpose: Movement commands (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"


/* Amount of experience a player gets for disarming a trap. - C. Blue
   (It's personal, not distributed over the party)
   diff,lvl go from 2,2 to 12,38 atm (some traps disabled).
   dlev was added to the formula, because some traps (arrow/bolt traps)
   depend on dungeon depth for determining the effect, too. */
#define TRAP_EXP(t_idx, dlev) \
    (((t_info[t_idx].difficulty * t_info[t_idx].minlevel) + ((dlev) > 5 ? (dlev) - 5 : 0)) * \
    (200 - t_info[t_idx].probability) / 100)

/* Maximum level a character may have reached so far to still be eligible
   to enter the Ironman Deep Dive Challenge. Two main possiblities:
   a) [12] for far-away from town
   b)  [1] for adjacent to town
   c)  [0] same as [1], it's a hack that means 'level 1 AND 0 max_exp'. <- RECOMMENDED */
#define IRONDEEPDIVE_MAXLEV 0


/*
 * Go up one level                                      -RAK-
 */
void do_cmd_go_up(int Ind)
{
	player_type *p_ptr = Players[Ind];
	monster_race *r_ptr = &r_info[p_ptr->body_monster];
	cave_type *c_ptr;
	struct worldpos *wpos = &p_ptr->wpos, old_wpos;
	bool tower = FALSE, dungeon = FALSE, surface = FALSE;
	cave_type **zcave;
	bool one_way = FALSE;
	int i;
	if(!(zcave = getcave(wpos))) return;

	if (wpos->wz > 0) tower = TRUE;
	if (wpos->wz < 0) dungeon = TRUE;
	if (wpos->wz == 0) surface = TRUE;

	if ((p_ptr->mode & MODE_DED_IDDC) && surface &&
	    (p_ptr->wpos.wx != WPOS_IRONDEEPDIVE_X || p_ptr->wpos.wy != WPOS_IRONDEEPDIVE_Y || 1 != WPOS_IRONDEEPDIVE_Z)) {
		msg_print(Ind, "\377yYou may not enter any other dungeon besides the Ironman Deep Dive Challenge!");
		return;
	}

	/* Is this a one-way tower? */
	if (tower) {
		if (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_IRON ||
		    wild_info[wpos->wy][wpos->wx].tower->flags1 & (DF1_FORCE_DOWN | DF1_NO_UP))
			one_way = TRUE;
	}

	/* Make sure he hasn't just changed depth */
	if (p_ptr->new_level_flag)
		return;

	/* Can we move ? */
	if (r_ptr->flags1 & RF1_NEVER_MOVE) {
		msg_print(Ind, "You cannot move by nature.");
		return;
	}

#if STAIR_FAIL_IF_CONFUSED
	/* Hack -- handle confusion */
	if (p_ptr->confused && magik(STAIR_FAIL_IF_CONFUSED)) {
		int dir = 5;

		/* Prevent walking nowhere */
		while (dir == 5)
			dir = rand_int(9) + 1;

		do_cmd_walk(Ind, dir, FALSE);
		return;
	}
#endif // STAIR_FAIL_IF_CONFUSED

	if (cfg.runlevel < 5 && surface) {
		msg_print(Ind,"The dungeon is closed");
		return;
	}

	/* not for some global events (Highlander Tournament) */
	if ((p_ptr->global_event_temp & PEVF_SEPDUN_00) && (p_ptr->wpos.wz == -1)) {
		msg_print(Ind, "The staircase is blocked by huge chunks of rocks!");
		if (!is_admin(p_ptr)) return;
	}

	/* Player grid */
	c_ptr = &zcave[p_ptr->py][p_ptr->px];

	/* Verify stairs if not a ghost, or admin wizard */
	if (!is_admin(p_ptr) &&
	    c_ptr->feat != FEAT_LESS && c_ptr->feat != FEAT_WAY_LESS &&
	    (!p_ptr->prob_travel || (c_ptr->info & CAVE_STCK)))
	{
		struct worldpos twpos;
		wpcopy(&twpos, wpos);
		if (twpos.wz < 0) twpos.wz++;
		if (!p_ptr->ghost) {
			msg_print(Ind, "I see no up staircase here.");
			return;
		}
#if 1	/* no need for this anymore - C. Blue */
	// argh there is.. people exploit the hell out of every possibility they see. - C. Blue
#ifdef SEPARATE_RECALL_DEPTHS
		else if ((get_recall_depth(&twpos, p_ptr) + 5 <= ABS(twpos.wz)) && (twpos.wz != 0)) /* player may return to town though! */
#else
		else if ((p_ptr->max_dlv + 5 <= getlevel(&twpos)) && (twpos.wz != 0)) /* player may return to town though! */
#endif
		{
			/* anti Ghost-dive */
			msg_print(Ind, "A mysterious force prevents you from going up.");
			return;
		}
#endif
	}
	if (surface) {
		if (!(wild_info[wpos->wy][wpos->wx].flags & WILD_F_UP)) {
			if (c_ptr->feat == FEAT_LESS || c_ptr->feat == FEAT_WAY_LESS)
				msg_print(Ind,"\377sOnly ivy-clad ruins of a former tower remain at this place..");
			else msg_print(Ind, "There is nothing above you.");
			return;
		} else if ((c_ptr->feat != FEAT_LESS && c_ptr->feat != FEAT_WAY_LESS) &&
			    ((!p_ptr->prob_travel || (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_NO_ENTRY_PROB)) &&
			    (!p_ptr->ghost || (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_NO_ENTRY_FLOAT)))) {
			msg_print(Ind, "There is nothing above you.");
			if (!is_admin(p_ptr)) return;
		}
	}
	if (tower && wild_info[wpos->wy][wpos->wx].tower->maxdepth == wpos->wz) {
		msg_print(Ind,"You are at the top of the tower!");
		return;
	}

#ifdef RPG_SERVER /* Exclude NO_DEATH dungeons from the gameplay */
	if ((surface) && (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_NO_DEATH)) {
		/* needed for 'Arena Monster Challenge' again, NO_DEATH are now simply empty (no items/monsters) */
		if (!(wpos->wx == cfg.town_x && wpos->wy == cfg.town_y) && !is_admin(p_ptr)) {
			msg_print(Ind,"\377sOnly ivy-clad ruins of a former tower remain at this place..");
			return;
		}
	}
#endif

/*	if (dungeon && !p_ptr->ghost && wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_IRON){*/
	if (dungeon && (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_IRON ||
	     wild_info[wpos->wy][wpos->wx].dungeon->flags1 & (DF1_FORCE_DOWN | DF1_NO_UP)) &&
	     (c_ptr->feat == FEAT_LESS || c_ptr->feat == FEAT_WAY_LESS)) {
		msg_print(Ind,"\377rThe stairways leading upwards are magically sealed in this dungeon.");
		if (!is_admin(p_ptr)) return;
	}
#if 1
	/* probability travel restrictions */
	if (dungeon) {
		if (c_ptr->feat != FEAT_LESS && c_ptr->feat != FEAT_WAY_LESS) {
			if (!p_ptr->ghost && (wild_info[wpos->wy][wpos->wx].dungeon->flags1 & (DF1_NO_RECALL | DF1_NO_UP | DF1_FORCE_DOWN))) {
				msg_print(Ind,"\377rA magical force prevents you from floating upwards.");
				if (!is_admin(p_ptr)) return;
			}
			if (wpos->wz == -1 && p_ptr->ghost && wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_NO_EXIT_FLOAT) {
				msg_print(Ind,"\377rA magical force prevents you from floating upwards.");
				if (!is_admin(p_ptr)) return;
			}
			if (wpos->wz == -1 && p_ptr->prob_travel && wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_NO_EXIT_PROB) {
				msg_print(Ind,"\377rA magical force prevents you from travelling upwards.");
				if (!is_admin(p_ptr)) return;
			}
		}
	} else {
		if (c_ptr->feat != FEAT_LESS && c_ptr->feat != FEAT_WAY_LESS &&
		    !p_ptr->ghost &&
		    ((wild_info[wpos->wy][wpos->wx].tower->flags1 & DF1_NO_RECALL) ||
		    (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_NO_RECALL_INTO) ||
		    (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_IRON))) {
			msg_print(Ind,"\377rA magical force prevents you from floating upwards.");
			if (!is_admin(p_ptr)) return;
		}
	}
#endif
#ifndef ARCADE_SERVER
	if(surface){
		dungeon_type *d_ptr = wild_info[wpos->wy][wpos->wx].tower;
#ifdef OBEY_DUNGEON_LEVEL_REQUIREMENTS
		//if(d_ptr->baselevel-p_ptr->max_dlv>2){
//		if ((!d_ptr->type && d_ptr->baselevel - p_ptr->max_dlv > 10) ||
		if ((!d_ptr->type && d_ptr->baselevel <= (p_ptr->lev * 3) / 2 + 7) ||
		    (d_ptr->type && d_info[d_ptr->type].min_plev > p_ptr->lev))
		{
			msg_print(Ind,"\377rAs you attempt to ascend, you are gripped by an uncontrollable fear.");
			if (!is_admin(p_ptr)) {
				set_afraid(Ind, 10);//+(d_ptr->baselevel-p_ptr->max_dlv));
				return;
			}
		}
#endif
		/* Nether Realm only for Kings/Queens*/
		if ((d_ptr->type == DI_NETHER_REALM) && !p_ptr->total_winner)
		{
			msg_print(Ind,"\377rAs you attempt to ascend, you are gripped by an uncontrollable fear.");
			if (!is_admin(p_ptr)) {
				set_afraid(Ind, 10);//+(d_ptr->baselevel-p_ptr->max_dlv));
				return;
			}
		}
	}
#endif
	if (p_ptr->inval && p_ptr->wpos.wz >= 10) {
		msg_print(Ind, "\377You may go no higher without a valid account.");
		return;
	}

	if (surface && p_ptr->wpos.wx == WPOS_IRONDEEPDIVE_X && p_ptr->wpos.wy == WPOS_IRONDEEPDIVE_Y && WPOS_IRONDEEPDIVE_Z > 0) {
		if (p_ptr->mode & MODE_PVP) {
			msg_format(Ind, "\377DPvP-mode characters are not eligible to enter the Ironman Deep Dive Challenge!");
			if (!is_admin(p_ptr)) return;
		}
		if (p_ptr->max_plv > IRONDEEPDIVE_MAXLEV &&
		    (IRONDEEPDIVE_MAXLEV || p_ptr->max_exp)) {
			if (IRONDEEPDIVE_MAXLEV) msg_format(Ind, "\377DYou may not enter once you exceeded character level %d!", IRONDEEPDIVE_MAXLEV);
			else msg_print(Ind, "\377DYou may not enter once you gained any experience!");
			if (!is_admin(p_ptr)) return;
		}
		/* disable WoR hint */
		p_ptr->warning_wor = 1;
	}

	/* S(he) is no longer afk */
	un_afk_idle(Ind);

	/* Remove the player from the old location */
	c_ptr->m_idx = 0;

	/* Show everyone that's he left */
	everyone_lite_spot(wpos, p_ptr->py, p_ptr->px);

	/* Forget his lite and viewing area */
	forget_lite(Ind);
	forget_view(Ind);

	/* Hack -- take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

	/* Success */
	if (c_ptr->feat == FEAT_LESS || c_ptr->feat == FEAT_WAY_LESS) {
		process_hooks(HOOK_STAIR, "d", Ind);
		if (surface) {
			msg_format(Ind, "\377%cYou enter %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, TRUE, wild_info[wpos->wy][wpos->wx].tower, 0, FALSE));
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s has entered %s..\377G]", COLOUR_DUNGEON, p_ptr->name, get_dun_name(wpos->wx, wpos->wy, TRUE, wild_info[wpos->wy][wpos->wx].tower, 0, FALSE));
			}
#endif
		}
#if 0 //Disable use of dungeon names
#ifdef IRONDEEPDIVE_MIXED_TYPES //Kurzel -- Hardcode (final transition floor is 2 currently, transition immediately after static towns, paranoia for last floor)
		else if (in_irondeepdive(wpos) && (iddc[ABS(wpos->wz)].step == 2 || ABS(wpos->wz) == 40 || ABS(wpos->wz) == 80 || ABS(wpos->wz) == 120) && ABS(wpos->wz) != 127) {
			msg_format(Ind, "\377%cYou enter %s..", COLOUR_DUNGEON, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s has entered %s..\377G]", COLOUR_DUNGEON, p_ptr->name, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
			}
		}
		else if (in_irondeepdive(wpos) && ABS(wpos->wz) != 127) {
			if ((!iddc[ABS(wpos->wz)].step && iddc[ABS(wpos->wz)+1].step) || ABS(wpos->wz) == 39 || ABS(wpos->wz) == 79 || ABS(wpos->wz) == 119)  {
				msg_format(Ind, "\377%cYou leave %s..", COLOUR_DUNGEON, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
				if (p_ptr->party)
				for (i = 1; i <= NumPlayers; i++) {
					if (Players[i]->conn == NOT_CONNECTED) continue;
					if (i == Ind) continue;
					if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
					if (Players[i]->party == p_ptr->party)
						msg_format(i, "\374\377G[\377%c%s has left %s..\377G]", COLOUR_DUNGEON, p_ptr->name, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
				}
			}
		}
#endif
#endif
		else if (wpos->wz == -1) {
			msg_format(Ind, "\377%cYou leave %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, FALSE, wild_info[wpos->wy][wpos->wx].dungeon, 0, FALSE));
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s has left %s..\377G]", COLOUR_DUNGEON, p_ptr->name, get_dun_name(wpos->wx, wpos->wy, FALSE, wild_info[wpos->wy][wpos->wx].dungeon, 0, FALSE));
			}
#endif
		}
		else if (c_ptr->feat == FEAT_WAY_LESS) {
			msg_print(Ind, "You enter the previous area.");
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz <= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
				msg_format(i, "\374\377G[\377%c%s took a staircase upwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz + 1) * 50);
			}
#else
			if (one_way && p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz <= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
				msg_format(i, "\374\377G[\377%c%s took a staircase upwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz + 1) * 50);
			}
#endif
		} else {
			msg_print(Ind, "You enter a maze of up staircases.");
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz <= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s took a staircase upwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz + 1) * 50);
			}
#else
			if (one_way && p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz <= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s took a staircase upwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz + 1) * 50);
			}
#endif
		}
		p_ptr->new_level_method = LEVEL_UP;
#ifdef USE_SOUND_2010
		sound(Ind, "staircase", NULL, SFX_TYPE_COMMAND, FALSE);
#endif
	} else {
		if (p_ptr->safe_float_turns) {
			msg_print(Ind, "Floating attempt blocked by client option.");
			return;
		}

#ifdef RPG_SERVER /* This is an iron-server... Prob trav should not work - the_sandman */
		msg_print(Ind, "This harsh world knows not what you're trying to do.");
		forget_view(Ind); //the_sandman
		if (!is_admin(p_ptr)) return;
#endif

		if (surface || tower) {
			if (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_IRON) {
				msg_print(Ind, "There is a magic barrier blocking your attempt.");
				if (!is_admin(p_ptr)) return;
			}
		} else {
			if (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_IRON) {
				msg_print(Ind, "There is a magic barrier blocking your attempt.");
				if (!is_admin(p_ptr)) return;
			}
		}

		if (surface) msg_format(Ind, "\377%cYou float into %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, TRUE, wild_info[wpos->wy][wpos->wx].tower, 0, FALSE));
		else if (wpos->wz == -1) msg_format(Ind, "\377%cYou float out of %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, FALSE, wild_info[wpos->wy][wpos->wx].dungeon, 0, FALSE));
		else msg_print(Ind, "You float upwards.");
		if (p_ptr->ghost) p_ptr->new_level_method = LEVEL_GHOST;
		else p_ptr->new_level_method = LEVEL_PROB_TRAVEL;
	}

	/* A player has left this depth */
	wpcopy(&old_wpos, wpos);
	wpos->wz++;
	new_players_on_depth(&old_wpos, -1, TRUE);
	new_players_on_depth(wpos, 1, TRUE);

	p_ptr->new_level_flag = TRUE;

    forget_view(Ind); //the_sandman
	/* He'll be safe for 2 turns */
	set_invuln_short(Ind, STAIR_GOI_LENGTH);

	/* Create a way back */
	create_down_stair = TRUE; /* appearently unused */
}

/*
 * Returns TRUE if we are in the Between...
 */
/* I want to be famous, a star of the screen,
 * but you can do something in between...
 */
static bool between_effect(int Ind, cave_type *c_ptr)
{
	player_type *p_ptr = Players[Ind];
	byte bx, by;
	struct c_special *cs_ptr;
	struct dun_level *l_ptr = getfloor(&p_ptr->wpos);
	int wid, hgt;

	if (!(cs_ptr = GetCS(c_ptr, CS_BETWEEN))) return (FALSE);

	/* allow void gates on surface levels (for 0,0 events) */
	if (l_ptr) {
		wid = l_ptr->wid - 1;
		hgt = l_ptr->hgt - 1;
	} else {
		wid = MAX_WID;
		hgt = MAX_HGT;
	}

	by = cs_ptr->sc.between.fy;
	bx = cs_ptr->sc.between.fx;

	/* (HACK) sanity check to cover cut-off vaults with missing void gate end-points! - C. Blue */
	if (bx < 1 || by < 1 || bx >= wid || by >= hgt) {
		msg_print(Ind, "The gate seems broken.");
		return(TRUE);
	}

	msg_print(Ind, "You fall into the void. Brrrr! It's deadly cold.");

//	if (PRACE_FLAG(PR1_TP))
	if (p_ptr->prace == RACE_DRACONIAN) {
		int reduc = ((p_ptr->ac + p_ptr->to_a) / 50) + 1;

		if (reduc < 1) reduc = 1; /* Really, really low AC, like some TL eating firestones - mikaelh */

		take_hit(Ind, distance(by, bx, p_ptr->py, p_ptr->px) / (10 * reduc), "going Between", 0);
	}

#ifdef USE_SOUND_2010
//	sound(Ind, "teleport", NULL, SFX_TYPE_COMMAND, TRUE);
	sound(Ind, "blink", NULL, SFX_TYPE_COMMAND, TRUE);
#endif

	swap_position(Ind, by, bx);

	/* To avoid being teleported back */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

	return (TRUE);
}

/* for special gates (event: Dungeon Keeper) - C. Blue
   Taking a transport beacon usually teleports the player to Bree
   and [successfully] terminates all global events for him. */
static bool beacon_effect(int Ind, cave_type *c_ptr) {
	player_type *p_ptr = Players[Ind];
	int d, k;
	char buf[1024];
	global_event_type *ge;
	object_type forge, *o_ptr = &forge;

#ifdef USE_SOUND_2010
//	sound(Ind, "teleport", NULL, SFX_TYPE_COMMAND, TRUE);
#endif

	/* Beacons in sector00 lead to Bree transportation */
	if (p_ptr->wpos.wx == WPOS_SECTOR00_X &&
	    p_ptr->wpos.wy == WPOS_SECTOR00_Y &&
	    p_ptr->wpos.wz == WPOS_SECTOR00_Z) {
		for (d = 0; d < MAX_GLOBAL_EVENTS; d++) {
			ge = &global_event[d];

			switch (p_ptr->global_event_type[d]) {
			case GE_DUNGEON_KEEPER:
				/* tell everyone + himself that he won */
				sprintf(buf, "\374\377a>>%s wins %s!<<", p_ptr->name, ge->title);
				msg_broadcast_format(0, buf);
				s_printf("%s EVENT_WON: %s wins %d (%s)\n", showtime(), p_ptr->name, d + 1, ge->title);
				//l_printf("%s \\{s%s has won %s\n", showdate(), p_ptr->name, ge->title);

				/* may only take part in one Labyrinth race per char -- boost him to level 3 */
				if (p_ptr->max_lev < 3) gain_exp_to_level(Ind, 3);
				/* extra optional niceness :-p boost already-level-3 chars to level 4 just for the heck of it.. */
				else gain_exp_to_level(Ind, 4);

				/* create reward parchment */
				k = lookup_kind(TV_PARCHMENT, SV_DEED_DUNGEONKEEPER);
				invcopy(o_ptr, k);
				o_ptr->number = 1;
				object_aware(Ind, o_ptr);
				object_known(o_ptr);
				o_ptr->discount = 0;
				o_ptr->level = 0;
				o_ptr->ident |= ID_MENTAL;
				//o_ptr->note = quark_add("Dungeon Keeper reward");
				inven_carry(Ind, o_ptr);
				break;
			case GE_NONE:
			default:
				break;
			}

			p_ptr->global_event_type[d] = GE_NONE; /* no longer participant */
		}

		msg_print(Ind, "\377GYou are transported out of here and far away!");
		p_ptr->recall_pos.wx = cfg.town_x;
		p_ptr->recall_pos.wy = cfg.town_y;
		p_ptr->recall_pos.wz = 0;
		p_ptr->new_level_method = LEVEL_OUTSIDE_RAND;
		p_ptr->global_event_temp = PEVF_PASS_00; /* clear all other flags, allow a final recall out */
		recall_player(Ind, "");
		return TRUE;
	}

	return FALSE;
}

/*
 * Go down one level
 */
void do_cmd_go_down(int Ind)
{
	player_type *p_ptr = Players[Ind];
	monster_race *r_ptr = &r_info[p_ptr->body_monster];
	cave_type *c_ptr;
	struct worldpos *wpos = &p_ptr->wpos, old_wpos;
	bool tower = FALSE, dungeon = FALSE, surface = FALSE;
	cave_type **zcave;
	bool one_way = FALSE; //ironman, no_up, force_down
	int i;
	if(!(zcave = getcave(wpos))) return;

	if (wpos->wz > 0) tower = TRUE;
	if (wpos->wz < 0) dungeon = TRUE;
	if (wpos->wz == 0) surface = TRUE;

	if ((p_ptr->mode & MODE_DED_IDDC) && surface &&
	    (p_ptr->wpos.wx != WPOS_IRONDEEPDIVE_X || p_ptr->wpos.wy != WPOS_IRONDEEPDIVE_Y || -1 != WPOS_IRONDEEPDIVE_Z)) {
		msg_print(Ind, "\377yYou may not enter any other dungeon besides the Ironman Deep Dive Challenge!");
		return;
	}

	/* Is this a one-way dungeon? */
	if (dungeon) {
		if (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_IRON ||
		    wild_info[wpos->wy][wpos->wx].dungeon->flags1 & (DF1_FORCE_DOWN | DF1_NO_UP))
			one_way = TRUE;
	}

	/* Make sure he hasn't just changed depth */
	if (p_ptr->new_level_flag)
		return;

	/* Can we move ? */
	if (r_ptr->flags1 & RF1_NEVER_MOVE) {
		msg_print(Ind, "You cannot move by nature.");
		return;
	}

	if (cfg.runlevel < 5 && surface) {
		msg_print(Ind,"The dungeon is closed");
		return;
	}

#if STAIR_FAIL_IF_CONFUSED
	/* Hack -- handle confusion */
	if (p_ptr->confused && magik(STAIR_FAIL_IF_CONFUSED)) {
		int dir = 5;

		/* Prevent walking nowhere */
		while (dir == 5)
			dir = rand_int(9) + 1;

		do_cmd_walk(Ind, dir, FALSE);
		return;
	}
#endif // STAIR_FAIL_IF_CONFUSED


	/* Player grid */
	c_ptr = &zcave[p_ptr->py][p_ptr->px];

	/* Hack -- Enter a store (and between gates, etc) */
	if ((!p_ptr->ghost || is_admin(p_ptr)) &&
			(c_ptr->feat == FEAT_SHOP))
#if 0
			(c_ptr->feat >= FEAT_SHOP_HEAD) &&
			(c_ptr->feat <= FEAT_SHOP_TAIL))
#endif	// 0
	{
		/* Disturb */
		disturb(Ind, 1, 0);

		/* Hack -- Enter store */
		command_new = '_';
		do_cmd_store(Ind);
		return;
	}

	if (c_ptr->feat == FEAT_BETWEEN) { 
		/* Check interference */
		if (interfere(Ind, 20)) { /* between gate interference chance */
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			return;
		}

		if (between_effect(Ind, c_ptr)) return;
		/* not jumped? strange.. */
	}

	if (c_ptr->feat == FEAT_BEACON) {
		/* Hack -- take a turn */
		p_ptr->energy -= level_speed(&p_ptr->wpos);

		/* Check interference */
		if (interfere(Ind, 20)) return; /* beacon interference chance */

		if (beacon_effect(Ind, c_ptr)) return;
		/* not transported? strange.. */
	}


	/* Verify stairs */
//      if (!p_ptr->ghost && (strcmp(p_ptr->name,cfg_admin_wizard)) && c_ptr->feat != FEAT_MORE && !p_ptr->prob_travel)
	if (!is_admin(p_ptr) &&
		c_ptr->feat != FEAT_MORE && c_ptr->feat != FEAT_WAY_MORE &&
		(!p_ptr->prob_travel || (c_ptr->info & CAVE_STCK)))
	{
		struct worldpos twpos;
		wpcopy(&twpos, wpos);
		if(twpos.wz>0) twpos.wz--;
		if (!p_ptr->ghost) {
			msg_print(Ind, "I see no down staircase here.");
			return;
		}
#if 1	// no need for this anymore - C. Blue
	// argh there is.. people exploit the hell out of every possibility they see. - C. Blue
#ifdef SEPARATE_RECALL_DEPTHS
		else if ((get_recall_depth(&twpos, p_ptr) + 5 <= ABS(twpos.wz)) && (twpos.wz != 0)) /* player may return to town though! */
#else
		else if ((p_ptr->max_dlv + 5 <= getlevel(&twpos)) && (twpos.wz != 0)) /* player may return to town though! */
#endif
		{
			/* anti Ghost-dive */
			msg_print(Ind, "A mysterious force prevents you from going down.");
			return;
		}
#endif
	}
	if (surface) {
		if (!(wild_info[wpos->wy][wpos->wx].flags & WILD_F_DOWN)) {
			if (c_ptr->feat == FEAT_MORE || c_ptr->feat == FEAT_WAY_MORE)
				msg_print(Ind,"\377sOnly mud-filled ruins of former catacombs remain at this place..");
			else msg_print(Ind, "There is nothing above you.");
			return;
		} else if ((c_ptr->feat != FEAT_MORE && c_ptr->feat != FEAT_WAY_MORE) &&
			    ((!p_ptr->prob_travel || (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_NO_ENTRY_PROB)) &&
			    (!p_ptr->ghost || (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_NO_ENTRY_FLOAT)))) {
			msg_print(Ind, "There is nothing below you.");
			if (!is_admin(p_ptr)) return;
		}
	}

	/* Verify maximum depth */
	if (dungeon && wild_info[wpos->wy][wpos->wx].dungeon->maxdepth == -wpos->wz) {
		msg_print(Ind, "You are at the bottom of the dungeon.");
		return;
	}

#ifdef RPG_SERVER /* Exclude NO_DEATH dungeons from the gameplay */
	if ((surface) && (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_NO_DEATH)) {
		msg_print(Ind,"\377sOnly mud-filled ruins of former catacombs remain at this place..");
		if (!is_admin(p_ptr)) return;
	}
#endif

	if ((p_ptr->global_event_temp & PEVF_SEPDUN_00) && (p_ptr->wpos.wz == 1)) {
		msg_print(Ind,"\377sThe way is blocked by huge chunks of rocks!");
		if (!is_admin(p_ptr)) return;
	}

/*	if (tower && !p_ptr->ghost && wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_IRON){*/
	if (tower && (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_IRON ||
	     wild_info[wpos->wy][wpos->wx].tower->flags1 & (DF1_FORCE_DOWN | DF1_NO_UP)) &&
	    (c_ptr->feat == FEAT_MORE || c_ptr->feat == FEAT_WAY_MORE))
	{
		msg_print(Ind,"\377rThe stairways leading downwards are magically sealed in this tower.");
		if (!is_admin(p_ptr)) return;
	}
#if 1
	/* probability travel restrictions */
	if (tower) {
		if ((c_ptr->feat != FEAT_MORE) && (c_ptr->feat != FEAT_WAY_MORE)) {
			if ((!p_ptr->ghost) && (wild_info[wpos->wy][wpos->wx].tower->flags1 & (DF1_NO_RECALL | DF1_NO_UP | DF1_FORCE_DOWN))) {
				msg_print(Ind,"\377rA magical force prevents you from travelling downwards.");
				if (!is_admin(p_ptr)) return;
			}
			if (wpos->wz == 1 && p_ptr->ghost && wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_NO_EXIT_FLOAT) {
				msg_print(Ind,"\377rA magical force prevents you from floating downwards.");
				if (!is_admin(p_ptr)) return;
			}
			if (wpos->wz == 1 && p_ptr->prob_travel && wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_NO_EXIT_PROB) {
				msg_print(Ind,"\377rA magical force prevents you from travelling downwards.");
				if (!is_admin(p_ptr)) return;
			}
		}
	} else {
		if ((c_ptr->feat != FEAT_MORE) && (c_ptr->feat != FEAT_WAY_MORE) &&
		    !p_ptr->ghost &&
		    ((wild_info[wpos->wy][wpos->wx].dungeon->flags1 & DF1_NO_RECALL) ||
		    (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_NO_RECALL_INTO) ||
		    (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_IRON))) {
			msg_print(Ind,"\377rA magical force prevents you from floating downwards.");
			if (!is_admin(p_ptr)) return;
		}
	}
#endif

#ifndef ARCADE_SERVER
	if(surface){
		dungeon_type *d_ptr=wild_info[wpos->wy][wpos->wx].dungeon;
#ifdef OBEY_DUNGEON_LEVEL_REQUIREMENTS
		//if(d_ptr->baselevel-p_ptr->max_dlv>2){
		//if(d_ptr->baselevel-p_ptr->max_dlv>2 ||
		//		d_info[d_ptr->type].min_plev > p_ptr->lev)
//		if((!d_ptr->type && d_ptr->baselevel-p_ptr->max_dlv > 10) ||
		if ((!d_ptr->type && d_ptr->baselevel <= (p_ptr->lev * 3) / 2 + 7) ||
		    (d_ptr->type && d_info[d_ptr->type].min_plev > p_ptr->lev)) {
			msg_print(Ind,"\377rAs you attempt to descend, you are gripped by an uncontrollable fear.");
			if (!is_admin(p_ptr)) {
				set_afraid(Ind, 10);//+(d_ptr->baselevel-p_ptr->max_dlv));
				return;
			}
		}
#endif
		/* Nether Realm only for Kings/Queens*/
		if ((d_ptr->type == DI_NETHER_REALM) && !p_ptr->total_winner) {
			msg_print(Ind,"\377rAs you attempt to descend, you are gripped by an uncontrollable fear.");
			if (!is_admin(p_ptr)) {
				set_afraid(Ind, 10);//+(d_ptr->baselevel-p_ptr->max_dlv));
				return;
			}
		}
	}
#endif

	if (p_ptr->inval && p_ptr->wpos.wz <= -10) {
		msg_print(Ind, "\377You may go no lower without a valid account.");
		return;
	}

	if (p_ptr->wpos.wz == 0 && p_ptr->wpos.wx == WPOS_IRONDEEPDIVE_X && p_ptr->wpos.wy == WPOS_IRONDEEPDIVE_Y && WPOS_IRONDEEPDIVE_Z < 0) {
		if (p_ptr->mode & MODE_PVP) {
			msg_format(Ind, "\377DPvP-mode characters are not eligible to enter the Ironman Deep Dive Challenge!");
			if (!is_admin(p_ptr)) return;
		}
		if (p_ptr->max_plv > IRONDEEPDIVE_MAXLEV &&
		    (IRONDEEPDIVE_MAXLEV || p_ptr->max_exp)) {
			if (IRONDEEPDIVE_MAXLEV) msg_format(Ind, "\377DYou may not enter once you exceeded character level %d!", IRONDEEPDIVE_MAXLEV);
			else msg_print(Ind, "\377DYou may not enter once you gained any experience!");
			if (!is_admin(p_ptr)) return;
		}
		/* disable WoR hint */
		p_ptr->warning_wor = 1;
	}

	/* S(he) is no longer afk */
	un_afk_idle(Ind);

	/* Remove the player from the old location */
	c_ptr->m_idx = 0;

	/* Show everyone that's he left */
	everyone_lite_spot(wpos, p_ptr->py, p_ptr->px);

	/* Forget his lite and viewing area */
	forget_lite(Ind);
	forget_view(Ind);

	/* Hack -- take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

	/* Success */
	if (c_ptr->feat == FEAT_MORE || c_ptr->feat == FEAT_WAY_MORE) {
		process_hooks(HOOK_STAIR, "d", Ind);
		if (surface) {
			msg_format(Ind, "\377%cYou enter %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, FALSE, wild_info[wpos->wy][wpos->wx].dungeon, 0, FALSE));
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s has entered %s..\377G]", COLOUR_DUNGEON, p_ptr->name, get_dun_name(wpos->wx, wpos->wy, FALSE, wild_info[wpos->wy][wpos->wx].dungeon, 0, FALSE));
			}
#endif
		}
#if 0 //Disable use of dungeon names
#ifdef IRONDEEPDIVE_MIXED_TYPES //Kurzel -- Hardcode (final transition floor is 2 currently, transition immediately after static towns, paranoia for last floor)
		else if (in_irondeepdive(wpos) && (iddc[ABS(wpos->wz)].step == 2 || ABS(wpos->wz) == 40 || ABS(wpos->wz) == 80 || ABS(wpos->wz) == 120) && ABS(wpos->wz) != 127) {
			msg_format(Ind, "\377%cYou enter %s..", COLOUR_DUNGEON, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s has entered %s..\377G]", COLOUR_DUNGEON, p_ptr->name, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
			}
		}
		else if (in_irondeepdive(wpos) && ABS(wpos->wz) != 127) {
			if ((!iddc[ABS(wpos->wz)].step && iddc[ABS(wpos->wz)+1].step) || ABS(wpos->wz) == 39 || ABS(wpos->wz) == 79 || ABS(wpos->wz) == 119)  {
				msg_format(Ind, "\377%cYou leave %s..", COLOUR_DUNGEON, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
				if (p_ptr->party)
				for (i = 1; i <= NumPlayers; i++) {
					if (Players[i]->conn == NOT_CONNECTED) continue;
					if (i == Ind) continue;
					if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
					if (Players[i]->party == p_ptr->party)
						msg_format(i, "\374\377G[\377%c%s has left %s..\377G]", COLOUR_DUNGEON, p_ptr->name, d_name + d_info[iddc[ABS(wpos->wz)+1].type].name);
				}
			}
		}
#endif
#endif
		else if (wpos->wz == 1) {
			msg_format(Ind, "\377%cYou leave %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, TRUE, wild_info[wpos->wy][wpos->wx].tower, 0, FALSE));
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s has left %s..\377G]", COLOUR_DUNGEON, p_ptr->name, get_dun_name(wpos->wx, wpos->wy, TRUE, wild_info[wpos->wy][wpos->wx].tower, 0, FALSE));
			}
#endif
		}
		else if (c_ptr->feat == FEAT_WAY_MORE) {
			msg_print(Ind, "You enter the next area.");
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz >= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s took a staircase downwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz - 1) * 50);
			}
#else
			if (one_way && p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz >= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s took a staircase downwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz - 1) * 50);
			}
#endif
		} else {
			msg_print(Ind, "You enter a maze of down staircases.");
#ifdef RPG_SERVER /* stair scumming in non-IRON dungeons might create mad spam otherwise */
			if (p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz >= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s took a staircase downwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz - 1) * 50);
			}
#else
			if (one_way && p_ptr->party)
			for (i = 1; i <= NumPlayers; i++) {
			        if (Players[i]->conn == NOT_CONNECTED) continue;
				if (i == Ind) continue;
				if (Players[i]->wpos.wx != wpos->wx || Players[i]->wpos.wy != wpos->wy) continue;
				if (Players[i]->wpos.wz >= 0) continue; /* must be in same dungeon/tower */
				if (Players[i]->party == p_ptr->party)
					msg_format(i, "\374\377G[\377%c%s took a staircase downwards to %dft..\377G]", COLOUR_DUNGEON, p_ptr->name, (wpos->wz - 1) * 50);
			}
#endif
		}
		p_ptr->new_level_method = LEVEL_DOWN;
#ifdef USE_SOUND_2010
		sound(Ind, "staircase", NULL, SFX_TYPE_COMMAND, FALSE);
#endif
	} else {
		if (p_ptr->safe_float_turns) {
			msg_print(Ind, "Floating attempt blocked by client option.");
			return;
		}

#ifdef RPG_SERVER	/* This is an iron-server... Prob trav should not work - the_sandman */
		msg_print(Ind, "This harsh world knows not what you're trying to do.");
	        forget_view(Ind);//the_sandman
		if (!is_admin(p_ptr)) return;
#endif

		if (surface || dungeon) {
			if (wild_info[wpos->wy][wpos->wx].dungeon->flags2 & DF2_IRON) {
				msg_print(Ind, "There is a magic barrier blocking your attempt.");
				if (!is_admin(p_ptr)) return;
			}
		} else {
			if (wild_info[wpos->wy][wpos->wx].tower->flags2 & DF2_IRON) {
				msg_print(Ind, "There is a magic barrier blocking your attempt.");
				if (!is_admin(p_ptr)) return;
			}
		}

		if (surface) msg_format(Ind, "\377%cYou float into %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, FALSE, wild_info[wpos->wy][wpos->wx].dungeon, 0, FALSE));
		else if (wpos->wz == 1) msg_format(Ind, "\377%cYou float out of %s..", COLOUR_DUNGEON, get_dun_name(wpos->wx, wpos->wy, TRUE, wild_info[wpos->wy][wpos->wx].tower, 0, FALSE));
		else msg_print(Ind, "You float downwards.");
		if (p_ptr->ghost) p_ptr->new_level_method = LEVEL_GHOST;
		else p_ptr->new_level_method = LEVEL_PROB_TRAVEL;
	}
	/* A player has left this depth */
	wpcopy(&old_wpos, wpos);
	wpos->wz--;
	new_players_on_depth(&old_wpos, -1, TRUE);
	new_players_on_depth(wpos, 1, TRUE);

	p_ptr->new_level_flag = TRUE;

	forget_view(Ind); //the_sandman

	/* He'll be safe for 2 turns */
	set_invuln_short(Ind, STAIR_GOI_LENGTH);

	/* Create a way back */
	create_up_stair = TRUE; /* appearently unused */
}



/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* S(he) is no longer afk */
	un_afk_idle(Ind);

	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

#if CHATTERBOX_LEVEL > 2	/* This can be noisy */
	if (!p_ptr->taciturn_messages)
		msg_print(Ind, "You carefully search things around you..");
#endif	// CHATTERBOX_LEVEL

	/* Repeat if requested */
	if (p_ptr->always_repeat) p_ptr->command_rep = PKT_SEARCH;

	/* Search */
        if (p_ptr->pclass == CLASS_ROGUE && !p_ptr->rogue_heavyarmor) {
                //Radius of 5 ... 15 squares
                detect_bounty(Ind, (p_ptr->lev/5) + 5);
        } else {
                search(Ind);
        }
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Stop searching */
	if (p_ptr->searching)
	{
		/* Clear the searching flag */
		p_ptr->searching = FALSE;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_STATE | PR_MAP);
	}

	/* Start searching */
	else
	{
		/* S(he) is no longer afk */
		un_afk_idle(Ind);

		/* Set the searching flag */
		p_ptr->searching = TRUE;
		break_shadow_running(Ind);
		stop_precision(Ind);

		/* Update stuff */
		p_ptr->update |= (PU_BONUS);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_STATE | PR_SPEED | PR_MAP);
	}
}



/*
 * Allocates objects upon opening a chest    -BEN-
 *
 * Disperse treasures from the chest "o_ptr", centered at (x,y).
 *
 * Small chests often contain "gold", while Large chests always contain
 * items.  Wooden chests contain 2 items, Iron chests contain 4 items,
 * and Steel chests contain 6 items.  The "value" of the items in a
 * chest is based on the "power" of the chest, which is in turn based
 * on the level on which the chest is generated.
 */
static void chest_death(int Ind, int y, int x, object_type *o_ptr)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;
	cave_type **zcave;

//	int             i, d, ny, nx;
	int             number, little;
	long		cash;

	if(!(zcave=getcave(wpos))) return;

	/* Must be a chest */
	if (o_ptr->tval != TV_CHEST) return;

	/* Small chests often hold "gold" */
	little = (o_ptr->sval < SV_CHEST_MIN_LARGE);

	/* Determine how much to drop (see above) */
	number = (o_ptr->sval % SV_CHEST_MIN_LARGE) * 2;

	/* Generate some treasure */
	if (o_ptr->pval && (number > 0)) {
		/* let's not be scrooges =) - C. Blue */
		cash = (((o_ptr->level + 10) * (o_ptr->level + 10)) * 3) / number;

		/* Drop some objects (non-chests) */
		for (; number > 0; --number) {
				/* Opening a chest -- this hack makes sure we don't find a chest in a chest, even though yo like chests */
				opening_chest = TRUE;

				/* Determine the "value" of the items */
//				object_level = ABS(o_ptr->pval) + 10;
				object_level = ABS(o_ptr->level) + 10;

				/* Small chests often drop gold */
				if (little && magik(75))
					place_gold(wpos, y, x, cash);
				else if (!little && magik(20))
					place_gold(wpos, y, x, cash);
				/* Otherwise drop an item */
				else {
					place_object_restrictor = RESF_NONE;
					/* mostly DROP_GOOD */
					place_object(wpos, y, x, magik(75) ? TRUE : FALSE, FALSE, FALSE, make_resf(p_ptr), default_obj_theme, 0, ITEM_REMOVAL_NORMAL);
				}

				/* for anti-cheeze make the dropped items o_ptr->owner = p_ptr->id (loot might be piled btw) */

				/* Reset the object level */
				object_level = getlevel(wpos);

				/* No longer opening a chest */
				opening_chest = FALSE;
		}
	}

	/* Empty */
	o_ptr->pval = 0;

	/* Known */
	object_known(o_ptr);
}


/*
 * Chests have traps too.
 *
 * Exploding chest destroys contents (and traps).
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int Ind, int y, int x, s16b o_idx)
{
	player_type *p_ptr = Players[Ind];
	object_type *o_ptr = &o_list[o_idx];

	int  trap;
	bool ident = FALSE;

	/* Only analyze chests */
	if (o_ptr->tval != TV_CHEST) return;

	/* Ignore disarmed chests */
	if (o_ptr->pval <= 0) return;

	/* Obtain the trap */
	trap = o_ptr->pval;

	/* Message */
//	msg_print(Ind, "You found a trap!");
	msg_print(Ind, "You triggered a trap!");

#ifdef USE_SOUND_2010
	sound(Ind, "trap_setoff", NULL, SFX_TYPE_MISC, FALSE);
#endif

	/* Set off trap */
	ident = player_activate_trap_type(Ind, y, x, o_ptr, o_idx);

	/* Some traps may destroy the chest */
	if (o_ptr && ident && !p_ptr->trap_ident[o_ptr->pval])
	{
		p_ptr->trap_ident[o_ptr->pval] = TRUE;
		msg_format(Ind, "You identified the trap as %s.",
			   t_name + t_info[trap].name);
	}
}


/*
 * Return the index of a house given an coordinate pair
 */
int pick_house(struct worldpos *wpos, int y, int x)
{
	int i;

	/* Check each house */
	for (i = 0; i < num_houses; i++)
	{
		/* Check this one */
		if (houses[i].dx == x && houses[i].dy == y && inarea(&houses[i].wpos, wpos))
		{
			/* Return */
			return i;
		}
	}

	/* Failure */
	return -1;
}

/* Test if a coordinate (player pos usually) is inside a building (or on its edge) - C. Blue */
bool inside_house(struct worldpos *wpos, int x, int y) {
#if 0
	int i;

	for (i = 0; i < num_houses; i++) {
		if (!inarea(&houses[i].wpos, wpos)) continue;

		if (houses[i].flags & HF_RECT) {
			if (houses[i].x <= x && houses[i].x + houses[i].coords.rect.width - 1 >= x &&
			    houses[i].y <= y && houses[i].y + houses[i].coords.rect.height - 1 >= y)
				return TRUE;
		} else {
			/* anyone willing to implement non-rectangular houses? */
		}
	}
#else
	cave_type *c_ptr = &(getcave(wpos)[y][x]);
	/* assume all houses are on the world surface (and vaults aren't) */
	if (wpos->wz == 0 && (c_ptr->info & CAVE_ICKY)) return TRUE;
#endif

	return FALSE;
}

/* Door change permissions
 * args example: ynn1 -> Party, Class, Race, MinLevel
 */
static bool chmod_door(int Ind, struct dna_type *dna, char *args){
	player_type *p_ptr = Players[Ind];
	if (!is_admin(p_ptr)) {
		if(dna->creator != p_ptr->dna) {
			/* note: the house 'master' is not necessarily the house owner! */
			msg_print(Ind, "Only the house master may change permissions.");
			return(FALSE);
		}
		/* more security needed... */
	}

	if (dna->a_flags & ACF_PARTY) {
		if (!p_ptr->party) {
			msg_print(Ind, "You are not in a party.");
			return(FALSE);
		}
		if (dna->owner_type != OT_PLAYER) {
			msg_print(Ind, "Only houses owned by the party owner can be given party access.");
			return FALSE;
		}
		if (strcmp(parties[p_ptr->party].owner, lookup_player_name(dna->owner))) {
			msg_print(Ind, "You must be owner of your party to allow party access.");
			return(FALSE);
		}
	}

	if ((dna->a_flags = args[1])) {
		dna->min_level = atoi(&args[2]);
	}

	return(TRUE);
}

/* Door change ownership */
static bool chown_door(int Ind, struct dna_type *dna, char *args, int x, int y){
	player_type *p_ptr = Players[Ind];
	int newowner = -1;
	int i, h_idx;

	/* to prevent house amount cheeze (houses_owned)
	   let's just say house ownership can't be transferred.
	   Changing the door access should be sufficient. */
	if (!is_admin(p_ptr)) {
		if (dna->creator != p_ptr->dna) return(FALSE);
		/* maybe make guild master only chown guild -> "guild hall" */
	}
	switch (args[1]) {
	case '1':
		/* Check house limit of target player! */
		i = name_lookup_loose(Ind, &args[2], FALSE, FALSE);
		if (!i) {
			msg_print(Ind, "Target player isn't logged on.");
			return(FALSE);
		}
		if (compat_pmode(Ind, i, TRUE)) {
			msg_format(Ind, "You cannot transfer houses to %s players!", compat_pmode(Ind, i, TRUE));
			return(FALSE);
		}
		if (Players[i]->inval) {
			msg_print(Ind, "You cannot transfer houses to players that aren't validated!");
			return(FALSE);
		}
		if (cfg.houses_per_player && (Players[i]->houses_owned >= ((Players[i]->lev > 50 ? 50 : Players[i]->lev) / cfg.houses_per_player)) && !is_admin(Players[i])) {
			if ((int)(Players[i]->lev / cfg.houses_per_player) == 0)
				msg_format(Ind, "That player needs to be at least level %d to own a house!", cfg.houses_per_player);
			else
				msg_print(Ind, "At his current level, that player cannot own more houses!");
			return (FALSE);
		}
		h_idx = pick_house(&p_ptr->wpos, y, x);
		/* paranoia */
		if (h_idx == -1) {
			msg_print(Ind, "There is no transferrable house there.");
			return FALSE;
		}
		if (houses[h_idx].flags & HF_MOAT) {
			if (cfg.castles_for_kings && !Players[i]->total_winner) {
				msg_print(Ind, "That player is neither king nor queen, neither emperor nor empress!");
				return FALSE;
			}
			if (cfg.castles_per_player && (Players[i]->castles_owned >= cfg.castles_per_player))  {
				if (cfg.castles_per_player == 1)
					msg_format(Ind, "That player already owns a castle!");
				else
					msg_format(Ind, "That player already owns %d castles!", cfg.castles_per_player);
				return FALSE;
			}
		}

		/* Finally change the owner */
		if (dna->owner_type == OT_GUILD) {
			guilds[dna->owner].h_idx = 0;
			Send_guild_config(dna->owner);
		}
		newowner = lookup_player_id_messy(&args[2]);
		if (!newowner) newowner = -1;
		break;
#ifdef ENABLE_GUILD_HALL
	case '2':
		h_idx = pick_house(&p_ptr->wpos, y, x);
		/* paranoia */
		if (h_idx == -1) {
			msg_print(Ind, "There is no transferrable house there.");
			return FALSE;
		}
		/* only the guild master can assign an OT_GUILD house */
		if (!p_ptr->guild || p_ptr->id != guilds[p_ptr->guild].master) {
			msg_print(Ind, "You must be the guild master to give house ownership to a guild.");
			return FALSE;
		}
		/* there can only be one guild hall */
		if (guilds[p_ptr->guild].h_idx) {
			msg_print(Ind, "The guild already has a guild hall.");
			return FALSE;
		}

		/* guild halls must be mang-style houses (technically too, for CAVE_GUILD_SUS flag to work!) */
		if ((houses[h_idx].flags & HF_TRAD)) {
			msg_print(Ind, "\377yGuild halls must not be list-type (store-like) houses.");
			return FALSE;
		}

		/* guild master 'loses' one own house */
		if (dna->owner_type == OT_PLAYER) {
			p_ptr->houses_owned--;
			if (houses[h_idx].flags & HF_MOAT) p_ptr->castles_owned--;
		}
		newowner = p_ptr->guild;
		msg_format(Ind, "This house is now owned by %s.", guilds[p_ptr->guild].name);
		break;
#endif
/*	case '3':
		newowner = party_lookup(&args[2]);
		break;
	case '4':
		for (i = 0; i < MAX_CLASS; i++) {
			if (!strcmp(&args[2], class_info[i].title))
				newowner = i;
		}
		break;
	case '5':
		for (i = 0; i < MAX_RACE; i++) {
			if (!strcmp(&args[2], race_info[i].title))
				newowner = i;
		}
		break; */
	default:
		msg_print(Ind,"\377ySorry, this feature is not available");
		if (!is_admin(p_ptr)) return FALSE;
	}

	if (newowner != -1) {
		if (args[1] == '1') {
			for (i = 1; i <= NumPlayers; i++) {     /* in game? maybe long winded */
				if (Players[i]->id == newowner) {
					if (dna->owner_type == OT_PLAYER) p_ptr->houses_owned--;
					Players[i]->houses_owned++;
					if (houses[h_idx].flags & HF_MOAT) {
						if (dna->owner_type == OT_PLAYER) p_ptr->castles_owned--;
						Players[i]->castles_owned++;
					}
					dna->creator = Players[i]->dna;
					dna->owner = newowner;
					dna->owner_type = args[1] - '0';
					dna->a_flags = ACF_NONE;
					return(TRUE);
				}
			}
			return(FALSE);
		}
		/* args[1] == '2' aka change to OT_GUILD: */
		dna->owner = newowner;
		dna->owner_type = args[1] - '0';
		if (dna->owner_type == OT_GUILD) {
			guilds[p_ptr->guild].h_idx = h_idx + 1;
			Send_guild_config(p_ptr->guild);
		}
		return(TRUE);
	}
	return FALSE;
}

/* Returns > 0 if access is granted,
   and <= 0 if access is denied.
   Note == FALSE -> no message will be printed. - C. Blue */
bool access_door(int Ind, struct dna_type *dna, bool note) {
	player_type *p_ptr = Players[Ind];
	if (!dna->owner) return(FALSE); /* house doesn't belong to anybody */
/*	if (is_admin(p_ptr))
		return(TRUE); - moved to allow more overview for admins when looking at
				house door colours on the world surface - C. Blue */

	/* Test for cumulative restrictions */
	if (p_ptr->dna != dna->creator) {
		if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level) {
			if (note) msg_print(Ind, "Your level doesn't match the house restriction yet.");
			return(FALSE); /* defies logic a bit, but for speed */
		}
		if ((dna->a_flags & ACF_CLASS) && (p_ptr->pclass != (dna->creator & 0xff))) {
			if (note) msg_print(Ind, "Your class doesn't match the house restriction.");
			return(FALSE);
		}
		if ((dna->a_flags & ACF_RACE) && (p_ptr->prace != ((dna->creator >> 8) & 0xff))) {
			if (note) msg_print(Ind, "Your race doesn't match the house restriction.");
			return(FALSE);
		}

		/* Allow ACF_WINNER and ACF_FALLEN_WINNER to actually stack positively */
		if ((dna->a_flags & ACF_WINNER) && (dna->a_flags & ACF_FALLENWINNER) && !p_ptr->once_winner) {
			if (note) msg_print(Ind, "You must be (fallen) royalty to match the house restriction.");
			return(FALSE);
		}
		else if ((dna->a_flags & ACF_WINNER) && !p_ptr->total_winner) {
			if (note) msg_print(Ind, "You must be royalty to match the house restriction.");
			return(FALSE);
		}
		else if ((dna->a_flags & ACF_FALLENWINNER) && (p_ptr->total_winner || !p_ptr->once_winner)) {
			if (note) msg_print(Ind, "You must be fallen royalty to match the house restriction.");
			return(FALSE);
		}

		if ((dna->a_flags & ACF_NOGHOST) && !(p_ptr->mode & MODE_NO_GHOST)) {
			/* extra message in case ACF_WINNER is also specified, just for fanciness */
			if (dna->a_flags & ACF_WINNER) {
				if (note) msg_print(Ind, "You must be emperor or empress to match the house restriction.");
			} else {
				if (note) msg_print(Ind, "You must be unworldly to match the house restriction.");
			}
			return(FALSE);
		}
	}

	switch (dna->owner_type) {
	case OT_PLAYER:
		/* new doors in new server different */
		if (p_ptr->id == dna->owner && p_ptr->dna == dna->creator)
			return(TRUE);
		if (dna->a_flags & ACF_PARTY){
			if (!p_ptr->party) return(FALSE);
			if (!strcmp(parties[p_ptr->party].owner, lookup_player_name(dna->owner)))
				return(TRUE);
		}
		return(FALSE);
	case OT_PARTY:
		if (!p_ptr->party) return(FALSE);
		if (player_in_party(dna->owner, Ind)) return(TRUE);
		return(FALSE);
	case OT_CLASS:
		if (p_ptr->pclass == dna->owner) return(TRUE);
		return(FALSE);
	case OT_RACE:
		if (p_ptr->prace == dna->owner) return(TRUE);
		return(FALSE);
	case OT_GUILD:
		if (!p_ptr->guild) return(FALSE);
		if (p_ptr->guild == dna->owner) return(TRUE);
		return(FALSE);
	}
	return(FALSE);
}

/* Note: these colours will be affected by nightly darkening, so shades
   shouldn't be used but completely different colours only..
   but since we don't have enough colours, it doesn't matter - C. Blue */
int access_door_colour(int Ind, struct dna_type *dna){
	player_type *p_ptr = Players[Ind];

	/* house doesn't belong to anybody? */
	if (!dna->owner) return(TERM_UMBER);

	/* house belongs to someone of incompatible mode!
	   in that case, no further checking is required,
	   since won't ever be able to access it.
	   (Added for PLAYER_STORES especially.) */
	if (p_ptr->mode & MODE_PVP) {
		if (!(dna->mode & MODE_PVP)) {
			return(TERM_L_DARK);
		}
	} else if (p_ptr->mode & MODE_EVERLASTING) {
		if (!(dna->mode & MODE_EVERLASTING)) {
			return(TERM_L_DARK);
		}
	} else if (dna->mode & MODE_PVP) {
		return(TERM_L_DARK);
	} else if (dna->mode & MODE_EVERLASTING) {
		return(TERM_L_DARK);
	}

	/* test house access permissions */
	switch (dna->owner_type) {
	case OT_PLAYER:
		/* new doors in new server different */
		if (p_ptr->id == dna->owner && p_ptr->dna == dna->creator)
			return(TERM_L_GREEN);
		if (dna->a_flags & ACF_PARTY){
			if (!p_ptr->party) return(TERM_SLATE);
			/* exploit fix: create party houses, then promote someone else to leader */
			if (!strcmp(parties[p_ptr->party].owner, lookup_player_name(dna->owner))) {
				if (dna->a_flags & ACF_CLASS) {
					if (p_ptr->pclass == (dna->creator & 0xff)) {
						if (dna->a_flags&ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
						return(TERM_WHITE);
					} else return(TERM_ORANGE);
				}
				if (dna->a_flags & ACF_RACE) {
					if (p_ptr->prace == ((dna->creator >> 8) & 0xff)) {
						if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
						return(TERM_WHITE);
					} else return(TERM_ORANGE);
				}

				/* ACF_WINNER and ACF_FALLENWINNER can be used together */
				if ((dna->a_flags & ACF_WINNER) && (dna->a_flags & ACF_FALLENWINNER)) {
					if (p_ptr->once_winner) {
						if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
						return(TERM_L_RED);
					} else return(TERM_RED);
				}
				else if (dna->a_flags & ACF_WINNER) {
					if (p_ptr->total_winner) {
						if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
						return(TERM_L_RED);
					} else return(TERM_RED);
				}
				else if (dna->a_flags & ACF_FALLENWINNER) {
					if (!p_ptr->total_winner && p_ptr->once_winner) {
						if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
						return(TERM_L_RED);
					} else return(TERM_RED);
				}

				if (dna->a_flags & ACF_NOGHOST) {
					if (p_ptr->mode & MODE_NO_GHOST) {
						if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
					}
					return(TERM_L_DARK);
				}
				if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
				return(TERM_L_BLUE);
			}
		}
		break;
	case OT_PARTY:
		if (!p_ptr->party) return(TERM_SLATE);
		if (player_in_party(dna->owner, Ind)) {
			if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
			return(TERM_L_BLUE);
		}
		break;
	case OT_CLASS:
		if (p_ptr->pclass == dna->owner) {
			if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
			return(TERM_WHITE);
		}
		break;
	case OT_RACE:
		if (p_ptr->pclass == dna->owner) {
			if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
			return(TERM_WHITE);
		}
		break;
	case OT_GUILD:
		if (!p_ptr->guild) return(TERM_SLATE);
		if (p_ptr->guild == dna->owner) {
			if (dna->a_flags & ACF_LEVEL && p_ptr->max_plv < dna->min_level && p_ptr->dna != dna->creator) return(TERM_YELLOW);
			return(TERM_VIOLET);
		}
		break;
	}

	/* we have no permission to access */
	return(TERM_SLATE);
//	if (dna->a_flags & ACF_STORE) return(TERM_MULTI); /* older idea. Instead, see PLAYER_STORE - C. Blue */
}

cptr get_house_owner(struct c_special *cs_ptr)
{
	static char string[80];
	struct dna_type *dna = cs_ptr->sc.ptr;
	strcpy(string,"nobody.");
	if (dna->owner) {
//		char *name;
		cptr name;
		switch (dna->owner_type) {
		case OT_PLAYER:
			if((name = lookup_player_name(dna->owner)))
			strcpy(string, name);
			break;
		case OT_PARTY:
			if(strlen(parties[dna->owner].name))
			strcpy(string, parties[dna->owner].name);
			break;
		case OT_CLASS:
			strcpy(string, class_info[dna->owner].title);
			strcat(string, "s");
			break;
		case OT_RACE:
			strcpy(string, race_info[dna->owner].title);
			strcat(string, "s");
			break;
		case OT_GUILD:
			strcpy(string, guilds[dna->owner].name);
			break;
		}
	}
	return (string);
}

/*
 * Open a closed door or closed chest.
 *
 * Note unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;
	cave_type **zcave;
	monster_race *r_ptr = &r_info[p_ptr->body_monster];

	int                             y, x, i, j;
	int                             flag;

	cave_type               *c_ptr;
	object_type             *o_ptr;

	bool more = FALSE;
	if (!(zcave = getcave(wpos))) return;


	/* Ghosts cannot open doors ; not in WRAITHFORM */
	if (((p_ptr->ghost || p_ptr->tim_wraith) && !is_admin(p_ptr)) ||
	    (p_ptr->body_monster && !(r_ptr->flags2 & RF2_OPEN_DOOR)
	    && !strchr("thpkng", r_info[p_ptr->body_monster].d_char))) {
		msg_print(Ind, "You cannot open things!");
		return;
	}

	/* Get a "repeated" direction */
	if (dir) {
		/* Get requested location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
		/* Get requested grid */
		c_ptr = &zcave[y][x];
		/* Get the object (if any) */
		o_ptr = &o_list[c_ptr->o_idx];

		/* Nothing useful */
		if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		      (c_ptr->feat <= FEAT_DOOR_TAIL)) &&
		    !((c_ptr->feat == FEAT_HOME)) &&
		    (o_ptr->tval != TV_CHEST))
		{
			/* Message */
			msg_print(Ind, "You see nothing there to open.");
		}

		/* Monster in the way */
		else if (c_ptr->m_idx > 0) {
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);
			/* Message */
			msg_print(Ind, "There is a monster in the way!");
			/* Attack */
			py_attack(Ind, y, x, TRUE);
		}

		/* Open a closed chest. */
		else if (o_ptr->tval == TV_CHEST) {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);
			/* Assume opened successfully */
			flag = TRUE;
			/* Attempt to unlock it */
			if (o_ptr->pval > 0) {
				trap_kind *t_ptr;
				t_ptr = &t_info[o_ptr->pval];

				/* Assume locked, and thus not open */
				flag = FALSE;
				/* Get the "disarm" factor */
				i = p_ptr->skill_dis;
				/* Penalize some conditions */
				if (p_ptr->blind || no_lite(Ind)) i = i / 10;
				if (p_ptr->confused || p_ptr->image) i = i / 10;
				/* Extract the difficulty */
				j = i - t_ptr->difficulty;
				// j = i - o_ptr->pval;
				/* Always have a small chance of success */
				if (j < 2) j = 2;
				/* Success -- May still have traps */
				if (rand_int(100) < j) {
					msg_print(Ind, "You have picked the lock.");
					flag = TRUE;
#ifdef USE_SOUND_2010
					sound(Ind, "open_pick", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
					/* opening it uses only 1x trdifficulty instead of 3x */
					gain_exp(Ind, TRAP_EXP(o_ptr->pval, getlevel(&p_ptr->wpos)) / 3);
				}

				/* Failure -- Keep trying */
				else {
					/* We may continue repeating */
					more = TRUE;
					msg_print(Ind, "You failed to pick the lock.");
				}
			}
			/* can't open a non-closed chest */
			else if (o_ptr->pval == 0) flag = FALSE;

			/* Allowed to open */
			if (flag) {
#ifdef USE_SOUND_2010
				sound(Ind, "open_chest", NULL, SFX_TYPE_COMMAND, FALSE);
#endif
				/* Apply chest traps, if any */
				chest_trap(Ind, y, x, c_ptr->o_idx);

				break_cloaking(Ind, 3);
				break_shadow_running(Ind); 
				stop_precision(Ind);
				stop_shooting_till_kill(Ind);

				/* Some traps might destroy the chest on setting off */
				if (o_ptr)
					/* Let the Chest drop items */
					chest_death(Ind, y, x, o_ptr);
			}
		}

		/* Jammed door */
		else if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x08) {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);
			/* Stuck */
			msg_print(Ind, "The door appears to be stuck.");
#ifdef USE_SOUND_2010
			sound(Ind, "open_door_stuck", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
		}

		/* Locked door */
		else if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x01) {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);
			/* Disarm factor */
			i = p_ptr->skill_dis;
			/* Penalize some conditions */
			if (p_ptr->blind || no_lite(Ind)) i = i / 10;
			if (p_ptr->confused || p_ptr->image) i = i / 10;
			/* Extract the lock power */
			j = c_ptr->feat - FEAT_DOOR_HEAD;
			/* Extract the difficulty XXX XXX XXX */
			j = i - (j * 4);
			/* Always have a small chance of success */
			if (j < 2) j = 2;
			/* Success */
			if (rand_int(100) < j) {
				/* Message */
				msg_print(Ind, "You have picked the lock.");
#ifdef USE_SOUND_2010
				sound(Ind, "open_pick", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
				break_cloaking(Ind, 3);
				break_shadow_running(Ind); 
				stop_precision(Ind);
				stop_shooting_till_kill(Ind);

				/* Set off trap */
				if (GetCS(c_ptr, CS_TRAPS)) player_activate_door_trap(Ind, y, x);

				/* Automatic bot detection - mikaelh */
				if (!(c_ptr->info & CAVE_MAGELOCK)) {
					p_ptr->silly_door_exp++;
					if (p_ptr->silly_door_exp >= 100) {
						msg_print(Ind, "Botting never pays off...");
						take_xp_hit(Ind, 1, "botting", TRUE, FALSE, TRUE);
					} else {
						/* Experience */
						gain_exp(Ind, 1);
					}
				}

				/* Open the door */
				c_ptr->feat = FEAT_OPEN;
				/* Notice */
				note_spot_depth(wpos, y, x);
				/* Redraw */
				everyone_lite_spot(wpos, y, x);
				/* Update some things */
				p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
			}

			/* Failure */
			else {
				/* Message */
				msg_print(Ind, "You failed to pick the lock.");

				/* We may keep trying */
				more = TRUE;
			}
		}

		/* Home */
		else if (c_ptr->feat == FEAT_HOME) {
			struct c_special *cs_ptr;
			if ((cs_ptr = GetCS(c_ptr, CS_DNADOOR))) { /* orig house failure */
				if (access_door(Ind, cs_ptr->sc.ptr, TRUE) || Players[Ind]->admin_dm) {
#if USE_MANG_HOUSE_ONLY || TRUE /* let'em open it, so that thevery can take place :) */
					/* Open the door */
					c_ptr->feat = FEAT_HOME_OPEN;
#ifdef USE_SOUND_2010
					sound(Ind, "open_door", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
#else	/* USE_MANG_HOUSE */
					msg_print(Ind, "Just walk in.");
#endif	/* USE_MANG_HOUSE */
					/* S(he) is no longer afk */
					un_afk_idle(Ind);
					break_cloaking(Ind, 3);
					break_shadow_running(Ind); 
					stop_precision(Ind);
					stop_shooting_till_kill(Ind);

					/* Take half a turn */
					p_ptr->energy -= level_speed(&p_ptr->wpos)/2;
					/* Notice */
					note_spot_depth(wpos, y, x);
					/* Redraw */
					everyone_lite_spot(wpos, y, x);
					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				} else {
					struct dna_type *dna = cs_ptr->sc.ptr;
					if (!strcmp(get_house_owner(cs_ptr), "nobody.")) {
						int factor, price;
						factor = (100 + adj_chr_gold[p_ptr->stat_ind[A_CHR]]) / 2;
						price = dna->price / 100 * factor;
						if (price < 100) price = 100;
						msg_format(Ind, "\377oThat house costs %d gold.", price);
					} else {
#ifdef PLAYER_STORES
						/* We don't have house access, but if it's set up as
						   a player-run store, we may enter it to buy things - C. Blue */
						disturb(Ind, 1, 0);
						if (!do_cmd_player_store(Ind, x, y))
#endif
						msg_format(Ind,"\377sThat house is owned by %s.",get_house_owner(cs_ptr));
					}
				}
				return;
			}
			if ((cs_ptr = GetCS(c_ptr, CS_KEYDOOR))){
				struct key_type *key = cs_ptr->sc.ptr;
				for ( j = 0; j < INVEN_PACK; j++){
					object_type *o_ptr = &p_ptr->inventory[j];
					if (o_ptr->tval == TV_KEY && o_ptr->sval == SV_HOUSE_KEY && o_ptr->pval == key->id) {
						c_ptr->feat = FEAT_HOME_OPEN;
						/* S(he) is no longer afk */
						un_afk_idle(Ind);
						break_cloaking(Ind, 3);
						break_shadow_running(Ind); 
						stop_precision(Ind);
						stop_shooting_till_kill(Ind);
						p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;
						note_spot_depth(wpos, y, x);
						everyone_lite_spot(wpos, y, x);
						p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
						msg_format(Ind, "\377gThe key fits in the lock. %d:%d",key->id, o_ptr->pval);
#ifdef USE_SOUND_2010
						sound(Ind, "open_pick", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
						return;
					} else if (is_admin(p_ptr)) {
						c_ptr->feat = FEAT_HOME_OPEN;
						/* S(he) is no longer afk */
						un_afk_idle(Ind);
						break_cloaking(Ind, 3);
						break_shadow_running(Ind); 
						stop_precision(Ind);
						stop_shooting_till_kill(Ind);
						p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;
						note_spot_depth(wpos, y, x);
						everyone_lite_spot(wpos, y, x);
						p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
						msg_format(Ind, "\377gThe door crashes open. %d",key->id);
#ifdef USE_SOUND_2010
						sound(Ind, "bash_door_break", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
						return;
					}
				}
				msg_print(Ind,"\377rYou need a key to open this door.");
#ifdef USE_SOUND_2010
				sound(Ind, "open_door_stuck", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
			}
		}

		/* Closed door */
		else {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);

			break_cloaking(Ind, 3);
			break_shadow_running(Ind);
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);

			/* Take half a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;

			if (!(p_ptr->confused || p_ptr->blind)
			    || magik(33)) {
				/* Set off trap */
				if (GetCS(c_ptr, CS_TRAPS)) player_activate_door_trap(Ind, y, x);

				/* Open the door */
				c_ptr->feat = FEAT_OPEN;
#ifdef USE_SOUND_2010
				sound(Ind, "open_door", NULL, SFX_TYPE_COMMAND, TRUE);
#endif
				/* Notice */
				note_spot_depth(wpos, y, x);
				/* Redraw */
				everyone_lite_spot(wpos, y, x);
				/* Update some things */
				p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
			} else {
				/* Message */
				msg_print(Ind, "You fumble..");

				/* We may keep trying */
				more = TRUE;
			}
		}
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(Ind, 0, 0);
	else if (p_ptr->always_repeat) p_ptr->command_rep = PKT_OPEN;
}


/*
 * Close an open door.
 */
void do_cmd_close(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	monster_race *r_ptr = &r_info[p_ptr->body_monster];
	struct worldpos *wpos=&p_ptr->wpos;

	int                     y, x, i;
	cave_type               *c_ptr;

	bool more = FALSE;
	cave_type **zcave;
	if(!(zcave=getcave(wpos))) return;


	/* Ghosts cannot close ; not in WRAITHFORM */
	if (((p_ptr->ghost || p_ptr->tim_wraith) && !is_admin(p_ptr)) ||
	    (p_ptr->body_monster && !(r_ptr->flags2 & RF2_OPEN_DOOR)
	    && !strchr("thpkng", r_info[p_ptr->body_monster].d_char)))
	{
		msg_print(Ind, "You cannot close things!");
		return;
	}

	/* Get a "repeated" direction */
	if (dir) {
		/* Get requested location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &zcave[y][x];

		/* Broken door */
		if (c_ptr->feat == FEAT_BROKEN) {
			/* Message */
			msg_print(Ind, "The door appears to be broken.");
		}

		/* Require open door */
		else if (c_ptr->feat != FEAT_OPEN && c_ptr->feat != FEAT_HOME_OPEN) {
			/* Message */
			msg_print(Ind, "You see nothing there to close.");
		}

		/* Monster in the way */
		else if (c_ptr->m_idx > 0) {
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Message */
			msg_print(Ind, "There is a monster in the way!");

			/* Attack */
			py_attack(Ind, y, x, TRUE);
		}

		/* Object in the way */
		else if (c_ptr->o_idx > 0) {
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Message */
			msg_print(Ind, "There is an object blocking the doorway!");
		}

		/* House door, close it */
		else if (c_ptr->feat == FEAT_HOME_OPEN) {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);

			/* Find this house */
			i = pick_house(wpos, y, x);

			break_cloaking(Ind, 2);
			break_shadow_running(Ind); 
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Close the door */
			c_ptr->feat = FEAT_HOME;
#ifdef USE_SOUND_2010
			sound(Ind, "close_door", NULL, SFX_TYPE_COMMAND, TRUE);
#endif

			/* Notice */
			note_spot_depth(wpos, y, x);

			/* Redraw */
			everyone_lite_spot(wpos, y, x);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
		}

		/* Close the door */
		else {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);
			break_cloaking(Ind, 2);
			break_shadow_running(Ind); 
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);

#ifdef USE_SOUND_2010
			sound(Ind, "close_door", NULL, SFX_TYPE_COMMAND, TRUE);
#endif

			/* Set off trap */
			if (GetCS(c_ptr, CS_TRAPS)) player_activate_door_trap(Ind, y, x);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Close the door */
			c_ptr->feat = FEAT_DOOR_HEAD + 0x00;

			/* Notice */
			note_spot_depth(wpos, y, x);

			/* Redraw */
			everyone_lite_spot(wpos, y, x);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
		}
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(Ind, 0, 0);
}


/*
 * Check the terrain around the location to see if erosion takes place.
 * TODO: expand this for more generic terrain types		- Jir -
 */
byte twall_erosion(worldpos *wpos, int y, int x)
{
	int tx, ty, d;
	byte feat = FEAT_FLOOR; /* todo: use something like 'place_floor'*/
	cave_type **zcave;
	cave_type *c_ptr;
	if(!(zcave = getcave(wpos))) return(FALSE);

	for (d = 1; d <= 9; d++) {
		if (d == 5) continue;

		tx = x + ddx[d];
		ty = y + ddy[d];

		if (!in_bounds(ty, tx)) continue;

		c_ptr = &zcave[ty][tx];
		if (c_ptr->feat == FEAT_DEEP_WATER) {
//			feat = FEAT_DEEP_WATER; /* <- this is only if FEAT_DEEP_WATER is also terraformable in turn (see cave_set_feat_live) */
			feat = FEAT_SHAL_WATER; /* <- this should be used otherwise */
			break;
		} else if (c_ptr->feat == FEAT_DEEP_LAVA) {
//			feat = FEAT_DEEP_LAVA; /* <- this is only if FEAT_DEEP_LAVA is also terraformable in turn (see cave_set_feat_live) */
			feat = FEAT_SHAL_LAVA; /* <- this should be used otherwise */
			break;
		}
	}

	/* Result */
	return (feat);
}

/*
 * Tunnel through wall.  Assumes valid location.
 *
 * Note that it is impossible to "extend" rooms past their
 * outer walls (which are actually part of the room).
 *
 * This will, however, produce grids which are NOT illuminated
 * (or darkened) along with the rest of the room.
 */
bool twall(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	byte *w_ptr = &p_ptr->cave_flag[y][x];
	struct worldpos *wpos = &p_ptr->wpos;
	cave_type **zcave;
	cave_type *c_ptr;
	if(!(zcave = getcave(wpos))) return(FALSE);
	c_ptr = &zcave[y][x];

	/* Paranoia -- Require a wall or door or some such */
	if (cave_floor_bold(zcave, y, x)) return (FALSE);

	/* Remove the feature */
//	c_ptr->feat = FEAT_FLOOR;
//	c_ptr->feat = twall_erosion(wpos, y, x);
	cave_set_feat_live(wpos, y, x, twall_erosion(wpos, y, x));

	/* Forget the "field mark" */
	*w_ptr &= ~CAVE_MARK;

	/* Result */
	return (TRUE);
}


/*
 * Tunnels through "walls" (including rubble and closed doors)
 *
 * Note that tunneling almost always takes time, since otherwise
 * you can use tunnelling to find monsters.  Also note that you
 * must tunnel in order to hit invisible monsters in walls (etc).
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
/* XXX possibly wrong */
/* Reveal a secret door if tunnelling triggered a trap on it? */
#define TRAP_REVEALS_DOOR
void do_cmd_tunnel(int Ind, int dir, bool quiet_borer) {
	player_type *p_ptr = Players[Ind];
	object_type *o_ptr = &p_ptr->inventory[INVEN_TOOL];
	struct worldpos *wpos = &p_ptr->wpos;

	int y, x, power = p_ptr->skill_dig + (quiet_borer ? 20000 : 0);
	int mining = get_skill(p_ptr, SKILL_DIG);
	int dug_feat = FEAT_NONE, tval = 0, sval = 0, special_k_idx = 0; //chest / golem base material / rune
	struct dun_level *l_ptr = getfloor(&p_ptr->wpos);

	int old_object_level = object_level;
	int find_level = getlevel(&p_ptr->wpos);

	object_type *o2_ptr = &p_ptr->inventory[INVEN_WIELD];
	object_type *o3_ptr = &p_ptr->inventory[INVEN_ARM];
	int wood_power = 0;

	cave_type *c_ptr;
	bool old_floor = FALSE;
	bool more = FALSE;
	feature_type *f_ptr;
	cave_type **zcave;

	object_type forge;
	char o_name[ONAME_LEN];

	if(!(zcave = getcave(wpos))) return;

	/* check if our weapons can help hacking down wood etc */
	if (o2_ptr->k_idx && !p_ptr->heavy_wield) {
		switch (o2_ptr->tval) {
		case TV_AXE: wood_power = 40; break;
		case TV_SWORD: wood_power = 20; break;
		}
		if (k_info[o2_ptr->k_idx].flags4 & (TR4_MUST2H | TR4_SHOULD2H)) wood_power <<= 1;
	}
	if (o3_ptr->k_idx && !p_ptr->heavy_wield) {
		switch (o3_ptr->tval) {
		case TV_AXE: if (wood_power < 40) wood_power = 40; break;
		case TV_SWORD: if (wood_power < 20) wood_power = 20; break;
		}
	}

	object_level = find_level;
	if (mining > find_level * 2) mining = find_level * 2;
	find_level += mining / 2;

	/* Ghosts have no need to tunnel ; not in WRAITHFORM */
	if (((p_ptr->ghost) || (p_ptr->tim_wraith))
	    && !p_ptr->admin_dm) {
		/* Message */
		msg_print(Ind, "You cannot tunnel.");
		return;
	}

	/* Must be have something to dig with, or power gets halved */
	if (!p_ptr->inventory[INVEN_TOOL].k_idx ||
	    (p_ptr->inventory[INVEN_TOOL].tval != TV_DIGGING))
		power >>= 1;

	/* have at least 1 digging power so you can dig through rubble/organic stuff */
	if (!power) power = 1;

	/* Get a direction to tunnel, or Abort */
	if (dir) {
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid */
		c_ptr = &zcave[y][x];

		f_ptr = &f_info[c_ptr->feat];

		/* Check the floor-hood */
		old_floor = cave_floor_bold(zcave, y, x);

		/* No tunnelling through emptiness */
		if ((cave_floor_bold(zcave, y, x)) || (c_ptr->feat == FEAT_PERM_CLEAR)) {
			msg_print(Ind, "You see nothing there to tunnel through.");
			disturb(Ind, 0, 0);
			return;
		}

		/* No tunnelling through doors */
//		else if (c_ptr->feat < FEAT_SECRET && c_ptr->feat >= FEAT_HOME_HEAD)
		else if ((f_info[c_ptr->feat].flags1 & FF1_DOOR) &&
				c_ptr->feat != FEAT_SECRET)
		{
//                      msg_print(Ind, "You cannot tunnel through doors.");

			/* Try opening it instead */
			do_cmd_open(Ind, dir);
			return;
		}

		/* A monster is in the way */
		else if (c_ptr->m_idx > 0) {
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			msg_print(Ind, "There is a monster in the way!");

			/* Attack */
			py_attack(Ind, y, x, TRUE);
		}

		/* Okay, try digging */
		else {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);
			break_cloaking(Ind, 0);
			break_shadow_running(Ind); 
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* not in monster KILL_WALL form or via magic; not on world surface: wpos->wz == 0 ! */
			if (l_ptr && !quiet_borer) {
				/* prepare to discover a special feature */
				if ((rand_int(5000) <= mining + 5) && can_go_up(wpos, 0x1)) dug_feat = FEAT_WAY_LESS;
				else if ((rand_int(5000) <= mining + 5) && can_go_down(wpos, 0x1)) dug_feat = FEAT_WAY_MORE;
				else if ((rand_int(3000) <= mining + 10) && !(l_ptr->flags1 & LF1_NO_WATER)) dug_feat = FEAT_FOUNTAIN;
				else if (rand_int(500) < ((l_ptr->flags1 & LF1_NO_LAVA) ? 0 : ((l_ptr->flags1 & LF1_LAVA) ? 50 : 3))) dug_feat = FEAT_SHAL_LAVA;
				else if (rand_int(500) < ((l_ptr->flags1 & LF1_NO_WATER) ? 0 : ((l_ptr->flags1 & LF1_WATER) ? 50 : 8))) dug_feat = FEAT_SHAL_WATER;

				/* prepare to find a special object */
				else if (rand_int(2000) <= mining) {
					tval = TV_CHEST;
#if 1 /* maybe todo: use get_obj_num_prep_tval() instead of this hardcoding chances.. */
					sval = rand_int(mining) * rand_int(50);
					/* 1,2,3, 5,6,7 - according to k_info.txt */
					if (sval > 1600) sval = 7;
					else if (sval > 1100) sval = 6;
					else if (sval > 700) sval = 5;
					else if (sval > 300) sval = 3;
					else if (sval > 100) sval = 2;
					else sval = 1;
#endif
					special_k_idx = lookup_kind(tval, sval);

				} else if (rand_int(1000) < (mining + 2) / 2) {
				    if (!rand_int(5)) {
					tval = TV_GOLEM;
#if 1 /* maybe todo: use get_obj_num_prep_tval() instead of this hardcoding chances.. */
					sval = rand_int(mining) * rand_int(50);
					/* 0..7 - according to k_info.txt */
					if (sval > 2000) sval = 7;
					else if (sval > 1600) sval = 6;
					else if (sval > 1300) sval = 5;
					else if (sval > 1000) sval = 4;
					else if (sval > 900) sval = 3;
					else if (sval > 600) sval = 2;
					else if (sval > 300) sval = 1;
					else sval = 0;
#endif
					special_k_idx = lookup_kind(tval, sval);
				    }
				    else {
					tval = TV_RUNE;
					get_obj_num_hook = NULL;
					get_obj_num_prep_tval(tval, RESF_MID);
					special_k_idx = get_obj_num(getlevel(&p_ptr->wpos), RESF_MID);
				    }
				}
			}
			/* if in monster KILL_WALL form or via magic */
			else if (l_ptr && quiet_borer) {
				/* prepare to discover a special feature */
				if (rand_int(500) < ((l_ptr->flags1 & LF1_NO_LAVA) ? 0 : ((l_ptr->flags1 & LF1_LAVA) ? 50 : 3))) dug_feat = FEAT_SHAL_LAVA;
				else if (rand_int(500) < ((l_ptr->flags1 & LF1_NO_WATER) ? 0 : ((l_ptr->flags1 & LF1_WATER) ? 50 : 8))) dug_feat = FEAT_SHAL_WATER;
			}

#if 0
			/* Titanium */
			if (c_ptr->feat >= FEAT_PERM_EXTRA)
				msg_print(Ind, "This seems to be permanent rock.");
#else	// 0
			//do_cmd_tunnel_test(int y, int x, FALSE)
			/* Must be tunnelable */
			if (!(f_info[c_ptr->feat].flags1 & FF1_TUNNELABLE) ||
				(f_info[c_ptr->feat].flags1 & FF1_PERMANENT))
			{
				/* Message */
				msg_print(Ind, f_text + f_info[c_ptr->feat].tunnel);

				/* Nope */
				return;
			}
#endif	// 0

			/* No destroying of town layouts */
			else if (!allow_terraforming(wpos, FEAT_TREE)) {
				msg_print(Ind, "You may not tunnel in this area.");
				return;
			}

			/* Rubble */
			else if (c_ptr->feat == FEAT_RUBBLE) {
				/* Remove the rubble */
				if (power > rand_int(200) && twall(Ind, y, x)) {
					/* Message */
					msg_print(Ind, "You have removed the rubble.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rubble", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif

					/* Hack -- place an object - Not in town (Khazad becomes l00t source) */
					if (!istown(wpos)) {
						/* discovered a special feature? */
						if (dug_feat == FEAT_FOUNTAIN) {
							place_fountain(wpos, y, x);
							note_spot_depth(wpos, y, x);
							everyone_lite_spot(wpos, y, x);
							msg_print(Ind, "You have laid open a fountain!");
							s_printf("DIGGING: %s found a fountain.\n", p_ptr->name);
						} else if (dug_feat != FEAT_NONE) {
							cave_set_feat_live(wpos, y, x, dug_feat);
							if (dug_feat == FEAT_WAY_MORE || dug_feat == FEAT_WAY_LESS) {
								msg_print(Ind, "You have uncovered a stairway!");
								s_printf("DIGGING: %s found a staircase.\n", p_ptr->name);
							} else {
//								s_printf("DIGGING: %s found water/lava.\n", p_ptr->name);
							}
						} else if (special_k_idx && tval != TV_GOLEM) {
							invcopy(&forge, special_k_idx);
							apply_magic(wpos, &forge, -2, TRUE, TRUE, TRUE, FALSE, TRUE);
							forge.number = 1;
//							forge.level = ;
							forge.marked2 = ITEM_REMOVAL_NORMAL;
							msg_print(Ind, "You have found something!");
							drop_near(&forge, -1, wpos, y, x);
							if (tval == TV_CHEST)
								s_printf("DIGGING: %s found a chest.\n", p_ptr->name);
							else if (tval == TV_RUNE)
								s_printf("DIGGING: %s found a rune.\n", p_ptr->name);
							else
								s_printf("DIGGING: %s found a specific non-golem item.\n", p_ptr->name);
						} else if (rand_int(120) < 10 + mining) {
							place_object_restrictor = RESF_NONE;
#if 1
							generate_object(&forge, wpos, magik(mining), magik(mining / 10), FALSE, make_resf(p_ptr) | RESF_MID,
								default_obj_theme, p_ptr->luck);
							object_desc(0, o_name, &forge, TRUE, 3);
							s_printf("DIGGING: %s found item: %s.\n", p_ptr->name, o_name);
							drop_near(&forge, -1, wpos, y, x);
#else
							place_object(wpos, y, x, magik(mining), magik(mining / 10), FALSE, make_resf(p_ptr) | RESF_MID,
								default_obj_theme, p_ptr->luck, ITEM_REMOVAL_NORMAL);
							s_printf("DIGGING: %s found a random item.\n", p_ptr->name);
#endif
							if (player_can_see_bold(Ind, y, x))
								msg_print(Ind, "You have found something!");
						}
					}

					/* Notice */
					note_spot_depth(wpos, y, x);

					/* Display */
					everyone_lite_spot(wpos, y, x);
				} else {
					/* Message, keep digging */
					msg_print(Ind, "You dig in the rubble.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rubble", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			}

			else if (c_ptr->feat == FEAT_TREE) {
				/* mow down the vegetation */
				if (((power > wood_power ? power : wood_power) > rand_int(400)) && twall(Ind, y, x)) { /* 400 */
					/* Message */
					msg_print(Ind, "You hack your way through the vegetation.");
					if (p_ptr->prace == RACE_ENT)
						msg_print(Ind, "You have a bad feeling about it.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif

					if (special_k_idx && tval == TV_GOLEM && sval == SV_GOLEM_WOOD) {
						invcopy(&forge, special_k_idx);
						apply_magic(wpos, &forge, -2, TRUE, TRUE, TRUE, FALSE, TRUE);
						forge.number = 1;
//						forge.level = ;
						forge.marked2 = ITEM_REMOVAL_NORMAL;
						msg_print(Ind, "You have found something!");
						drop_near(&forge, -1, wpos, y, x);
						s_printf("DIGGING: %s found a wood piece.\n", p_ptr->name);
					}

					/* Notice */
					note_spot_depth(wpos, y, x);
					/* Display */
					everyone_lite_spot(wpos, y, x);
				} else {
					/* Message, keep digging */
					msg_print(Ind, "You attempt to clear a path.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			} else if (c_ptr->feat == FEAT_BUSH) {
				/* mow down the vegetation */
				if (((power > wood_power ? power : wood_power) > rand_int(300)) && twall(Ind, y, x)) { /* 400 */
					/* Message */
					msg_print(Ind, "You hack your way through the vegetation.");
					if (p_ptr->prace == RACE_ENT)
						msg_print(Ind, "You have a bad feeling about it.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif

					/* Notice */
					note_spot_depth(wpos, y, x);

					/* Display */
					everyone_lite_spot(wpos, y, x);
				} else {
					/* Message, keep digging */
					msg_print(Ind, "You attempt to clear a path.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			} else if (c_ptr->feat == FEAT_IVY) {
				/* mow down the vegetation */
				if (((power > wood_power ? power : wood_power) > rand_int(200)) && twall(Ind, y, x)) { /* 400 */
					/* Message */
					msg_print(Ind, "You hack your way through the vegetation.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif

					/* Notice */
					note_spot_depth(wpos, y, x);

					/* Display */
					everyone_lite_spot(wpos, y, x);
				} else {
					/* Message, keep digging */
					msg_print(Ind, "You attempt to clear a path.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			} else if (c_ptr->feat == FEAT_DEAD_TREE) {
				/* mow down the vegetation */
				if (((power > wood_power ? power : wood_power) > rand_int(300)) && twall(Ind, y, x)) { /* 600 */
					/* Message */
					msg_print(Ind, "You hack your way through the vegetation.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif

					/* Notice */
					note_spot_depth(wpos, y, x);

					/* Display */
					everyone_lite_spot(wpos, y, x);
				} else {
					/* Message, keep digging */
					msg_print(Ind, "You attempt to clear a path.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_tree", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			}

			/* Quartz / Magma */
//			else if (c_ptr->feat >= FEAT_MAGMA)
			/* Quartz / Magma / Sandwall */
			else if (((c_ptr->feat >= FEAT_MAGMA) &&
				(c_ptr->feat <= FEAT_QUARTZ_K)) ||
				((c_ptr->feat >= FEAT_SANDWALL) &&
				 (c_ptr->feat <= FEAT_SANDWALL_K))) {
				bool okay = FALSE;
				bool gold = FALSE;
				bool hard = FALSE;
				bool soft = FALSE;

				/* Found gold */
				if ((c_ptr->feat >= FEAT_MAGMA_H) &&
				    (c_ptr->feat <= FEAT_QUARTZ_K)) gold = TRUE;

				if ((c_ptr->feat == FEAT_SANDWALL_H) ||
				    (c_ptr->feat == FEAT_SANDWALL_K)) {
					gold = TRUE;
					soft = TRUE;
				} else
				/* Extract "quartz" flag XXX XXX XXX */
				if ((c_ptr->feat - FEAT_MAGMA) & 0x01) hard = TRUE;

				/* Quartz */
				if (hard) {
					okay = (power > 20 + rand_int(800)); /* 800 */
				}
				/* Sandwall */
				else if (soft) {
					okay = (power > 5 + rand_int(250));
				}
				/* Magma */
				else {
					okay = (power > 10 + rand_int(400)); /* 400 */
				}

				if (istown(wpos)) gold = FALSE;

				/* Success */
				if (okay && twall(Ind, y, x)) {
					msg_print(Ind, "You have finished the tunnel.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif

					/* Found treasure */
					if (gold) {
						if (special_k_idx && tval == TV_GOLEM && sval != SV_GOLEM_WOOD) {
							invcopy(&forge, special_k_idx);
							apply_magic(wpos, &forge, -2, TRUE, TRUE, TRUE, FALSE, TRUE);
							forge.number = 1;
//							forge.level = ;
							forge.marked2 = ITEM_REMOVAL_NORMAL;
							drop_near(&forge, -1, wpos, y, x);
							s_printf("DIGGING: %s found a metal piece.\n", p_ptr->name);
						} else {
							object_level = find_level;
							/* abuse tval and sval as simple counters */
							tval = rand_int(mining / 5);
							for (sval = 0; sval <= tval; sval++) {
								/* Place some gold */
								place_gold(wpos, y, x, 0);
							}
							object_level = old_object_level;
						}
						note_spot_depth(wpos, y, x);
						everyone_lite_spot(wpos, y, x);
						msg_print(Ind, "You have found something!");
					} else if (istown(wpos)) {
						/* nothing */
					/* found special feature */
					} else if (dug_feat == FEAT_FOUNTAIN) {
						if (magik(35)) {
							place_fountain(wpos, y, x);
							note_spot_depth(wpos, y, x);
							everyone_lite_spot(wpos, y, x);
							msg_print(Ind, "You have laid open a fountain!");
							s_printf("DIGGING: %s found a fountain.\n", p_ptr->name);
						}
					} else if (dug_feat != FEAT_NONE &&
					    dug_feat != FEAT_WAY_MORE &&
					    dug_feat != FEAT_WAY_LESS) {
						if (magik(100)) {
							cave_set_feat_live(wpos, y, x, dug_feat);
//							s_printf("DIGGING: %s found water/lava.\n", p_ptr->name);
						}
					} else if (!rand_int(10) && special_k_idx && (tval == TV_RUNE)) {
							invcopy(&forge, special_k_idx);
							apply_magic(wpos, &forge, -2, TRUE, TRUE, TRUE, FALSE, TRUE);
							forge.number = 1;
//							forge.level = ;
							forge.marked2 = ITEM_REMOVAL_NORMAL;
							msg_print(Ind, "You have found something!");
							drop_near(&forge, -1, wpos, y, x);
							s_printf("DIGGING: %s found a rune.\n", p_ptr->name);
					}
				}

				/* Failure */
				else {
					/* Message, continue digging */
					msg_print(Ind, f_text + f_ptr->tunnel);
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
#if 0
				/* Failure (quartz) */
				else if (hard) {
					/* Message, continue digging */
					msg_print(Ind, "You tunnel into the quartz vein.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}

				/* Failure (magma) */
				else {
					/* Message, continue digging */
					msg_print(Ind, "You tunnel into the magma vein.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
#endif	/* 0 */
			}

			/* Default to secret doors */
			else if (c_ptr->feat == FEAT_SECRET) {
#if 0
				/* Message, keep digging */
				msg_print(Ind, "You tunnel into the granite wall.");
				more = TRUE;
 #ifdef USE_SOUND_2010
				if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
 #endif
#else
				struct c_special *cs_ptr;
				int featm = c_ptr->feat; /* just to kill compiler warning */
				if ((cs_ptr = GetCS(c_ptr, CS_MIMIC))) {
					featm = cs_ptr->sc.omni;
				} else { /* Apply "mimic" field */
					featm = f_info[featm].mimic;
				}

				/* Is the mimicked feat un-tunnelable? */
				if (!(f_info[featm].flags1 & FF1_TUNNELABLE) ||
				    (f_info[featm].flags1 & FF1_PERMANENT)) {
					/* Message */
					msg_print(Ind, f_text + f_info[featm].tunnel);
					return;
				}


				/* hack: 'successful' tunnelling reveals the secret door */
			    	if (power > 40 + rand_int(1600)) { /* just assume 1600 as for Granite Wall */
                                        struct c_special *cs_ptr;
                                        msg_print(Ind, "You have found a secret door!");
                                        c_ptr->feat = FEAT_DOOR_HEAD + 0x00;
                                        /* Clear mimic feature */
                                        if ((cs_ptr = GetCS(c_ptr, CS_MIMIC))) cs_erase(c_ptr, cs_ptr);

                                        note_spot_depth(wpos, y, x);
                                        everyone_lite_spot(wpos, y, x);
			    	} else {
					msg_print(Ind, f_text + f_info[featm].tunnel);
					more = TRUE;
				}

 #ifdef USE_SOUND_2010
				/* sound: for now assume that only such features get mimicked that
				   would cause 'tunnel_rock' sfx, eg no trees or rubble. - C. Blue */
				if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
 #endif
#endif

				/* Set off trap */
				if (GetCS(c_ptr, CS_TRAPS)) {
#ifdef TRAP_REVEALS_DOOR
					struct c_special *cs_ptr;
#endif
					player_activate_door_trap(Ind, y, x);
					/* got disturbed! */
					more = FALSE;
#ifdef TRAP_REVEALS_DOOR
					/* Message */
					msg_print(Ind, "You have found a secret door.");
					/* Pick a door XXX XXX XXX */
					c_ptr->feat = FEAT_DOOR_HEAD + 0x00;
					/* Clear mimic feature */
					if ((cs_ptr = GetCS(c_ptr, CS_MIMIC))) cs_erase(c_ptr, cs_ptr);

					/* Notice */
					note_spot_depth(wpos, y, x);
					/* Redraw */
					everyone_lite_spot(wpos, y, x);
#endif
				}

				/* Hack -- Search */
				if (more) search(Ind);
			}
			/* Granite + misc (Ice..) */
			else if (c_ptr->feat >= FEAT_WALL_EXTRA) {
				/* Tunnel */
				if ((power > 40 + rand_int(1600)) && twall(Ind, y, x)) { /* 1600 */
					msg_print(Ind, "You have finished the tunnel.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif

					if (!istown(wpos)) {
						/* found special feature */
						if (dug_feat == FEAT_FOUNTAIN) {
							if (magik(50)) {
								place_fountain(wpos, y, x);
								note_spot_depth(wpos, y, x);
								everyone_lite_spot(wpos, y, x);
								msg_print(Ind, "You have laid open a fountain!");
								s_printf("DIGGING: %s found a fountain.\n", p_ptr->name);
							}
						} else if (dug_feat != FEAT_NONE &&
						    dug_feat != FEAT_WAY_MORE &&
						    dug_feat != FEAT_WAY_LESS) {
							if (magik(100)) {
								cave_set_feat_live(wpos, y, x, dug_feat);
//								s_printf("DIGGING: %s found water/lava.\n", p_ptr->name);
							}
						} else if (!rand_int(20) && special_k_idx && (tval == TV_RUNE)) {
							invcopy(&forge, special_k_idx);
							apply_magic(wpos, &forge, -2, TRUE, TRUE, TRUE, FALSE, TRUE);
							forge.number = 1;
//							forge.level = ;
							forge.marked2 = ITEM_REMOVAL_NORMAL;
							msg_print(Ind, "You have found something!");
							drop_near(&forge, -1, wpos, y, x);
							s_printf("DIGGING: %s found a rune.\n", p_ptr->name);
						}
					}
				}

				/* Keep trying */
				else {
					/* We may continue tunelling */
					//msg_print(Ind, "You tunnel into the granite wall.");
					msg_print(Ind, f_text + f_ptr->tunnel);
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rock", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			}

			/* Spider Webs */
			else if (c_ptr->feat == FEAT_WEB) {
				/* Tunnel - hack: swords/axes help similarly as for trees/bushes/ivy */
				if ((((power > wood_power) ? power : wood_power) > rand_int(100)) && twall(Ind, y, x)) {
					msg_print(Ind, "You have cleared the web.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rubble", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
				/* Keep trying */
				else {
					/* We may continue tunelling */
					msg_print(Ind, "You try to clear the spider web.");
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rubble", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			}

			/* Other stuff - Note: There is none left, actually.
			   Ice walls are treated as 'Granite Wall' above already! */
			else {
				/* Tunnel */
				if ((power > 30 + rand_int(1200)) && twall(Ind, y, x)) {
					msg_print(Ind, "You have finished the tunnel.");
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rubble", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}

				/* Keep trying */
				else {
					/* We may continue tunelling */
					msg_print(Ind, f_text + f_ptr->tunnel);
					more = TRUE;
#ifdef USE_SOUND_2010
					if (!quiet_borer) sound(Ind, "tunnel_rubble", NULL, SFX_TYPE_NO_OVERLAP, TRUE);
#endif
				}
			}

		}

		/* Notice "blockage" changes */
		if (old_floor != cave_floor_bold(zcave, y, x)) {
			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
		}
	} else { /* dir == 0 */
		/* we didn't dig into anything, but since we might still cause earthquakes,
		   we need to deduct one turn of energy appropriately:
		   We basically stroke the floor to cause an earthquake instead. */
		p_ptr->energy -= level_speed(&p_ptr->wpos);
	}

	/* Apply Earthquakes */
	if (p_ptr->inventory[INVEN_TOOL].k_idx) {
		u32b fx, f5;
		object_flags(o_ptr, &fx, &fx, &fx, &fx, &f5, &fx);
		if ((p_ptr->impact || (f5 & TR5_IMPACT)) &&
		    (randint(200) < power) && magik(50)
#ifdef ALLOW_NO_QUAKE_INSCRIPTION
		    && !check_guard_inscription(o_ptr->note, 'E')
#endif
		    ) {
			earthquake(&p_ptr->wpos, p_ptr->py, p_ptr->px, 10);
			break_cloaking(Ind, 0); /* redundant, done above already */
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);
		}
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(Ind, 0, 0);
	else if (p_ptr->always_repeat) p_ptr->command_rep = PKT_TUNNEL;
}


/*
 * Try to identify a trap when disarming it.	- Jir -
 */
static void do_id_trap(int Ind, int t_idx)
{
	player_type *p_ptr = Players[Ind];
	trap_kind *tr_ptr = &t_info[t_idx];
	int power;

	/* Already known? */
	if (p_ptr->trap_ident[t_idx]) return;

	/* Need proper skill */
//	if (get_skill(p_ptr, SKILL_DISARM) < 20) return;
//	if (!get_skill(p_ptr, SKILL_DISARM)) return;
	if (!get_skill(p_ptr, SKILL_TRAPPING)) return;

	/* Impossible? */
	if (tr_ptr->flags & FTRAP_NO_ID) return;
	if (UNAWARENESS(p_ptr) || no_lite(Ind)) return;

	/* Check for failure (hard) */
	power = (tr_ptr->difficulty + 2) * (tr_ptr->minlevel + 10) * 5;
	if (tr_ptr->flags & FTRAP_EASY_ID) power /= 10;

//	if (randint(power) > get_skill(p_ptr, SKILL_DISARM)) return;
	if (randint(power) > get_skill(p_ptr, SKILL_TRAPPING)) return;

	/* Good job */
	p_ptr->trap_ident[t_idx] = TRUE;
	/* Some traps seem unnamed atm - looks ugly */
	if (strlen(t_name + tr_ptr->name))
		msg_format(Ind, "You identified that trap as %s.", t_name + tr_ptr->name);

	/* Fair reward; so it's double exp when disarming AND
	   for the first time also IDing - C. Blue */
	gain_exp(Ind, TRAP_EXP(t_idx, getlevel(&p_ptr->wpos)));
}

/*
 * Disarms a trap, or chest     -RAK-
 */

/* A nice idea:
 * Disarming a trap will have a disarm_skill% chance of dropping a 
 * random trap kit? =) - the_sandman
 */
void do_cmd_disarm(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;

	int                 y, x, i, j, power;

	cave_type               *c_ptr;
	byte                    *w_ptr;
	object_type             *o_ptr;
	trap_kind *t_ptr;
	int t_idx = 0;

	bool more = FALSE, done = FALSE;
	cave_type **zcave;
	if (!(zcave = getcave(wpos))) return;

	/* Ghosts cannot disarm ; not in WRAITHFORM */
	if ((p_ptr->ghost) || (p_ptr->tim_wraith)) {
		msg_print(Ind, "You cannot disarm things!");
		if (!is_admin(p_ptr)) return;
	}

	break_shadow_running(Ind);

	/* Get a direction (or abort) */
	if (dir) {
		struct c_special *cs_ptr;
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &zcave[y][x];
		w_ptr = &p_ptr->cave_flag[y][x];

		/* Access the item */
		o_ptr = &o_list[c_ptr->o_idx];

		/* Access the trap */
		if ((cs_ptr = GetCS(c_ptr, CS_TRAPS)))
			t_idx = cs_ptr->sc.trap.t_idx;

		/* Nothing useful */
#if 0
		if (!((c_ptr->feat >= FEAT_TRAP_HEAD) &&
		      (c_ptr->feat <= FEAT_TRAP_TAIL)) &&
		    (o_ptr->tval != TV_CHEST))

//			!(c_ptr->special.sc.ptr->found)) &&
#endif	// 0

		if ((!t_idx || !cs_ptr->sc.trap.found) &&
		    (o_ptr->tval != TV_CHEST) &&
		    !(cs_ptr = GetCS(c_ptr, CS_MON_TRAP))) {
			/* Message */
			msg_print(Ind, "You see nothing there to disarm.");
			done = TRUE;
		}

		/* Monster in the way */
		else if (c_ptr->m_idx > 0) {
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Message */
			msg_print(Ind, "There is a monster in the way!");

			/* Attack */
			py_attack(Ind, y, x, TRUE);

			done = TRUE;
		}

		/* Normal disarm */
		else if (o_ptr->tval == TV_CHEST) {
			t_ptr = &t_info[o_ptr->pval];

			/* Disarm the chest */
//			more = do_cmd_disarm_chest(y, x, c_ptr->o_idx);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Get the "disarm" factor */
			i = p_ptr->skill_dis;

			/* Penalize some conditions */
			if (p_ptr->blind || no_lite(Ind)) i = i / 10;
			if (p_ptr->confused || p_ptr->image) i = i / 10;

			/* Extract the difficulty */
			j = i - t_ptr->difficulty * 3;

			/* Always have a small chance of success */
			if (j < 2) j = 2;

			/* Must find the trap first. */
			if (!object_known_p(Ind, o_ptr))
				msg_print(Ind, "I don't see any traps.");

			/* Already disarmed/unlocked */
			else if (o_ptr->pval <= 0)
				msg_print(Ind, "The chest is not trapped.");

			/* Success (get a lot of experience) */
			else if (rand_int(100) < j) {
				/* S(he) is no longer afk */
				un_afk_idle(Ind);

				msg_print(Ind, "You have disarmed the chest.");
#ifdef USE_SOUND_2010
				sound(Ind, "disarm", NULL, SFX_TYPE_COMMAND, FALSE);
#endif
				gain_exp(Ind, TRAP_EXP(o_ptr->pval, getlevel(&p_ptr->wpos)));
				o_ptr->pval = (0 - o_ptr->pval);
				do_id_trap(Ind, o_ptr->pval);
				done = TRUE;
			}

			/* Failure -- Keep trying */
			else if ((i > 5) && (randint(i) > 5)) {
				/* S(he) is no longer afk */
				un_afk_idle(Ind);

				/* We may keep trying */
				more = TRUE;
				done = TRUE;
				msg_print(Ind, "You failed to disarm the chest.");

				break_shadow_running(Ind);
				stop_precision(Ind);
				stop_shooting_till_kill(Ind);
			}

			/* Failure -- Set off the trap */
			else {
				/* S(he) is no longer afk */
				un_afk_idle(Ind);

				msg_print(Ind, "You set off a trap!");
				chest_trap(Ind, y, x, c_ptr->o_idx);
				break_cloaking(Ind, 0);
				break_shadow_running(Ind);
				stop_precision(Ind);
				stop_shooting_till_kill(Ind);
				done = TRUE;
			}

			/* XXX hrm it's ugly */
			if ((!t_idx || !cs_ptr->sc.trap.found) &&
					!(cs_ptr=GetCS(c_ptr, CS_MON_TRAP)))
				done = TRUE;
		}

		/* Disarm the trap */
/*		else if (c_ptr->feat == FEAT_MON_TRAP) */
		if (!done && cs_ptr->type == CS_MON_TRAP) { /* same thing.. */
			/* S(he) is no longer afk */
			un_afk_idle(Ind);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			break_shadow_running(Ind);
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);

			msg_print(Ind, "You disarm the monster trap.");
#ifdef USE_SOUND_2010
			sound(Ind, "disarm", NULL, SFX_TYPE_COMMAND, FALSE);
#endif
			do_cmd_disarm_mon_trap_aux(wpos, y, x);
			more = FALSE;
			done = TRUE;

		}

		/* Disarm a trap */
		if (!done && (t_idx && cs_ptr->sc.trap.found)) {
			cptr name;

			break_shadow_running(Ind);
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);

			/* Access trap name */
			if (p_ptr->trap_ident[t_idx])
				name = (t_name + t_info[t_idx].name);
			else
				name = "unknown trap";

//			cptr name = (f_name + f_info[c_ptr->feat].name);

			/* S(he) is no longer afk */
			un_afk_idle(Ind);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Get the "disarm" factor */
			i = p_ptr->skill_dis;

			/* Penalize some conditions */
			if (p_ptr->blind || no_lite(Ind)) i = i / 10;
			if (p_ptr->confused || p_ptr->image) i = i / 10;

			/* XXX XXX XXX Variable power? */

			/* Extract trap "power" */
			power = t_info[t_idx].difficulty * 3;
//			power = 5;

			/* Extract the difficulty */
			j = i - power;

			/* Always have a small chance of success */
			if (j < 2) j = 2;

			/* Success */
			if (rand_int(100) < j) {
				/* Message */
				msg_format(Ind, "You have disarmed the %s.", name);
#ifdef USE_SOUND_2010
				sound(Ind, "disarm", NULL, SFX_TYPE_COMMAND, FALSE);
#endif

				/* A chance to drop a trapkit; equal to the trapping skill */
				int sdis = (int)(p_ptr->s_info[SKILL_TRAPPING].value / 1000);
				if (magik(sdis)) {
					object_type forge;
					object_type* yay = &forge;
					invcopy(yay, lookup_kind(TV_TRAPKIT, randint(6)));

					/* Let's make it so that there is always a chance for an awesome trap-- 
					   more so the better you are at trapping (from 1% ... 6%). 
					   Since the trap appears ~50% of the time at max trapping, 
					   the total chance for this is about 3% */
					if (magik((sdis/10)+1)) {
						apply_magic(&p_ptr->wpos, yay, -2, TRUE, TRUE, TRUE, FALSE, TRUE);
						drop_near(yay, 0, &p_ptr->wpos, p_ptr->py, p_ptr->px);
						msg_print(Ind, "You have created a wonderful trapkit using pieces of the disarmed trap.");
					} else {
						apply_magic(&p_ptr->wpos, yay, -2, TRUE, FALSE, FALSE, FALSE, TRUE);
						drop_near(yay, 0, &p_ptr->wpos, p_ptr->py, p_ptr->px);
						msg_print(Ind, "You have fashioned a trapkit of a sort from the disarmed trap.");
					}
				}

				/* Reward */
				gain_exp(Ind, TRAP_EXP(t_idx, getlevel(&p_ptr->wpos)));

				/* Try to identify it */
				do_id_trap(Ind, t_idx);

				/* Remove the trap */
				cs_erase(c_ptr, cs_ptr);
//				c_ptr->feat = FEAT_FLOOR;

#if 1
				/* Forget the "field mark" */
				everyone_forget_spot(wpos, y, x);

				/* Notice */
				note_spot_depth(wpos, y, x);

				/* Redisplay the grid */
				everyone_clear_ovl_spot(wpos, y, x);
#endif

				/* move the player onto the trap grid
				   except if it was a trap on a closed door,
				   since we might not want to open it yet,
				   especially while being cloaked */
				if (dir != 5 &&
				    !(c_ptr->feat >= FEAT_DOOR_HEAD &&
				    c_ptr->feat <= FEAT_DOOR_TAIL))
					move_player(Ind, dir, FALSE, NULL);
				else
					everyone_lite_spot(wpos, y, x);
			}

			/* Failure -- Keep trying */
			else if ((i > 5) && (randint(i) > 5)) {
				break_shadow_running(Ind);
				stop_precision(Ind);
				stop_shooting_till_kill(Ind);

				/* Message */
				msg_format(Ind, "You failed to disarm the %s.", name);

				/* We may keep trying */
				more = TRUE;
			}

			/* Failure -- Set off the trap */
			else {
				/* Message */
				msg_format(Ind, "You set off the %s!", name);

				/* Move the player onto the trap */
				if (dir != 5) move_player(Ind, dir, FALSE, NULL); /* moving doesn't 100% imply setting it off */
				else hit_trap(Ind); /* but we can allow this weakness, assuming that you are less likely to get hit if you stand besides the trap instead of right on it */
				break_cloaking(Ind, 0);
				break_shadow_running(Ind);
				stop_precision(Ind);
				stop_shooting_till_kill(Ind);
			}

			/* only aesthetic */
			done = TRUE;
		}
	}

	/* Cancel repeat unless told not to */
	if (!more) disturb(Ind, 0, 0);
	else if (p_ptr->always_repeat) p_ptr->command_rep = PKT_DISARM;
}


/*
 * Bash open a door, success based on character strength
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 *
 * For an open door, pval is positive for a broken door.
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Breatures can also open or bash doors, see elsewhere.
 *
 * We need to use character body weight for something, or else we need
 * to no longer give female characters extra starting gold.
 */
void do_cmd_bash(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;
	monster_race *r_ptr = &r_info[p_ptr->body_monster];

	int                 y, x;
	int                     bash, temp, num;

	cave_type               *c_ptr;

	bool            more = FALSE, water = FALSE, tiny_bash;
	cave_type **zcave;


	if (!(zcave = getcave(wpos))) return;

	/* Ghosts cannot bash ; not in WRAITHFORM */
	if (p_ptr->ghost || p_ptr->tim_wraith) {
		/* Message */
		msg_print(Ind, "You cannot bash things!");
		if (!is_admin(p_ptr)) return;
	}

	tiny_bash = ((p_ptr->fruit_bat && !p_ptr->body_monster) ||
	    (p_ptr->body_monster && !(r_ptr->flags2 & RF2_BASH_DOOR)));

	/* Get a "repeated" direction */
	if (dir) {
		/* Bash location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid */
		c_ptr = &zcave[y][x];

		/* for leaderless guild houses */
		if ((zcave[y][x].info & CAVE_GUILD_SUS)) return;

		if (c_ptr->feat == FEAT_DEEP_WATER ||
		    c_ptr->feat == FEAT_SHAL_WATER)
			tiny_bash = water = TRUE;

		/* Monster in the way */
		if (c_ptr->m_idx > 0) {
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Message */
//			msg_print(Ind, "There is a monster in the way!");

			/* Attack */
			py_attack(Ind, y, x, TRUE);
		}

		/* Nothing useful */
		else if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		    (c_ptr->feat <= FEAT_DOOR_TAIL)) &&
		    c_ptr->feat != FEAT_HOME)
		{
			int item;
			/* Hack -- 'kick' an item ala NetHack */
			if ((item = c_ptr->o_idx)) {
				object_type *o_ptr = &o_list[c_ptr->o_idx];

				if (nothing_test(o_ptr, p_ptr, &p_ptr->wpos, x, y)) return;

#if 0 /* allow water-bashing of 1 grid range, same as for fruit bats now - C. Blue */
				if (water) {
					/* S(he) is no longer afk */
					un_afk_idle(Ind);

					/* Take a turn */
					p_ptr->energy -= level_speed(&p_ptr->wpos);

					msg_print(Ind, "Splash!");
					break_cloaking(Ind, 0);
					break_shadow_running(Ind);
					stop_precision(Ind);
					stop_shooting_till_kill(Ind);
					return;
				}
#endif

				/* Potions smash open */
				if (o_ptr->tval == TV_POTION ||
						o_ptr->tval == TV_POTION2 ||
						o_ptr->tval == TV_FLASK ||
						o_ptr->tval == TV_BOTTLE)
				{
					char            o_name[ONAME_LEN];
					object_desc(Ind, o_name, o_ptr, FALSE, 3);

					/* S(he) is no longer afk */
					un_afk_idle(Ind);

					/* Take a turn */
					p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;

					/* Message */
					/* TODO: handle blindness */
					msg_format_near_site(y, x, wpos, 0, TRUE, "The %s shatters!", o_name);
#ifdef USE_SOUND_2010
					sound_near_site(y, x, wpos, 0, "shatter_potion", NULL, SFX_TYPE_MISC, FALSE);
#endif

					//			if (potion_smash_effect(0, wpos, y, x, o_ptr->sval))
					if (k_info[o_ptr->k_idx].tval == TV_POTION)
						/* This should harm the player too, but for now no way :/ */
						potion_smash_effect(0 - Ind, wpos, y, x, o_ptr->sval);

					num = 1;//o_ptr->number;
					floor_item_increase(item, -num);
					floor_item_optimize(item);

					break_cloaking(Ind, 0);
					break_shadow_running(Ind);
					stop_precision(Ind);
					stop_shooting_till_kill(Ind);
					return;
				}

				if (water) msg_print(Ind, "Splash!");
				do_cmd_throw(Ind, dir, 0 - c_ptr->o_idx, tiny_bash ? 2 : 1);

				/* Set off trap (Hack -- handle like door trap) */
				if(GetCS(c_ptr, CS_TRAPS)) player_activate_door_trap(Ind, y, x);

				return;
			}
			else
			/* Message */
			msg_print(Ind, "You see nothing there to bash.");
		}

		/* Bash a closed door */
		else {
			if ((p_ptr->fruit_bat && !p_ptr->body_monster) ||
			    (p_ptr->body_monster && !(r_ptr->flags2 & RF2_BASH_DOOR)))
			{
				msg_print(Ind, "You cannot bash doors!");
				if (!is_admin(p_ptr)) return;
			}

			/* S(he) is no longer afk */
			un_afk_idle(Ind);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Message */
			msg_print(Ind, "You smash into the door!");

			/* Hack -- Bash power based on strength */
			/* (Ranges from 3 to 20 to 100 to 200) */
			bash = adj_str_blow[p_ptr->stat_ind[A_STR]];

			/* Extract door power */
			temp = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

			/* Compare bash power to door power XXX XXX XXX */
			temp = (bash - (temp * 10));

			/* Hack -- always have a chance */
			if (temp < 1) temp = 1;

			/* Hack -- attempt to bash down the door */
			if (rand_int(100) < temp && c_ptr->feat != FEAT_HOME) {
				/* Message */
				msg_print(Ind, "The door crashes open!");

#ifdef USE_SOUND_2010
				sound(Ind, "bash_door_break", NULL, SFX_TYPE_COMMAND, TRUE);
#endif

				/* Set off trap */
				if (GetCS(c_ptr, CS_TRAPS)) player_activate_door_trap(Ind, y, x);

				/* Break down the door */
				if (rand_int(100) < 50) c_ptr->feat = FEAT_BROKEN;
				/* Open the door */
				else c_ptr->feat = FEAT_OPEN;

				/* Notice */
				note_spot_depth(wpos, y, x);

				/* Redraw */
				everyone_lite_spot(wpos, y, x);

				/* Hack -- Fall through the door */
				move_player(Ind, dir, FALSE, NULL);

				/* Update some things */
				p_ptr->update |= (PU_VIEW | PU_LITE);
				p_ptr->update |= (PU_DISTANCE);
			}

			/* Saving throw against stun */
			else if (rand_int(100) < adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
				 p_ptr->lev)
			{
				/* Message */
				msg_print(Ind, "The door holds firm.");

#ifdef USE_SOUND_2010
				sound(Ind, "bash_door_hold", NULL, SFX_TYPE_COMMAND, TRUE);
#endif

				/* Allow repeated bashing */
				more = TRUE;
			}

			/* High dexterity yields coolness */
			else {
				/* Message */
				msg_print(Ind, "You are off-balance.");

#ifdef USE_SOUND_2010
				sound(Ind, "bash_door_hold", NULL, SFX_TYPE_COMMAND, TRUE);
#endif

				/* Hack -- Lose balance ala paralysis */
				(void)set_paralyzed(Ind, p_ptr->paralyzed + 2 + rand_int(2));
			}
			break_cloaking(Ind, 0);
			break_shadow_running(Ind);
			stop_precision(Ind);
			stop_shooting_till_kill(Ind);
		}
	}

	/* Unless valid action taken, cancel bash */
	if (!more) disturb(Ind, 0, 0);
	else if (p_ptr->always_repeat) p_ptr->command_rep = PKT_BASH;
}



/*
 * Find the index of some "spikes", if possible.
 *
 * XXX XXX XXX Let user choose a pile of spikes, perhaps?
 */
/* Now this can be used for any tvals.	- Jir - */
//static bool get_spike(int Ind, int *ip)
bool get_something_tval(int Ind, int tval, int *ip)
{
	player_type *p_ptr = Players[Ind];

	int i;

	/* Check every item in the pack */
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &(p_ptr->inventory[i]);

		/* Check the "tval" code */
		if (o_ptr->tval == tval)
		{
			if (!can_use(Ind, o_ptr)) continue;

			/* Save the spike index */
			(*ip) = i;

			/* Success */
			return (TRUE);
		}
	}

	/* Oops */
	return (FALSE);
}



/*
 * Jam a closed door with a spike
 *
 * This command may NOT be repeated
 */
void do_cmd_spike(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;

	int                  y, x, item;

	cave_type               *c_ptr;
	cave_type **zcave;
	if (!(zcave = getcave(wpos))) return;

	/* Ghosts cannot spike ; not in WRAITHFORM */
	if ((p_ptr->ghost) || (p_ptr->tim_wraith)) {
		/* Message */
		msg_print(Ind, "You cannot spike doors!");

		return;
	}

	/* Get a "repeated" direction */
	if (dir) {
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &zcave[y][x];

		/* Require closed door */
		if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		      (c_ptr->feat <= FEAT_DOOR_TAIL))) {
			/* Message */
			msg_print(Ind, "You see nothing there to spike.");
		}

		/* Get a spike */
//		else if (!get_spike(Ind, &item))
		else if (!get_something_tval(Ind, TV_SPIKE, &item)) {
			/* Message */
			msg_print(Ind, "You have no spikes!");
		}

		/* Is a monster in the way? */
		else if (c_ptr->m_idx > 0) {
			/* S(he) is no longer afk */
			un_afk_idle(Ind);

			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Message */
			msg_print(Ind, "There is a monster in the way!");

			/* Attack */
			py_attack(Ind, y, x, TRUE);
		}

		/* Go for it */
		else {
			/* Take a turn */
			p_ptr->energy -= level_speed(&p_ptr->wpos);

			/* Successful jamming */
			msg_print(Ind, "You jam the door with a spike.");

			/* Set off trap */
			if (GetCS(c_ptr, CS_TRAPS)) player_activate_door_trap(Ind, y, x);

			/* Convert "locked" to "stuck" XXX XXX XXX */
			if (c_ptr->feat < FEAT_DOOR_HEAD + 0x08) c_ptr->feat += 0x08;

			/* Add one spike to the door */
			if (c_ptr->feat < FEAT_DOOR_TAIL) c_ptr->feat++;

			/* Use up, and describe, a single spike, from the bottom */
			inven_item_increase(Ind, item, -1);
			inven_item_describe(Ind, item);
			inven_item_optimize(Ind, item);
		}
	}
}



/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int Ind, int dir, int pickup)
{
	player_type *p_ptr = Players[Ind];
	cave_type *c_ptr;

	bool more = FALSE;
	cave_type **zcave;
	if(!(zcave = getcave(&p_ptr->wpos))) return;

	if (!p_ptr->warning_numpadmove &&
	    (dir == 1 || dir == 3 || dir == 7 || dir == 9))
		p_ptr->warning_numpadmove = 1;

	/* Make sure he hasn't just switched levels */
	if (p_ptr->new_level_flag) return;

	/* Get a "repeated" direction */
	if (dir) {
		/* Hack -- handle confusion */
		if (p_ptr->confused) {
			dir = 5;

			/* Prevent walking nowhere */
			while (dir == 5)
				dir = rand_int(9) + 1;
		}

		/* Handle the cfg.door_bump_open option */
		if (cfg.door_bump_open) {
			struct c_special *cs_ptr;
			/* Get requested grid */
			c_ptr = &zcave[p_ptr->py+ddy[dir]][p_ptr->px+ddx[dir]];

			/* This should be cfg.trap_bump_disarm? */
			if (cfg.door_bump_open & BUMP_OPEN_TRAP &&
			    p_ptr->easy_disarm &&
			    (cs_ptr=GetCS(c_ptr, CS_TRAPS)) &&
			    cs_ptr->sc.trap.found &&
			    !c_ptr->o_idx &&
			    !UNAWARENESS(p_ptr) &&
			    !no_lite(Ind) )
			{
				do_cmd_disarm(Ind, dir);
				return;
			}

			if (cfg.door_bump_open & BUMP_OPEN_DOOR &&
				p_ptr->easy_open &&
				(c_ptr->feat >= FEAT_DOOR_HEAD) && 
				(c_ptr->feat <= FEAT_DOOR_TAIL) &&
				!p_ptr->ghost && !p_ptr->tim_wraith) /* players in WRAITHFORM can't open doors - mikaelh */
			{
				do_cmd_open(Ind, dir);
				return;
			}
			else
			if (cfg.door_bump_open & BUMP_OPEN_DOOR &&
				p_ptr->easy_open &&
				(c_ptr->feat == FEAT_HOME_HEAD) &&
				!p_ptr->ghost && !p_ptr->tim_wraith) /* players in WRAITHFORM can't open doors - mikaelh */
			{
				if ((cs_ptr = GetCS(c_ptr, CS_DNADOOR))){ /* orig house failure */
					if ((!(cfg.door_bump_open & BUMP_OPEN_HOUSE) ||
					    !access_door(Ind, cs_ptr->sc.ptr, FALSE)) &&
					    !admin_p(Ind)) {
						do_cmd_open(Ind, dir);
						return;
					}
				}
			}
		}
		/* Actually move the character */
		move_player(Ind, dir, pickup, NULL);
		p_ptr->warning_autoret_ok = 0;

		/* Take a turn */
		if (!(p_ptr->melee_sprint || p_ptr->shadow_running)) {
			if (p_ptr->mode & MODE_PVP)
				p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;
			else
				p_ptr->energy -= level_speed(&p_ptr->wpos);
		} else {
			if (p_ptr->mode & MODE_PVP)
				p_ptr->energy -= level_speed(&p_ptr->wpos) / 4;
			else
				p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;
		}

		/* Allow more walking */
		more = TRUE;
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(Ind, 0, 0);
}



/*
 * Start running.
 */
/* Hack -- since this command has different cases of energy requirements and
 * if we don't have enough energy sometimes we want to queue and sometimes we
 * don't, we do all of the energy checking within this function.  If after all
 * is said and done we want to queue the command, we return a 0.  If we don't,
 * we return a 2.
 */
int do_cmd_run(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	cave_type *c_ptr, **zcave;

	/* slower 'running' movement over certain terrain */
	int real_speed = cfg.running_speed;
	if(!(zcave = getcave(&p_ptr->wpos))) return(FALSE);
	c_ptr = &zcave[p_ptr->py][p_ptr->px];

	if (!p_ptr->warning_numpadmove &&
	    (dir == 1 || dir == 3 || dir == 7 || dir == 9))
		p_ptr->warning_numpadmove = 1;

	eff_running_speed(&real_speed, p_ptr, c_ptr);
#if 1 /* NEW_RUNNING_FEAT */
	/* running over floor grids, or special grids that we couldn't run over without according ability? Used by see_wall !*/
	p_ptr->running_on_floor = FALSE;
#endif

	/* Get a "repeated" direction */
	if (dir) {
		/* Make sure we have an empty space to run into */
		if (see_wall(Ind, dir, p_ptr->py, p_ptr->px)) {
			/* Handle the cfg_door_bump option */
			if (cfg.door_bump_open && p_ptr->easy_open && !p_ptr->ghost && !p_ptr->tim_wraith) /* players in WRAITHFORM can't open doors - mikaelh */
			{
				/* Get requested grid */
				c_ptr = &zcave[p_ptr->py+ddy[dir]][p_ptr->px+ddx[dir]];

				if (((c_ptr->feat >= FEAT_DOOR_HEAD) && 
				      (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
				    ((c_ptr->feat == FEAT_HOME)))
				{
					/* Check if we have enough energy to open the door */
					if (p_ptr->energy >= level_speed(&p_ptr->wpos)) {
						/* If so, open it. */
						do_cmd_open(Ind, dir);
					}
					return 2;
				}
			}

			/* Message */
			msg_print(Ind, "You cannot run in that direction.");

			/* Disturb */
			disturb(Ind, 0, 0);

			return 2;
		}

		/* Make sure we have enough energy to start running */
		if (p_ptr->energy >= (level_speed(&p_ptr->wpos) * (real_speed + 1)) / real_speed)
//		if (p_ptr->energy >= level_speed(&p_ptr->wpos)) /* otherwise auto-retaliation will never allow running */
		{
			char consume_full_energy;

			/* Hack -- Set the run counter */
			p_ptr->running = 20000; //enough to cross the world horizontally (was 1000)

			/* First step */
			run_step(Ind, dir, &consume_full_energy);

			if (consume_full_energy) {
				/* Consume the normal full amount of energy in case we have e.g. attacked a monster */
				p_ptr->energy -= level_speed(&p_ptr->wpos);
			} else {
				/* Reset the player's energy so he can't sprint several spaces
				 * in the first round of running.  */
				p_ptr->energy = level_speed(&p_ptr->wpos);
			}
			return 2;
		}
		/* If we don't have enough energy to run and monsters aren't around,
		 * try to queue the run command.
		 */
		else return 0;
	}
	return 2;
}



/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_stay(int Ind, int pickup)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;
	cave_type *c_ptr;
	cave_type **zcave;
	if(!(zcave=getcave(wpos))) return;
	
	if (p_ptr->new_level_flag) return;

	c_ptr = &zcave[p_ptr->py][p_ptr->px];

/* We don't want any of this */
#if 0
	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

	/* Spontaneous Searching */
	if ((p_ptr->skill_fos >= 75) || (0 == rand_int(76 - p_ptr->skill_fos)))
	{
		search(Ind);
	}

	/* Continuous Searching */
	if (p_ptr->searching)
	{
		search(Ind);
	}
#endif


	/* Hack -- enter a store if we are on one */
	if (c_ptr->feat == FEAT_SHOP)
#if 0
	if ((c_ptr->feat >= FEAT_SHOP_HEAD) &&
	    (c_ptr->feat <= FEAT_SHOP_TAIL))
#endif	// 0
	{
		/* Disturb */
		disturb(Ind, 0, 0);

		/* Hack -- enter store */
		command_new = '_';
	}


	/* Try to Pick up anything under us */
	carry(Ind, pickup, 1);
}

/*
 * Determines the odds of an object breaking when thrown at a monster
 *
 * Note that artifacts never break, see the "drop_near()" function.
 */
int breakage_chance(object_type *o_ptr) {
	/* artifacts never break */
	if (artifact_p(o_ptr)) return 0;

	/* Special: Light armour and shields seldom break */
	if (is_textile_armour(o_ptr->tval, o_ptr->sval)) return 2;
	if (o_ptr->tval == TV_SHIELD) return 2;

	/* Examine the item type */
	switch (o_ptr->tval) {
	/* Always break */
	case TV_FLASK:
	case TV_POTION:
	case TV_POTION2:
	case TV_BOTTLE:
		return 100;

	/* Often break */
	case TV_FOOD:
	case TV_JUNK:
	case TV_LITE:
	case TV_SKELETON:
	case TV_TRAPKIT:
		return 50;

	/* Sometimes break */
	//case TV_SPIKE:
	case TV_SCROLL:
	case TV_WAND:
		return 25;

	case TV_SHOT:
	case TV_BOLT:
	case TV_ARROW:
		if (o_ptr->sval == SV_AMMO_MAGIC && !cursed_p(o_ptr)) return (0);
		else if (o_ptr->name2 == EGO_ETHEREAL || o_ptr->name2b == EGO_ETHEREAL) return (10);
		else if (o_ptr->tval == TV_SHOT) return (10);
		else if (o_ptr->tval == TV_BOLT) return (15);
		return (20);

	/* seldom break */
	case TV_BOOMERANG:
	case TV_FIRESTONE:
	case TV_RING:
	case TV_AMULET:
	//case TV_RUNE:
	case TV_GEM:
		return 2;

	/* never break */
	case TV_GAME:
	case TV_GOLD:
	case TV_KEY:
		return 0;

	/* special */
	case TV_GOLEM:
		if (o_ptr->sval <= SV_GOLEM_ADAM) return 0; /* massive piece */
		if (o_ptr->sval >= SV_GOLEM_ATTACK) return 25; /* scroll */
		return 10; /* arm/leg */
	}

	/* Default: Rarely break */
	/* (eg rod, staff, mstaff, weapon, other armour, tool) */
	return 10;
}

/* Add a nice ball if needed */
static void do_arrow_brand_effect(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];

	switch (p_ptr->bow_brand_t) {
	case BRAND_BALL_FIRE:
		project(0 - Ind, 2, &p_ptr->wpos, y, x, 30, GF_FIRE, PROJECT_NORF | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, "");
		break;
	case BRAND_BALL_COLD:
		project(0 - Ind, 2, &p_ptr->wpos, y, x, 35, GF_COLD, PROJECT_NORF | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, "");
		break;
	case BRAND_BALL_ELEC:
		project(0 - Ind, 2, &p_ptr->wpos, y, x, 40, GF_ELEC, PROJECT_NORF | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, "");
		break;
	case BRAND_BALL_ACID:
		project(0 - Ind, 2, &p_ptr->wpos, y, x, 45, GF_ACID, PROJECT_NORF | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, "");
		break;
	case BRAND_BALL_SOUN:
		project(0 - Ind, 2, &p_ptr->wpos, y, x, 30, GF_SOUND, PROJECT_NORF | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, "");
		break;
	case BRAND_VORP:
		/* Nothing here */
		break;
	case BRAND_CHAO:
		/* XXX This allows the target player to 'dodge' the effect... */
		project(0 - Ind, 0, &p_ptr->wpos, y, x, 1, GF_CONFUSION, PROJECT_NORF | PROJECT_JUMP | PROJECT_KILL, "");
		break;
	}
}

/* Exploding arrow */
//static void do_arrow_explode(int Ind, object_type *o_ptr, int y, int x)
void do_arrow_explode(int Ind, object_type *o_ptr, worldpos *wpos, int y, int x, int might)
{
//	player_type *p_ptr = Players[Ind];
	int rad = 0;
//	int dam = (damroll(o_ptr->dd, o_ptr->ds) + o_ptr->to_d) * 2 * ((might/3)+1);
//	int dam = (damroll(o_ptr->dd, o_ptr->ds) + o_ptr->to_d) * 4;
//	int dam = (damroll(o_ptr->dd, o_ptr->ds) + 5) * 3 + o_ptr->to_d;
	int dam = (damroll(o_ptr->dd, o_ptr->ds) + 10) * 2 + o_ptr->to_d;
	int flag = PROJECT_NORF | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP;

	switch(o_ptr->sval)
	{
		case SV_AMMO_LIGHT: rad = 1; break;
		case SV_AMMO_NORMAL: rad = 2; break;
		case SV_AMMO_HEAVY: rad = 3; break;
		//case SV_AMMO_MAGIC <- magic arrows only, don't explode
		case SV_AMMO_SILVER: rad = 2; break;
	}
	
	if ((rad > 2) && (
	    (o_ptr->pval == GF_KILL_WALL) ||
	    (o_ptr->pval == GF_DISINTEGRATE) ||
	    (o_ptr->pval == GF_AWAY_ALL) ||
	    (o_ptr->pval == GF_TURN_ALL) ||
	    (o_ptr->pval == GF_NEXUS) ||
	    (o_ptr->pval == GF_GRAVITY) ||
	    (o_ptr->pval == GF_TIME) ||
	    (o_ptr->pval == GF_STUN)))
		rad = 2;

	project(0 - Ind, rad, wpos, y, x, dam, o_ptr->pval, flag, "");
}

/*
 * Return multiplier of an object
 */
int get_shooter_mult(object_type *o_ptr)
{
	/* Assume a base multiplier */
	int tmul = 1;

	/* Analyze the launcher */
	switch (o_ptr->sval)
	{
		case SV_SLING:
		{
			/* Sling and ammo */
			tmul = 2;
			break;
		}

		case SV_SHORT_BOW:
		{
			/* Short Bow and Arrow */
			tmul = 2;
			break;
		}

		case SV_LONG_BOW:
		{
			/* Long Bow and Arrow */		
			tmul = 3;
			break;
		}

		/* Light Crossbow and Bolt */
		case SV_LIGHT_XBOW:
		{
			tmul = 3;
			break;
		}

		/* Heavy Crossbow and Bolt */
		case SV_HEAVY_XBOW:
		{
			tmul = 4;
			break;
		}
	}
	return tmul;
}


/* Turn off afk mode? Or is this just auto-retaliation. */
bool retaliating_cmd = FALSE;

/*
 * Fire an object from the pack or floor.
 *
 * You may only fire items that "match" your missile launcher.
 *
 * You must use slings + pebbles/shots, bows + arrows, xbows + bolts.
 *
 * See "calc_boni()" for more calculations and such.
 *
 * Note that "firing" a missile is MUCH better than "throwing" it.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Objects are more likely to break if they "attempt" to hit a monster.
 *
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 *
 * The "extra shot" code works by decreasing the amount of energy
 * required to make each shot, spreading the shots out over time.
 *
 * Note that when firing missiles, the launcher multiplier is applied
 * after all the bonuses are added in, making multipliers very useful.
 *
 * Note that Bows of "Extra Might" get extra range and an extra bonus
 * for the damage multiplier.
 *
 * Note that Bows of "Extra Shots" give an extra shot.
 */
/* Added a lot of hacks to handle boomerangs.	- Jir - */
/* Added another lot of hacks to handle quiver-slot.	- Jir - */
/* XXX it's... way too dirty. consider using 'project()' */
void do_cmd_fire(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind], *q_ptr;
	struct worldpos *wpos=&p_ptr->wpos;

	int                     i, j, y, x, ny, nx, ty, tx, bx, by;
#ifdef PY_FIRE_ON_WALL
	int			prev_x, prev_y; /* for flare missile being able to light up a room under projectable_wall() conditions */
#endif
	int                     tdam, tdis, thits, tmul;
	int                     bonus, chance, tries = 100;
	int                     cur_dis, visible, real_dis;
	int breakage = 0, num_ricochet = 0, ricochet_chance = 0;
	int item = INVEN_AMMO;
	int archery = get_archery_skill(p_ptr);

	object_type         throw_obj;
	object_type             *o_ptr;
	object_type             *j_ptr;
	bool            hit_body = FALSE;

	int                     missile_attr;
	int                     missile_char;

#ifdef OPTIMIZED_ANIMATIONS
	/* Projectile path */
	int path_y[MAX_RANGE];
	int path_x[MAX_RANGE];
	int path_num = 0;
#endif
	
	char brand_msg[MAX_CHARS] = { '\0' };

        bool            drain_msg = TRUE;
        int             drain_result = 0, drain_heal = 0;
        int             drain_left = MAX_VAMPIRIC_DRAIN_RANGED;
	bool		drainable = TRUE;
	bool		ranged_double_real = FALSE, ranged_flare_body = FALSE;
	
	int		break_chance;
	int		sfx = 0;
	u32b f1, f1a, fx, esp;

	char            o_name[ONAME_LEN];
	bool returning = FALSE, magic = FALSE, boomerang = FALSE, ethereal = FALSE;
	bool target_ok = target_okay(Ind);
#if 0 //DEBUG
#ifdef USE_SOUND_2010
	int otval;
#endif
#endif
	cave_type **zcave;
	if (!(zcave = getcave(wpos))) return;

	/* New '+' feat in 4.4.6.2 */
	if (dir == 11) {
		get_aim_dir(Ind);
		p_ptr->current_fire = 1;
		return;
	}

	if (p_ptr->shooting_till_kill) { /* we were shooting till kill last turn? */
		p_ptr->shooting_till_kill = FALSE; /* well, gotta re-test for another success now.. */
		if (dir == 5) p_ptr->shooty_till_kill = TRUE; /* so for now we are just ATTEMPTING to shoot till kill (assumed we have a monster for target) */
	}

	/* To continue shooting_till_kill, require clean LOS to target
	   with no other monsters in the way, so we won't wake up more monsters accidentally. */
	if (p_ptr->shooty_till_kill) {
		if (target_ok) {
#ifndef PY_FIRE_ON_WALL
			if (!projectable_real(Ind, p_ptr->py, p_ptr->px, p_ptr->target_row, p_ptr->target_col, MAX_RANGE)) {
#else
			if (!projectable_wall_real(Ind, p_ptr->py, p_ptr->px, p_ptr->target_row, p_ptr->target_col, MAX_RANGE)) {
#endif
				return;
			}
		} else {
			/* don't fire-till-kill at ourself! */
			p_ptr->shooty_till_kill = FALSE;
		}
	}

	/* Break goi/manashield */
#if 0
	if (p_ptr->invuln) {
		set_invuln(Ind, 0);
	}
	if (p_ptr->tim_manashield) {
		set_tim_manashield(Ind, 0);
	}
	if (p_ptr->tim_wraith) {
		set_tim_wraith(Ind, 0);
	}
#endif	// 0

	/* Get the "bow" (if any) */
	j_ptr = &(p_ptr->inventory[INVEN_BOW]);

	/* if no launcher is equipped check for boomerangs
	   inscribed '!L' in inventory - C. Blue */
	if ((!j_ptr->tval || !j_ptr->number) && item_tester_hook_wear(Ind, INVEN_BOW)) {
		for (i = 0; i < INVEN_PACK; i++) {
			if (p_ptr->inventory[i].tval == TV_BOOMERANG &&
			    check_guard_inscription(p_ptr->inventory[i].note, 'L')) {
				do_cmd_wield(Ind, i, 0x0);
				break;
			}
		}
	}

	/* Require a launcher */
	if (!j_ptr->tval) {
		msg_print(Ind, "You have nothing to fire with.");
		return;
	}

	/* Extract the item flags */
	object_flags(j_ptr, &f1, &fx, &fx, &fx, &fx, &esp);

	if (j_ptr->tval == TV_BOOMERANG) {
		boomerang = TRUE;
		item = INVEN_BOW;
	}

	o_ptr = &(p_ptr->inventory[item]);

	/* if quiver is empty, try auto-reloading with items
	   inscribed '!L' in inventory - C. Blue */
	if (!o_ptr->tval || !o_ptr->number) {
		for (i = 0; i < INVEN_PACK; i++) {
			if (is_ammo(p_ptr->inventory[i].tval) &&
			    check_guard_inscription(p_ptr->inventory[i].note, 'L')) {
				do_cmd_wield(Ind, i, 0x0);
				break;
			}
		}
	}

	/* ethereal ammo? */
	if (o_ptr->name2 == EGO_ETHEREAL || o_ptr->name2b == EGO_ETHEREAL) {
		returning = TRUE;
		ethereal = TRUE;
	}

	if( check_guard_inscription( o_ptr->note, 'f' )) {
		msg_print(Ind, "The item's inscription prevents it.");
		return;
	}

	if (!can_use_verbose(Ind, o_ptr)) return;

	if (!o_ptr->tval || !o_ptr->number) {
		msg_print(Ind, "Your quiver is empty!");
		return;
	}
	
	if (o_ptr->tval != p_ptr->tval_ammo && !boomerang) {
		msg_print(Ind, "You cannot fire that!");
		return;
	}

	/* Extract the item flags */
        object_flags(o_ptr, &f1a, &fx, &fx, &fx, &fx, &esp);

	/* Use a base distance */
//	tdis = 10;

	/* Use the proper number of shots */
//	thits = boomerang? 1 : p_ptr->num_fire;
	thits = p_ptr->num_fire;

	if (!boomerang) {
		/* Actually "fire" the object */
		bonus = (p_ptr->to_h + p_ptr->to_h_ranged * (p_ptr->ranged_precision ? 2 : 1)
			+ o_ptr->to_h + j_ptr->to_h);
		chance = (p_ptr->skill_thb + (bonus * BTH_PLUS_ADJ));

    		tmul = get_shooter_mult(j_ptr);
	} else {
		/* Actually "fire" the object */
		bonus = (p_ptr->to_h + p_ptr->to_h_ranged + o_ptr->to_h);
		chance = (p_ptr->skill_thb + (bonus * BTH_PLUS_ADJ));

		/* Assume a base multiplier */
		tmul = 1;

		/* Hack -- sorta magic */
		returning = TRUE;
	}

	/* Is this magic Arrow or magic shots or magic bolts? */
	if (is_ammo(o_ptr->tval) && o_ptr->sval == SV_AMMO_MAGIC) {
	        magic = TRUE;
		if (!cursed_p(o_ptr)) returning = TRUE;
	}
	/* Artifact ammo doesn't drop to floor */
	if (artifact_p(o_ptr)) {
		returning = TRUE;

		/* making art ammo less reusable over the history of many chars, increasing demand.. */
//		o_ptr->level = 0;
	}

	/* Get extra "power" from "extra might" */
//	if (p_ptr->xtra_might) tmul++;
	if (!boomerang) tmul += p_ptr->xtra_might;

	/* Base range */
	tdis = 9 + 3 * tmul;
	if (boomerang) tdis = 15;

	/* Play fairly */
#ifdef ARROW_DIST_LIMIT
	if (tdis > ARROW_DIST_LIMIT) tdis = ARROW_DIST_LIMIT; /* == MAX_RANGE */
#else
	if (tdis > MAX_RANGE) tdis = MAX_RANGE;
#endif

	/* Distance to target too great?
	   Use distance() to form a 'natural' circle shaped radius instead of a square shaped radius,
	   monsters do this too.
	   If target is not ok, it means we fire at our own grid, so don't return() in that case. */
	if (dir == 5 && target_ok && distance(p_ptr->py, p_ptr->px, p_ptr->target_row, p_ptr->target_col) > tdis) return;

	/* Only fire in direction 5 if we have a target */
	/* Also only fire at all at a desired target, if we're actually within shooting range - C. Blue */
	if (dir == 5) {
		if (target_ok) {
			/* some minor launchers cannot achieve required range for the specified target - C. Blue */
#ifndef PY_FIRE_ON_WALL
			if (!projectable(&p_ptr->wpos, p_ptr->py, p_ptr->px, p_ptr->target_row, p_ptr->target_col, tdis)) return;
#else
			if (!projectable_wall(&p_ptr->wpos, p_ptr->py, p_ptr->px, p_ptr->target_row, p_ptr->target_col, tdis)) return;
#endif
		} else {
			/* for 4.4.6+, allow shooting (exploding) ammo 'at oneself' */
			if (!is_newer_than(&p_ptr->version, 4, 4, 5, 10, 0, 0)) return;
		}
	}

	/* Check if monsters around him/her hinder this */
//  if (interfere(Ind, cfg.spell_interfere * 3)) return;
	/* boomerang is harder to intercept since it can just be swung as weapon :> - C. Blue */
	if (boomerang) {
		if (interfere(Ind, 25)) return; /* boomerang interference chance */
	} else {
		if (interfere(Ind, 50)) {
			if (p_ptr->ranged_barrage &&
			    o_ptr->number >= 6 &&
			    p_ptr->cst >= 9)
				p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;
			else
				/* Take a (partial) turn */
				p_ptr->energy -= (level_speed(&p_ptr->wpos) /
				    (thits * (p_ptr->ranged_double && o_ptr->number >= 2 && p_ptr->cst >= 1 ? 2 : 1)));

			return; /* shooting interference chance */
		}
	}

	if (p_ptr->ranged_flare) {
		if (boomerang) {
			msg_print(Ind, "You cannot use shooting techniques with a boomerang!");
			p_ptr->ranged_flare = FALSE;
		} else if (p_ptr->cst < 2) {
			msg_print(Ind, "Not enough stamina for a flare missile.");
			p_ptr->ranged_flare = FALSE;
		} else if (check_guard_inscription(p_ptr->inventory[INVEN_AMMO].note, 'k')) {
			msg_print(Ind, "Your ammo's inscription (!k) prevents using it as flare missile.");
			p_ptr->ranged_flare = FALSE;
		} else {
			p_ptr->cst -= 2;
			msg_format_near(Ind, "%s fires a flare missile.", p_ptr->name);
		}
	}
	if (p_ptr->ranged_precision) {
		if (boomerang) {
			msg_print(Ind, "You cannot use shooting techniques with a boomerang!");
			p_ptr->ranged_precision = FALSE;
		} else if (p_ptr->cst < 7) {
			msg_print(Ind, "Not enough stamina for a precision shot.");
			p_ptr->ranged_precision = FALSE;
		} else p_ptr->cst -= 7;
	}
        if (p_ptr->ranged_double) {
		if (boomerang) {
			msg_print(Ind, "You cannot use shooting techniques with a boomerang!");
			p_ptr->ranged_double = FALSE;
		} else if (o_ptr->number < 2) {
			msg_print(Ind, "You need at least 2 projectiles for a double-shot!");
			p_ptr->ranged_double = FALSE;
		}
		else if (p_ptr->cst >= 1) { /* don't toggle off even if we reach 0 stamina */
			ranged_double_real = TRUE;
		}
	}
	if (p_ptr->ranged_barrage) {
		if (boomerang) {
			msg_print(Ind, "You cannot use shooting techniques with a boomerang!");
			p_ptr->ranged_barrage = FALSE;
		} else if (o_ptr->number < 6) { /* && !returning) {  :-p */
			msg_print(Ind, "You need at least 6 projectiles for a barrage!");
			p_ptr->ranged_barrage = FALSE;
		} else if (p_ptr->cst < 9) {
			msg_print(Ind, "Not enough stamina for barrage.");
			p_ptr->ranged_barrage = FALSE;
		} else {
			p_ptr->cst -= 9;
			msg_format_near(Ind, "%s fires a multi-shot barrage!", p_ptr->name);
		}
	}
	p_ptr->redraw |= PR_STAMINA;

	/* silent fail */
	if (returning) {
		/* hack - allow use of magic ammo for flare now,
		   but in that case make it non-returning since it burns on the floor,
		   serving as light source, as normal ammo would */
		/* make sure artifact magic ammo doesn't work for 'flare'
		   because we don't want arts to get destroyed */
		if (!magic || artifact_p(o_ptr)) p_ptr->ranged_flare = FALSE;
	}

	/* Double # of shots? */
	if (ranged_double_real) {
		thits *= 2;

		/* prevent possible exploit (fire 4 rounds with a 5bpr shooter, then switch to 2bpr shooter)
		   although hardly someone will use that =-p */
		if (p_ptr->ranged_double_used >= thits) p_ptr->ranged_double_used = 0;

		if (!p_ptr->ranged_double_used) {
			if (!rand_int(5)) p_ptr->cst--; /* artificially: slow drain */
			p_ptr->redraw |= PR_STAMINA;
			p_ptr->ranged_double_used = thits;
		}
		p_ptr->ranged_double_used--;
	}


	if (p_ptr->ranged_barrage)
		p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;
	else
		/* Take a (partial) turn */
		p_ptr->energy -= (level_speed(&p_ptr->wpos) / thits);

	/* if we intended to shoot till kill, then we did succeed now,
	   passed all tests that might return() instead, and ARE shooting
	   to kill. Note: Cursed arrow failure is exempt a.t.m. - C. Blue */
	if (p_ptr->shooty_till_kill) p_ptr->shooting_till_kill = TRUE;

	if (!boomerang && cursed_p(o_ptr) && magik(50)) {
		msg_print(Ind, "You somehow failed to fire!");
		return;
	}

#if (STARTEQ_TREATMENT > 1)
        if (o_ptr->owner == p_ptr->id && p_ptr->max_plv < cfg.newbies_cannot_drop && !is_admin(p_ptr) &&
            !((o_ptr->tval == 1) && (o_ptr->sval >= 9)))
                o_ptr->level = 0;
#endif

	/* S(he) is no longer afk */
	if (p_ptr->afk && !retaliating_cmd) un_afk_idle(Ind);
	retaliating_cmd = FALSE;

	/* Adjust VAMPIRIC draining to shooting speed, since
	   do_cmd_fire only handles ONE shot :/ */
	drain_left /= thits;

	/* Hack -- suppress messages */
	if (p_ptr->taciturn_messages) suppress_message = TRUE;

	/* Ricochets ? */
#if 0 // DG - no
	if (get_skill(p_ptr, SKILL_RICOCHET) && !magic && !boomerang && !p_ptr->ranged_barrage)
	{
		num_ricochet = get_skill_scale(p_ptr, SKILL_RICOCHET, 6);
		num_ricochet = (num_ricochet < 0) ? 0 : num_ricochet;
		ricochet_chance = 45 + get_skill_scale(p_ptr, SKILL_RICOCHET, 50);
	}
#else	// 0
	/* Sling mastery yields bullet ricochets */
	if (archery == SKILL_SLING && !boomerang && !magic && !ethereal && !p_ptr->ranged_barrage) {
		num_ricochet = randint(get_skill_scale_fine(p_ptr, SKILL_SLING, 3));//6
		num_ricochet = (num_ricochet < 0) ? 0 : num_ricochet;
		ricochet_chance = 33 + get_skill_scale(p_ptr, SKILL_SLING, 42);//45+(50)
	}
#endif
	/* Create a "local missile object" */
	throw_obj = *o_ptr;
	throw_obj.number = p_ptr->ranged_barrage ? 6 : 1; /* doesn't work anyway for boomerangs since it tested for 6 items */

	/* Use the missile object */
	o_ptr = &throw_obj;

#if 0 //DEBUG
#ifdef USE_SOUND_2010
	/* save for sfx later (after the missile object was maybe already destroyed) */
	otval = o_ptr->tval;
#endif
#endif

	/* Describe the object */
	object_desc(Ind, o_name, o_ptr, FALSE, 3);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(o_ptr);
	missile_char = object_char(o_ptr);


	/* Start at the player */
	y = p_ptr->py;
	x = p_ptr->px;
        by = y;
        bx = x;

	/* Predict the "target" location */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_ok) {
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}

	break_chance = breakage_chance(o_ptr);

	break_cloaking(Ind, 0);
	break_shadow_running(Ind);

	/* Reduce and describe inventory */
	if (!boomerang) {
		/* C. Blue - Artifact ammo never runs out (similar to magic arrows:) */
		if (artifact_p(o_ptr)) {
			/* doesn't run out/drop */
		} else if (ethereal) { /* being nice in regards to ranged_barrage break_chance */
			if (cursed_p(o_ptr) ? TRUE : magik(break_chance * (p_ptr->ranged_barrage ? 3 : 1))) {
				if (item >= 0) {
					inven_item_increase(Ind, item, -1);
//					inven_item_describe(Ind, item);
					msg_format(Ind, "\377wThe %s fades away.", o_name);
					inven_item_optimize(Ind, item);
				} else {
					floor_item_increase(0 - item, -1);
					floor_item_optimize(0 - item);
				}
			}
		} else if (!magic || cursed_p(o_ptr)) {
			if (item >= 0) {
				inven_item_increase(Ind, item, p_ptr->ranged_barrage ? -6 : -1);
				inven_item_describe(Ind, item);
				inven_item_optimize(Ind, item);
			}
			/* Reduce and describe floor item */
			else {
				floor_item_increase(0 - item, p_ptr->ranged_barrage ? -6 : -1);
				floor_item_optimize(0 - item);
			}
		} else {
			if (item >= 0) {
				inven_item_describe(Ind, item);
				inven_item_optimize(Ind, item);
			}
			/* Reduce and describe floor item */
			else {
				floor_item_optimize(0 - item);
			}
		}
	}

	/* Hack -- Handle stuff */
	handle_stuff(Ind);

#ifdef USE_SOUND_2010
	if (p_ptr->cut_sfx_attack) {
		sfx = extract_energy[p_ptr->pspeed] * p_ptr->num_fire;
		if (sfx) {
			p_ptr->count_cut_sfx_attack += 10000 / sfx;
			if (p_ptr->count_cut_sfx_attack >= 200) { /* 5 attacks per turn */
				p_ptr->count_cut_sfx_attack -= 200;
				if (p_ptr->count_cut_sfx_attack >= 200) p_ptr->count_cut_sfx_attack = 0;
				sfx = 0;
			}
		}
	}
	if (sfx == 0)
 #if 0 //DEBUG
		switch (otval) {
 #else
		switch (o_ptr->tval) {
 #endif
		case TV_SHOT: sound(Ind, "fire_shot", NULL, SFX_TYPE_ATTACK, FALSE); break;
		case TV_ARROW: sound(Ind, "fire_arrow", NULL, SFX_TYPE_ATTACK, FALSE); break;
		case TV_BOLT: sound(Ind, "fire_bolt", NULL, SFX_TYPE_ATTACK, FALSE); break;
		case TV_BOOMERANG: sound(Ind, "fire_boomerang", NULL, SFX_TYPE_ATTACK, FALSE); break;
	}
#endif

	if (!returning && !ethereal
	    && !strstr(o_name, "!=")
	    && p_ptr->warning_autopickup == 0) {
		p_ptr->warning_autopickup = 1;
		msg_print(Ind, "\374\377yHINT: Press '{' key and inscribe your ammunition '\377R!=\377y' to pick it up automatically!");
		s_printf("warning_autopickup: %s\n", p_ptr->name);
	}

#ifdef PY_FIRE_ON_WALL
	/* For flare missile under projectable_wall() settings */
	prev_x = bx;
	prev_y = by;
#endif

	while (TRUE) {
		/* Travel until stopped */
#if 0 /* I think it travelled once too far, since it's mmove2'ed in the very beginning of every for-pass, */
/* and it goes from 0 to tdis, that's tdis+1 steps in total. Correct me if wrong~~  C. Blue */
//		for (cur_dis = 0; cur_dis <= tdis; )
		for (cur_dis = real_dis = 0; real_dis <= tdis; )
#else
//		for (cur_dis = 0; cur_dis < tdis; )
		for (cur_dis = real_dis = 0; real_dis < tdis; )
#endif
		{
			ny = y;
			nx = x;

			/* Hack -- Stop at the target */
			if ((y == ty) && (x == tx)) break;

			/* Calculate the new location (see "project()") */
			//			mmove2(&ny, &nx, p_ptr->py, p_ptr->px, ty, tx);
			mmove2(&ny, &nx, by, bx, ty, tx);

#ifdef DOUBLE_LOS_SAFETY
		    /* skip checks if we already used projectable..() routines to
		       determine that it's fine, so it'd be redundant and also require
		       additional code to comply with DOUBLE_LOS_SAFETY. */
		    if (dir != 5) {
#endif
			/* Sanity check because server crashed here on 2012-01-10 - mikaelh */
			if (!in_bounds_array(ny, nx)) break;

#ifndef PY_FIRE_ON_WALL
			/* Stopped by walls/doors */
			if (!cave_contact(zcave, ny, nx)) break;
#else
			/* Stopped by protected grids (Inn doors, also stopping monsters' ranged attacks!) */
			if (f_info[zcave[ny][nx].feat].flags1 & (FF1_BLOCK_LOS | FF1_BLOCK_CONTACT)) break;
#endif
#ifdef DOUBLE_LOS_SAFETY
		    }
		    /* End of skipping redundant checks for targetted shots. */
#endif

			/* Advance the distance */
			cur_dis++;

			/* Distance to target too great?
			   Use distance() to form a 'natural' circle shaped radius instead of a square shaped radius,
			   monsters do this too */
			real_dis = distance(by, bx, ny, nx);

#ifdef PY_FIRE_ON_WALL
			/* For flare missile under projectable_wall() settings */
			prev_x = x;
			prev_y = y;
#endif

			/* Save the new location */
			x = nx;
			y = ny;

#ifndef OPTIMIZED_ANIMATIONS
			/* Save the old "player pointer" */
			q_ptr = p_ptr;

			/* Display it for each player */
			for (i = 1; i < NumPlayers + 1; i++)
			{
				int dispx, dispy;

				/* Use this player */
				p_ptr = Players[i];

				/* If he's not playing, skip him */
				if (p_ptr->conn == NOT_CONNECTED)
					continue;

				/* If he's not here, skip him */
				if(!inarea(&p_ptr->wpos, wpos))
					continue;

				/* The player can see the (on screen) missile */
				if (panel_contains(y, x) && player_can_see_bold(i, y, x))
				{
					/* Draw, Hilite, Fresh, Pause, Erase */
					dispy = y - p_ptr->panel_row_prt;
					dispx = x - p_ptr->panel_col_prt;

					/* Remember the projectile */
					p_ptr->scr_info[dispy][dispx].c = missile_char;
					p_ptr->scr_info[dispy][dispx].a = missile_attr;

					/* Tell the client */
					Send_char(i, dispx, dispy, missile_attr, missile_char);

					/* Flush and wait */
					if (cur_dis % (boomerang?2:tmul)) Send_flush(i);

					/* Restore */
					lite_spot(i, y, x);
				}
			}

			/* Restore the player pointer */
			p_ptr = q_ptr;

#else /* OPTIMIZED_ANIMATIONS */

			/* Save the projectile path */
			if (path_num < MAX_RANGE) {
				path_y[path_num] = y;
				path_x[path_num] = x;
				path_num++;
			}
#endif /* OPTIMIZED_ANIMATIONS */

			/* Player here, hit him */
			if (zcave[y][x].m_idx < 0) {
				if (cfg.use_pk_rules == PK_RULES_NEVER) {
					/* Stop looking */
					if (!p_ptr->bow_brand || (p_ptr->bow_brand_t != BRAND_VORP)) break;
				} else {

					cave_type *c_ptr = &zcave[y][x];
					q_ptr = Players[0 - c_ptr->m_idx];

					/* AD hack -- "pass over" players in same party */
					if ((p_ptr->pkill & PKILL_KILLER ||
					    check_hostile(Ind, 0 - c_ptr->m_idx) ||
					    magik(NEUTRAL_FIRE_CHANCE)) &&
					    (check_blood_bond(Ind, 0 - c_ptr->m_idx) ||
					     p_ptr->party == 0 ||
					     !player_in_party(p_ptr->party, 0 - c_ptr->m_idx) ||
					     magik(FRIEND_FIRE_CHANCE))
					    && !p_ptr->admin_dm) {

						/* Check the visibility */
						visible = p_ptr->play_vis[0 - c_ptr->m_idx];

						/* Note the collision */
						hit_body = TRUE;

						if (cfg.use_pk_rules == PK_RULES_DECLARE) {
							if (zcave[p_ptr->py][p_ptr->px].info & CAVE_NOPK || zcave[q_ptr->py][q_ptr->px].info & CAVE_NOPK) {
								if (visible && (!player_in_party(Players[0 - c_ptr->m_idx]->party, Ind))) {
									p_ptr->target_who = 0;
									do_player_drop_items(Ind, 40, FALSE);
									imprison(Ind, 100, "attempted murder");
								}
							}
						}

						/* Did we hit it (penalize range) */

#ifndef PVP_AC_REDUCTION
						if (test_hit_fire(chance - cur_dis, q_ptr->ac + q_ptr->to_a, visible)
						    && (!q_ptr->shadow_running || !rand_int(3))) {
#else
//						if (test_hit_fire(chance - cur_dis, ((q_ptr->ac + q_ptr->to_a) * 2) / 3, visible)) {
						if (test_hit_fire(chance - cur_dis,
						    (q_ptr->ac + q_ptr->to_a > AC_CAP) ? AC_CAP : q_ptr->ac + q_ptr->to_a,
						    visible)
						    && (!q_ptr->shadow_running || !rand_int(3))) {
#endif
							char p_name[80];
							bool dodged = FALSE;

							/* Get the name */
							strcpy(p_name, q_ptr->name);

#ifndef NEW_DODGING
							if (get_skill(q_ptr, SKILL_DODGE)) {
//								int chance = (q_ptr->dodge_level - p_ptr->lev - archery) / 3;
								int chance = (q_ptr->dodge_level - p_ptr->lev - get_skill(p_ptr, archery)) / 3;

								if ((chance > 0) && magik(chance)) {
									//				msg_print(Ind, "You dodge a magical attack!");
									msg_format(0 - c_ptr->m_idx, "\377%cYou dodge the projectile!", COLOUR_DODGE_GOOD);
									if (visible) msg_format(Ind, "\377%c%s dodges %s.", COLOUR_DODGE_NEAR, p_name, o_name);
									dodged = TRUE;
/* gotta remove the dodged=TRUE stuff etc, we simply continue!*/	continue;
								}
							}
#else
//							if (magik(apply_dodge_chance(0 - c_ptr->m_idx, p_ptr->lev + get_skill(p_ptr, archery) / 3 + 1000))) { /* hack - more difficult to dodge ranged attacks */
							if (magik(apply_dodge_chance(0 - c_ptr->m_idx, p_ptr->lev + get_skill(p_ptr, archery) / 3)) && (!p_ptr->ranged_barrage || magik(50))) {
								// msg_print(Ind, "You dodge a magical attack!");
								msg_format(0 - c_ptr->m_idx, "\377%cYou dodge the projectile!", COLOUR_DODGE_GOOD);
								if (visible) msg_format(Ind, "\377%c%s dodges %s.", COLOUR_DODGE_NEAR, p_name, o_name);
								continue;
							}
#endif

#ifdef USE_BLOCKING /* Parry/Block - belongs to new-NR-viability changes */
				                        /* choose whether to attempt to block or to parry (can't do both at once),
				                           50% chance each, except for if weapon is missing (anti-retaliate-inscription
				                    	   has been left out, since if you want max block, you'll have to take off your weapon!) */
				                	if (q_ptr->shield_deflect &&
							    (!q_ptr->inventory[INVEN_WIELD].k_idx || magik(q_ptr->combat_stance == 1 ? 75 : 50))) {
								if (magik(apply_block_chance(q_ptr, q_ptr->shield_deflect + 15))) { /* boost for PvP! */
									if (visible) msg_format(Ind, "\377%c%s blocks %s!", COLOUR_BLOCK_PLY, p_name, o_name);
									msg_format(0 - c_ptr->m_idx, "\377%cYou block %s's attack!", COLOUR_BLOCK_GOOD, p_ptr->name);
#ifdef USE_SOUND_2010
					                                if (sfx == 0) sound(Ind, "block_shield_projectile", NULL, SFX_TYPE_ATTACK, FALSE);
#endif
									continue;
								}
							}
#endif
#if 0 /* disable for now? */
#ifdef USE_PARRYING
							if (q_ptr->weapon_parry) {
								// && !p_ptr->ranged_barrage etc, any prepared bow stance?
								if (magik(apply_parry_chance(q_ptr, q_ptr->weapon_parry + 5))) { /* boost for PvP! */
									msg_format(0 - c_ptr->m_idx, "\377%cYou parry %s's attack!", COLOUR_PARRY_GOOD, p_ptr->name);
									if (visible) msg_format(Ind, "\377%c%s parries %s!", COLOUR_PARRY_PLY, p_name, o_name);
#ifdef USE_SOUND_2010
					                                if (sfx == 0) sound(Ind, "parry_weapon", "parry", SFX_TYPE_ATTACK, FALSE);
#endif
									continue;
								}
							}
#endif
#endif

							if (!dodged) {	// 'goto' would be cleaner
								if (!boomerang) {
									/* Base damage from thrown object plus launcher bonus */
									tdam = damroll(o_ptr->dd, o_ptr->ds);
									if (p_ptr->ranged_flare) tdam += damroll(2, 6); /* compare to dice in k_info for oil flask */
									tdam = tot_dam_aux_player(Ind, o_ptr, tdam, q_ptr, brand_msg, FALSE);
									if (p_ptr->bow_brand) tdam += p_ptr->bow_brand_d;
									tdam += o_ptr->to_d;
									tdam += j_ptr->to_d + p_ptr->to_d_ranged;
								} else {
									 /* Base damage from thrown object */
								        tdam = damroll(o_ptr->dd, o_ptr->ds);
									tdam = tot_dam_aux_player(Ind, o_ptr, tdam, q_ptr, brand_msg, FALSE);
									tdam += o_ptr->to_d;
									tdam += p_ptr->to_d_ranged;
								}
								ranged_flare_body = TRUE;

								/* Boost the damage */
								tdam *= tmul;

								/* Apply special damage XXX XXX XXX */
								tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h + p_ptr->to_h_ranged, tdam);

								/* factor in AC */
								tdam -= (tdam * (((q_ptr->ac + q_ptr->to_a) < AC_CAP) ? (q_ptr->ac + q_ptr->to_a) : AC_CAP) / AC_CAP_DIV);

								/* XXX Reduce damage by 1/3 */
								tdam = (tdam + PVP_SHOOT_DAM_REDUCTION - 1) / PVP_SHOOT_DAM_REDUCTION;
								
								if (ranged_double_real) tdam = (tdam * 35) / 100;
								if (p_ptr->ranged_precision) {
									if (q_ptr->afraid) tdam *= 5;
									else tdam *= 3;
									p_ptr->ranged_precision = FALSE;
								}
								if (p_ptr->ranged_barrage) tdam *= 2; // maybe 3 even

								/* No negative damage */
								if (tdam < 0) tdam = 0;

						                /* can't attack while in WRAITHFORM (explosion still works) */
							        if (p_ptr->tim_wraith && !q_ptr->tim_wraith) tdam = 0;

								/* Handle unseen player */
								if (!visible) {
									/* Invisible player */
									msg_format(Ind, "The %s finds a mark.", o_name);
									msg_format(0 - c_ptr->m_idx, "You are hit by a %s!", o_name);
								}
								/* Handle visible player */
								else {
									/* Messages */
									msg_format(Ind, "The %s hits %s for \377y%d \377wdamage.", o_name, p_name, tdam);
									if ((o_name[0] == 'A') || (o_name[0] == 'E') || (o_name[0] == 'I') || (o_name[0] == 'O') || (o_name[0] == 'U') ||
									    (o_name[0] == 'a') || (o_name[0] == 'e') || (o_name[0] == 'i') || (o_name[0] == 'o') || (o_name[0] == 'u'))
										msg_format(0 - c_ptr->m_idx, "%^s hits you with an %s for \377R%d \377wdamage.", p_ptr->name, o_name, tdam);
									else	msg_format(0 - c_ptr->m_idx, "%^s hits you with a %s for \377R%d \377wdamage.", p_ptr->name, o_name, tdam);

									/* Track this player's health */
									health_track(Ind, c_ptr->m_idx);
								}

								/* If this was intentional, make target hostile */
								if (check_hostile(Ind, 0 - c_ptr->m_idx)) {
									/* Make target hostile if not already */
									if (!check_hostile(0 - c_ptr->m_idx, Ind)) {
										bool result = FALSE;

										if (Players[0 - c_ptr->m_idx]->pvpexception < 2)
											result = add_hostility(0 - c_ptr->m_idx, p_ptr->name, FALSE);

										/* Log it if no blood bond - mikaelh */
										if (!player_list_find(p_ptr->blood_bond, Players[0 - c_ptr->m_idx]->id)) {
											s_printf("%s attacked %s (shot; result %d).\n", p_ptr->name, Players[0 - c_ptr->m_idx]->name, result);
										}
									}
								}
//less spam for now - C. Blue					if (strlen(brand_msg) > 0) msg_print(Ind, brand_msg);

								if ((p_ptr->bow_brand && (p_ptr->bow_brand_t == BRAND_CHAO)) && !q_ptr->resist_conf && !boomerang)
								{
									(void)set_confused(0 - c_ptr->m_idx, q_ptr->confused + 5);
								}
								
								if (p_ptr->ranged_barrage) {
									set_stun(0 - c_ptr->m_idx, q_ptr->stun + 35 + get_skill_scale(p_ptr, SKILL_COMBAT, 5));
								}


								/* Take damage */
								take_hit(0 - c_ptr->m_idx, tdam, p_ptr->name, Ind);

								/* XXX confusion arrow is not handled right
								 * in do_arrow_brand_effect */
								if (!boomerang && p_ptr->bow_brand_t
										&& p_ptr->bow_brand_t != BRAND_CHAO) 
									do_arrow_brand_effect(Ind, y, x);

								if (!boomerang && !magic && o_ptr->pval)
									do_arrow_explode(Ind, o_ptr, wpos, y, x, tmul);

								/* Stop looking */
								if (!p_ptr->bow_brand || (p_ptr->bow_brand_t != BRAND_VORP) || boomerang) break;
							}
						}

					} /* end hack */
				}
			}

			/* Monster here, Try to hit it */
			if (zcave[y][x].m_idx > 0) {
				cave_type *c_ptr = &zcave[y][x];

				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				monster_race *r_ptr = race_inf(m_ptr);
				drainable = !((r_ptr->flags3 & RF3_UNDEAD) ||
					    /* (r_ptr->flags3 & RF3_DEMON) ||*/
					    (r_ptr->flags3 & RF3_NONLIVING) ||
					    (strchr("Egv", r_ptr->d_char)));

				/* Do not hit pets - the_sandman */
				if (m_ptr->pet) break;

				/* Check the visibility */
				visible = p_ptr->mon_vis[c_ptr->m_idx];

				/* Note the collision */
				/* Note the collision */
				if(!(o_ptr->tval == 1 && o_ptr->sval == 9))
					hit_body = TRUE;

				/* Did we hit it (penalize range) */
				if (test_hit_fire(chance - cur_dis, m_ptr->ac, visible)) {
					bool fear = FALSE;
					char m_name[MNAME_LEN];

					/* Assume a default death */
					cptr note_dies = " dies";

					/* Get "the monster" or "it" */
					monster_desc(Ind, m_name, c_ptr->m_idx, 0);

					/* Some monsters get "destroyed" */
					if ((r_ptr->flags3 & RF3_DEMON) ||
					    (r_ptr->flags3 & RF3_UNDEAD) ||
					    (r_ptr->flags2 & RF2_STUPID) ||
					    (strchr("Evg", r_ptr->d_char))) {
						/* Special note at death */
						note_dies = " is destroyed";
					}


					/* Handle reflection - it's back, though weaker */
#if 0 /* hm doesn't make sense, but is pretty unbalanced even :/ */
					/* New: Boomerangs can't be deflected, nor can exploding ammo (!) - C. Blue */
					if ((r_ptr->flags2 & RF2_REFLECTING) && !boomerang && !o_ptr->pval && magik(50)) {
#else
					if ((r_ptr->flags2 & RF2_REFLECTING) && magik(50)) {
#endif
						if (visible) msg_format(Ind, "The %s was deflected.", o_name);
						num_ricochet = 1;
						hit_body = 1;

						if (!boomerang && !magic && o_ptr->pval)
							do_arrow_explode(Ind, o_ptr, wpos, y, x, tmul);

						break;
					}

#ifdef USE_BLOCKING
				        /* handle blocking (deflection) */
				        if (strchr("hHJkpPty", r_ptr->d_char) && /* leaving out Yeeks (else Serpent Man 'J') */
				            !(r_ptr->flags3 & RF3_ANIMAL) && !(r_ptr->flags8 & RF8_NO_BLOCK)
				            && !rand_int(24 - r_ptr->level / 10)) { /* small chance to block arrows */
				                if (visible) {
				                        char hit_desc[MAX_CHARS];
				                        sprintf(hit_desc, "\377%c%s blocks.", COLOUR_BLOCK_MON, m_name);
				                        hit_desc[0] = toupper(hit_desc[0]);
				                        msg_print(Ind, hit_desc);
				                }
						hit_body = 1;
						if (!boomerang && !magic && o_ptr->pval)
							do_arrow_explode(Ind, o_ptr, wpos, y, x, tmul);
				                break;
				        }
#endif

					/* Handle unseen monster */
					if (!visible) {
						/* Invisible monster */
						msg_format(Ind, "The %s finds a mark.", o_name);
					}
					/* Handle visible monster */
					else {
						/* Message */
						msg_format(Ind, "The %s hits %s.", o_name, m_name);

						/* Hack -- Track this monster race */
						if (visible) recent_track(m_ptr->r_idx);

						/* Hack -- Track this monster */
						if (visible) health_track(Ind, c_ptr->m_idx);
					}

					if (!boomerang) {
						/* Base damage from thrown object plus launcher bonus */
						tdam = damroll(o_ptr->dd, o_ptr->ds);
						if (p_ptr->ranged_flare) tdam += damroll(2, 6); /* compare to dice in k_info for oil flask */
						tdam = tot_dam_aux(Ind, o_ptr, tdam, m_ptr, brand_msg, FALSE);
						if (p_ptr->bow_brand) tdam += p_ptr->bow_brand_d;
						tdam += o_ptr->to_d;
//						tdam = tot_dam_aux(Ind, o_ptr, tdam, m_ptr, brand_msg, FALSE);
						tdam += j_ptr->to_d + p_ptr->to_d_ranged;
//						tdam = tot_dam_aux(Ind, o_ptr, tdam, m_ptr, brand_msg, FALSE);
					} else {
						 /* Base damage from thrown object */
					        tdam = damroll(o_ptr->dd, o_ptr->ds);
						tdam = tot_dam_aux(Ind, o_ptr, tdam, m_ptr, brand_msg, FALSE);
						tdam += o_ptr->to_d;
						tdam += p_ptr->to_d_ranged;
					}
					ranged_flare_body = TRUE;

//less spam for now - C. Blue		if (strlen(brand_msg) > 0) msg_print(Ind, brand_msg);

					/* Boost the damage */
					tdam *= tmul;

					/* Apply special damage XXX XXX XXX */
					tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h + p_ptr->to_h_ranged, tdam);

					if (ranged_double_real) tdam = (tdam * 35) / 100;
					if (p_ptr->ranged_precision) {
						if (m_ptr->monfear) tdam *= 5;
						else tdam *= 3;
						p_ptr->ranged_precision = FALSE;
					}
					if (p_ptr->ranged_barrage) tdam *= 2; // maybe 3 even

			                /* can't attack while in WRAITHFORM (explosion still works) */
					/* wraithed players can attack wraithed monsters - mikaelh */
					if (p_ptr->tim_wraith && 
					    ((r_ptr->flags2 & RF2_KILL_WALL) || !(r_ptr->flags2 & RF2_PASS_WALL))) tdam = 0;

					/* No negative damage */
					if (tdam < 0) tdam = 0;

#if 0
					/* XXX consider using project() with GF_CONF */
					if ((p_ptr->bow_brand && (p_ptr->bow_brand_t == BRAND_CHAO)) &&
							!(r_ptr->flags3 & RF3_NO_CONF) &&
							!(r_ptr->flags4 & RF4_BR_CONF) &&
							!(r_ptr->flags4 & RF4_BR_CHAO) && !boomerang)
					{
						int i;

						/* Already partially confused */
						if (m_ptr->confused) {
							i = m_ptr->confused + p_ptr->lev;
						}
						/* Was not confused */
						else {
							i = p_ptr->lev;
						}
						/* Apply confusion */
						m_ptr->confused = (i < 200) ? i : 200;

						if (visible) {
							char m_name[MNAME_LEN];

							/* Get "the monster" or "it" */
							monster_desc(Ind, m_name, c_ptr->m_idx, 0);

							/* Message */
							msg_format(Ind, "%s appears confused.", m_name);
						}
					}
#endif	// 0

					/* Vampiric drain  - Launcher or ammo can be vampiric!
					   - for now vampiric from items helps a bit,
					   - vampiric from vampire form doesn't (relies on melee-biting!) */
					if ((magik(p_ptr->vampiric_ranged)) && drainable)
						drain_result = m_ptr->hp;
					else
						drain_result = 0;

					if (p_ptr->admin_godly_strike) {
						p_ptr->admin_godly_strike--;
						if (!(r_ptr->flags1 & RF1_UNIQUE)) tdam = m_ptr->hp + 1;
					}

					/* Hit the monster, check for death */
					if (mon_take_hit(Ind, c_ptr->m_idx, tdam, &fear, note_dies))
					{
						/* note: if the monster we hit wasn't the one targetted, then continue shooting.
						         It can only mean that this monster was invisible to us, hence the
							 character couldn't have control over avoiding to target it. */
						if ((dir == 5) && ((y == ty) && (x == tx))) {
							/* Dead targetted monster */
					    		p_ptr->shooting_till_kill = FALSE;
						}
					}
					/* No death */
					else {
						/* Message */
						message_pain(Ind, c_ptr->m_idx, tdam);

						/* VAMPIRIC: Are we draining it?  A little note: If the monster is
						dead, the drain does not work... */
						if (drain_result) {
							drain_result -= m_ptr->hp;  /* Calculate the difference */
						
							if (drain_result > 0) { /* Did we really hurt it? */
								drain_heal = randint(2) + damroll(2,(drain_result / 16));/* was 4,../6 -- was /8 for 50 max_drain */

								if (drain_left) {
									if (drain_heal < drain_left) {
									        drain_left -= drain_heal;
									} else {
										drain_heal = drain_left;
										drain_left = 0;
									}

									if (drain_msg) {
									        msg_format(Ind, "Your shot drains life from %s!", m_name);
									        drain_msg = FALSE;
									}

									hp_player_quiet(Ind, drain_heal, TRUE);
									/* We get to keep some of it! */
								}
							}
						}
						
						if (p_ptr->ranged_barrage) {
							if (!(r_ptr->flags3 & RF3_NO_STUN)) {
								if (!m_ptr->stunned) msg_format(Ind, "\377y%^s is stunned.", m_name);
								else msg_format(Ind, "\377y%^s appears more stunned.", m_name);
								m_ptr->stunned = m_ptr->stunned + 20 + get_skill_scale(p_ptr, SKILL_COMBAT, 5);
							} else {
								msg_format(Ind, "\377o%^s resists the effect.", m_name);
							}
						}

						/* Take note */
						if (fear && visible) {
							char m_name[MNAME_LEN];

#ifdef USE_SOUND_2010
#else
							sound(Ind, SOUND_FLEE);
#endif

							/* Get the monster name (or "it") */
							monster_desc(Ind, m_name, c_ptr->m_idx, 0);

							/* Message */
							if (!streq(m_name, "Morgoth, Lord of Darkness"))
								msg_format(Ind, "%^s flees in terror!", m_name);
							else
								msg_format(Ind, "%^s retreats!", m_name);
						}
					}

					if (!boomerang && p_ptr->bow_brand_t) 
						do_arrow_brand_effect(Ind, y, x);

					if (!boomerang && !magic && o_ptr->pval)
						do_arrow_explode(Ind, o_ptr, wpos, y, x, tmul);

					/* Stop looking */
					if (!p_ptr->bow_brand || (p_ptr->bow_brand_t != BRAND_VORP) || boomerang) break;
				}
			}

#ifdef DOUBLE_LOS_SAFETY
		    /* skip checks if we already used projectable..() routines to test. */
		    if (dir != 5) {
#endif
#ifdef PY_FIRE_ON_WALL
			/* Stopped by walls/doors */
			if (!cave_contact(zcave, ny, nx)) break;
#endif
#ifdef DOUBLE_LOS_SAFETY
		    }
#endif
		}

#ifdef DOUBLE_LOS_SAFETY
	    /* skip checks if we already used projectable..() routines to test. */
	    if (dir != 5) {
#endif
		/* Extra (exploding) hack: */
		if (!cave_contact(zcave, ny, nx)/* Stopped by walls/doors ?*/
		    || (dir == 5 && !target_ok)) { /* fired 'at oneself'? */
			if (!boomerang && !magic && o_ptr->pval) {
				do_arrow_explode(Ind, o_ptr, wpos, y, x, tmul);
			}
		}
#ifdef DOUBLE_LOS_SAFETY
	    } else {
		if (!target_ok) { /* fired 'at oneself'? */
			if (!boomerang && !magic && o_ptr->pval) {
				do_arrow_explode(Ind, o_ptr, wpos, y, x, tmul);
			}
		}
	    }
#endif

		/*todo: even if not hit_body, boomerangs should have chance to drop to the ground.. */

		/* Chance of breakage (during attacks) */
		if (archery == SKILL_BOOMERANG) {
			/* finer resolution to match reduced break rate of boomerangs - C. Blue */
			j = (hit_body ? break_chance : 0) * 100;
//			j = (j * (1000 - get_skill_scale(p_ptr, archery, 950))) / 1000;
			j = (j * (2500 - get_skill_scale(p_ptr, archery, 2450))) / 5000;
		} else {
			j = (hit_body ? break_chance : break_chance / 3) * 100;
//			j = (j * (100 - get_skill_scale(p_ptr, archery, 80))) / 100; <- for when base ammo break chance was 50% in breakage_chance
			j = (j * (100 - get_skill_scale(p_ptr, archery, 90))) / 100;
		}

		/* Break ? */
		if((((o_ptr->pval != 0) && !boomerang) || (rand_int(10000) < j))
		    && !magic && !ethereal && !artifact_p(o_ptr)) {
			breakage = 100;
			if (boomerang) /* change a large part of the break chance to actually
					  result in not returning instead of real breaking - C. Blue */
			{
				if (magik(90)) {
					msg_format(Ind, "\377oYour %s drops to the ground.",o_name);
#if 1 /* hope this works */
					inven_item_increase(Ind, item, -1);
					inven_item_optimize(Ind, item);
					drop_near_severe(Ind, &throw_obj, 0, wpos, y, x);
#else /* silly - like failing to catch it instead of it not returning ;-p */
					inven_drop(Ind, INVEN_BOW, 1);
#endif
				} else {
					msg_format(Ind, "\377oYour %s is destroyed.",o_name);
					inven_item_increase(Ind, item, -1);
					inven_item_optimize(Ind, item);
				}

				/* Recalculate bonuses */
				p_ptr->update |= (PU_BONUS);

				/* Recalculate torch */
				p_ptr->update |= (PU_TORCH);

				/* Recalculate mana */
				p_ptr->update |= (PU_MANA | PU_HP | PU_SANITY);

				/* Redraw */
				p_ptr->redraw |= (PR_PLUSSES | PR_ARMOR);

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
			}
			break;
		}

		/* If no break and if Archer, the ammo can ricochet */
//		if ((num_ricochet) && (hit_body) && (magik(45 + p_ptr->lev)))
//		if ((num_ricochet) && (hit_body) && (magik(50 + p_ptr->lev)) && magik(95))
		if ((num_ricochet) && (hit_body) && (magik(ricochet_chance)) && !p_ptr->ranged_barrage) {
			/* Some players do not want ricocheting shots as to not wake up other
			 * monsters. How about we remove ricocheting shots for slingers, but
			 * instead, adds a chance to do a double damage (than normal and/or crit)
			 * shots (i.e., seperate than crit and stackable bonus)?  - the_sandman */
			if (check_guard_inscription(p_ptr->inventory[INVEN_AMMO].note, 'R') ||
			    check_guard_inscription(p_ptr->inventory[INVEN_BOW].note, 'R')) {
				num_ricochet = 0;
				break;
			}

			byte d = 5;

			/* New target location */
			while (d == 5 && tries--)
				d = rand_int(10);

			if (d != 5) { /* extreme safety */
#ifdef DOUBLE_LOS_SAFETY
				/* remove dir 5, or we'll likely panic when we go through all 99 grids without blocking checks */
				dir = d;
#endif

				num_ricochet--;
				hit_body = FALSE;

				/* New base location */
				by = y;
				bx = x;

				tx = p_ptr->px + 99 * ddx[d];
				ty = p_ptr->py + 99 * ddy[d];

				msg_format(Ind, "The %s ricochets!", o_name);
			} else break;
		} else break;
	}

#ifndef OPTIMIZED_ANIMATIONS
	/* Back in the U.S.S.R */
	if (boomerang && !breakage)
	{
		/* Save the old "player pointer" */
		q_ptr = p_ptr;

		for (cur_dis = 0; cur_dis <= tdis; )
		{
			/* Hack -- Stop at the target */
			if ((y == q_ptr->py) && (x == q_ptr->px)){
				break;
			}

			/* Calculate the new location (see "project()") */
			ny = y;
			nx = x;
			//			mmove2(&ny, &nx, p_ptr->py, p_ptr->px, ty, tx);
			mmove2(&ny, &nx, ty, tx, q_ptr->py, q_ptr->px);

#ifdef DOUBLE_LOS_SAFETY
		    /* skip checks if we already used projectable..() routines to test. */
		    if (dir != 5) {
#endif
			/* Stopped by walls/doors */
			if (!cave_contact(zcave, ny, nx)) break;
#ifdef DOUBLE_LOS_SAFETY
		    }
#endif

			/* Advance the distance */
			cur_dis++;

			/* Save the new location */
			x = nx;
			y = ny;

			/* Display it for each player */
			for (i = 1; i < NumPlayers + 1; i++)
			{
				int dispx, dispy;

				/* Use this player */
				p_ptr = Players[i];

				/* If he's not playing, skip him */
				if (p_ptr->conn == NOT_CONNECTED)
					continue;

				/* If he's not here, skip him */
				if(!inarea(&p_ptr->wpos, wpos))
					continue;

				/* The player can see the (on screen) missile */
				if (panel_contains(y, x) && player_can_see_bold(i, y, x))
				{
					/* Draw, Hilite, Fresh, Pause, Erase */
					dispy = y - p_ptr->panel_row_prt;
					dispx = x - p_ptr->panel_col_prt;

					/* Remember the projectile */
					p_ptr->scr_info[dispy][dispx].c = missile_char;
					p_ptr->scr_info[dispy][dispx].a = missile_attr;

					/* Tell the client */
					Send_char(i, dispx, dispy, missile_attr, missile_char);

					/* Flush and wait */
					if (cur_dis % (boomerang?2:tmul)) Send_flush(i);

					/* Restore */
					lite_spot(i, y, x);
				}
			}
		}
		/* Restore the player pointer */
		p_ptr = q_ptr;
	}
#endif /* OPTMIZED_ANIMATIONS */

#ifdef OPTIMIZED_ANIMATIONS
	if (path_num) {
		/* Pick a random spot along the path */
		j = rand_int(path_num);
		ny = path_y[j];
		nx = path_x[j];

		/* Save the old "player pointer" */
		q_ptr = p_ptr;

		/* Draw a projectile here for everyone */
		for (i = 1; i < NumPlayers + 1; i++)
		{
			int dispx, dispy;

			/* Use this player */
			p_ptr = Players[i];

			/* If he's not playing, skip him */
			if (p_ptr->conn == NOT_CONNECTED)
				continue;

			/* If he's not here, skip him */
			if(!inarea(&p_ptr->wpos, wpos))
				continue;

			/* The player can see the (on screen) missile */
			if (panel_contains(ny, nx) && player_can_see_bold(i, ny, nx))
			{
				/* Draw */
				dispy = ny - p_ptr->panel_row_prt;
				dispx = nx - p_ptr->panel_col_prt;

				/* Remember the projectile */
				p_ptr->scr_info[dispy][dispx].c = missile_char;
				p_ptr->scr_info[dispy][dispx].a = missile_attr;

				/* Tell the client */
				Send_char(i, dispx, dispy, missile_attr, missile_char);

				/* Flush once */
				Send_flush(i);
			}

			/* Restore later */
			everyone_lite_later_spot(wpos, ny, nx);
		}

		/* Restore the player pointer */
		p_ptr = q_ptr;
	}
#endif /* OPTIMIZED_ANIMATIONS */

	/* Hack -- "Never litter the floor" inscription {!g} */
	if(check_guard_inscription(o_ptr->note, 'g') )
//			|| p_ptr->max_plv < cfg.newbies_cannot_drop)
	{
		breakage = 101;
	}

	if (p_ptr->ranged_flare && !boomerang) {
		if (!hit_body && !ranged_flare_body) {
			object_type forge;
			(void)project(0 - Ind, 2, wpos, y, x, damroll(2, 6), GF_LITE_WEAK, PROJECT_NORF | PROJECT_GRID | PROJECT_KILL, "");
#ifndef PY_FIRE_ON_WALL
			lite_room(Ind, wpos, y, x);
#else
			lite_room(Ind, wpos, prev_y, prev_x);
#endif
			breakage = 0;
			invcopy(&forge, lookup_kind(o_ptr->tval, SV_AMMO_CHARRED));
			forge.to_h = -rand_int(10);
			forge.to_d = -rand_int(10);
			*o_ptr = forge;
			o_ptr->ident |= ID_KNOWN | ID_SENSED_ONCE;

			if (magic) { /* hack: the one exception where magic ammo doesn't return */
				inven_item_increase(Ind, item, -1);
				inven_item_describe(Ind, item);
				inven_item_optimize(Ind, item);
				/* Window stuff */
//				p_ptr->window |= (PW_EQUIP);
			}
		} else {
			breakage = 101; /* projectile gets destroyed by burning brightly + hitting someone */
		}
	}

	p_ptr->ranged_flare = FALSE;
	p_ptr->ranged_barrage = FALSE; /* whether we hit a monster or fired into empty air (ie missed).. */

	/* Drop (or break) near that location */
	/* by C. Blue - Now art ammo never drops, since it doesn't run out */
	if (!returning) {
#ifdef PY_FIRE_ON_WALL
		if (!cave_los(zcave, y, x)) /* target coordinates were in a wall? */
			/* drop it right there _before_ the wall, not 'in' the wall and then calling scatter().. */
			drop_near(o_ptr, breakage, wpos, prev_y, prev_x);
		else
#endif
		drop_near(o_ptr, breakage, wpos, y, x);
	}

	suppress_message = FALSE;
	
	/* hack for auto-retaliator hack in dungeon.c fire_till_kill-related call of do_cmd_fire() */
	p_ptr->auto_retaliating = FALSE;
}

/*
 * Check if neighboring monster(s) interferes with player's action.  - Jir -
 */
bool interfere(int Ind, int chance)
{
	player_type *p_ptr = Players[Ind], *q_ptr;
	monster_race *r_ptr;
	monster_type *m_ptr;
	int d, i, tx, ty, x = p_ptr->px, y = p_ptr->py;
	int calmness = get_skill_scale(p_ptr, SKILL_CALMNESS, 80);
	cave_type **zcave;
	if(!(zcave=getcave(&p_ptr->wpos))) return(FALSE);

	/* monster doesn't know the player is actually next to it?
	   (ignore cloak_neutralized for the time being) */
	if (p_ptr->cloaked) return(FALSE);

	chance = chance * (100 - calmness) / 100;

	/* Check if monsters around him/her hinder the action */
	for (d = 1; d <= 9; d++)
	{
		if (d == 5) continue;

		tx = x + ddx[d];
		ty = y + ddy[d];

		if (!in_bounds(ty, tx)) continue;

		if (!(i = zcave[ty][tx].m_idx)) continue;
		if (i > 0)
		{
			m_ptr = &m_list[i];
			r_ptr = race_inf(m_ptr);
//			if (r_info[m_list[i].r_idx].flags1 & RF1_NEVER_MOVE)
			/* monster doesn't act? */
			if (r_ptr->flags1 & RF1_NEVER_MOVE) continue;
			if (r_ptr->flags7 & RF7_NEVER_ACT) return(FALSE);
			/* Sleeping etc.. monsters don't interfere o_O - C. Blue */
			if (m_ptr->csleep || m_ptr->monfear || m_ptr->stunned || m_ptr->confused)
				continue;
			/* Pet never interfere  - the_sandman */
			if (m_ptr->pet) continue; 
		}
		else
		{
			q_ptr = Players[-i];
			/* hostile player? */
			if (!check_hostile(Ind, -i) ||
				q_ptr->paralyzed ||
				q_ptr->stun >= 100 ||
				q_ptr->confused ||
				q_ptr->afraid ||
				(r_info[q_ptr->body_monster].flags1 & RF1_NEVER_MOVE))
				continue;
#ifdef ENABLE_STANCES
			if (q_ptr->combat_stance == 1) switch(q_ptr->combat_stance_power) {
				case 0: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 23);
				case 1: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 26);
				case 2: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 29);
				case 3: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 35);
			} else if (q_ptr->combat_stance == 2) switch(q_ptr->combat_stance_power) {
				case 0: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 53);
				case 1: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 56);
				case 2: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 59);
				case 3: chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 65);
			} else
#endif
			chance += get_skill_scale(q_ptr, SKILL_INTERCEPT, 50);
		}

		if (chance > 95) chance = 95;
		if (rand_int(100) < chance)
		{
			char m_name[MNAME_LEN];
			if (i > 0)
			{
				monster_desc(Ind, m_name, i, 0);
			}
			else
			{
				/* FIXME: even not visible... :( */
//				strcpy(m_name, q_ptr->name);
				/* fixed :) */
				player_desc(Ind, m_name, -i, 0);
			}
			msg_format(Ind, "\377%c%^s interferes with your attempt!", COLOUR_IC_MON, m_name);
			return TRUE;
		}
	}
#if 0
	for (tx = x - 1; tx <= x + 1; tx++)
	{
		for (ty = y - 1; ty <= y + 1; ty++)
		{
			if (!(i = cave[p_ptr->dun_depth][ty][tx].m_idx)) continue;
			if (i > 0)
			{
				if (r_info[m_list[i].r_idx].flags1 & RF1_NEVER_MOVE)
					continue;
			}
			else
			{
				/* hostile player? */
				if (!check_hostile(Ind, -i) ||
					Players[-i]->paralyzed ||
					Players[-i]->stunned >= 100 ||
					Players[-i]->confused ||
					r_info[Players[-i]->body_monster].flags1 & RF1_NEVER_MOVE)
					continue;
			}

			if (rand_int(100) < chance)
			{
				char m_name[MNAME_LEN];
				if (i > 0)
				{
					monster_desc(Ind, m_name, i, 0);
				}
				else
				{
					/* even not visible... :( */
					strcpy(m_name, Players[-i]->name);
				}
				msg_format(Ind, "\377%c%^s interferes with your attempt!", COLOUR_IC_MON, m_name);
				return TRUE;
			}
		}
	}
#endif

	return FALSE;
}


/*
 * Throw an object from the pack or floor.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Should throwing a weapon do full damage?  Should it allow the magic
 * to hit bonus of the weapon to have an effect?  Should it ever cause
 * the item to be destroyed?  Should it do any damage at all?
 */
void do_cmd_throw(int Ind, int dir, int item, char bashing)
{
	player_type *p_ptr = Players[Ind], *q_ptr;
	struct worldpos *wpos=&p_ptr->wpos;

	int                     i, j, y, x, ny, nx, ty, tx;
	int                     chance, tdam, tdis;
	int                     mul, div;
	int                     cur_dis, visible, real_dis;
	int			moved_number = 1;

	object_type         throw_obj;
	object_type             *o_ptr;

	bool            hit_body = FALSE, target_ok = target_okay(Ind);

	bool hit_wall = FALSE;

	int                     missile_attr;
	int                     missile_char;

#ifdef OPTIMIZED_ANIMATIONS
	/* Projectile path */
	int path_y[MAX_RANGE];
	int path_x[MAX_RANGE];
	int path_num = 0;
#endif

	char            o_name[ONAME_LEN];
	u32b f1, f2, f3, f4, f5, esp;

	char brand_msg[80] = { '\0' };

	cave_type **zcave;
	if(!(zcave=getcave(wpos))) return;

	/* New '+' feat in 4.4.6.2 */
	if (dir == 11) {
		get_aim_dir(Ind);
		p_ptr->current_throw = item;
		return;
	}

	if (dir == 5) {
		if (target_ok) {
#ifndef PY_FIRE_ON_WALL
			if (!projectable_real(Ind, p_ptr->py, p_ptr->px, p_ptr->target_row, p_ptr->target_col, MAX_RANGE))
#else
			if (!projectable_wall_real(Ind, p_ptr->py, p_ptr->px, p_ptr->target_row, p_ptr->target_col, MAX_RANGE))
#endif
				return;
		} else return;
	}

	/* throwing works even while in WRAITHFORM (due to slow speed of
	   object, allowing it to lose the wraithform ;) */

	/* Break goi/manashield */
#if 0
	if (p_ptr->invuln) set_invuln(Ind, 0);
	if (p_ptr->tim_manashield) set_tim_manashield(Ind, 0);
	if (p_ptr->tim_wraith) set_tim_wraith(Ind, 0);
#endif	// 0

	/* Access the item (if in the pack) */
	if (item >= 0) o_ptr = &(p_ptr->inventory[item]);
	else {
		if (-item >= o_max)
			return; /* item doesn't exist */
		o_ptr = &o_list[0 - item];
	}

	if (o_ptr->tval == 0) {
		if (!bashing) msg_print(Ind, "\377rYou cannot throw that.");
		else msg_print(Ind, "\377rYou cannot bash that.");
		return;
	}

	if (!bashing) {
		/* Handle the newbies_cannot_drop option */
#if (STARTEQ_TREATMENT == 1)
		if ((p_ptr->max_plv < cfg.newbies_cannot_drop || p_ptr->inval) && !is_admin(p_ptr)
		    && !(o_ptr->tval==1 && o_ptr->sval>=9 /* rugby ball, chess pieces */)) // && (object_value(0, o_ptr) > 0))
		{
/*			msg_format(Ind, "Please don't litter the %s.",
			    istown(wpos) ? "town":(wpos->wz ? "dungeon":"Nature"));*/
			msg_format(Ind, "You need to be at least level %d to throw items.", cfg.newbies_cannot_drop);
			return;
		}
#endif
	}

	if (!bashing && check_guard_inscription( o_ptr->note, 'v' )) {
		msg_print(Ind, "The item's inscription prevents it.");
		return;
	};

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Hack - Cannot throw away 'no drop' cursed items */
	if (cursed_p(o_ptr) && (f4 & TR4_CURSE_NO_DROP) && item >= 0 && !bashing) {
		/* Oops */
		msg_print(Ind, "Hmmm, you seem to be unable to throw it.");

		/* Nope */
		return;
	}

	if (!bashing) {
#if (STARTEQ_TREATMENT > 1)
	        if (o_ptr->owner == p_ptr->id && p_ptr->max_plv < cfg.newbies_cannot_drop && !is_admin(p_ptr) &&
    		    !((o_ptr->tval == 1) && (o_ptr->sval >= 9)))
            		o_ptr->level = 0;
#endif
	}

	/* S(he) is no longer afk */
	un_afk_idle(Ind);

	/* Handle rugby ball */
	if (o_ptr->tval == 1 && o_ptr->sval == 9 && !bashing){
		msg_print(Ind, "\377yYou pass the ball");
		msg_format_near(Ind, "\377y%s passes the ball", p_ptr->name);
	}

	break_cloaking(Ind, 0);
	break_shadow_running(Ind);
	stop_precision(Ind);
	stop_shooting_till_kill(Ind);


	/* Create a "local missile object" */
	throw_obj = *o_ptr;
	/* hack: if we kick (bash) certain types of items around, move the
	   whole stack instead of just one.
	   note: currently number has no effect on throwing range (weight!) */
	if (bashing) {
		if ((o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_TORCH)
		    || is_cheap_misc(o_ptr->tval))
//		    || is_ammo(o_ptr->tval))
			moved_number = o_ptr->number;
	}
	throw_obj.number = moved_number;


	/*
	 * Hack -- If rods or wands are dropped, the total maximum timeout or 
	 * charges need to be allocated between the two stacks.  If all the items 
	 * are being dropped, it makes for a neater message to leave the original 
	 * stack's pval alone. -LM-
	 */
	if (o_ptr->tval == TV_WAND) throw_obj.pval = divide_charged_item(o_ptr, 1);

	/* Reduce and describe inventory */
	if (item >= 0) {
		inven_item_increase(Ind, item, -moved_number);
		inven_item_describe(Ind, item);
		inven_item_optimize(Ind, item);
	}

	/* Reduce and describe floor item */
	else {
		floor_item_increase(0 - item, -moved_number);
		floor_item_optimize(0 - item);
	}

	/* Use the local object */
	o_ptr = &throw_obj;

	/* Description */
	object_desc(Ind, o_name, o_ptr, FALSE, 3);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(o_ptr);
	missile_char = object_char(o_ptr);


	/* Extract a "distance multiplier" */
	mul = 10;

	/* Enforce a minimum "weight" of one pound */
	div = ((o_ptr->weight > 10) ? o_ptr->weight : 10);

	/* Hack -- Distance -- Reward strength, penalize weight */
	tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;

	/* hack for rugby - all throws capped */
	if (o_ptr->tval == 1 && o_ptr->sval == 9) {
		if (tdis < 5) tdis = 5;
	}

	/* Max distance of 10 */
	if (tdis > 10) tdis = 10;

	/* Fruit bat/water bashing? Actually limit throwing too! */
	if (bashing == 2) tdis = 1; /* at least allow minimal item movement, especially for bashing piles */

	/* Chance of hitting */
	chance = (p_ptr->skill_tht + (p_ptr->to_h * BTH_PLUS_ADJ));


	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);


	/* Start at the player */
	y = p_ptr->py;
	x = p_ptr->px;

	/* Predict the "target" location */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_ok) {
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}


	/* Hack -- Handle stuff */
	handle_stuff(Ind);


	/* Travel until stopped */
	for (cur_dis = real_dis = 0; real_dis <= tdis; ) {
		/* Hack -- Stop at the target */
		if ((y == ty) && (x == tx)) break;

		/* Calculate the new location (see "project()") */
		ny = y;
		nx = x;
		mmove2(&ny, &nx, p_ptr->py, p_ptr->px, ty, tx);

#ifdef DOUBLE_LOS_SAFETY
	    /* skip checks if we already used projectable..() routines to
	       determine that it's fine, so it'd be redundant and also require
	       additional code to comply with DOUBLE_LOS_SAFETY. */
	    if (dir != 5) {
#endif
		/* Stopped by protected grids (Inn doors, also stopping monsters' ranged attacks!) */
		if (f_info[zcave[ny][nx].feat].flags1 & (FF1_BLOCK_LOS | FF1_BLOCK_CONTACT)) break;

		/* Stopped by walls/doors */
		if (!cave_los(zcave, ny, nx)) {
			hit_wall = TRUE;
			break;
		}
#ifdef DOUBLE_LOS_SAFETY
	    }
#endif

		/* Advance the distance */
		cur_dis++;

		real_dis = distance(p_ptr->py, p_ptr->px, ny, nx);

		/* Save the new location */
		x = nx;
		y = ny;

#ifndef OPTIMIZED_ANIMATIONS
		/* Save the old "player pointer" */
		q_ptr = p_ptr;

		/* Display it for each player */
		for (i = 1; i < NumPlayers + 1; i++) {
			int dispx, dispy;

			/* Use this player */
			p_ptr = Players[i];

			/* If he's not playing, skip him */
			if (p_ptr->conn == NOT_CONNECTED)
				continue;

			/* If he's not here, skip him */
			if(!inarea(wpos, &p_ptr->wpos))
				continue;

			/* The player can see the (on screen) missile */
			if (panel_contains(y, x) && player_can_see_bold(i, y, x)) {
				/* Draw, Hilite, Fresh, Pause, Erase */
				dispy = y - p_ptr->panel_row_prt;
				dispx = x - p_ptr->panel_col_prt;

				/* Remember the projectile */
				p_ptr->scr_info[dispy][dispx].c = missile_char;
				p_ptr->scr_info[dispy][dispx].a = missile_attr;

				/* Tell the client */
				Send_char(i, dispx, dispy, missile_attr, missile_char);

				/* Flush and wait */
				if (cur_dis % 2) Send_flush(i);

				/* Restore */
				lite_spot(i, y, x);
			}
		}

		/* Restore the player pointer */
		p_ptr = q_ptr;

#else /* OPTIMIZED_ANIMATIONS */

		/* Save the projectile path */
		if (path_num < MAX_RANGE)
		{
			path_y[path_num] = y;
			path_x[path_num] = x;
			path_num++;
		}
#endif /* OPTIMIZED_ANIMATIONS */

		/* Player here, try to hit him */
		if (zcave[y][x].m_idx < 0) {
			/* rugby */
			if(o_ptr->tval == 1 && o_ptr->sval == 9){
				cave_type *c_ptr = &zcave[y][x];
				if(rand_int(11) > 6){
					msg_format_near(0 - c_ptr->m_idx, "\377y%s catches the ball", Players[0-c_ptr->m_idx]->name);
					msg_print(0 - c_ptr->m_idx, "\377yYou catch the ball");
					inven_carry(0 - c_ptr->m_idx, o_ptr);
				}
				else{
					msg_format_near(0 - c_ptr->m_idx, "\377r%s misses the ball", Players[0-c_ptr->m_idx]->name);
					msg_print(0 - c_ptr->m_idx, "\377rYou miss the ball");
					o_ptr->marked2 = ITEM_REMOVAL_NEVER;
					drop_near(o_ptr, -1, wpos, y, x);
				}
				/* and stop */
				return;
			}
			if (cfg.use_pk_rules == PK_RULES_NEVER) {
				/* Stop looking */
				break;
			} else {
				cave_type *c_ptr = &zcave[y][x];

				q_ptr = Players[0 - c_ptr->m_idx];

				/* Check the visibility */
				visible = p_ptr->play_vis[0 - c_ptr->m_idx];

				/* Note the collision */
				hit_body = TRUE;

				/* Did we hit him (penalize range) */
#ifndef PVP_AC_REDUCTION
				if (test_hit_fire(chance - cur_dis, q_ptr->ac + q_ptr->to_a, visible)) {
#else
//				if (test_hit_fire(chance - cur_dis, ((q_ptr->ac + q_ptr->to_a) * 2) / 3, visible)) {
				if (test_hit_fire(chance - cur_dis,
				    (q_ptr->ac + q_ptr->to_a > AC_CAP) ? AC_CAP : q_ptr->ac + q_ptr->to_a,
				    visible)) {
#endif
					char p_name[80];

					/* Get his name */
					strcpy(p_name, q_ptr->name);

					/* Hack -- Base damage from thrown object */
					tdam = damroll(o_ptr->dd, o_ptr->ds);
					tdam = tot_dam_aux_player(Ind, o_ptr, tdam, q_ptr, brand_msg, TRUE);
					tdam += o_ptr->to_d;
					/* Apply special damage XXX XXX XXX */
					tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h, tdam);
//less spam for now - C. Blue		if (strlen(brand_msg) > 0) msg_print(Ind, brand_msg);

					/* No negative damage */
					if (tdam < 0) tdam = 0;

					/* XXX Reduce damage by 1/3 */
					tdam = (tdam + PVP_THROW_DAM_REDUCTION - 1) / PVP_THROW_DAM_REDUCTION;

					/* Handle unseen player */
					if (!visible) {
						/* Messages */
						msg_format(Ind, "The %s finds a mark!", o_name);
						msg_format(0 - c_ptr->m_idx, "You are hit by a %s!", o_name);
					}

					/* Handle visible player */
					else {
						/* Messages */
						msg_format(Ind, "The %s hits %s for \377y%d \377wdamage.", o_name, p_name, tdam);
						if ((o_name[0] == 'A') || (o_name[0] == 'E') || (o_name[0] == 'I') || (o_name[0] == 'O') || (o_name[0] == 'U') ||
						    (o_name[0] == 'a') || (o_name[0] == 'e') || (o_name[0] == 'i') || (o_name[0] == 'o') || (o_name[0] == 'u'))
							msg_format(0 - c_ptr->m_idx, "%^s hits you with an %s for \377R%d \377wdamage.", p_ptr->name, o_name, tdam);
						else	msg_format(0 - c_ptr->m_idx, "%^s hits you with a %s for \377R%d \377wdamage.", p_ptr->name, o_name, tdam);

						/* Track player's health */
						health_track(Ind, c_ptr->m_idx);
					}
//less spam for now - C. Blue		if (strlen(brand_msg) > 0) msg_print(Ind, brand_msg);

					/* Take damage */
					take_hit(0 - c_ptr->m_idx, tdam, p_ptr->name, Ind);

					/* Stop looking */
					break;
				}
			}
		}

		/* Monster here, Try to hit it */
		if (zcave[y][x].m_idx > 0) {
			cave_type *c_ptr = &zcave[y][x];

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
                        monster_race *r_ptr = race_inf(m_ptr);

			/* Check the visibility */
			visible = p_ptr->mon_vis[c_ptr->m_idx];

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, m_ptr->ac, visible)) {
				bool fear = FALSE;
				char m_name[MNAME_LEN];

				/* Get "the monster" or "it" */
				monster_desc(Ind, m_name, c_ptr->m_idx, 0);


				/* Assume a default death */
				cptr note_dies = " dies";

				/* Some monsters get "destroyed" */
				if ((r_ptr->flags3 & RF3_DEMON) ||
				    (r_ptr->flags3 & RF3_UNDEAD) ||
				    (r_ptr->flags2 & RF2_STUPID) ||
				    (strchr("Evg", r_ptr->d_char)))
				{
					/* Special note at death */
					note_dies = " is destroyed";
				}

				/* Handle reflection - it's back, though weaker */
#if 0 /* hm doesn't make sense, but is pretty unbalanced even :/ */
				/* New: Boomerangs can't be deflected, nor can exploding ammo (!) - C. Blue */
				if ((r_ptr->flags2 & RF2_REFLECTING) && !boomerang && !o_ptr->pval && magik(50)) {
#else
				if ((r_ptr->flags2 & RF2_REFLECTING) && magik(50)) {
#endif
					if (visible) msg_format(Ind, "The %s was deflected.", o_name);
					break;
				}

#ifdef USE_BLOCKING
			        /* handle blocking (deflection) */
			        if (strchr("hHJkpPty", r_ptr->d_char) && /* leaving out Yeeks (else Serpent Man 'J') */
			            !(r_ptr->flags3 & RF3_ANIMAL) && !(r_ptr->flags8 & RF8_NO_BLOCK)
			            && !rand_int(5 - r_ptr->level / 50)) { /* decent chance to block throws */
			                if (visible) {
			                        char hit_desc[MAX_CHARS];
			                        sprintf(hit_desc, "\377%c%s blocks.", COLOUR_BLOCK_MON, m_name);
			                        hit_desc[0] = toupper(hit_desc[0]);
			                        msg_print(Ind, hit_desc);
			                }
			                break;
			        }
#endif


				/* Handle unseen monster */
				if (!visible) {
					/* Invisible monster */
					msg_format(Ind, "The %s finds a mark.", o_name);
				}

				/* Handle visible monster */
				else {
					/* Message */
					msg_format(Ind, "The %s hits %s.", o_name, m_name);

					/* Hack -- Track this monster race */
					if (visible) recent_track(m_ptr->r_idx);

					/* Hack -- Track this monster */
					if (visible) health_track(Ind, c_ptr->m_idx);
				}

				/* Hack -- Base damage from thrown object */
				tdam = damroll(o_ptr->dd, o_ptr->ds);
				tdam = tot_dam_aux(Ind, o_ptr, tdam, m_ptr, brand_msg, TRUE);
				tdam += o_ptr->to_d;
				/* Apply special damage XXX XXX XXX */
				tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h, tdam);
//less spam for now - C. Blue	if (strlen(brand_msg) > 0) msg_print(Ind, brand_msg);

				/* No negative damage */
				if (tdam < 0) tdam = 0;

				if (p_ptr->admin_godly_strike) {
					p_ptr->admin_godly_strike--;
					tdam = m_ptr->hp + 1;
				}

				/* Hit the monster, check for death */
				if (mon_take_hit(Ind, c_ptr->m_idx, tdam, &fear, note_dies)) {
					/* Dead monster */
				}

				/* No death */
				else {
					/* Message */
					message_pain(Ind, c_ptr->m_idx, tdam);

					/* Take note */
					if (fear && visible) {
						char m_name[MNAME_LEN];

#ifdef USE_SOUND_2010
#else
						sound(Ind, SOUND_FLEE);
#endif

						/* Get the monster name (or "it") */
						monster_desc(Ind, m_name, c_ptr->m_idx, 0);

						/* Message */
						if (!streq(m_name, "Morgoth, Lord of Darkness"))
							msg_format(Ind, "%^s flees in terror!", m_name);
						else
							msg_format(Ind, "%^s retreats!", m_name);
					}
				}

				/* Stop looking */
				break;
			}
		}
	}

#ifdef OPTIMIZED_ANIMATIONS
	if (path_num) {
		/* Pick a random spot along the path */
		j = rand_int(path_num);
		ny = path_y[j];
		nx = path_x[j];

		/* Save the old "player pointer" */
		q_ptr = p_ptr;

		/* Draw a projectile here for everyone */
		for (i = 1; i < NumPlayers + 1; i++) {
			/* Use this player */
			p_ptr = Players[i];

			/* If he's not playing, skip him */
			if (p_ptr->conn == NOT_CONNECTED)
				continue;

			/* If he's not here, skip him */
			if(!inarea(&p_ptr->wpos, wpos))
				continue;

			/* The player can see the (on screen) missile */
			if (panel_contains(ny, nx) && player_can_see_bold(i, ny, nx))
			{
				int dispx, dispy;

				/* Draw */
				dispy = ny - p_ptr->panel_row_prt;
				dispx = nx - p_ptr->panel_col_prt;

				/* Remember the projectile */
				p_ptr->scr_info[dispy][dispx].c = missile_char;
				p_ptr->scr_info[dispy][dispx].a = missile_attr;

				/* Tell the client */
				Send_char(i, dispx, dispy, missile_attr, missile_char);

				/* Flush once */
				Send_flush(i);
			}

			/* Restore later */
			everyone_lite_later_spot(wpos, ny, nx);
		}

		/* Restore the player pointer */
		p_ptr = q_ptr;
	}
#endif /* OPTIMIZED_ANIMATIONS */

	/* Chance of breakage (during attacks) */
	j = (hit_body ? breakage_chance(o_ptr) : 0);

	/* Potions smash open */
	/* BTW, why not o_ptr->tval ? :) */
	if (k_info[o_ptr->k_idx].tval == TV_POTION ||
		k_info[o_ptr->k_idx].tval == TV_POTION2 ||
		k_info[o_ptr->k_idx].tval == TV_FLASK ||
		k_info[o_ptr->k_idx].tval == TV_BOTTLE)
	{
		if ((hit_body) || (hit_wall) || (randint(100) < j)) {
			/* Message */
			/* TODO: handle blindness */
			msg_format_near_site(y, x, wpos, 0, TRUE, "The %s shatters!", o_name);
#ifdef USE_SOUND_2010
			sound_near_site(y, x, wpos, 0, "shatter_potion", NULL, SFX_TYPE_MISC, FALSE);
#endif

//			if (potion_smash_effect(0, wpos, y, x, o_ptr->sval))
			if (k_info[o_ptr->k_idx].tval == TV_POTION) {
				if (potion_smash_effect(0 - Ind, wpos, y, x, o_ptr->sval))
//				if (potion_smash_effect(PROJECTOR_POTION, wpos, y, x, o_ptr->sval))
				{
#if 0
					if (cave[y][x].m_idx) {
						char m_name[MNAME_LEN];
						monster_desc(m_name, &m_list[cave[y][x].m_idx], 0);
						switch (is_friend(&m_list[cave[y][x].m_idx]))
						{
							case 1:
								msg_format("%^s gets angry!", m_name);
								change_side(&m_list[cave[y][x].m_idx]);
								break;
							case 0:
								msg_format("%^s gets angry!", m_name);
								m_list[cave[y][x].m_idx].status = MSTATUS_NEUTRAL_M;
								break;
						}
					}
#endif	// 0
				}
			}

			return;
		} else {
			j = 0;
		}
	}

	/* Drop (or break) near that location */
	drop_near_severe(Ind, o_ptr, j, wpos, y, x);
}

static void destroy_house(int Ind, struct dna_type *dna) {
	player_type *p_ptr = Players[Ind];
	int i;
	if (!is_admin(p_ptr)) {
		msg_print(Ind, "\377rYour attempts to destroy the house fail.");
		return;
	}
	for (i = 0; i < num_houses; i++) {
		if (houses[i].dna == dna) {
			if (houses[i].flags & HF_STOCK) {
				msg_print(Ind, "\377oThat house may not be destroyed");
//				return;
			}
			/* quicker than copying back an array. */
			msg_print(Ind, "\377DThe house crumbles away.");
			fill_house(&houses[i], FILL_MAKEHOUSE, NULL);
			houses[i].flags |= HF_DELETED;

#if 0 /* let's do this in fill_house(), so it also takes care of poly-houses - C. Blue */
			/* redraw map so change becomes visible */
			if (houses[i].flags & HF_RECT) {
				for (x = 0; x < houses[i].coords.rect.width; x++)
				for (y = 0; y < houses[i].coords.rect.height; y++)
					everyone_lite_spot(&houses[i].wpos, houses[i].y + y, houses[i].x + x);
			}
#endif
			break;
		}
	}
}

void house_admin(int Ind, int dir, char *args){
	player_type *p_ptr=Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;
	int x,y;
	int success=0;
	cave_type *c_ptr;
	struct dna_type *dna;
	cave_type **zcave;
	if (!(zcave = getcave(wpos))) return;	//(FALSE);

	if(dir && args){
		/* Get requested direction */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
		/* Get requested grid */
		c_ptr = &zcave[y][x];
		if (c_ptr->feat == FEAT_HOME || c_ptr->feat == FEAT_HOME_OPEN) {
			struct c_special *cs_ptr;
			if ((cs_ptr = GetCS(c_ptr, CS_DNADOOR))) {
				/* Test if we just want to enter a player store: Doesn't require access. */
				if (args[0] == 'S') {
					disturb(Ind, 1, 0);
					if (!do_cmd_player_store(Ind, x, y))
						msg_print(Ind, "There is no player store there.");
					return;
				}

				dna = cs_ptr->sc.ptr;
				if (access_door(Ind, dna, FALSE) || admin_p(Ind)) {
					switch (args[0]) {
						case 'O':
							success=chown_door(Ind, dna, args, x, y);
							break;
						case 'M':
							success=chmod_door(Ind, dna, args);
							break;
						case 'K':
							destroy_house(Ind, dna);
							return;
						case 'P':
							paint_house(Ind, x, y, atoi(&args[1]));
							return;
					}
					if (success) {
						msg_print(Ind,"\377gChange successful.");
						/* take note of door colour change */
						everyone_lite_spot(wpos, y, x);
					} else msg_print(Ind,"\377oChange failed.");
				} else msg_print(Ind,"\377oAccess not permitted.");
			} else msg_print(Ind,"\377yNot a door of a private house.");
		} else msg_print(Ind,"\377sThere is no home door there.");
	}
	return;
}

/*
 * Buy a house.  It is assumed that the player already knows the
 * price.
 
 Hacked to sell houses for half price. -APD-
 Doesn't use a turn / disable AFK at this time. - C. Blue
 */
void do_cmd_purchase_house(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;

	int y, x, h_idx;
	int factor;
//	int64_t price; /* I'm hoping this will be 64 bits.  I dont know if it will be portable. */
//	s64b price;
	s32b price;

	cave_type *c_ptr = NULL;
	struct dna_type *dna;
	cave_type **zcave;
	if (!(zcave = getcave(wpos))) return;

	/* Ghosts cannot buy houses */
	if ((p_ptr->ghost) && !is_admin(p_ptr)) {
		/* Message */
		msg_print(Ind, "You cannot buy a house.");

		return;
	}

	if (p_ptr->inval) {
		msg_print(Ind, "\377yYou may not buy/sell a house, wait for an admin to validate your account.");
		return;
	}


	if(dir > 9) dir = 0;	/* temp hack */

	/* Be sure we have a direction */
	if (dir > 0) {
		struct c_special *cs_ptr;
		/* Get requested direction */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get requested grid */
		if (in_bounds2(wpos, y, x)) c_ptr = &zcave[y][x];

		/* Check for a house */
		if (!(c_ptr && (c_ptr->feat == FEAT_HOME || c_ptr->feat == FEAT_HOME_OPEN)
		    && (cs_ptr = GetCS(c_ptr, CS_DNADOOR)))) {
			/* No house, message */
			msg_print(Ind, "You see nothing to buy/sell there.");
			return;
		}

		dna = cs_ptr->sc.ptr;

		h_idx = pick_house(&p_ptr->wpos, y, x);
		/* paranoia */
		if (h_idx == -1) {
			msg_print(Ind, "That's not a house to buy/sell!");
			return;
		}


		/* Take player's CHR into account somewhat */
		factor = (100 + adj_chr_gold[p_ptr->stat_ind[A_CHR]]) / 2;

		/* Check for already-owned house */
		if (dna->owner) {
			price = dna->price / factor * 100;
			if (price < 100) price = 100;

			if (access_door(Ind, dna, FALSE) || admin_p(Ind)) {
				if (p_ptr->dna == dna->creator) {
					if (!gain_au(Ind, price / 2, FALSE, FALSE)) return;

					/* sell house */
					msg_format(Ind, "You sell your house for %d gold.", price / 2);
					if (dna->owner_type != OT_GUILD) {
						p_ptr->houses_owned--;
						if (houses[h_idx].flags & HF_MOAT) p_ptr->castles_owned--;
					}

					/* make sure we don't get stuck by selling it while inside - C. Blue */
					if (zcave[p_ptr->py][p_ptr->px].info & CAVE_ICKY)
						teleport_player_force(Ind, 1);
				} else {
					if (!is_admin(p_ptr)) {
						msg_print(Ind,"You cannot sell that house");
						return;
					}
					else
						msg_print(Ind,"The house is reset");
				}
				if (dna->owner_type == OT_GUILD) {
					guilds[dna->owner].h_idx = 0;
					Send_guild_config(dna->owner);

					/* and in case it was suspended due to leaderlessness,
					   so the next guild buying this house won't get a surprise.. */
					houses[h_idx].flags &= ~HF_GUILD_SUS;
					fill_house(&houses[h_idx], FILL_GUILD_SUS_UNDO, NULL);
				}
				dna->creator = 0L;
				dna->owner = 0L;
				dna->a_flags = ACF_NONE;

				/* Erase house contents */
				kill_house_contents(&houses[h_idx]);

				/* take note of door colour change */
				c_ptr->feat = FEAT_HOME; /* make sure door is closed, in case it was open when we sold it */
				everyone_lite_spot(wpos, y, x);
				return;
			}
			msg_print(Ind,"That house does not belong to you!");
			return;
		}

		/* we are buying */
		price = dna->price / 100 * factor;
		if (price < 100) price = 100;

		if (price>p_ptr->au) {
			msg_print(Ind,"You do not have enough gold!");
			return;
		}
		if (cfg.houses_per_player && (p_ptr->houses_owned >= ((p_ptr->lev > 50 ? 50 : p_ptr->lev) / cfg.houses_per_player)) && !is_admin(p_ptr)) {
			if ((int)(p_ptr->lev / cfg.houses_per_player) == 0)
				msg_format(Ind, "You need to be at least level %d to buy a house!", cfg.houses_per_player);
			else if ((int)(p_ptr->lev / cfg.houses_per_player) == 1)
				msg_print(Ind, "At your level, you cannot own more than 1 house!");
			else
				msg_format(Ind, "At your level, you cannot own more than %d houses!", (int)((p_ptr->lev > 50 ? 50 : p_ptr->lev) / cfg.houses_per_player));
			return;
		}
		if (houses[h_idx].flags & HF_MOAT) {
			if (cfg.castles_for_kings && !p_ptr->total_winner) {
				msg_print(Ind, "You must be king or queen, emperor or empress, to buy a castle!");
				return;
			}
			if (cfg.castles_per_player && (p_ptr->castles_owned >= cfg.castles_per_player))  {
				if (cfg.castles_per_player == 1)
					msg_format(Ind, "You cannot own more than 1 castle!");
				else
					msg_format(Ind, "You cannot own more than %d castles!", cfg.castles_per_player);
				return;
			}
		}
		msg_format(Ind, "You buy the house for %d gold.", price);
		p_ptr->au -= price;
		p_ptr->houses_owned++;
		if (houses[h_idx].flags & HF_MOAT) p_ptr->castles_owned++;
		dna->creator = p_ptr->dna;
		dna->mode = p_ptr->mode;
		dna->owner = p_ptr->id;
		dna->owner_type = OT_PLAYER;
		dna->a_flags = ACF_NONE;
		dna->min_level = 1;

		/* Erase house contents */
		kill_house_contents(&houses[h_idx]);

		/* Redraw */
		p_ptr->redraw |= (PR_GOLD);

		/* take note of door colour change */
		everyone_lite_spot(wpos, y, x);
	}
}

/* King own a territory */
void do_cmd_own(int Ind)
{
	player_type *p_ptr = Players[Ind];
	char buf[100];
	
	if (!p_ptr->total_winner)
	{
		msg_format(Ind, "You must be a %s to own a land!", (p_ptr->male)?"king":"queen");
		return;
	}

	if (!p_ptr->own1.wx && !p_ptr->own2.wx && !p_ptr->own1.wy && !p_ptr->own2.wy && !p_ptr->own1.wz && !p_ptr->own2.wz)
	{
		msg_print(Ind, "You can't own more than 2 terrains.");
		return;
	}
	
	if (wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].own)
	{
		msg_print(Ind, "Sorry this land is owned by someone else.");
		return;
	}
	if(p_ptr->wpos.wz)
	{
		msg_print(Ind, "Sorry you can't own the dungeon");
		return;
	}
	
	if(istownarea(&p_ptr->wpos, 3))
	{
		msg_print(Ind, "Sorry this land is owned by the town.");
		return;
	}
	
	/* Ok all check did lets appropriate */
	if(p_ptr->own1.wx || p_ptr->own1.wy || p_ptr->own1.wz)
	{
		wpcopy(&p_ptr->own2, &p_ptr->wpos);
	}
	else
	{
		wpcopy(&p_ptr->own1, &p_ptr->wpos);
	}
	wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].own = p_ptr->id;
	
	if (p_ptr->mode & (MODE_HARD | MODE_NO_GHOST)) {
		snprintf(buf, sizeof(buf), "%s %s now owns (%d,%d).", (p_ptr->male)?"Emperor":"Empress", p_ptr->name, p_ptr->wpos.wx, p_ptr->wpos.wy);
	} else {
		snprintf(buf, sizeof(buf), "%s %s now owns (%d,%d).", (p_ptr->male)?"King":"Queen", p_ptr->name, p_ptr->wpos.wx, p_ptr->wpos.wy);
	}
	msg_broadcast(Ind, buf);
	msg_print(Ind, buf);
}



/* mental fusion - mindcrafter special ability */
void do_cmd_fusion(int Ind) {
#ifdef TEST_SERVER
	player_type *p_ptr = Players[Ind], *q_ptr;
	cave_type **zcave;//, *c_ptr;
	int x, y, dir, i;

	if (p_ptr->esp_link) {
		msg_print(Ind, "\377yYou are already in a mind fusion.");
		return;
	}

	if (!(zcave = getcave(&p_ptr->wpos))) return;

	/* search grids adjacent to player for someone with open mind to fuse */
	for (dir = 0; dir < 8; dir++) {
		y = p_ptr->py + ddy_ddd[dir];
		x = p_ptr->px + ddx_ddd[dir];
		if (!in_bounds2(&p_ptr->wpos, y, x)) continue;

		/* Get requested grid, check for friendly open minded player */
		i = -zcave[y][x].m_idx;
		if (i <= 0) continue;
		q_ptr = Players[i];

		/* Skip disconnected players */
		if (q_ptr->conn == NOT_CONNECTED) continue;
		/* Skip DM if not DM himself */
		if (q_ptr->admin_dm && !p_ptr->admin_dm) continue;
		/* Skip Ghosts */
		if (q_ptr->ghost && !q_ptr->admin_dm) continue;
		/* Skip players not in the same party */
		if (q_ptr->party == 0 || p_ptr->party != q_ptr->party)
		/* Skip players who haven't opened their mind */
		if (!(q_ptr->esp_link_flags & LINKF_OPEN)) continue;
		/* Skip players who are already in a fusion */
		if (q_ptr->esp_link) continue;

		/* found one - establish! */
		q_ptr->esp_link = p_ptr->id;
		p_ptr->esp_link = q_ptr->id;
		q_ptr->esp_link_type = LINK_DOMINANT; /* transmit to me */
		p_ptr->esp_link_type = LINK_DOMINATED; /* receive from him */
		p_ptr->esp_link_end = q_ptr->esp_link_end = 0;
		q_ptr->esp_link_flags = LINKF_VIEW | LINKF_PAIN | LINKF_MISC | LINKF_OBJ;
		p_ptr->esp_link_flags = LINKF_VIEW_DEDICATED; /* don't show own map info */
		p_ptr->update |= PU_MUSIC; /* don't hear own music */
		/* redraw map of target player, which will redraw our view too */
		q_ptr->redraw |= PR_MAP;

		/* notify and done */
		msg_format(Ind, "\377BMind fusion with %s was successful!", q_ptr->name);
		msg_format(i, "\377BMind fusion with %s was successful!", p_ptr->name);
		return;
	}

	/* failure */
	msg_print(Ind, "\377yNo eligible target found nearby.");
#endif
}

/*
 * Rogue special ability - enter cloaking (stealth+search+special invisibility) mode - C. Blue
 */
void do_cmd_cloak(int Ind)
{
	player_type *p_ptr = Players[Ind];

#ifdef ENABLE_MCRAFT
 #if 0 /* instead via spell schools atm */
	/* hack: also use the key for fusion, 
	   a new mindcrafter special ability - C. Blue */
	if (p_ptr->pclass == CLASS_MINDCRAFTER) {
		do_cmd_fusion(Ind);
		return;
	}
 #endif
#endif

#ifdef ARCADE_SERVER
return;
#endif

#ifndef ENABLE_CLOAKING
	return;
#endif
	/* Don't allow cloaking while inside a store */
	if (p_ptr->store_num != -1) return;

        if (p_ptr->pclass != CLASS_ROGUE) return;

    if (!p_ptr->cloaked) {
	if (p_ptr->lev < LEARN_CLOAKING_LEVEL) {
		msg_format(Ind, "\377yYou need to be level %d to learn how to cloak yourself effectively.", LEARN_CLOAKING_LEVEL);
		return;
	}
	if (p_ptr->body_monster) { /* in case of vampire bat, for vampire rogue! */
		msg_print(Ind, "\377yYou cannot cloak yourself while in bat form.");
		return;
	}
	if (p_ptr->rogue_heavyarmor) {
		msg_print(Ind, "\377yYour armour is too heavy to cloak yourself effectively.");
		return;
	}
	if (p_ptr->inventory[INVEN_WIELD].k_idx && (k_info[p_ptr->inventory[INVEN_WIELD].k_idx].flags4 & (TR4_MUST2H | TR4_SHOULD2H))
	    && !instakills(Ind)) {
		msg_print(Ind, "\377yYour weapon is too large to cloak yourself effectively.");
		return;
	}
	if (p_ptr->inventory[INVEN_ARM].k_idx && p_ptr->inventory[INVEN_ARM].tval == TV_SHIELD) {
		msg_print(Ind, "\377yYou cannot cloak yourself effectively while carrying a shield.");
		return;
	}
	if (p_ptr->stormbringer) {
		msg_print(Ind, "\377yYou cannot cloak yourself while wielding the Stormbringer!");
		return;
	}
	if (p_ptr->aggravate) {
		msg_print(Ind, "\377yYou cannot cloak yourself while using aggravating items!");
		return;
	}
	if (p_ptr->ghost) {
		msg_print(Ind, "\377yYou cannot cloak yourself while you are dead!");
		if (!is_admin(p_ptr)) return;
	}
	if (p_ptr->blind) {
		msg_print(Ind, "\377yYou cannot cloak yourself while blind.");
		return;
	}
	if (p_ptr->confused) {
		msg_print(Ind, "\377yYou cannot cloak yourself while confused.");
		return;
	}
	if (p_ptr->stun >= 50) {
		msg_print(Ind, "\377yYou cannot cloak yourself while heavily stunned.");
		return;
	}
	if (p_ptr->paralyzed) { /* just paranoia, no energy anyway */
		msg_print(Ind, "\377yYou cannot cloak yourself while paralyzed.");
		if (!is_admin(p_ptr)) return;
	}
    }

	/* S(he) is no longer afk */
//	if (p_ptr->afk) toggle_afk(Ind, "");
	p_ptr->idle_char = 0;

	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

	if (p_ptr->cloaked > 1) {
		stop_cloaking(Ind);
	} else if (p_ptr->cloaked == 1) {
		msg_print(Ind, "\377WYou uncloak yourself.");
		p_ptr->cloaked = 0;
	} else {
		break_shadow_running(Ind);
		stop_precision(Ind);
		stop_shooting_till_kill(Ind);
		/* stop other actions like running.. */
		disturb(Ind, 1, 0);
		/* prepare to cloak.. */
		msg_print(Ind, "\377sYou begin to cloak your appearance...");
		msg_format_near(Ind, "\377w%s begins to cloak %s appearance...", p_ptr->name, p_ptr->male ? "his" : "her");
		p_ptr->cloaked = 10; /* take this number of seconds-1 to complete */
	}
	p_ptr->update |= (PU_BONUS | PU_LITE | PU_VIEW | PU_MONSTERS);
	p_ptr->redraw |= (PR_STATE | PR_SPEED);
}

/* break cloaking -
   'discovered=0' means that the character just cannot keep himself cloaked,
   while 'discovered > 0' stands for a suspicious action that might have been
   noted by an observing monster and hence neutralizes his disguise for n turns. - C. Blue */
void break_cloaking(int Ind, int discovered) {
	if (!discovered) {
		if (Players[Ind]->cloaked) {
			msg_print(Ind, "\377oYour camouflage drops!");
			if (Players[Ind]->cloaked == 1) msg_format_near(Ind, "%s appears before your eyes.", Players[Ind]->name);
			Players[Ind]->cloaked = 0;
			Players[Ind]->update |= (PU_BONUS | PU_LITE | PU_VIEW);
			Players[Ind]->redraw |= (PR_STATE | PR_SPEED);
		}
	}
	/* neutralize cloak effect for a number of <discovered> turns,
	   due to a suspicious action we performed that might have been
	   noticed by a monster */
	if (discovered > Players[Ind]->cloak_neutralized) Players[Ind]->cloak_neutralized = discovered;
}

/* stop cloaking preparations */
void stop_cloaking(int Ind) {
	if (Players[Ind]->cloaked > 1) {
		msg_print(Ind, "\377yYou stop cloaking preparations.");
		Players[Ind]->cloaked = 0;
		Players[Ind]->update |= (PU_BONUS | PU_LITE | PU_VIEW | PU_MONSTERS);
		Players[Ind]->redraw |= (PR_STATE | PR_SPEED);

	}
}

/* stop ranged technique 'barrage' preparation */
void stop_precision(int Ind) {
	if (Players[Ind]->ranged_precision) {
		Players[Ind]->ranged_precision = FALSE;
		msg_print(Ind, "\377yYou stop aiming.");
	}
}

/* stop shooting-till-kill */
void stop_shooting_till_kill(int Ind) {
	Players[Ind]->shooting_till_kill = FALSE;
	Players[Ind]->shoot_till_kill_book = 0;
	Players[Ind]->shoot_till_kill_spell = 0;
	if (Players[Ind]->shoot_till_kill_rcraft) {
		Players[Ind]->FTK_e_flags = 0;
		Players[Ind]->FTK_m_flags = 0;
		Players[Ind]->FTK_energy = 0;
	}
}

/*
 * Rogue special ability - shadow running mode - C. Blue
 */
void shadow_run(int Ind)
{
	player_type *p_ptr = Players[Ind];

        if (p_ptr->shadow_running) {
		p_ptr->shadow_running = FALSE;
                msg_print(Ind, "Your silhouette stabilizes and your movements return to normal.");
                msg_format_near(Ind, "%s silhouette stabilizes and %s movements return to normal.", p_ptr->name, p_ptr->male ? "his" : "her");
	        p_ptr->update |= (PU_BONUS | PU_VIEW);
	        p_ptr->redraw |= (PR_STATE | PR_SPEED);
	        /* update so everyone sees the colour animation */
	        everyone_lite_spot(&p_ptr->wpos, p_ptr->py, p_ptr->px);
		return;
        }

	if (p_ptr->rogue_heavyarmor) {
		msg_print(Ind, "\377yYour armour is too heavy for effective shadow running.");
		return;
	}
	if (p_ptr->inventory[INVEN_WIELD].k_idx && (k_info[p_ptr->inventory[INVEN_WIELD].k_idx].flags4 & (TR4_MUST2H | TR4_SHOULD2H))
	    && !instakills(Ind)) {
		msg_print(Ind, "\377yYour weapon is too large for effective shadow running.");
		return;
	}
	if (p_ptr->inventory[INVEN_ARM].k_idx && p_ptr->inventory[INVEN_ARM].tval == TV_SHIELD) {
		msg_print(Ind, "\377yYou cannot shadow run effectively while carrying a shield.");
		return;
	}
	if (p_ptr->stormbringer) {
		msg_print(Ind, "\377yYou cannot shadow run while wielding the Stormbringer!");
		return;
	}
	if (p_ptr->ghost) {
		msg_print(Ind, "\377yYou cannot shadow run while you are dead!");
		if (!is_admin(p_ptr)) return;
	}
	if (p_ptr->blind) {
		msg_print(Ind, "\377yYou cannot shadow run while blind.");
		return;
	}
	if (p_ptr->confused) {
		msg_print(Ind, "\377yYou cannot shadow run while confused.");
		return;
	}
	if (p_ptr->stun) {
		msg_print(Ind, "\377yYou cannot start shadow running while stunned.");
		return;
	}
	if (p_ptr->paralyzed) { /* just paranoia, no energy anyway */
		msg_print(Ind, "\377yYou cannot start shadow running while paralyzed.");
		if (!is_admin(p_ptr)) return;
	}

	if (p_ptr->cst < 10) { msg_print(Ind, "Not enough stamina!"); return; }

	p_ptr->cst -= 10;
	un_afk_idle(Ind);
	disturb(Ind, 1, 0); /* stop resting, searching and running */

	break_cloaking(Ind, 0);
	stop_precision(Ind);
	stop_shooting_till_kill(Ind);
	p_ptr->shadow_running = TRUE;
	msg_print(Ind, "Your silhouette turns shadowy and your movements become lightning-fast!");
	msg_format_near(Ind, "%s's silhouette turns shadowy and %s movements become lightning-fast!", p_ptr->name, p_ptr->male ? "his" : "her");
	p_ptr->update |= (PU_BONUS | PU_VIEW);
	p_ptr->redraw |= (PR_STATE | PR_SPEED);
	/* update so everyone sees the colour animation */
	everyone_lite_spot(&p_ptr->wpos, p_ptr->py, p_ptr->px);
}

/* break shadow running */
void break_shadow_running(int Ind) {
	if (Players[Ind]->shadow_running) {
	        msg_print(Ind, "Your silhouette stabilizes and your movements return to normal.");
    		msg_format_near(Ind, "%s silhouette stabilizes and %s movements return to normal.", Players[Ind]->name, Players[Ind]->male ? "his" : "her");
		Players[Ind]->shadow_running = FALSE;
	        Players[Ind]->update |= (PU_BONUS | PU_VIEW);
    		Players[Ind]->redraw |= (PR_STATE | PR_SPEED);
		/* update so everyone sees the colour animation */
                everyone_lite_spot(&Players[Ind]->wpos, Players[Ind]->py, Players[Ind]->px); 
	}
}
