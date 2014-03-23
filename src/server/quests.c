/* This file is for providing a framework for quests, allowing easy addition,
   editing and removal of them via a 'q_info.txt' file designed from scratch.
   You may modify/use it freely as long as you give proper credit. - C. Blue

   I hope I covered all sorts of quests, even though some quests might be a bit
   clunky to implement (quest needs to 'quietly' spawn a followup quest etc).
   Note: This is a new approach. Not to be confused with events, old xorders,
         or the sketches of quest_type, quest[] and plots[] code in bldg.c.

   Quests can make use of Send_request_key/str/num/cfr/abort()
   functions to provide the possibility of dialogue with npcs.
   Quest npcs should be neutral and non-attackable (at least
   not on accident) and should probably not show up on ESP,
   neither should quest objects show up on object detection.


   Note about current mechanics:
   To make a quest spawn a new target questor ('Find dude X in the dungeon and talk to him')
   it is currently necessary to spawn a new quest via activate_quest[], have the player
   auto_accept_quiet it and silently terminate the current quest. The new quest should
   probably have almost the same title too in case the player can review the title so as to
   not confuse him.. maybe this could be improved on in the future :-p
   how to possibly improve: spawn sub questors with their own dialogues.

   Code quirks:
   Currently instead of checking pInd against 0, all those functions additionally verify
   against p_ptr->individual to make double-sure it's a (non)individual quest.
   This is because some external functions that take 'Ind' instead of 'pInd' might just
   forward their Ind everywhere into the static functions. :-p
   (As a result, some final 'global quest' lines in set/get functions might never get
   called so they are actually obsolete.)

   Regarding party members, that could be done by: Scanning area for party members on
   questor interaction, ask them if they want to join the quest y/n, and then duplicating
   all quest message output to them. Further, any party member who is first to do so  can
   complete a goal or invoke a dialogue. Only the party member who INVOKES a dialogue can
   make a keyword-based decision, the others can just watch the dialogue passively.
   Stages cannot proceed until all party members are present at the questor, if the questor
   requires returning to him, or until all members are at the quest target sector, if any.

   Special quirk:
   Questors that are monsters use m_ptr->quest = q_idx.
   Questors that are objects, and quest items (that aren't questors) use o_ptr->quest = q_idx + 1.
*/


#define SERVER
#include "angband.h"


/* set log level (0 to disable, 1 for normal logging, 2 for debug logging,
   3 for very verbose debug logging, 4 every single goal test is logged (deliver_xy -> every step)) */
#define QDEBUG 3
/* Disable a quest on error? */
//#define QERROR_DISABLE
/* Otherwise just put it on this cooldown (minutes) */
#define QERROR_COOLDOWN 30
/* Default cooldown in minutes. */
#define QI_COOLDOWN_DEFAULT 5

/* Should we teleport item piles away to make exclusive room for questor objects,
   or should we just drop the questor object onto the pile? :) */
#define QUESTOR_OBJECT_EXCLUSIVE
/* If above is defined, should the questor item destroy all items on its grid
   when it spawns? Otherwise it'll just drop on top of them. */
#define QUESTOR_OBJECT_CRUSHES


static void quest_goal_check_reward(int pInd, int q_idx);
static bool quest_goal_check(int pInd, int q_idx, bool interacting);
static void quest_dialogue(int Ind, int q_idx, int questor_idx, bool repeat, bool interact_acquire, bool force_prompt);
static void quest_imprint_tracking_information(int Ind, int py_q_idx, bool target_flagging_only);
static void quest_check_goal_kr(int Ind, int q_idx, int py_q_idx, monster_type *m_ptr, object_type *o_ptr);


/* error messages for quest_acquire() */
static cptr qi_msg_deactivated = "\377sThis quest is currently unavailable.";
static cptr qi_msg_cooldown = "\377yYou cannot acquire this quest again just yet.";
static cptr qi_msg_prereq = "\377yYou have not completed the required prerequisite quests yet.";
static cptr qi_msg_minlev = "\377yYour level is too low to acquire this quest.";
static cptr qi_msg_maxlev = "\377oYour level is too high to acquire this quest.";
static cptr qi_msg_race = "\377oYour race is not eligible to acquire this quest.";
static cptr qi_msg_class = "\377oYour class is not eligible to acquire this quest.";
static cptr qi_msg_truebat = "\377oYou must be a born fruit bat to acquire this quest.";
static cptr qi_msg_bat = "\377yYou must be a fruit bat to acquire this quest.";
static cptr qi_msg_form = "\377yYou cannot acquire this quest in your current form.";
static cptr qi_msg_mode = "\377oYour character mode is not eligible for acquiring this quest.";
static cptr qi_msg_done = "\377oYou cannot acquire this quest again.";
static cptr qi_msg_max = "\377yYou are already pursuing the maximum possible number of concurrent quests.";


/* Called every minute to check which quests to activate/deactivate depending on time/daytime */
void process_quests(void) {
	quest_info *q_ptr;
	int i, j;
	bool night = IS_NIGHT_RAW, day = !night, morning = IS_MORNING, forenoon = IS_FORENOON, noon = IS_NOON;
	bool afternoon = IS_AFTERNOON, evening = IS_EVENING, midnight = IS_MIDNIGHT, deepnight = IS_DEEPNIGHT;
	int minute = bst(MINUTE, turn), hour = bst(HOUR, turn);
	bool active;
	qi_stage *q_stage;

	for (i = 0; i < max_q_idx; i++) {
		q_ptr = &q_info[i];
		if (q_ptr->defined == FALSE) continue;

		if (q_ptr->cur_cooldown) q_ptr->cur_cooldown--;
		if (q_ptr->disabled || q_ptr->cur_cooldown) continue;

		/* check if quest should be active */
		active = FALSE;

		if (q_ptr->night && night) active = TRUE; /* don't include pseudo-night like for Halloween/Newyearseve */
		if (q_ptr->day && day) active = TRUE;

		/* specialty: if day is true, then these can falsify it again */
		if (!q_ptr->day) {
			if (q_ptr->morning && morning) active = TRUE;
			if (q_ptr->forenoon && forenoon) active = TRUE;
			if (q_ptr->noon && noon) active = TRUE;
			if (q_ptr->afternoon && afternoon) active = TRUE;
			if (q_ptr->evening && evening) active = TRUE;
		} else {
			if (!q_ptr->morning && morning) active = FALSE;
			if (!q_ptr->forenoon && forenoon) active = FALSE;
			if (!q_ptr->noon && noon) active = FALSE;
			if (!q_ptr->afternoon && afternoon) active = FALSE;
			if (!q_ptr->evening && evening) active = FALSE;
		}
		/* specialty: if night is true, then these can falsify it again */
		if (!q_ptr->night) {
			if (q_ptr->evening && evening) active = TRUE;
			if (q_ptr->midnight && midnight) active = TRUE;
			if (q_ptr->deepnight && deepnight) active = TRUE;
		} else {
			if (!q_ptr->evening && evening) active = FALSE;
			if (!q_ptr->midnight && midnight) active = FALSE;
			if (!q_ptr->deepnight && deepnight) active = FALSE;
		}

		/* very specific time? (in absolute daily minutes) */
		if (q_ptr->time_start != -1) {
			if (minute >= q_ptr->time_start) {
				if (q_ptr->time_stop == -1 ||
				    minute < q_ptr->time_stop)
					active = TRUE;
			}
		}

		/* quest is inactive and must be activated */
		if (!q_ptr->active && active)
			quest_activate(i);
		/* quest is active and must be deactivated */
		else if (q_ptr->active && !active)
			quest_deactivate(i);


		/* handle individual cooldowns here too for now.
		   NOTE: this means player has to stay online for cooldown to expire, hmm */
		for (j = 1; j <= NumPlayers; j++)
			if (Players[j]->quest_cooldown[i])
				Players[j]->quest_cooldown[i]--;


		q_stage = quest_cur_qi_stage(i);
		/* handle automatically timed stage actions */ //TODO: implement this for individual quests too
		if (q_stage->timed_countdown < 0) {
			if (hour == -q_stage->timed_countdown)
				quest_set_stage(0, i, q_stage->timed_countdown_stage, q_stage->timed_countdown_quiet);
		} else if (q_stage->timed_countdown > 0) {
			q_stage->timed_countdown--;
			if (!q_stage->timed_countdown)
				quest_set_stage(0, i, q_stage->timed_countdown_stage, q_stage->timed_countdown_quiet);
		}
	}
}

/* Helper function to pick one *set* flag at random,
   for determining questor spawn locations */
static u32b quest_pick_flag(u32b flagarray, int size) {
	int i;
	int flags = 0, choice;

	if (flagarray == 0x0) return 0x0;

	/* count flags that are set */
	for (i = 0; i < size; i++)
		if ((flagarray & (0x1 << i))) flags++;

	/* pick one */
	choice = randint(flags);

	/* translate back into flag and return it */
	for (i = 0; i < size; i++) {
		if (!(flagarray & (0x1 << i))) continue;
		if (--choice) continue;
		return (0x1 << i);
	}
	return 0x0; //paranoia
}

static bool quest_special_spawn_location(struct worldpos *wpos, s16b *x_result, s16b *y_result, struct qi_location *qi_loc) {
	int i, tries;
	cave_type **zcave;
	u32b choice, wild = RF8_WILD_TOO_MASK & ~(RF8_WILD_TOWN | RF8_WILD_EASY);
	int x, y, x2 = 63, y2 = 63; //compiler warning

	wpos->wx = 63; //default for cases of acute paranoia
	wpos->wy = 63;
	wpos->wz = 0;

		/* ---------- Try to pick a (random) spawn location ---------- */

		/* check 'L:' info in q_info.txt for details on starting location */
#if QDEBUG > 2
		s_printf(" SWPOS,XY: %d,%d,%d - %d,%d\n", qi_loc->start_wpos.wx, qi_loc->start_wpos.wy, qi_loc->start_wpos.wz, qi_loc->start_x, qi_loc->start_y);
		s_printf(" SLOCT, STAR: %d,%d\n", qi_loc->s_location_type, qi_loc->s_towns_array);
#endif
		/* specified exact questor starting wpos? */
		if (qi_loc->start_wpos.wx != -1) {
			wpos->wx = qi_loc->start_wpos.wx;
			wpos->wy = qi_loc->start_wpos.wy;
			wpos->wz = qi_loc->start_wpos.wz;

			/* vary strict wpos a bit into adjacent terrain patches of same type? */
			if (qi_loc->terrain_patch && !wpos->wz) {
				tries = 500;
				while (--tries) {
					x2 = wpos->wx - QI_TERRAIN_PATCH_RADIUS + rand_int(QI_TERRAIN_PATCH_RADIUS * 2 + 1);
					y2 = wpos->wy - QI_TERRAIN_PATCH_RADIUS + rand_int(QI_TERRAIN_PATCH_RADIUS * 2 + 1);
					if (distance(y2, x2, wpos->wy, wpos->wx) <= QI_TERRAIN_PATCH_RADIUS &&
					    wild_info[y2][x2].type == wild_info[wpos->wy][wpos->wx].type) break;
				}
				if (tries) { /* yay */
					wpos->wx = x2;
					wpos->wy = y2;
				}
			}
		}
		/* ok, pick one starting location randomly from all eligible ones */
		else {
			choice = quest_pick_flag(qi_loc->s_location_type, 4);
			/* no or non-existing type specified */
			if (!choice) return FALSE;

			switch (choice) {
			case QI_SLOC_SURFACE:
				/* all terrains are possible? */
				if ((qi_loc->s_terrains & RF8_WILD_TOO)) {
					choice = quest_pick_flag(wild, 32);
				}
				/* pick from specified list */
				else choice = quest_pick_flag(qi_loc->s_terrains & wild, 32);
				/* no or non-existing type specified */
				if (!choice) return FALSE;

				/* pick one wpos location randomly, that matches our terrain */
				tries = 2000;
				while (--tries) {
					x = rand_int(MAX_WILD_X);
					y = rand_int(MAX_WILD_Y);
					switch (wild_info[y][x].type) {
					case WILD_OCEAN: if (choice == RF8_WILD_OCEAN) break;
					case WILD_LAKE: if (choice == RF8_WILD_OCEAN) break; /* hmm */
					case WILD_GRASSLAND: if (choice == RF8_WILD_GRASS) break;
					case WILD_FOREST: if (choice == RF8_WILD_WOOD) break;
					case WILD_SWAMP: if (choice == RF8_WILD_SWAMP) break;
					case WILD_DENSEFOREST: if (choice == RF8_WILD_WOOD) break;
					case WILD_WASTELAND: if (choice == RF8_WILD_WASTE) break;
					case WILD_DESERT: if (choice == RF8_WILD_DESERT) break;
					case WILD_ICE: if (choice == RF8_WILD_ICE) break;
					}
				}
				if (!tries) /* engage emergency eloquence */
					for (x = 0; x < MAX_WILD_X; x++)
						for (y = 0; y < MAX_WILD_Y; y++) {
							switch (wild_info[y][x].type) {
							case WILD_OCEAN: if (choice == RF8_WILD_OCEAN) break;
							case WILD_LAKE: if (choice == RF8_WILD_OCEAN) break; /* hmm */
							case WILD_GRASSLAND: if (choice == RF8_WILD_GRASS) break;
							case WILD_FOREST: if (choice == RF8_WILD_WOOD) break;
							case WILD_SWAMP: if (choice == RF8_WILD_SWAMP) break;
							case WILD_DENSEFOREST: if (choice == RF8_WILD_WOOD) break;
							case WILD_WASTELAND: if (choice == RF8_WILD_WASTE) break;
							case WILD_DESERT: if (choice == RF8_WILD_DESERT) break;
							case WILD_ICE: if (choice == RF8_WILD_ICE) break;
							}
						}
				/* paranoia.. */
				if (x == MAX_WILD_X && y == MAX_WILD_Y) {
					/* back to test server :-p */
					x = MAX_WILD_X - 1;
					y = MAX_WILD_Y - 1;
				}

				wpos->wx = x;
				wpos->wy = y;
				wpos->wz = 0;
				break;

			case QI_SLOC_TOWN:
				choice = quest_pick_flag(qi_loc->s_towns_array, 5);
				/* no or non-existing type specified */
				if (!choice) return FALSE;

				/* assume non-dungeon town */
				wpos->wz = 0;

				switch (choice) { /* TODO: such hardcode. wow. */
				case QI_STOWN_BREE:
					wpos->wx = 32;
					wpos->wy = 32;
					break;
				case QI_STOWN_GONDOLIN:
					wpos->wx = 27;
					wpos->wy = 13;
					break;
				case QI_STOWN_MINASANOR:
					wpos->wx = 25;
					wpos->wy = 58;
					break;
				case QI_STOWN_LOTHLORIEN:
					wpos->wx = 59;
					wpos->wy = 51;
					break;
				case QI_STOWN_KHAZADDUM:
					wpos->wx = 26;
					wpos->wy = 5;
					break;
				case QI_STOWN_WILD:
					return FALSE;
					break;
				case QI_STOWN_DUNGEON:
					return FALSE;
					break;
				case QI_STOWN_IDDC:
					return FALSE;
					break;
				case QI_STOWN_IDDC_FIXED:
					return FALSE;
					break;
				}
				break;

			//TODO implement location types
			case QI_SLOC_DUNGEON:
				return FALSE;
				break;

			case QI_SLOC_TOWER:
				return FALSE;
				break;
			}
		}

		/* Allocate & generate cave */
		if (!(zcave = getcave(wpos))) {
			//dealloc_dungeon_level(wpos);
			alloc_dungeon_level(wpos);
			generate_cave(wpos, NULL);
			zcave = getcave(wpos);
		}

		/* Specified exact starting location within given wpos? */
		if (qi_loc->start_x != -1) {
			x = qi_loc->start_x;
			y = qi_loc->start_y;

			/* vary strict starting loc a bit, within a radius? */
			if (qi_loc->radius) {
				tries = 100;
				while (--tries) {
					x2 = qi_loc->start_x - qi_loc->radius + rand_int(qi_loc->radius * 2 + 1);
					y2 = qi_loc->start_y - qi_loc->radius + rand_int(qi_loc->radius * 2 + 1);
					if (!in_bounds(y2, x2)) continue;
					if (!cave_naked_bold(zcave, y2, x2)) continue;
					if (distance(y2, x2, y, x) <= qi_loc->radius) break;
				}
				if (!tries) { /* no fun */
					x = qi_loc->start_x;
					y = qi_loc->start_y;
				}
			}
		} else {
			/* find a random spawn loc */
			i = 1000; //tries
			while (i) {
				i--;
				/* not too close to level borders maybe */
				x = rand_int(MAX_WID - 6) + 3;
				y = rand_int(MAX_HGT - 6) + 3;
				if (!in_bounds(y2, x2)) continue;
				if (!cave_naked_bold(zcave, y, x)) continue;
				break;
			}
			if (!i) {
				return FALSE;
				s_printf("QUEST_SPECIAL_SPAWN_LOCATION: No free spawning location.\n");
			}
		}

		*x_result = x;
		*y_result = y;

	return TRUE;
}

static bool questor_monster(int q_idx, qi_questor *q_questor, int questor_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	int i, m_idx;
	cave_type **zcave, *c_ptr;
	monster_race *r_ptr, *rbase_ptr;
	monster_type *m_ptr;

	/* data written back to q_info[] */
	struct worldpos wpos = {63, 63, 0}; //default for cases of acute paranoia
	int x, y;


	wpos.wx = q_questor->current_wpos.wx;
	wpos.wy = q_questor->current_wpos.wy;
	wpos.wz = q_questor->current_wpos.wz;

	x = q_questor->current_x;
	y = q_questor->current_y;

	zcave = getcave(&wpos);
	c_ptr = &zcave[y][x];


	/* If no monster race index is set for the questor, don't spawn him. (paranoia) */
	if (!q_questor->ridx) return FALSE;

	/* ---------- Try to spawn the questor monster ---------- */

	m_idx = m_pop();
	if (!m_idx) {
		s_printf(" QUEST_CANCELLED: No free monster index to pop questor.\n");
		q_ptr->active = FALSE;
#ifdef QERROR_DISABLE
		q_ptr->disabled = TRUE;
#else
		q_ptr->cur_cooldown = QERROR_COOLDOWN;
#endif
		return FALSE;
	}

	/* make sure no other player/moster is occupying our spawning grid */
	if (c_ptr->m_idx < 0) {
		int Ind = -c_ptr->m_idx;
		player_type *p_ptr = Players[Ind];

		teleport_player(Ind, 1, TRUE);
		/* check again.. */
		if (c_ptr->m_idx < 0) teleport_player(Ind, 10, TRUE);
		/* and again.. (someone funny stone-walled the whole map?) */
		if (c_ptr->m_idx < 0) teleport_player(Ind, 200, TRUE);
		/* check again. If still here, just transport him to Bree for now -_- */
		if (c_ptr->m_idx < 0) {
			p_ptr->new_level_method = LEVEL_RAND;
			p_ptr->recall_pos.wx = cfg.town_x;
			p_ptr->recall_pos.wy = cfg.town_y;
			p_ptr->recall_pos.wz = 0;
			recall_player(Ind, "A strange force teleports you far away.");
		}
	} else if (c_ptr->m_idx > 0) {
		/* is it ANOTHER questor? uhoh */
		if (m_list[c_ptr->m_idx].questor) {
			/* we don't mess with questor locations for consistencies sake */
			s_printf(" QUEST_CANCELLED: Questor of quest %d occupies questor spawn location.\n", m_list[c_ptr->m_idx].quest);
			q_ptr->active = FALSE;
#ifdef QERROR_DISABLE
			q_ptr->disabled = TRUE;
#else
			q_ptr->cur_cooldown = QERROR_COOLDOWN;
#endif
			return FALSE;
		}

		teleport_away(c_ptr->m_idx, 1);
		/* check again.. */
		if (c_ptr->m_idx > 0) teleport_away(c_ptr->m_idx, 2);
		/* aaaand again.. */
		if (c_ptr->m_idx > 0) teleport_away(c_ptr->m_idx, 10);
		/* wow. this patience. */
		if (c_ptr->m_idx > 0) teleport_away(c_ptr->m_idx, 200);
		/* out of patience */
		if (c_ptr->m_idx > 0) delete_monster_idx(c_ptr->m_idx, TRUE);
	}
	c_ptr->m_idx = m_idx;

	m_ptr = &m_list[m_idx];
	MAKE(m_ptr->r_ptr, monster_race);
	r_ptr = m_ptr->r_ptr;
	rbase_ptr = &r_info[q_questor->ridx];

	m_ptr->questor = TRUE;
	m_ptr->questor_idx = questor_idx;
	m_ptr->quest = q_idx;
	m_ptr->r_idx = q_questor->ridx;
	/* m_ptr->special = TRUE; --nope, this is unfortunately too much golem'ized.
	   Need code cleanup!! Maybe rename it to m_ptr->golem and add a new m_ptr->special. */
	r_ptr->extra = 0;
	m_ptr->mind = 0;
	m_ptr->owner = 0;

	r_ptr->flags1 = rbase_ptr->flags1;
	r_ptr->flags2 = rbase_ptr->flags2;
	r_ptr->flags3 = rbase_ptr->flags3;
	r_ptr->flags4 = rbase_ptr->flags4;
	r_ptr->flags5 = rbase_ptr->flags5;
	r_ptr->flags6 = rbase_ptr->flags6;
	r_ptr->flags7 = rbase_ptr->flags7;
	r_ptr->flags8 = rbase_ptr->flags8;
	r_ptr->flags9 = rbase_ptr->flags9;
	r_ptr->flags0 = rbase_ptr->flags0;

	r_ptr->flags1 |= RF1_FORCE_MAXHP;
	r_ptr->flags3 |= RF3_RES_TELE | RF3_RES_NEXU;
	r_ptr->flags7 |= RF7_NO_TARGET | RF7_NEVER_ACT;
	if (q_questor->invincible) r_ptr->flags7 |= RF7_NO_DEATH; //for now we just use NO_DEATH flag for invincibility
	r_ptr->flags8 |= RF8_GENO_PERSIST | RF8_GENO_NO_THIN | RF8_ALLOW_RUNNING | RF8_NO_AUTORET;
	r_ptr->flags9 |= RF9_IM_TELE;

	r_ptr->text = 0;
	r_ptr->name = rbase_ptr->name;
	m_ptr->fx = x;
	m_ptr->fy = y;

	r_ptr->d_attr = q_questor->rattr;
	r_ptr->d_char = q_questor->rchar;
	r_ptr->x_attr = q_questor->rattr;
	r_ptr->x_char = q_questor->rchar;
	
	r_ptr->aaf = rbase_ptr->aaf;
	r_ptr->mexp = rbase_ptr->mexp;
	r_ptr->hdice = rbase_ptr->hdice;
	r_ptr->hside = rbase_ptr->hside;
	
	m_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
	m_ptr->hp = m_ptr->maxhp;
	m_ptr->org_maxhp = m_ptr->maxhp; /* CON */
	m_ptr->speed = rbase_ptr->speed;
	m_ptr->mspeed = m_ptr->speed;
	m_ptr->ac = rbase_ptr->ac;
	m_ptr->org_ac = m_ptr->ac; /* DEX */

	m_ptr->level = rbase_ptr->level;
	m_ptr->exp = MONSTER_EXP(m_ptr->level);

	for (i = 0; i < 4; i++) {
		m_ptr->blow[i].effect = rbase_ptr->blow[i].effect;
		m_ptr->blow[i].method = rbase_ptr->blow[i].method;
		m_ptr->blow[i].d_dice = rbase_ptr->blow[i].d_dice;
		m_ptr->blow[i].d_side = rbase_ptr->blow[i].d_side;

		m_ptr->blow[i].org_d_dice = rbase_ptr->blow[i].d_dice;
		m_ptr->blow[i].org_d_side = rbase_ptr->blow[i].d_side;
	}

	r_ptr->freq_innate = rbase_ptr->freq_innate;
	r_ptr->freq_spell = rbase_ptr->freq_spell;

#ifdef MONSTER_ASTAR
	if (r_ptr->flags0 & RF0_ASTAR) {
		/* search for an available A* table to use */
		for (i = 0; i < ASTAR_MAX_INSTANCES; i++) {
			/* found an available instance? */
			if (astar_info_open[i].m_idx == -1) {
				astar_info_open[i].m_idx = m_idx;
				astar_info_open[i].nodes = 0; /* init: start with empty set of nodes */
				astar_info_closed[i].nodes = 0; /* init: start with empty set of nodes */
				m_ptr->astar_idx = i;
				break;
			}
		}
		/* no instance available? Mark us (-1) to use normal movement instead */
		if (i == ASTAR_MAX_INSTANCES) {
			m_ptr->astar_idx = -1;
			/* cancel quest because of this? */
			s_printf(" QUEST_CANCELLED: No free A* index for questor.\n");
			q_ptr->active = FALSE;
 #ifdef QERROR_DISABLE
			q_ptr->disabled = TRUE;
 #else
			q_ptr->cur_cooldown = QERROR_COOLDOWN;
 #endif
			return FALSE;
		}
	}
#endif

	m_ptr->clone = 0;
	m_ptr->cdis = 0;
	wpcopy(&m_ptr->wpos, &wpos);

	m_ptr->stunned = 0;
	m_ptr->confused = 0;
	m_ptr->monfear = 0;
	//r_ptr->sleep = rbase_ptr->sleep;
	r_ptr->sleep = 0;

	m_ptr->questor_invincible = q_questor->invincible; //for now handled by RF7_NO_DEATH
	m_ptr->questor_hostile = FALSE;
	q_questor->mo_idx = m_idx;

	update_mon(c_ptr->m_idx, TRUE);
#if QDEBUG > 1
	s_printf(" QUEST_SPAWNED: Questor '%s' (m_idx %d) at %d,%d,%d - %d,%d.\n", q_questor->name, m_idx, wpos.wx, wpos.wy, wpos.wz, x, y);
#endif

	q_questor->talk_focus = 0;
	return TRUE;
}

#ifdef QUESTOR_OBJECT_EXCLUSIVE
 #ifndef QUESTOR_OBJECT_CRUSHES
/* Helper function for questor_object() to teleport away a whole pile of items */
static void teleport_objects_away(struct worldpos *wpos, s16b x, s16b y, int dist) {
	u16b this_o_idx, next_o_idx = 0;
	int j;
	s16b cx, cy;

	cave_type **zcave = getcave(wpos), *c_ptr = &zcave[y][x];
	object_type tmp_obj;

  #if 1 /* scatter the pile? */
    for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx) {
	tmp_obj = o_list[this_o_idx];
	for (j = 0; j < 10; j++) {
		cx = x + dist - rand_int(dist * 2);
		cy = y + dist - rand_int(dist * 2);
		if (cx == x || cy == y) continue;

		if (!in_bounds(cy, cx)) continue;
		if (!cave_floor_bold(zcave, cy, cx) ||
		    cave_perma_bold(zcave, cy, cx)) continue;

//		(void)floor_carry(cy, cx, &tmp_obj);
		drop_near(&tmp_obj, 0, wpos, cy, cx);
		delete_object_idx(this_o_idx, FALSE);
		break;
	}
    }
  #else /* 'teleport' the pile as a whole? --- UNTESTED!!! */
	for (j = 0; j < 10; j++) {
		cx = x + dist - rand_int(dist * 2);
		cy = y + dist - rand_int(dist * 2);
		if (cx == x || cy == y) continue;

		if (!in_bounds(cy, cx)) continue;
		if (!cave_floor_bold(zcave, cy, cx) ||
		    cave_perma_bold(zcave, cy, cx)) continue;

		/* no idea if this works */
		if (!cave_naked_bold(zcave, cy, cx)) continue;
		zcave[cy][cx].o_idx = zcave[y][x].o_idx;
		zcave[y][x].o_idx = 0;
		break;
	}
  #endif
}
 #endif
#endif

static bool questor_object(int q_idx, qi_questor *q_questor, int questor_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	int o_idx, i;
	object_type *o_ptr;
	cave_type **zcave, *c_ptr;
	u32b resf = RESF_NOTRUEART;

	/* data written back to q_info[] */
	struct worldpos wpos;
	int x, y;


	wpos.wx = q_questor->current_wpos.wx;
	wpos.wy = q_questor->current_wpos.wy;
	wpos.wz = q_questor->current_wpos.wz;

	x = q_questor->current_x;
	y = q_questor->current_y;

	zcave = getcave(&wpos);
	c_ptr = &zcave[y][x];

	/* If no monster race index is set for the questor, don't spawn him. (paranoia) */
	if (!q_questor->otval || !q_questor->osval) return FALSE;


	/* ---------- Try to spawn the questor object ---------- */

	o_idx = o_pop();
	if (!o_idx) {
		s_printf(" QUEST_CANCELLED: No free object index to pop questor.\n");
		q_ptr->active = FALSE;
#ifdef QERROR_DISABLE
		q_ptr->disabled = TRUE;
#else
		q_ptr->cur_cooldown = QERROR_COOLDOWN;
#endif
		return FALSE;
	}

	/* make sure no other object is occupying our spawning grid */
	if (c_ptr->o_idx) {
		/* is it ANOTHER questor? uhoh */
		if (o_list[c_ptr->o_idx].questor) {
			/* we don't mess with questor locations for consistencies sake */
			s_printf(" QUEST_CANCELLED: Questor of quest %d occupies questor spawn location.\n", o_list[c_ptr->o_idx].quest);
			q_ptr->active = FALSE;
#ifdef QERROR_DISABLE
			q_ptr->disabled = TRUE;
#else
			q_ptr->cur_cooldown = QERROR_COOLDOWN;
#endif
			return FALSE;
		}

#ifdef QUESTOR_OBJECT_EXCLUSIVE
 #ifdef QUESTOR_OBJECT_CRUSHES
		delete_object(&wpos, y, x, TRUE);
 #else
		/* teleport the whole pile of objects away */
		teleport_objects_away(&wpos, x, y, 1);
		/* check again.. */
		if (c_ptr->o_idx > 0) teleport_objects_away(&wpos, x, y, 2);
		/* aaaand again.. */
		if (c_ptr->o_idx > 0) teleport_objects_away(&wpos, x, y, 10);
		/* Illusionate is looting D pits again */
		if (c_ptr->o_idx > 0) teleport_objects_away(&wpos, x, y, 200);
		/* out of patience */
		if (c_ptr->o_idx > 0) delete_object_idx(c_ptr->o_idx, TRUE);
 #endif
#else
		/* just drop the questor onto the pile of stuff (if any).
		   -- fall through -- */
#endif
	}

	/* 'drop' it */
	o_ptr = &o_list[o_idx];
	invcopy(o_ptr, lookup_kind(q_questor->otval, q_questor->osval));
	o_ptr->ix = x;
	o_ptr->iy = y;
	wpcopy(&o_ptr->wpos, &wpos);

	//o_ptr->attr = q_questor->oattr;//o_ptr->xtra3 =
	o_ptr->name1 = q_questor->oname1;
	o_ptr->name2 = q_questor->oname2;
	o_ptr->name2b = q_questor->oname2b;
	apply_magic(&wpos, o_ptr, -2, FALSE, q_questor->ogood, q_questor->ogreat, q_questor->overygreat, resf);
	o_ptr->pval = q_questor->opval;
	o_ptr->bpval = q_questor->obpval;
	o_ptr->level = q_questor->olev;

	o_ptr->next_o_idx = c_ptr->o_idx; /* on top of pile, if any */
	o_ptr->stack_pos = 0;
	c_ptr->o_idx = o_idx;

	o_ptr->marked = 0;
	o_ptr->held_m_idx = 0;

	/* imprint questor status and details */
	o_ptr->questor = TRUE;
	o_ptr->questor_idx = questor_idx;
	o_ptr->quest = q_idx;
	o_ptr->questor_invincible = q_questor->invincible;
	q_questor->mo_idx = o_idx;

	/* Clear visibility flags */
	for (i = 1; i <= NumPlayers; i++) Players[i]->obj_vis[o_idx] = FALSE;
	/* Note the spot */
	note_spot_depth(&wpos, y, x);
	/* Draw the spot */
	everyone_lite_spot(&wpos, y, x);

#if QDEBUG > 1
	s_printf(" QUEST_SPAWNED: Questor '%s' (o_idx %d) at %d,%d,%d - %d,%d.\n", q_questor->name, o_idx, wpos.wx, wpos.wy, wpos.wz, x, y);
#endif

	q_questor->talk_focus = 0;
	return TRUE;
}

/* Spawn questor, prepare sector/floor, make things static if requested, etc. */
bool quest_activate(int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	int i, q;
	qi_questor *q_questor;

	/* catch bad mistakes */
	if (!q_ptr->defined) {
		s_printf("QUEST_UNDEFINED: Cancelled attempt to activate quest %d.\n", q_idx);
		return FALSE;
	}

	/* Quest is now 'active' */
#if QDEBUG > 0
 #if QDEBUG > 1
	for (i = 1;i <= NumPlayers; i++)
		if (is_admin(Players[i]))
			msg_format(i, "Quest '%s' (%d,%s) activated.", q_name + q_ptr->name, q_idx, q_ptr->codename);
 #endif
	s_printf("%s QUEST_ACTIVATE: '%s'(%d,%s) by %s\n", showtime(), q_name + q_ptr->name, q_idx, q_ptr->codename, q_ptr->creator);
#endif
	q_ptr->active = TRUE;


	/* Spawn questors (monsters/objects that players can
	   interact with to acquire this quest): */
	for (q = 0; q < q_ptr->questors; q++) {
		q_questor = &q_ptr->questor[q];

		/* find a suitable spawn location for the questor */
		if (!quest_special_spawn_location(&q_questor->current_wpos, &q_questor->current_x, &q_questor->current_y, &q_questor->q_loc)) {
			s_printf(" QUEST_CANCELLED: Couldn't obtain questor spawning location.\n");
			q_ptr->active = FALSE;
#ifdef QERROR_DISABLE
			q_ptr->disabled = TRUE;
#else
			q_ptr->cur_cooldown = QERROR_COOLDOWN;
#endif
			return FALSE;
		}
		if (q_questor->static_floor) new_players_on_depth(&q_questor->current_wpos, 1, TRUE);

		/* generate and spawn the questor */
		switch (q_questor->type) {
		case QI_QUESTOR_NPC:
			if (!questor_monster(q_idx, q_questor, q)) return FALSE;
			break;
		case QI_QUESTOR_ITEM_PICKUP:
			if (!questor_object(q_idx, q_questor, q)) return FALSE;
			break;
		default: ;
			//TODO: other questor-types
			/* discard */
		}
	}

	/* Initialise with starting stage 0 */
	q_ptr->turn_activated = turn;
	q_ptr->cur_stage = -1;
	quest_set_stage(0, q_idx, 0, FALSE);

	return TRUE;
}

/* Helper function for quest_deactivate().
   Search and destroy an object-type questor, similar to erase_guild_key(). */
static void quest_erase_object_questor(int q_idx, int questor_idx) {
	int i, j;
	object_type *o_ptr;

	int slot;
	hash_entry *ptr;
	player_type *p_ptr;

	/* Players online */
	for (j = 1; j <= NumPlayers; j++) {
		p_ptr = Players[j];
		/* scan his inventory */
		for (i = 0; i < INVEN_TOTAL; i++) {
			o_ptr = &p_ptr->inventory[i];
			if (!o_ptr->k_idx) continue;
			if (!o_ptr->questor || o_ptr->quest != q_idx || o_ptr->questor_idx != questor_idx) continue;

			s_printf("QUESTOR_OBJECT_ERASE: player '%s'\n", p_ptr->name);
			inven_item_increase(j, i, -1);
			inven_item_describe(j, i);
			inven_item_optimize(j, i);
			return;
		}
	}

	/* hack */
	NumPlayers++;
	MAKE(Players[NumPlayers], player_type);
	p_ptr = Players[NumPlayers];
	p_ptr->inventory = C_NEW(INVEN_TOTAL, object_type);
	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		ptr = hash_table[slot];
		while (ptr) {
			/* clear his data (especially inventory) */
			o_ptr = p_ptr->inventory;
			WIPE(p_ptr, player_type);
			p_ptr->inventory = o_ptr;
			p_ptr->Ind = NumPlayers;
			C_WIPE(p_ptr->inventory, INVEN_TOTAL, object_type);
			/* set his supposed name */
			strcpy(p_ptr->name, ptr->name);
			/* generate savefile name */
			process_player_name(NumPlayers, TRUE);
			/* try to load him! */
			if (!load_player(NumPlayers)) {
				/* bad fail */
				s_printf("QUESTOR_OBJECT_ERASE: load_player '%s' failed\n", p_ptr->name);
				/* unhack */
				C_FREE(p_ptr->inventory, INVEN_TOTAL, object_type);
				KILL(p_ptr, player_type);
				NumPlayers--;
				return;
			}
			/* scan his inventory */
			for (i = 0; i < INVEN_TOTAL; i++) {
				o_ptr = &p_ptr->inventory[i];
				if (!o_ptr->k_idx) continue;
				if (!o_ptr->questor || o_ptr->quest != q_idx || o_ptr->questor_idx != questor_idx) continue;

				s_printf("QUESTOR_OBJECT_ERASE: savegame '%s'\n", p_ptr->name);
				o_ptr->tval = o_ptr->sval = o_ptr->k_idx = 0;
				/* write savegame back */
				save_player(NumPlayers);
				/* unhack */
				C_FREE(p_ptr->inventory, INVEN_TOTAL, object_type);
				KILL(p_ptr, player_type);
				NumPlayers--;
				return;
			}
			/* advance to next character */
			ptr = ptr->next;
		}
	}
	/* unhack */
	C_FREE(p_ptr->inventory, INVEN_TOTAL, object_type);
	KILL(p_ptr, player_type);
	NumPlayers--;

	/* hm, failed to locate the questor object. Maybe someone actually lost it. */
	s_printf("OBJECT_QUESTOR_ERASE: not found\n");
}

/* Despawn questors, unstatic sectors/floors, etc. */
void quest_deactivate(int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_questor *q_questor;
	int i, j, m_idx, o_idx;
	cave_type **zcave, *c_ptr;
	bool fail = FALSE;

	/* data reread from q_info[] */
	struct worldpos wpos;

#if QDEBUG > 0
	s_printf("%s QUEST_DEACTIVATE: '%s' (%d,%s) by %s\n", showtime(), q_name + q_ptr->name, q_idx, q_ptr->codename, q_ptr->creator);
#endif
	q_ptr->active = FALSE;


	for (i = 0; i < q_ptr->questors; i++) {
		q_questor = &q_ptr->questor[i];

		/* get quest information */
		wpos.wx = q_questor->current_wpos.wx;
		wpos.wy = q_questor->current_wpos.wy;
		wpos.wz = q_questor->current_wpos.wz;

		/* Allocate & generate cave */
		if (!(zcave = getcave(&wpos))) {
			//dealloc_dungeon_level(&wpos);
			alloc_dungeon_level(&wpos);
			generate_cave(&wpos, NULL);
			zcave = getcave(&wpos);
		}
		c_ptr = &zcave[q_questor->current_y][q_questor->current_x];

		/* unmake quest */
#if QDEBUG > 1
		s_printf(" deleting questor %d at %d,%d,%d - %d,%d\n", c_ptr->m_idx, wpos.wx, wpos.wy, wpos.wz, q_questor->current_x, q_questor->current_y);
#endif

		switch (q_questor->type) {
		case QI_QUESTOR_NPC:
			m_idx = c_ptr->m_idx;
			if (m_idx == q_questor->mo_idx) {
				if (m_list[m_idx].questor && m_list[m_idx].quest == q_idx) {
#if QDEBUG > 1
					s_printf(" ..ok.\n");
#endif
					delete_monster_idx(c_ptr->m_idx, TRUE);
				} else {
#if QDEBUG > 1
					s_printf(" ..failed: Monster is not a questor or has a different quest idx.\n");
#endif
					fail = TRUE;
				}
			} else {
#if QDEBUG > 1
				s_printf(" ..failed: Monster has different idx than the questor.\n");
#endif
				fail = TRUE;
			}
			/* scan the entire monster list to catch the questor */
			if (fail) {
#if QDEBUG > 1
				s_printf(" Scanning entire monster list..");
#endif
				for (j = 0; j < m_max; j++) {
					if (!m_list[j].questor) continue;
					if (m_list[j].quest != q_idx) continue;
					if (m_list[j].questor_idx != i) continue;
					s_printf("found it at %d!\n", j);
					delete_monster_idx(j, TRUE);
					//:-p break;
				}
			}
			break;
		case QI_QUESTOR_ITEM_PICKUP:
			o_idx = c_ptr->o_idx;
			if (o_idx == q_questor->mo_idx) {
				if (o_list[o_idx].questor && o_list[o_idx].quest == q_idx) {
#if QDEBUG > 1
					s_printf(" ..ok.\n");
#endif
					delete_object_idx(c_ptr->o_idx, TRUE);
				} else
#if QDEBUG > 1
					s_printf(" ..failed: Object is not a questor or has a different quest idx.\n");
#endif
					fail = TRUE;
			} else {
#if QDEBUG > 1
				s_printf(" ..failed: Object has different idx than the questor.\n");
#endif
				fail = TRUE;
			}
			/* scan the entire object list to catch the questor */
			if (fail)  {
#if QDEBUG > 1
				s_printf(" Scanning entire object list..");
#endif
				for (j = 0; j < o_max; j++) {
					if (!o_list[j].questor) continue;
					if (o_list[j].quest != q_idx + 1) continue;
					if (o_list[j].questor_idx != i) continue;
					s_printf("found it at %d!\n", j);
					delete_object_idx(j, TRUE);
					//:-p break;
				}
				/* last resort, like for trueart/guildkey erasure, scan everyone's inventory -_- */
				quest_erase_object_questor(q_idx, i);
			}
			break;
		default: ;
			/* discard */
		}

		if (q_questor->static_floor) new_players_on_depth(&wpos, 0, FALSE);
	}
}

/* return a current quest's cooldown. Either just uses q_ptr->cur_cooldown directly for global
   quests, or p_ptr->quest_cooldown for individual quests. */
s16b quest_get_cooldown(int pInd, int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];

	if (pInd && q_ptr->individual) return Players[pInd]->quest_cooldown[q_idx];
	return q_ptr->cur_cooldown;
}

/* set a current quest's cooldown. Either just uses q_ptr->cur_cooldown directly for global
   quests, or p_ptr->quest_cooldown for individual quests. */
void quest_set_cooldown(int pInd, int q_idx, s16b cooldown) {
	quest_info *q_ptr = &q_info[q_idx];

	if (pInd && q_ptr->individual) Players[pInd]->quest_cooldown[q_idx] = cooldown;
	else q_ptr->cur_cooldown = cooldown;
}

/* Erase all temporary quest data of the player. */
static void quest_initialise_player_tracking(int Ind, int py_q_idx) {
	player_type *p_ptr = Players[Ind];
	int i;

	/* initialise the global info by deriving it from the other
	   concurrent quests we have _except_ for py_q_idx.. oO */
	p_ptr->quest_any_k = FALSE;
	p_ptr->quest_any_k_target = FALSE;
	p_ptr->quest_any_k_within_target = FALSE;

	p_ptr->quest_any_r = FALSE;
	p_ptr->quest_any_r_target = FALSE;
	p_ptr->quest_any_r_within_target = FALSE;

	p_ptr->quest_any_deliver_xy = FALSE;
	p_ptr->quest_any_deliver_xy_within_target = FALSE;

	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++) {
		/* skip this quest (and unused quests, checked in quest_imprint_tracking_information()) */
		if (i == py_q_idx) continue;
		/* expensive mechanism, sort of */
		quest_imprint_tracking_information(Ind, i, TRUE);
	}

	/* clear direct data */
	p_ptr->quest_kill[py_q_idx] = FALSE;
	p_ptr->quest_retrieve[py_q_idx] = FALSE;

	p_ptr->quest_deliver_pos[py_q_idx] = FALSE;
	p_ptr->quest_deliver_xy[py_q_idx] = FALSE;

	/* for 'individual' quests, reset temporary quest data or it might get carried over from previous stage? */
	for (i = 0; i < QI_GOALS; i++) p_ptr->quest_goals[py_q_idx][i] = FALSE;

	/* restore correct target/deliver situational tracker ('..within_target' flags!) */
	quest_check_player_location(Ind);
}

/* a quest has successfully ended, clean up */
static void quest_terminate(int pInd, int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i, j;

	q_ptr->dirty = TRUE;

	/* give players credit */
	if (pInd && q_ptr->individual) {
		p_ptr = Players[pInd];
#if QDEBUG > 0
		s_printf("%s QUEST_TERMINATE_INDIVIDUAL: %s(%d) completes '%s'(%d,%s)\n", showtime(), p_ptr->name, pInd, q_name + q_ptr->name, q_idx, q_ptr->codename);
#endif

		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (p_ptr->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) return; //oops?

		if (p_ptr->quest_done[q_idx] < 10000) p_ptr->quest_done[q_idx]++;

		/* he is no longer on the quest, since the quest has finished */
		p_ptr->quest_idx[j] = -1;
		//good colours for '***': C-confusion (yay), q-inertia (pft, extended), E-meteor (mh, extended)
		msg_format(pInd, "\374\377C***\377u You have completed the quest \"\377U%s\377u\"! \377C***", q_name + q_ptr->name);
		//msg_print(pInd, "\374 ");

		/* don't respawn the questor *immediately* again, looks silyl */
		if (q_ptr->cooldown == -1) p_ptr->quest_cooldown[q_idx] = QI_COOLDOWN_DEFAULT;
		else p_ptr->quest_cooldown[q_idx] = q_ptr->cooldown;

		/* individual quests don't get cleaned up (aka completely reset)
		   by deactivation, except for this temporary tracking data,
		   or it would continue spamming quest checks eg on delivery_xy locs. */
		quest_initialise_player_tracking(pInd, j);
		return;
	}

#if QDEBUG > 0
	s_printf("%s QUEST_TERMINATE_GLOBAL: '%s'(%d,%s) completed by", showtime(), q_name + q_ptr->name, q_idx, q_ptr->codename);
#endif
	for (i = 1; i <= NumPlayers; i++) {
		p_ptr = Players[i];

		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (p_ptr->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) continue;

#if QDEBUG > 0
		s_printf(" %s(%d)", q_name + q_ptr->name, q_ptr->codename);
#endif
		if (p_ptr->quest_done[q_idx] < 10000) p_ptr->quest_done[q_idx]++;

		/* he is no longer on the quest, since the quest has finished */
		p_ptr->quest_idx[j] = -1;
		msg_format(i, "\374\377C***\377u You have completed the quest \"\377U%s\377u\"! \377C***", q_name + q_ptr->name);
		//msg_print(i, "\374 ");

		/* clean up temporary tracking data,
		   or it would continue spamming quest checks eg on delivery_xy locs. */
		quest_initialise_player_tracking(i, j);
	}
#if QDEBUG > 0
	s_printf(".\n");
#endif

	/* don't respawn the questor *immediately* again, looks silyl */
	if (q_ptr->cooldown == -1) q_ptr->cur_cooldown = QI_COOLDOWN_DEFAULT;
	else q_ptr->cur_cooldown = q_ptr->cooldown;

	/* clean up */
	quest_deactivate(q_idx);
}

/* return a current quest goal. Either just uses q_ptr->goals directly for global
   quests, or p_ptr->quest_goals for individual quests.
   'nisi' will cause returning FALSE if the quest goal is marked as 'nisi',
   even if it is also marked as 'completed'. */
static bool quest_get_goal(int pInd, int q_idx, int goal, bool nisi) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i, stage = quest_get_stage(pInd, q_idx);
	qi_goal *q_goal = &quest_qi_stage(q_idx, stage)->goal[goal];

	if (!pInd || !q_ptr->individual) {
		if (nisi && q_goal->nisi) return FALSE;
		return q_goal->cleared; /* no player? global goal */
	}

	p_ptr = Players[pInd];
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	if (i == MAX_CONCURRENT_QUESTS) {
		if (nisi && q_goal->nisi) return FALSE;
		return q_goal->cleared; /* player isn't on this quest. return global goal. */
	}

	if (q_ptr->individual) {
		if (nisi && p_ptr->quest_goals_nisi[i][goal]) return FALSE;
		return p_ptr->quest_goals[i][goal]; /* individual quest */
	}

	if (nisi && q_goal->nisi) return FALSE;
	return q_goal->cleared; /* global quest */
}
/* just return the 'nisi' state of a quest goal (for ungoal_r) */
static bool quest_get_goal_nisi(int pInd, int q_idx, int goal) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i, stage = quest_get_stage(pInd, q_idx);
	qi_goal *q_goal = &quest_qi_stage(q_idx, stage)->goal[goal];

	if (!pInd || !q_ptr->individual) return q_goal->nisi; /* global quest */

	p_ptr = Players[pInd];
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	if (i == MAX_CONCURRENT_QUESTS) return q_goal->nisi;  /* player isn't on this quest. return global goal. */

	if (q_ptr->individual) return p_ptr->quest_goals_nisi[i][goal]; /* individual quest */

	return q_goal->nisi; /* global quest */
}

/* Returns the current quest->stage struct. */
qi_stage *quest_cur_qi_stage(int q_idx) {
	return &q_info[q_idx].stage[q_info[q_idx].cur_stage];
}
/* Returns a quest->stage struct to a 'stage' index used in q_info.txt. */
qi_stage *quest_qi_stage(int q_idx, int stage) {
	return &q_info[q_idx].stage[q_info[q_idx].stage_idx[stage]];
}
/* return the current quest stage. Either just uses q_ptr->cur_stage directly for global
   quests, or p_ptr->quest_stage for individual quests. */
s16b quest_get_stage(int pInd, int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i;

	if (!pInd || !q_ptr->individual) return q_ptr->cur_stage; /* no player? global stage */

	p_ptr = Players[pInd];
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	if (i == MAX_CONCURRENT_QUESTS) return 0; /* player isn't on this quest: pick stage 0, the entry stage */

	if (q_ptr->individual) return p_ptr->quest_stage[i]; /* individual quest */
	return q_ptr->cur_stage; /* global quest */
}

/* return current quest flags. Either just uses q_ptr->flags directly for global
   quests, or p_ptr->quest_flags for individual quests. */
static u16b quest_get_flags(int pInd, int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i;

	if (!pInd || !q_ptr->individual) return q_ptr->flags; /* no player? global goal */

	p_ptr = Players[pInd];
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	if (i == MAX_CONCURRENT_QUESTS) return q_ptr->flags; /* player isn't on this quest. return global goal. */

	if (q_ptr->individual) return p_ptr->quest_flags[i]; /* individual quest */
	return q_ptr->flags; /* global quest */
}
/* set/clear quest flags */
static void quest_set_flags(int pInd, int q_idx, u16b set_mask, u16b clear_mask) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i;

	if (!pInd || !q_ptr->individual) {
		q_ptr->flags |= set_mask; /* no player? global flags */
		q_ptr->flags &= ~clear_mask;
		return;
	}

	p_ptr = Players[pInd];
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	if (i == MAX_CONCURRENT_QUESTS) {
		q_ptr->flags |= set_mask; /* player isn't on this quest. return global flags. */
		q_ptr->flags &= ~clear_mask;
		return;
	}

	if (q_ptr->individual) {
		p_ptr->quest_flags[i] |= set_mask; /* individual quest */
		p_ptr->quest_flags[i] &= ~clear_mask; 
		return;
	}

	/* global quest */
	q_ptr->flags |= set_mask;
	q_ptr->flags &= ~clear_mask;
}
/* according to Z lines, change flags when a quest goal has finally be resolved. */
//TODO for optional goals too..
static void quest_goal_changes_flags(int pInd, int q_idx, int stage, int goal) {
	qi_goal *q_goal = &quest_qi_stage(q_idx, stage)->goal[goal];

	quest_set_flags(pInd, q_idx, q_goal->setflags, q_goal->clearflags);
}

/* mark a quest goal as reached.
   Also check if we can now proceed to the next stage or set flags or hand out rewards.
   'nisi' is TRUE for kill/retrieve quests that depend on a delivery.
   This was added for handling flag changes induced by completing stage goals. */
static void quest_set_goal(int pInd, int q_idx, int goal, bool nisi) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i, stage = quest_get_stage(pInd, q_idx);
	qi_goal *q_goal = &quest_qi_stage(q_idx, stage)->goal[goal];

#if QDEBUG > 2
	s_printf("QUEST_GOAL_SET: (%s,%d) goal %d%s by %d\n", q_ptr->codename, q_idx, goal, nisi ? "n" : "", pInd);
#endif
	if (!pInd || !q_ptr->individual) {
		if (!q_goal->cleared || !nisi) q_goal->nisi = nisi;
		q_goal->cleared = TRUE; /* no player? global goal */

		/* change flags according to Z lines? */
		if (!q_goal->nisi) quest_goal_changes_flags(0, q_idx, stage, goal);

		(void)quest_goal_check(0, q_idx, FALSE);
		return;
	}

	p_ptr = Players[pInd];
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	if (i == MAX_CONCURRENT_QUESTS) {
		if (!q_goal->cleared || !nisi) q_goal->nisi = nisi;
		q_goal->cleared = TRUE; /* player isn't on this quest. return global goal. */

		/* change flags according to Z lines? */
		if (!q_goal->nisi) quest_goal_changes_flags(0, q_idx, stage, goal);

		(void)quest_goal_check(0, q_idx, FALSE);
		return;
	}

	if (q_ptr->individual) {
		if (!p_ptr->quest_goals[i][goal] || !nisi) p_ptr->quest_goals_nisi[i][goal] = nisi;
		p_ptr->quest_goals[i][goal] = TRUE; /* individual quest */

		/* change flags according to Z lines? */
		if (!p_ptr->quest_goals_nisi[i][goal]) quest_goal_changes_flags(pInd, q_idx, stage, goal);

		(void)quest_goal_check(pInd, q_idx, FALSE);
		return;
	}

	if (!q_goal->cleared || !nisi) q_goal->nisi = nisi;
	q_goal->cleared = TRUE; /* global quest */
	/* change flags according to Z lines? */

	if (!q_goal->nisi) quest_goal_changes_flags(0, q_idx, stage, goal);

	/* also check if we can now proceed to the next stage or set flags or hand out rewards */
	(void)quest_goal_check(0, q_idx, FALSE);
}
/* mark a quest goal as no longer reached. ouch. */
static void quest_unset_goal(int pInd, int q_idx, int goal) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr;
	int i, stage = quest_get_stage(pInd, q_idx);
	qi_goal *q_goal = &quest_qi_stage(q_idx, stage)->goal[goal];

#if QDEBUG > 2
	s_printf("QUEST_GOAL_UNSET: (%s,%d) goal %d by %d\n", q_ptr->codename, q_idx, goal, pInd);
#endif
	if (!pInd || !q_ptr->individual) {
		q_goal->cleared = FALSE; /* no player? global goal */
		return;
	}

	p_ptr = Players[pInd];
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	if (i == MAX_CONCURRENT_QUESTS) {
		q_goal->cleared = FALSE; /* player isn't on this quest. return global goal. */
		return;
	}

	if (q_ptr->individual) {
		p_ptr->quest_goals[i][goal] = FALSE; /* individual quest */
		return;
	}

	q_goal->cleared = FALSE; /* global quest */
}

/* spawn/hand out special quest items on stage start.
   Note: We don't need pInd/individual checks here, because quest items that
         get handed out will always be given to the player who has the
         questors's 'talk_focus', be it a global or individual quest. */
static void quest_spawn_questitems(int q_idx, int stage) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_stage *q_stage = quest_qi_stage(q_idx, stage);
	qi_questitem *q_qitem;
	object_type forge, *o_ptr = &forge;
	int i, j, py;

	struct worldpos wpos;
	s16b x, y, o_idx;
	cave_type **zcave, *c_ptr;

	for (i = 0; i < q_stage->qitems; i++) {
		q_qitem = &q_stage->qitem[i];

		/* generate the object */
		invcopy(o_ptr, lookup_kind(TV_SPECIAL, SV_QUEST));
		o_ptr->pval = q_qitem->opval;
		o_ptr->xtra1 = q_qitem->ochar;
		o_ptr->xtra2 = q_qitem->oattr;
		o_ptr->xtra3 = i;
		o_ptr->xtra4 = TRUE;
		o_ptr->weight = q_qitem->oweight;
		o_ptr->number = 1;
		o_ptr->level = q_qitem->olev;

		o_ptr->quest = q_idx;
		o_ptr->quest_stage = stage;

		/* just have a questor hand it over? */
		if (q_qitem->questor_gives != 255) {
			/* omit despawned-check, w/e */
			/* check for questor's validly focussed player */
			py = q_ptr->questor[q_qitem->questor_gives].talk_focus;
			if (!py) return; /* oops? */
			msg_format(py, "\374\377GYou received '%s'!", q_qitem->name); //for now. This might need some fine tuning
			/* own it */
			o_ptr->owner = Players[py]->id;
			o_ptr->mode = Players[py]->mode;
			inven_carry(py, o_ptr);
			continue;
		}

		/* spawn it 80 days away~ */
		if (!quest_special_spawn_location(&wpos, &x, &y, &q_qitem->q_loc)) {
			s_printf("QUEST_SPAWN_QUESTITEMS: Couldn't obtain spawning location.\n");
			return;

		}

		/* create a new world object from forge */
		o_idx = o_pop();
		if (!o_idx) {
			s_printf("QUEST_SPAWN_QUESTITEMS: No free object index to pop item.\n");
			return;
		}
		o_list[o_idx] = *o_ptr; /* transfer forged object */
		o_ptr = &o_list[o_idx];
		o_ptr->ix = x;
		o_ptr->iy = y;
		wpcopy(&o_ptr->wpos, &wpos);

		/* 'drop' it */
		if (!(zcave = getcave(&wpos))) {
			//dealloc_dungeon_level(wpos);
			alloc_dungeon_level(&wpos);
			generate_cave(&wpos, NULL);
			zcave = getcave(&wpos);
		}
		c_ptr = &zcave[y][x];
		o_ptr->next_o_idx = c_ptr->o_idx; /* on top of pile, if any */
		o_ptr->stack_pos = 0;
		c_ptr->o_idx = o_idx;

		o_ptr->marked = 0;
		o_ptr->held_m_idx = 0;

		/* Clear visibility flags */
		for (j = 1; j <= NumPlayers; j++) Players[j]->obj_vis[o_idx] = FALSE;
		/* Note the spot */
		note_spot_depth(&wpos, y, x);
		/* Draw the spot */
		everyone_lite_spot(&wpos, y, x);
	}
}
/* perform automatic things (quest spawn/stage change) in a stage */
static bool quest_stage_automatics(int pInd, int q_idx, int stage) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_stage *q_stage = quest_qi_stage(q_idx, stage);

	/* auto-spawn (and acquire) new quest? */
	if (q_stage->activate_quest != -1) {
		if (!q_info[q_stage->activate_quest].disabled &&
		    q_info[q_stage->activate_quest].defined) {
#if QDEBUG > 0
			s_printf("%s QUEST_ACTIVATE_AUTO: '%s'(%d,%s) stage %d\n",
			    showtime(), q_name + q_info[q_stage->activate_quest].name,
			    q_stage->activate_quest, q_info[q_stage->activate_quest].codename, stage);
#endif
			quest_activate(q_stage->activate_quest);
			if (q_stage->auto_accept)
				quest_acquire_confirmed(pInd, q_stage->activate_quest, q_stage->auto_accept_quiet);
		}
#if QDEBUG > 0
		else s_printf("%s QUEST_ACTIVATE_AUTO --failed--: '%s'(%d)\n",
		    showtime(), q_name + q_info[q_stage->activate_quest].name,
		    q_stage->activate_quest);
#endif
	}

	/* auto-change stage (timed)? */
	if (q_stage->change_stage != -1) {
		/* not a timed change? instant then */
		if (//!q_ptr->timed_ingame &&
		    q_stage->timed_ingame_abs != -1 && !q_stage->timed_real) {
#if QDEBUG > 0
			s_printf("%s QUEST_STAGE_AUTO: '%s'(%d,%s) %d->%d\n",
			    showtime(), q_name + q_ptr->name, q_idx, q_ptr->codename, quest_get_stage(pInd, q_idx), q_stage->change_stage);
#endif
			quest_set_stage(pInd, q_idx, q_stage->change_stage, q_stage->quiet_change);
			/* don't imprint/play dialogue of this stage anymore, it's gone~ */
			return TRUE;
		}
		/* start the clock */
		/*cannot do this, cause quest scheduler is checking once per minute atm
		if (q_stage->timed_ingame) {
			q_stage->timed_countdown = q_stage->timed_ingame;//todo: different resolution than real minutes
			q_stage->timed_countdown_stage = q_stage->change_stage;
			q_stage->timed_countdown_quiet = q_stage->quiet_change;
		} else */
		if (q_stage->timed_ingame_abs != -1) {
			q_stage->timed_countdown = -q_stage->timed_ingame_abs;
			q_stage->timed_countdown_stage = q_stage->change_stage;
			q_stage->timed_countdown_quiet = q_stage->quiet_change;
		} else if (q_stage->timed_real) {
			q_stage->timed_countdown = q_stage->timed_real;
			q_stage->timed_countdown_stage = q_stage->change_stage;
			q_stage->timed_countdown_quiet = q_stage->quiet_change;
		}
	}

	return FALSE;
}
/* Imprint the current stage's tracking information of one of our pursued quests
   into our temporary quest data. This means target-flagging and kill/retrieve counting.
   If 'target_flagging_only' is set, the counters aren't touched. */
static void quest_imprint_tracking_information(int Ind, int py_q_idx, bool target_flagging_only) {
	player_type *p_ptr = Players[Ind];
	quest_info *q_ptr;
	int i, stage, q_idx;
	qi_stage *q_stage;
	qi_goal *q_goal;

	/* no quest pursued in this slot? */
	if (p_ptr->quest_idx[py_q_idx] == -1) return;
	q_idx = p_ptr->quest_idx[py_q_idx];
	q_ptr = &q_info[q_idx];

	/* our quest is unavailable for some reason? */
	if (!q_ptr->defined || !q_ptr->active) return;
	stage = quest_get_stage(Ind, q_idx);
	q_stage = quest_qi_stage(q_idx, stage);

	/* now set goal-dependant (temporary) quest info again */
	for (i = 0; i < q_stage->goals; i++) {
		q_goal = &q_stage->goal[i];

		/* set deliver location helper info */
		if (q_goal->deliver) {
			p_ptr->quest_deliver_pos[py_q_idx] = TRUE;
			/* note: deliver has no 'basic wpos check', since the whole essence of
			   "delivering" is to actually move somewhere. */
			/* finer check? */
			if (q_goal->deliver->pos_x != -1) p_ptr->quest_any_deliver_xy = TRUE;
		}

		/* set kill/retrieve tracking counter if we have such goals in this stage */
		if (q_goal->kill) {
			if (!target_flagging_only)
				p_ptr->quest_kill_number[py_q_idx][i] = q_goal->kill->number;

			/* set target location helper info */
			p_ptr->quest_kill[py_q_idx] = TRUE;
			/* assume it's a restricted target "at least" */
			p_ptr->quest_any_k_target = TRUE;
			/* if it's not a restricted target, it's active everywhere */
			if (!q_goal->target_pos) {
				p_ptr->quest_any_k = TRUE;
				p_ptr->quest_any_k_within_target = TRUE;
			}
		}
		if (q_goal->retrieve) {
			if (!target_flagging_only)
				p_ptr->quest_retrieve_number[py_q_idx][i] = q_goal->retrieve->number;

			/* set target location helper info */
			p_ptr->quest_retrieve[py_q_idx] = TRUE;
			/* assume it's a restricted target "at least" */
			p_ptr->quest_any_r_target = TRUE;
			/* if it's not a restricted target, it's active everywhere */
			if (!q_goal->target_pos) {
				p_ptr->quest_any_r = TRUE;
				p_ptr->quest_any_r_within_target = TRUE;
			}
		}
	}
}
/* Actually, for retrieval quests, check right away whether the player
   carries any matching items and credit them!
   (Otherwise either dropping them would reduce his counter, or picking
   them up again would wrongly increase it or w/e depending on what
   other approach is taken and how complicated you want it to be.) */
void quest_precheck_retrieval(int Ind, int q_idx, int py_q_idx) {
	player_type *p_ptr = Players[Ind];
	int i;

#if 0	/* done in qcg_kr() anyway */
	quest_info *q_ptr = &q_info[q_idx];
	int stage = quest_get_stage(Ind, q_idx);
	qi_stage *q_stage = quest_qi_stage(Ind, stage);

	for (i = 0; i < q_stage->goals; i++) {
		if (!q_stage->goal[i].retrieve) continue;
		...
	}*/
#endif
	for (i = 0; i < INVEN_PACK; i++) {
		if (!p_ptr->inventory[i].k_idx) continue;
		quest_check_goal_kr(Ind, q_idx, py_q_idx, NULL, &p_ptr->inventory[i]);
	}
}
/* Update our current target location requirements and check them right away.
   This function is to be called right after a stage change.
   Its purpose is to check if the player already carries all required items
   for a retrieval quest and is even at the correct deliver position already.
   A stage change will occur then, induced from the goal-checks called in the
   functions called in this function eventually.
   Without this function, the player would have to..
   -drop the eligible items he's already carrying and pick them up again and..
   -change wpos once, then go back, to be able to deliver.
   Let this function also be known as...badumtsh.. <INSTAEND HACK>! */
static void quest_instacheck_retrieval(int Ind, int q_idx, int py_q_idx) {
	/* Check if our current wpos/location is already an important target
	   for retrieval quests that we just acquired */
	quest_check_player_location(Ind);
	/* If it isn't (for retrieval quests), we're done. */
	if (!Players[Ind]->quest_any_r_target) return;

	/* check for items already in our inventory that fulfil any
	   retrieval goals we just acquired right away */
	quest_precheck_retrieval(Ind, q_idx, py_q_idx); /* sets goals as 'nisi'.. */
	quest_check_goal_deliver(Ind); /* ..turns them in! */
}
/* Store some information of the current stage in the p_ptr array to track his
   progress more easily in the game code:
   -How many kills/items he has to retrieve and
   -whether he is currently within a target location to do so.
   Additionally it checks right away if the player already carries any requested
   items and credits him for them right away. */
static void quest_imprint_stage(int Ind, int q_idx, int py_q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr = Players[Ind];
	int stage;

	/* for 'individual' quests: imprint the individual stage for a player.
	   the globally set q_ptr->cur_stage is in this case just a temporary value,
	   set by quest_set_stage() for us, that won't be of any further consequence. */
	stage = q_ptr->cur_stage;
	if (q_ptr->individual) p_ptr->quest_stage[py_q_idx] = stage;


	/* find out if we are pursuing any sort of target locations */

	/* first, initialise all temporary info */
	quest_initialise_player_tracking(Ind, py_q_idx);

	/* now set/add our new stage's info */
	quest_imprint_tracking_information(Ind, py_q_idx, FALSE);
}
/* Advance quest to a different stage (or start it out if stage is 0) */
void quest_set_stage(int pInd, int q_idx, int stage, bool quiet) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_stage *q_stage;
	int i, j, k, py_q_idx = -1;
	bool anything;

#if QDEBUG > 0
	s_printf("%s QUEST_STAGE: '%s'(%d,%s) %d->%d\n", showtime(), q_name + q_ptr->name, q_idx, q_ptr->codename, quest_get_stage(pInd, q_idx), stage);
	s_printf(" actq %d, autoac %d, cstage %d\n", quest_qi_stage(q_idx, stage)->activate_quest, quest_qi_stage(q_idx, stage)->auto_accept, quest_qi_stage(q_idx, stage)->change_stage);
#endif

	/* dynamic info */
	//int stage_prev = quest_get_stage(pInd, q_idx);

	/* new stage is active.
	   for 'individual' quests this is just a temporary value used by quest_imprint_stage().
	   It is still used for the other stage-checking routines in this function too though:
	    quest_goal_check_reward(), quest_terminate() and the 'anything' check. */
	q_ptr->cur_stage = stage;
	q_ptr->dirty = TRUE;
	q_stage = quest_qi_stage(q_idx, stage);

	/* For non-'individual' quests,
	   if a participating player is around the questor, entering a new stage..
	   - qutomatically invokes the quest dialogue with him again (if any)
	   - store information of the current stage in the p_ptr array,
	     eg the target location for easier lookup */
	if (!q_ptr->individual || !pInd) { //the !pInd part is paranoia
		for (i = 1; i <= NumPlayers; i++) {
			Players[i]->quest_eligible = 0;
			for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
				if (Players[i]->quest_idx[j] == q_idx) break;
			if (j == MAX_CONCURRENT_QUESTS) continue;
			Players[i]->quest_eligible = j + 1;

			/* play automatic narration if any */
			if (!quiet) {
				/* pre-scan narration if any line at all exists and passes the flag check */
				anything = FALSE;
				for (k = 0; k < QI_TALK_LINES; k++) {
					if (q_stage->narration[k] &&
					    ((q_stage->narration_flags[k] & quest_get_flags(pInd, q_idx)) == q_stage->narration_flags[k])) {
						anything = TRUE;
						break;
					}
				}
				/* there is narration to display? */
				if (anything) {
					msg_print(i, "\374 ");
					msg_format(i, "\374\377u<\377U%s\377u>", q_name + q_ptr->name);
					for (k = 0; k < QI_TALK_LINES; k++) {
						if (!q_stage->narration[k]) break;
						if ((q_stage->narration_flags[k] & quest_get_flags(pInd, q_idx)) != q_stage->narration_flags[k]) continue;
						msg_format(i, "\374\377U%s", q_stage->narration[k]);
					}
					//msg_print(i, "\374 ");
				}
			}
		}

		/* hand out/spawn any special quest items */
		quest_spawn_questitems(q_idx, stage);

		for (i = 1; i <= NumPlayers; i++) {
			if (!Players[i]->quest_eligible) continue;

			/* update player's quest tracking data */
			quest_imprint_stage(i, q_idx, Players[i]->quest_eligible - 1);
		}

		/* perform automatic actions (spawn new quest, (timed) further stage change)
		   Note: Should imprint correct stage on player before calling this,
		         otherwise automatic stage change will "skip" a stage in between ^^.
		         This should be rather cosmetic though. */
		if (quest_stage_automatics(pInd, q_idx, stage)) return;

		for (i = 1; i <= NumPlayers; i++) {
			if (!Players[i]->quest_eligible) continue;

			/* play questors' stage dialogue */
			if (!quiet)
				for (k = 0; k < q_ptr->questors; k++) {
					if (!inarea(&Players[i]->wpos, &q_ptr->questor[k].current_wpos)) continue;
					quest_dialogue(i, q_idx, k, FALSE, FALSE, FALSE);
				}
		}
	} else { /* individual quest */
		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (Players[pInd]->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) return; //paranoia, shouldn't happen

		/* play automatic narration if any */
		if (!quiet) {
			/* pre-scan narration if any line at all exists and passes the flag check */
			anything = FALSE;
			for (k = 0; k < QI_TALK_LINES; k++) {
				if (q_stage->narration[k] &&
				    ((q_stage->narration_flags[k] & quest_get_flags(pInd, q_idx)) == q_stage->narration_flags[k])) {
					anything = TRUE;
					break;
				}
			}
			/* there is narration to display? */
			if (anything) {
				msg_print(pInd, "\374 ");
				msg_format(pInd, "\374\377u<\377U%s\377u>", q_name + q_ptr->name);
				for (k = 0; k < QI_TALK_LINES; k++) {
					if (!q_stage->narration[k]) break;
					msg_format(pInd, "\374\377U%s", q_stage->narration[k]);
				}
				//msg_print(pInd, "\374 ");
			}
		}

		/* hand out/spawn any special quest items */
		quest_spawn_questitems(q_idx, stage);

		/* perform automatic actions (spawn new quest, (timed) further stage change) */
		if (quest_stage_automatics(pInd, q_idx, stage)) return;

		/* update players' quest tracking data */
		quest_imprint_stage(pInd, q_idx, j);
		/* play questors' stage dialogue */
		if (!quiet)
			for (k = 0; k < q_ptr->questors; k++) {
				if (!inarea(&Players[pInd]->wpos, &q_ptr->questor[k].current_wpos)) continue;
				quest_dialogue(pInd, q_idx, k, FALSE, FALSE, FALSE);
			}
		py_q_idx = j;
	}


	/* check for handing out rewards! (in case they're free) */
	quest_goal_check_reward(pInd, q_idx);


	/* quest termination? */
	if (q_ptr->ending_stage && q_ptr->ending_stage == stage) {
#if QDEBUG > 0
		s_printf("%s QUEST_STAGE_ENDING: '%s'(%d,%s) %d\n", showtime(), q_name + q_ptr->name, q_idx, q_ptr->codename, stage);
#endif
		quest_terminate(pInd, q_idx);
	}


	/* auto-quest-termination? (actually redundant with ending_stage, just for convenience:)
	   If a stage has no dialogue keywords, or stage goals, or timed/auto stage change
	   effects or questor-movement/tele/revert-from-hostile effects, THEN the quest will end. */
	   //TODO: implement that questor actions stuff :-p
	anything = FALSE;

	/* optional goals play no role, obviously */
	for (i = 0; i < q_stage->goals; i++)
		if (q_stage->goal[i].kill || q_stage->goal[i].retrieve || q_stage->goal[i].deliver) {
			anything = TRUE;
			break;
		}
	/* now check remaining dialogue options (keywords) */
	for (i = 0; i < q_ptr->keywords; i++)
		if (q_ptr->keyword[i].stage_ok[stage] && /* keyword is available in this stage */
		    q_ptr->keyword[i].stage != -1 && /* and it's not just a keyword-reply without a stage change? */
		    !q_ptr->keyword[i].any_stage) { /* and it's not valid in ANY stage? that's not sufficient */
			anything = TRUE;
			break;
		}
	/* check auto/timed stage changes */
	if (q_stage->change_stage != -1) anything = TRUE;
	//if (q_ptr->timed_ingame) anything = TRUE;
	if (q_stage->timed_ingame_abs != -1) anything = TRUE;
	if (q_stage->timed_real) anything = TRUE;

	/* really nothing left to do? */
	if (!anything) {
#if QDEBUG > 0
		s_printf("%s QUEST_STAGE_EMPTY: '%s'(%d,%s) %d\n", showtime(), q_name + q_ptr->name, q_idx, q_ptr->codename, stage);
#endif
		quest_terminate(pInd, q_idx);
	}

#if 1	/* INSTAEND HACK */
	/* hack - we couldn't do this in quest_imprint_stage(), see the note there:
	   check if we already auto-completed the quest stage this quickly just by standing there! */
	if (py_q_idx == -1) { //global quest
		for (i = 1; i <= NumPlayers; i++) {
			for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
				if (Players[i]->quest_idx[j] == q_idx) break;
			if (j == MAX_CONCURRENT_QUESTS) continue;

			quest_instacheck_retrieval(i, q_idx, j);
		}
	} else { /* individual quest */
		quest_instacheck_retrieval(pInd, q_idx, py_q_idx);
	}
#endif
}

void quest_acquire_confirmed(int Ind, int q_idx, bool quiet) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr = Players[Ind];
	int i, j;

	/* re-check (this was done in quest_acquire() before,
	   but we cannot carry over this information. :/
	   Also, it might have changed meanwhile by some very odd means that we don't want to think about --
	   does the player have capacity to pick up one more quest? */
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == -1) break;
	if (i == MAX_CONCURRENT_QUESTS) { /* paranoia - this should not be possible to happen */
		//if (!quiet)
			msg_print(Ind, "\377RYou cannot pick up this quest - please tell an admin about this!");
		return;
	}

	p_ptr->quest_idx[i] = q_idx;
	strcpy(p_ptr->quest_codename[i], q_ptr->codename);
#if QDEBUG > 1
	s_printf("%s QUEST_ACQUIRED: (%d,%d,%d;%d,%d) %s (%d) has quest '%s'(%d,%s).\n",
	    showtime(), p_ptr->wpos.wx, p_ptr->wpos.wy, p_ptr->wpos.wz, p_ptr->px, p_ptr->py, p_ptr->name, Ind, q_name + q_ptr->name, q_idx, q_ptr->codename);
#endif

	/* for 'individual' quests, reset temporary quest data or it might get carried over from previous quest */
	p_ptr->quest_stage[i] = 0; /* note that individual quests can ONLY start in stage 0, obviously */
	p_ptr->quest_flags[i] = 0x0000;
	for (j = 0; j < QI_GOALS; j++) p_ptr->quest_goals[i][j] = FALSE;

	/* reset temporary quest helper info */
	quest_initialise_player_tracking(Ind, i);

	/* let him know about just acquiring the quest? */
	if (!quiet) {
		//msg_print(Ind, "\374 "); /* results in double empty line, looking bad */
		//msg_format(Ind, "\374\377U**\377u You have acquired the quest \"\377U%s\377u\". \377U**", q_name + q_ptr->name);
		switch (q_ptr->privilege) {
		case 0: msg_format(Ind, "\374\377uYou have acquired the quest \"\377U%s\377u\".", q_name + q_ptr->name);
			break;
		case 1: msg_format(Ind, "\374\377uYou have acquired the quest \"\377U%s\377u\". (\377yprivileged\377u)", q_name + q_ptr->name);
			break;
		case 2: msg_format(Ind, "\374\377uYou have acquired the quest \"\377U%s\377u\". (\377ohighly privileged\377u)", q_name + q_ptr->name);
			break;
		case 3: msg_format(Ind, "\374\377uYou have acquired the quest \"\377U%s\377u\". (\377radmins only\377u)", q_name + q_ptr->name);
		}
		//msg_print(Ind, "\374 "); /* keep one line spacer to echoing our entered keyword */
	}

	/* hack: for 'individual' quests we use q_ptr->cur_stage as temporary var to store the player's personal stage,
	   which is then transferred to the player again in quest_imprint_stage(). Yeah.. */
	if (q_ptr->individual) q_ptr->cur_stage = 0; /* 'individual' quest entry stage is always 0 */
	/* store information of the current stage in the p_ptr array,
	   eg the target location for easier lookup */
	quest_imprint_stage(Ind, q_idx, i);

#if 1 /* INSTAEND HACK */
	/* hack - we couldn't do this in quest_imprint_stage(), see the note there:
	   check if we already auto-completed the quest stage this quickly just by standing there! */
	q_ptr->dirty = FALSE;
	quest_instacheck_retrieval(Ind, q_idx, i);
	/* stage advanced or quest even terminated? */
	//if (q_ptr->cur_stage != 0 || p_ptr->quest_idx[i] == -1) return;
	if (q_ptr->dirty) return;
#endif

	/* re-prompt for keyword input, if any */
	quest_dialogue(Ind, q_idx, p_ptr->interact_questor_idx, TRUE, FALSE, FALSE);
}

/* Acquire a quest, WITHOUT CHECKING whether the quest actually allows this at this stage!
   Returns TRUE if we are eligible for acquiring it (not just if we actually did acquire it). */
static bool quest_acquire(int Ind, int q_idx, bool quiet) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr = Players[Ind];
	int i, j;
	bool ok;

	/* quest is deactivated? -- paranoia here */
	if (!q_ptr->active) {
		msg_print(Ind, qi_msg_deactivated);
		return FALSE;
	}
	/* quest is on [individual] cooldown? */
	if (quest_get_cooldown(Ind, q_idx)) {
		msg_print(Ind, qi_msg_cooldown);
		return FALSE;
	}

	/* quests is only for admins or privileged accounts at the moment? (for testing purpose) */
	switch (q_ptr->privilege) {
	case 1: if (!p_ptr->privileged && !is_admin(p_ptr)) return FALSE;
		break;
	case 2: if (p_ptr->privileged != 2 && !is_admin(p_ptr)) return FALSE;
		break;
	case 3: if (!is_admin(p_ptr)) return FALSE;
	}

	/* matches prerequisite quests? (ie this is a 'follow-up' quest) */
	for (i = 0; i < QI_PREREQUISITES; i++) {
		if (!q_ptr->prerequisites[i][0]) continue;
		for (j = 0; j < MAX_Q_IDX; j++) {
			if (strcmp(q_info[j].codename, q_ptr->prerequisites[i])) continue;
			if (!p_ptr->quest_done[j]) {
				msg_print(Ind, qi_msg_prereq);
				return FALSE;
			}
		}
	}
	/* level / race / class / mode / body restrictions */
	if (q_ptr->minlev && q_ptr->minlev > p_ptr->lev) {
		msg_print(Ind, qi_msg_minlev);
		return FALSE;
	}
	if (q_ptr->maxlev && q_ptr->maxlev < p_ptr->max_plv) {
		msg_print(Ind, qi_msg_maxlev);
		return FALSE;
	}
	if (!(q_ptr->races & (0x1 << p_ptr->prace))) {
		msg_print(Ind, qi_msg_race);
		return FALSE;
	}
	if (!(q_ptr->classes & (0x1 << p_ptr->pclass))) {
		msg_print(Ind, qi_msg_class);
		return FALSE;
	}
	ok = FALSE;
	if (!q_ptr->must_be_fruitbat && !q_ptr->must_be_monster) ok = TRUE;
	if (q_ptr->must_be_fruitbat && p_ptr->fruit_bat <= q_ptr->must_be_fruitbat) ok = TRUE;
	if (q_ptr->must_be_monster && p_ptr->body_monster == q_ptr->must_be_monster
	    && q_ptr->must_be_fruitbat != 1) /* hack: disable must_be_monster if must be a TRUE fruit bat.. */
		ok = TRUE;
	if (!ok) {
		if (q_ptr->must_be_fruitbat == 1) msg_print(Ind, qi_msg_truebat);
		else if (q_ptr->must_be_fruitbat || q_ptr->must_be_monster == 37) msg_print(Ind, qi_msg_bat);
		else msg_print(Ind, qi_msg_form);
		return FALSE;
	}
	ok = FALSE;
	if (q_ptr->mode_norm && !(p_ptr->mode & (MODE_EVERLASTING | MODE_PVP))) ok = TRUE;
	if (q_ptr->mode_el && (p_ptr->mode & MODE_EVERLASTING)) ok = TRUE;
	if (q_ptr->mode_pvp && (p_ptr->mode & MODE_PVP)) ok = TRUE;
	if (!ok) {
		msg_print(Ind, qi_msg_mode);
		return FALSE;
	}

	/* has the player completed this quest already/too often? */
	if (p_ptr->quest_done[q_idx] > q_ptr->repeatable && q_ptr->repeatable != -1) {
		if (!quiet) msg_print(Ind, qi_msg_done);
		return FALSE;
	}

	/* does the player have capacity to pick up one more quest? */
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == -1) break;
	if (i == MAX_CONCURRENT_QUESTS) {
		if (!quiet) msg_print(Ind, qi_msg_max);
		return FALSE;
	}

	/* voila, player may acquire this quest! */
#if QDEBUG > 1
	s_printf("%s QUEST_MAY_ACQUIRE: (%d,%d,%d;%d,%d) %s (%d) may acquire quest '%s'(%d,%s).\n",
	    showtime(), p_ptr->wpos.wx, p_ptr->wpos.wy, p_ptr->wpos.wz, p_ptr->px, p_ptr->py, p_ptr->name, Ind, q_name + q_ptr->name, q_idx, q_ptr->codename);
#endif
	return TRUE;
}

/* Check if player completed a deliver goal to a questor. */
static void quest_check_goal_deliver_questor(int Ind, int q_idx, int py_q_idx) {
	player_type *p_ptr = Players[Ind];
	int j, k, stage;
	quest_info *q_ptr = &q_info[q_idx];;
	qi_stage *q_stage;
	qi_goal *q_goal;
	qi_deliver *q_del;

#if QDEBUG > 3
	s_printf("QUEST_CHECK_GOAL_DELIVER_QUESTOR: by %d,%s - quest (%s,%d) stage %d\n",
	    Ind, p_ptr->name, q_ptr->codename, q_idx, quest_get_stage(Ind, q_idx));
#endif

	/* quest is deactivated? */
	if (!q_ptr->active) return;

	stage = quest_get_stage(Ind, q_idx);
	q_stage = quest_qi_stage(q_idx, stage);

	/* pre-check if we have completed all kill/retrieve goals of this stage,
	   because we can only complete any deliver goal if we have fetched all
	   the stuff (bodies, or kill count rather, and objects) that we ought to
	   'deliver', duh */
	for (j = 0; j < q_stage->goals; j++) {
		q_goal = &q_stage->goal[j];
#if QDEBUG > 3
		if (q_goal->kill || q_goal->retrieve) {
			s_printf(" --FOUND GOAL %d (k%d,m%d)..", j, q_ptr->kill[j], q_ptr->retrieve[j]);
			if (!quest_get_goal(Ind, q_idx, j, FALSE)) {
				s_printf("MISSING.\n");
				break;
			} else s_printf("DONE.\n");

		}
#else
		if ((q_goal->kill || q_goal->retrieve)
		    && !quest_get_goal(Ind, q_idx, j, FALSE))
			break;
#endif
	}
	if (j != q_stage->goals) {
#if QDEBUG > 3
		s_printf(" MISSING kr GOAL\n");
#endif
		return;
	}

	/* check the quest goals, whether any of them wants a delivery to this location */
	for (j = 0; j < q_stage->goals; j++) {
		q_goal = &q_stage->goal[j];
		if (!q_goal->deliver) continue;
		q_del = q_goal->deliver;

		/* handle only specific to-questor goals here */
		if (q_goal->deliver->return_to_questor == 255) continue;
#if QDEBUG > 3
		s_printf(" FOUND return-to-questor GOAL %d.\n", j);
#endif

		/* First mark all 'nisi' quest goals as finally resolved,
		   to change flags accordingly if defined by a Z-line.
		   It's important to do this before removing retieved items,
		   just in case UNGOAL will kick in, in inven_item_increase()
		   and unset our quest goal :-p. */
		for (k = 0; k < q_stage->goals; k++)
			if (quest_get_goal(Ind, q_idx, k, TRUE)) {
				quest_set_goal(Ind, q_idx, k, FALSE);
				quest_goal_changes_flags(Ind, q_idx, stage, k);
			}

		/* we have completed a delivery-return goal!
		   We need to un-nisi it here before UNGOAL is called in inven_item_increase(). */
		quest_set_goal(Ind, q_idx, j, FALSE);

		/* for item retrieval goals therefore linked to this deliver goal,
		   remove all quest items now finally that we 'delivered' them.. */
		if (q_ptr->individual) {
			for (k = INVEN_PACK - 1; k >= 0; k--) {
				if (p_ptr->inventory[k].quest == q_idx + 1 &&
				    p_ptr->inventory[k].quest_stage == stage) {
					inven_item_increase(Ind, k, -99);
					//inven_item_describe(Ind, k);
					inven_item_optimize(Ind, k);
				}
			}
		} else {
			//TODO (not just here): implement global retrieval quests..
		}
	}
}

/* A player interacts with the questor (bumps him if a creature :-p).
   This displays quest talk/narration, turns in goals, or may acquire it.
   The file pointer is for object questors who get examined by the player. */
#define ALWAYS_DISPLAY_QUEST_TEXT
void quest_interact(int Ind, int q_idx, int questor_idx, FILE *fff) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr = Players[Ind];
	int i, j, stage = quest_get_stage(Ind, q_idx);
	bool not_acquired_yet = FALSE, may_acquire = FALSE;
	qi_stage *q_stage = quest_qi_stage(q_idx, stage);
	qi_questor *q_questor = &q_ptr->questor[questor_idx];
	qi_goal *q_goal; //for return_to_questor check

	/* quest is deactivated? */
	if (!q_ptr->active) return;

	/* quests is only for admins or privileged accounts at the moment? (for testing purpose) */
	switch (q_ptr->privilege) {
	case 1: if (!p_ptr->privileged && !is_admin(p_ptr)) return;
		break;
	case 2: if (p_ptr->privileged != 2 && !is_admin(p_ptr)) return;
		break;
	case 3: if (!is_admin(p_ptr)) return;
	}

	/* questor interaction may (automatically) acquire the quest */
	/* has the player not yet acquired this quest? */
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++)
		if (p_ptr->quest_idx[i] == q_idx) break;
	/* nope - test if he's eligible for picking it up!
	   Otherwise, the questor will remain silent for him. */
	if (i == MAX_CONCURRENT_QUESTS) not_acquired_yet = TRUE;

	/* check for deliver quest goals that require to return to a questor */
	if (!not_acquired_yet)
		for (j = 0; j < q_stage->goals; j++) {
			q_goal = &q_stage->goal[j];
			if (!q_goal->deliver || q_goal->deliver->return_to_questor == 255) continue;

			q_ptr->dirty = FALSE;
			quest_check_goal_deliver_questor(Ind, q_idx, j);
			/* hack: check for stage change/termination */
			//TODO add 'dirty' flag to quest on stage change, so we can check it here
			//if (stage != p_ptr->quest_stage[i]) return;
			if (q_ptr->dirty) return;
		}


	/* cannot interact with the questor during this stage? */
	if (!q_questor->talkable) return;

	if (not_acquired_yet) {
		/* do we accept players by questor interaction at all? */
		if (!q_questor->accept_interact) return;
		/* do we accept players to acquire this quest in the current quest stage? */
		if (!q_stage->accepts) return;

		/* if player cannot pick up this quest, questor remains silent */
		if (!(may_acquire = quest_acquire(Ind, q_idx, FALSE))) return;
	}


	/* if we're not here for quest acquirement, just check for quest goals
	   that have been completed and just required delivery back here */
	if (!may_acquire && quest_goal_check(Ind, q_idx, TRUE)) return;

	/* Special hack - object questor.
	   That means we're in the middle of the item examination screen. Add to it! */
	if (q_questor->type == QI_QUESTOR_ITEM_PICKUP) {
		if (q_stage->talk_examine[questor_idx])
			fprintf(fff, " \377GYou notice something about this item! (See your chat window.)\n");
		else /* '>_> */
			fprintf(fff, " \377GIt seems the item itself is speaking to you! (See your chat window.)\n");
	}

	/* questor interaction qutomatically invokes the quest dialogue, if any */
	q_questor->talk_focus = Ind; /* only this player can actually respond with keywords -- TODO: ensure this happens for non 'individual' quests only */
	quest_dialogue(Ind, q_idx, questor_idx, FALSE, may_acquire, TRUE);

	/* prompt him to acquire this quest if he hasn't yet */
	if (may_acquire) Send_request_cfr(Ind, RID_QUEST_ACQUIRE + q_idx, format("Accept the quest \"%s\"?", q_name + q_ptr->name), TRUE);
}

/* Talk vs keyword dialogue between questor and player.
   This is initiated either by bumping into the questor or by entering a new
   stage while in the area of the questor.
   Note that if the questor is focussed, only that player may respond with keywords.

   'repeat' will repeat requesting an input and skip the usual dialogue. Used for
   keyword input when a keyword wasn't recognized.

   'interact_acquire' must be set if this dialogue spawns from someone interacting
   initially with the questor who is eligible to acquire the quest.
   In that case, the player won't get the enter-a-keyword-prompt. Instead, our
   caller function quest_interact() will prompt him to acquire the quest first.

   'force_prompt' is set if we're called from quest_interact(), and will always
   give us a keyword-prompt even though there are no valid ones.
   Assumption is that if we're called from quest_interact() we bumped on purpose
   into the questor because we wanted to try out a keyword (as a player, we don't
   know that there maybe are none.. >:D). */
static void quest_dialogue(int Ind, int q_idx, int questor_idx, bool repeat, bool interact_acquire, bool force_prompt) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr = Players[Ind];
	int i, k, stage = quest_get_stage(Ind, q_idx);
	qi_stage *q_stage = quest_qi_stage(q_idx, stage);
	bool anything, obvious_keyword = FALSE, more_hack = FALSE, yn_hack = FALSE;

	if (!repeat) {
		/* pre-scan talk if any line at all passes the flag check */
		anything = FALSE;
		for (k = 0; k < QI_TALK_LINES; k++) {
			if (q_stage->talk[questor_idx][k] &&
			    ((q_stage->talk_flags[questor_idx][k] & quest_get_flags(Ind, q_idx)) == q_stage->talk_flags[questor_idx][k])) {
				anything = TRUE;
				break;
			}
		}
		/* there is NPC talk to display? */
		if (anything) {
			p_ptr->interact_questor_idx = questor_idx;
			msg_print(Ind, "\374 ");
			if (q_stage->talk_examine[questor_idx])
				msg_format(Ind, "\374\377uYou examine <\377B%s\377u>:", q_ptr->questor[questor_idx].name);
			else
				msg_format(Ind, "\374\377u<\377B%s\377u> speaks to you:", q_ptr->questor[questor_idx].name);
			for (i = 0; i < QI_TALK_LINES; i++) {
				if (!q_stage->talk[questor_idx][i]) break;
				if ((q_stage->talk_flags[questor_idx][k] & quest_get_flags(Ind, q_idx)) != q_stage->talk_flags[questor_idx][k]) continue;
				msg_format(Ind, "\374\377U%s", q_stage->talk[questor_idx][i]);

				/* lines that contain obvious keywords, ie those marked by '[[..]]',
				   will always force a reply-prompt for convenience! */
				if (strchr(q_stage->talk[questor_idx][i], '\377')) obvious_keyword = TRUE;
			}
			//msg_print(Ind, "\374 ");
		}
		if (obvious_keyword) force_prompt = TRUE;
	} else force_prompt = TRUE; /* repeating what? to talk of course, what else oO -> force_prompt! */

	/* No keyword-interaction possible if we haven't acquired the quest yet. */
	if (interact_acquire) return;

	/* If there are any keywords in this stage, prompt the player for a reply.
	   If the questor is focussed on one player, only he can give a reply,
	   otherwise the first player to reply can advance the stage. */
	if (!q_ptr->individual && q_ptr->questor[questor_idx].talk_focus && q_ptr->questor[questor_idx].talk_focus != Ind) return;

	p_ptr->interact_questor_idx = questor_idx;
	/* check if there's at least 1 keyword available in our stage/from our questor */
	/* NEW: however.. >:) that keyword must be 'obvious'! Or prompt must be forced.
	        We won't disclose 'secret' keywords easily by prompting without any 'apparent'
	        reason for the prompt, and we don't really want to go the route of prompting
	        _everytime_ because it's a bit annoying to have to close the prompt everytime.
	        So we leave it to the player to bump into the questor (->force_prompt) if he
	        wants to try and find out secret keywords.. */

	/* Also we scan here for "~" hack keywords. */
	for (i = 0; i < q_ptr->keywords; i++)
		if (q_ptr->keyword[i].stage_ok[stage] &&
		    q_ptr->keyword[i].questor_ok[questor_idx]
		    && !q_ptr->keyword[i].any_stage/* and it mustn't use a wildcard stage. that's not sufficient. */
		    )
			break;
	if (q_ptr->keyword[i].keyword[0] == 'y' &&
	    q_ptr->keyword[i].keyword[1] == 0)
		yn_hack = TRUE;

	/* ..continue scanning, now for press-SPACEBAR-for-more hack */
	for (; i < q_ptr->keywords; i++)
		if (q_ptr->keyword[i].stage_ok[stage] &&
		    q_ptr->keyword[i].questor_ok[questor_idx]
		    && !q_ptr->keyword[i].any_stage/* and it mustn't use a wildcard stage. that's not sufficient. */
		    && q_ptr->keyword[i].keyword[0] == 0) {
			more_hack = TRUE;
			break;
		}

	if (force_prompt) { /* at least 1 keyword available? */
		/* hack: if 1st keyword is "~", display a "more" prompt */
		if (more_hack)
			Send_request_str(Ind, RID_QUEST + q_idx, "? (blank for more)> ", "");
		else {
			/* hack: if 1st keyword is "y" just give a yes/no choice instead of an input prompt?
			   we assume that 2nd keyword is a "n" then. */
			if (yn_hack)
				Send_request_cfr(Ind, RID_QUEST + q_idx, "? (choose yes or no)>", FALSE);
			else /* normal prompt for keyword input */
				Send_request_str(Ind, RID_QUEST + q_idx, "?> ", "");
		}
	}
}

/* Player replied in a questor dialogue by entering a keyword. */
void quest_reply(int Ind, int q_idx, char *str) {
	quest_info *q_ptr = &q_info[q_idx];
	player_type *p_ptr = Players[Ind];
	int i, j, k, stage = quest_get_stage(Ind, q_idx), questor_idx = p_ptr->interact_questor_idx;
	char *c;
	qi_keyword *q_key;
	qi_kwreply *q_kwr;

	/* trim leading/trailing spaces */
	while (*str == ' ') str++;
	while (str[strlen(str) - 1] == ' ') str[strlen(str) - 1] = 0;

#if 0
	if (!str[0] || str[0] == '\e') return; /* player hit the ESC key.. */
#else /* distinguish RETURN key, to allow empty "" keyword (to 'continue' a really long talk for example) */
	if (str[0] == '\e') return; /* player hit the ESC key.. */
#endif

	/* convert input to all lower-case */
	c = str;
	while (*c) {
		*c = tolower(*c);
		c++;
	}

	/* echo own reply for convenience */
	msg_print(Ind, "\374 ");
	if (!strcmp(str, "y")) msg_print(Ind, "\374\377u>\377UYes");
	else if (!strcmp(str, "n")) msg_print(Ind, "\374\377u>\377UNo");
	else msg_format(Ind, "\374\377u>\377U%s", str);

	/* scan keywords for match */
	for (i = 0; i < q_ptr->keywords; i++) {
		q_key = &q_ptr->keyword[i];

		if (!q_key->stage_ok[stage] || /* no more keywords? */
		    !q_key->questor_ok[questor_idx]) continue;

		/* check if required flags match to enable this keyword */
		if ((q_key->flags & quest_get_flags(Ind, q_idx)) != q_key->flags) continue;

		if (strcmp(q_key->keyword, str)) continue; /* not matching? */

		/* reply? */
		for (j = 0; j < q_ptr->kwreplies; j++) {
			q_kwr = &q_ptr->kwreply[j];

			if (!q_kwr->stage_ok[stage] || /* no more keywords? */
			    !q_kwr->questor_ok[questor_idx]) continue;

			/* check if required flags match to enable this keyword */
			if ((q_kwr->flags & quest_get_flags(Ind, q_idx)) != q_kwr->flags) continue;

			for (k = 0; k < QI_KEYWORDS_PER_REPLY; k++) {
				if (q_kwr->keyword_idx[k] == i) {
					msg_print(Ind, "\374 ");
					if (q_ptr->stage[stage].talk_examine[questor_idx])
						msg_format(Ind, "\374\377uYou examine <\377B%s\377u>:", q_ptr->questor[questor_idx].name);
					else
						msg_format(Ind, "\374\377u<\377B%s\377u> speaks to you:", q_ptr->questor[questor_idx].name);
					/* we can re-use j here ;) */
					for (j = 0; j < q_kwr->lines; j++) {
						if ((q_kwr->replyflags[j] & quest_get_flags(Ind, q_idx)) != q_kwr->replyflags[j]) continue;
						msg_format(Ind, "\374\377U%s", q_kwr->reply[j]);
					}
					//msg_print(Ind, "\374 ");

					j = q_ptr->kwreplies; //hack->break! -- 1 reply found is enough
					break;
				}
			}
		}

		/* flags change? */
		quest_set_flags(Ind, q_idx, q_key->setflags, q_key->clearflags);

		/* stage change? */
		if (q_key->stage != -1) {
			quest_set_stage(Ind, q_idx, q_key->stage, FALSE);
			return;
		}
		/* Instead of return'ing, re-prompt for dialogue
		   if we only changed flags or simply got a keyword-reply. */
		str[0] = 0; /* don't give the 'nothing to say' line, our keyword already matched */
		break;
	}
	/* it was nice small-talking to you, dude */
#if 1
	/* if keyword wasn't recognised, repeat input prompt instead of just 'dropping' the convo */
	quest_dialogue(Ind, q_idx, questor_idx, TRUE, FALSE, FALSE);
	/* don't give 'wassup?' style msg if we just hit RETURN.. silyl */
	if (str[0]) {
		msg_print(Ind, "\374 ");
		msg_format(Ind, "\374\377u<\377B%s\377u> has nothing to say about that.", q_ptr->questor[questor_idx].name);
		//msg_print(Ind, "\374 ");
	}
#endif
	return;
}

/* Test kill quest goal criteria vs an actually killed monster, for a match.
   Main criteria (r_idx vs char+attr+level) are OR'ed.
   (Unlike for retrieve-object matches where they are actually AND'ed.) */
static bool quest_goal_matches_kill(int q_idx, int stage, int goal, monster_type *m_ptr) {
	int i;
	monster_race *r_ptr = race_inf(m_ptr);
	qi_kill *q_kill = quest_qi_stage(q_idx, stage)->goal[goal].kill;

	/* check for race index */
	for (i = 0; i < 10; i++) {
		/* no monster specified? */
		if (q_kill->ridx[i] == 0) continue;
		/* accept any monster? */
		if (q_kill->ridx[i] == -1) return TRUE;
		/* specified an r_idx */
		if (q_kill->ridx[i] == m_ptr->r_idx) return TRUE;
	}

	/* check for char/attr/level combination - treated in pairs (AND'ed) over the index */
	for (i = 0; i < 5; i++) {
		/* no char specified? */
		if (q_kill->rchar[i] == 255) continue;
		 /* accept any char? */
		if (q_kill->rchar[i] != 254 &&
		    /* specified a char? */
		    q_kill->rchar[i] != r_ptr->d_char) continue;

		/* no attr specified? */
		if (q_kill->rattr[i] == 255) continue;
		 /* accept any attr? */
		if (q_kill->rattr[i] != 254 &&
		    /* specified an attr? */
		    q_kill->rattr[i] != r_ptr->d_attr) continue;

		/* min/max level check -- note that we use m-level, not r-level :-o */
		if ((!q_kill->rlevmin || q_kill->rlevmin <= m_ptr->level) &&
		    (!q_kill->rlevmax || q_kill->rlevmax >= m_ptr->level))
			return TRUE;
	}

	/* pft */
	return FALSE;
}

/* Test retrieve-item quest goal criteria vs an actually retrieved object, for a match.
   (Note that the for-blocks are AND'ed unlike for kill-matches where they are OR'ed.) */
static bool quest_goal_matches_object(int q_idx, int stage, int goal, object_type *o_ptr) {
	int i;
	byte attr;
	qi_retrieve *q_ret = quest_qi_stage(q_idx, stage)->goal[goal].retrieve;
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* First let's find out the object's attr..which is uggh not so cool (from cave.c).
	   Note that d_attr has the correct get base colour, especially for flavoured items! */
#if 0
	attr = k_ptr->k_attr;
#else /* this is correct */
	attr = k_ptr->d_attr;
#endif
	if (o_ptr->tval == TV_BOOK && is_custom_tome(o_ptr->sval))
		attr = get_book_name_color(0, o_ptr);
	/* hack: colour of fancy shirts or custom objects can vary  */
	if ((o_ptr->tval == TV_SOFT_ARMOR && o_ptr->sval == SV_SHIRT) ||
	    (o_ptr->tval == TV_SPECIAL && o_ptr->sval == SV_CUSTOM_OBJECT)) {
		if (!o_ptr->xtra1) o_ptr->xtra1 = attr; //wut.. remove this hack? should be superfluous anyway
			attr = o_ptr->xtra1;
	}
	if ((k_info[o_ptr->k_idx].flags5 & TR5_ATTR_MULTI))
	    //#ifdef CLIENT_SHIMMER whatever..
		attr = TERM_HALF;

	/* check for tval/sval */
	for (i = 0; i < 10; i++) {
		/* no tval specified? */
		if (q_ret->otval[i] == 0) continue;
		/* accept any tval? */
		if (q_ret->otval[i] != -1 &&
		    /* specified a tval */
		    q_ret->otval[i] != o_ptr->tval) continue;;

		/* no sval specified? */
		if (q_ret->osval[i] == 0) continue;
		/* accept any sval? */
		if (q_ret->osval[i] != -1 &&
		    /* specified a sval */
		    q_ret->osval[i] != o_ptr->sval) continue;;

		break;
	}
	if (i == 10) return FALSE;

	/* check for pval/bpval/attr/name1/name2/name2b
	   note: let's treat pval/bpval as minimum values instead of exact values for now. */
	for (i = 0; i < 5; i++) {
		/* no pval specified? */
		if (q_ret->opval[i] == 9999) continue;
		/* accept any pval? */
		if (q_ret->opval[i] != -9999 &&
		    /* specified a pval? */
		    q_ret->opval[i] < o_ptr->pval) continue;

		/* no bpval specified? */
		if (q_ret->obpval[i] == 9999) continue;
		/* accept any bpval? */
		if (q_ret->obpval[i] != -9999 &&
		    /* specified a bpval? */
		    q_ret->obpval[i] < o_ptr->bpval) continue;

		/* no attr specified? */
		if (q_ret->oattr[i] == 255) continue;
		/* accept any attr? */
		if (q_ret->oattr[i] != 254 &&
		    /* specified a attr? */
		    q_ret->oattr[i] != attr) continue;

		/* no name1 specified? */
		if (q_ret->oname1[i] == -3) continue;
		 /* accept any name1? */
		if (q_ret->oname1[i] != -1 &&
		 /* accept any name1, but MUST be != 0? */
		    (q_ret->oname1[i] != -2 || !o_ptr->name1) &&
		    /* specified a name1? */
		    q_ret->oname1[i] != o_ptr->name1) continue;

		/* no name2 specified? */
		if (q_ret->oname2[i] == -3) continue;
		 /* accept any name2? */
		if (q_ret->oname2[i] != -1 &&
		 /* accept any name2, but MUST be != 0? */
		    (q_ret->oname2[i] != -2 || !o_ptr->name2) &&
		    /* specified a name2? */
		    q_ret->oname2[i] != o_ptr->name2) continue;

		/* no name2b specified? */
		if (q_ret->oname2b[i] == -3) continue;
		 /* accept any name2b? */
		if (q_ret->oname2b[i] != -1 &&
		 /* accept any name2b, but MUST be != 0? */
		    (q_ret->oname2b[i] != -2 || !o_ptr->name2b) &&
		    /* specified a name2b? */
		    q_ret->oname2b[i] != o_ptr->name2b) continue;

		break;
	}
	if (i == 5) return FALSE;

	/* finally, test against minimum value */
	if (q_ret->ovalue <= object_value_real(0, o_ptr)) return TRUE;

	/* pft */
	return FALSE;
}

/* Check if player completed a kill or item-retrieve goal.
   The monster slain or item retrieved must therefore be given to this function for examination.
   Note: The mechanics for retrieving quest items at a specific target position are a bit tricky:
         If n items have to be retrieved, each one has to be a different item that gets picked at
         the target pos. When an item is lost from the player's inventory again however, it mayb be
         retrieved anywhere, ignoring the target location specification. This requires the quest
         items to be marked when they get picked up at the target location, to free those marked
         ones from same target loc restrictions for re-pickup. */
static void quest_check_goal_kr(int Ind, int q_idx, int py_q_idx, monster_type *m_ptr, object_type *o_ptr) {
	quest_info *q_ptr = &q_info[q_idx];;
	player_type *p_ptr = Players[Ind];
	int j, k, stage;
	bool nisi;
	qi_stage *q_stage;
	qi_goal *q_goal;

	/* quest is deactivated? */
	if (!q_ptr->active) return;

	stage = quest_get_stage(Ind, q_idx);
	q_stage = quest_qi_stage(q_idx, stage);

	/* For handling Z-lines: flags changed depending on goals completed:
	   pre-check if we have any pending deliver goal in this stage.
	   If so then we can only set the quest goal 'nisi' (provisorily),
	   and hence flags won't get changed yet until it is eventually resolved
	   when we turn in the delivery.
	   Now also used for limiting/fixing ungoal_r: only nisi-goals can be unset. */
	nisi = FALSE;
	for (j = 0; j < q_stage->goals; j++)
		if (q_stage->goal[j].deliver) {
			nisi = TRUE;
			break;
		}

#if QDEBUG > 2
	s_printf(" CHECKING k/r-GOAL IN QUEST (%s,%d) stage %d.\n", q_ptr->codename, q_idx, stage);
#endif
	/* check the quest goals, whether any of them wants a target to this location */
	for (j = 0; j < q_stage->goals; j++) {
		q_goal = &q_stage->goal[j];

		/* no k/r goal? */
		if (!q_goal->kill && !q_goal->retrieve) continue;
#if QDEBUG > 2
		s_printf(" FOUND kr GOAL %d (k=%d,r=%d).\n", j, q_goal->kill ? TRUE : FALSE, q_goal->retrieve ? TRUE : FALSE);
#endif

		/* location-restricted?
		   Exempt already retrieved items that were just lost temporarily on the way! */
		if (q_goal->target_pos &&
		    (!o_ptr || !(o_ptr->quest == q_idx + 1 && o_ptr->quest_stage == stage))) {
			/* extend target terrain over a wide patch? */
			if (q_goal->target_terrain_patch) {
				/* different z-coordinate = instant fail */
				if (p_ptr->wpos.wz != q_goal->target_wpos.wz) continue;
				/* are we within range and have same terrain type? */
				if (distance(p_ptr->wpos.wy, p_ptr->wpos.wx,
				    q_goal->target_wpos.wy, q_goal->target_wpos.wx) > QI_TERRAIN_PATCH_RADIUS ||
				    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type !=
				    wild_info[q_goal->target_wpos.wy][q_goal->target_wpos.wx].type)
					continue;
			}
			/* just check a single, specific wpos? */
			else if (!inarea(&q_goal->target_wpos, &p_ptr->wpos)) continue;

			/* check for exact x,y location? */
			if (q_goal->target_pos_x != -1 &&
			    distance(q_goal->target_pos_y, q_goal->target_pos_x, p_ptr->py, p_ptr->px) > q_goal->target_pos_radius)
				continue;
		}
#if QDEBUG > 2
		s_printf(" PASSED/NO LOCATION CHECK.\n");
#endif

	///TODO: implement for global quests too!
		/* check for kill goal here */
		if (m_ptr && q_goal->kill) {
			if (!quest_goal_matches_kill(q_idx, stage, j, m_ptr)) continue;
			/* decrease the player's kill counter, if we got all, goal is completed! */
			p_ptr->quest_kill_number[py_q_idx][j]--;
			if (p_ptr->quest_kill_number[py_q_idx][j]) continue; /* not yet */

			/* we have completed a target-to-xy goal! */
			quest_set_goal(Ind, q_idx, j, nisi);
			continue;
		}

	///TODO: implement for global quests too!
		/* check for retrieve-item goal here */
		if (o_ptr && q_goal->retrieve) {
			/* Targetted retrieval quests only allow unowned items,
			   so you cannot carry them there by yourself first.. */
			if (q_goal->target_pos && o_ptr->owner) continue;

			if (!quest_goal_matches_object(q_idx, stage, j, o_ptr)) continue;

			/* discard (old) items from another quest or quest stage that just look similar!
			   Those cannot simply be reused. */
			if (o_ptr->quest && (o_ptr->quest != q_idx + 1 || o_ptr->quest_stage != stage)) continue;

			/* mark the item as quest item, so we know we found it at the designated target loc (if any) */
			o_ptr->quest = q_idx + 1;
			o_ptr->quest_stage = stage;

			/* decrease the player's retrieve counter, if we got all, goal is completed! */
			p_ptr->quest_retrieve_number[py_q_idx][j] -= o_ptr->number;
			if (p_ptr->quest_retrieve_number[py_q_idx][j] > 0) continue; /* not yet */

			/* we have completed a target-to-xy goal! */
			quest_set_goal(Ind, q_idx, j, nisi);

			/* if we don't have to deliver in this stage,
			   we can just remove all the quest items right away now,
			   since they've fulfilled their purpose of setting the quest goal. */
			for (k = 0; k < q_stage->goals; k++)
				if (q_stage->goal[k].deliver) {
#if QDEBUG > 2
					s_printf(" CANNOT REMOVE RETRIEVED ITEMS: awaiting delivery.\n");
#endif
					break;
				}
			if (k == q_stage->goals) {
#if QDEBUG > 2
				s_printf(" REMOVE RETRIEVED ITEMS.\n");
#endif
				if (q_ptr->individual) {
					for (k = INVEN_PACK - 1; k >= 0; k--) {
						if (p_ptr->inventory[k].quest == q_idx + 1 &&
						    p_ptr->inventory[k].quest_stage == stage) {
							inven_item_increase(Ind, k, -99);
							//inven_item_describe(Ind, k);
							inven_item_optimize(Ind, k);
						}
					}
				} else {
					//TODO (not just here): implement global retrieval quests..
				}
			}
			continue;
		}
	}
}
/* Small intermediate function to reduce workload.. (1. for k-goals) */
void quest_check_goal_k(int Ind, monster_type *m_ptr) {
	player_type *p_ptr = Players[Ind];
	int i;

#if QDEBUG > 3
	s_printf("QUEST_CHECK_GOAL_r: by %d,%s\n", Ind, p_ptr->name);
#endif
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++) {
#if 0
		/* player actually pursuing a quest? -- paranoia (quest_kill should be FALSE then) */
		if (p_ptr->quest_idx[i] == -1) continue;
#endif
		/* reduce workload */
		if (!p_ptr->quest_kill[i]) continue;

		quest_check_goal_kr(Ind, p_ptr->quest_idx[i], i, m_ptr, NULL);
	}
}
/* Small intermediate function to reduce workload.. (2. for r-goals) */
void quest_check_goal_r(int Ind, object_type *o_ptr) {
	player_type *p_ptr = Players[Ind];
	int i;

#if QDEBUG > 3
	s_printf("QUEST_CHECK_GOAL_k: by %d,%s\n", Ind, p_ptr->name);
#endif
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++) {
#if 0
		/* player actually pursuing a quest? -- paranoia (quest_retrieve should be FALSE then) */
		if (p_ptr->quest_idx[i] == -1) continue;
#endif
		/* reduce workload */
		if (!p_ptr->quest_retrieve[i]) continue;

		quest_check_goal_kr(Ind, p_ptr->quest_idx[i], i, NULL, o_ptr);
	}
}
/* Check if we have to un-set an item-retrieval quest goal because we lost <num> items!
   Note: If we restricted quest stages to request no overlapping item types, we could also add
         'quest_goal' to object_type so we won't need lookups in here, but currently there is no
         such restriction and I don't think we need it. */
void quest_check_ungoal_r(int Ind, object_type *o_ptr, int num) {
	quest_info *q_ptr;
	player_type *p_ptr = Players[Ind];
	int i, j, q_idx, stage;
	qi_stage *q_stage;

#if QDEBUG > 3
	s_printf("QUEST_CHECK_UNGOAL_r: by %d,%s\n", Ind, p_ptr->name);
#endif
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++) {
#if 0
		/* player actually pursuing a quest? -- paranoia (quest_retrieve should be FALSE then) */
		if (p_ptr->quest_idx[i] == -1) continue;
#endif
		/* reduce workload */
		if (!p_ptr->quest_retrieve[i]) continue;

		q_idx = p_ptr->quest_idx[i];
		q_ptr = &q_info[q_idx];

		/* quest is deactivated? */
		if (!q_ptr->active) continue;

		stage = quest_get_stage(Ind, q_idx);
		q_stage = quest_qi_stage(q_idx, stage);
#if QDEBUG > 2
		s_printf(" (UNGOAL) CHECKING FOR QUEST (%s,%d) stage %d.\n", q_ptr->codename, q_idx, stage);
#endif

		/* check the quest goals, whether any of them wants a retrieval */
		for (j = 0; j < q_stage->goals; j++) {
			/* no r goal? */
			if (!q_stage->goal[j].retrieve) continue;

			/* only nisi-goals: we can lose quest items on our way
			   to deliver them, but if we don't have to deliver
			   them and we already collected all, then the goal is
			   set in stone...uh or cleared in stone? */
			if (!quest_get_goal_nisi(Ind, q_idx, j)) continue;

			/* phew, item has nothing to do with this quest goal? */
			if (!quest_goal_matches_object(q_idx, stage, j, o_ptr)) continue;
#if QDEBUG > 2
			s_printf(" (UNGOAL) FOUND r GOAL %d by %d,%s\n", j, Ind, p_ptr->name);
#endif

			/* increase the player's retrieve counter again */
			p_ptr->quest_retrieve_number[i][j] += num;
			if (p_ptr->quest_retrieve_number[i][j] <= 0) continue; /* still cool */

			/* we have un-completed a target-to-xy goal, ow */
			quest_unset_goal(Ind, q_idx, j);
			continue;
		}
	}
}

/* When we're called then we know that this player just arrived at a fitting
   wpos for a deliver quest. We can now complete the corresponding quest goal accordingly. */
static void quest_handle_goal_deliver_wpos(int Ind, int py_q_idx, int q_idx, int stage) {
	player_type *p_ptr = Players[Ind];
	quest_info *q_ptr = &q_info[q_idx];
	int j, k;
	qi_stage *q_stage = quest_qi_stage(q_idx, stage);
	qi_goal *q_goal;
	qi_deliver *q_del;

#if QDEBUG > 2
	s_printf("QUEST_HANDLE_GOAL_DELIVER_WPOS: by %d,%s - quest (%s,%d) stage %d\n",
	    Ind, p_ptr->name, q_ptr->codename, q_idx, stage);
#endif

	/* pre-check if we have completed all kill/retrieve goals of this stage,
	   because we can only complete any deliver goal if we have fetched all
	   the stuff (bodies, or kill count rather, and objects) that we ought to
	   'deliver', duh */
	for (j = 0; j < q_stage->goals; j++) {
		q_goal = &q_stage->goal[j];
#if QDEBUG > 2
		if (q_goal->kill || q_goal->retrieve) {
			s_printf(" --FOUND GOAL %d (k%d,m%d)..", j, q_goal->kill, q_goal->retrieve);
			if (!quest_get_goal(Ind, q_idx, j, FALSE)) {
				s_printf("MISSING.\n");
				break;
			} else s_printf("DONE.\n");

		}
#else
		if ((q_goal->kill || q_goal->retrieve)
		    && !quest_get_goal(Ind, q_idx, j, FALSE))
			break;
#endif
	}
	if (j != QI_GOALS) {
#if QDEBUG > 2
		s_printf(" MISSING kr GOAL\n");
#endif
		return;
	}

	/* check the quest goals, whether any of them wants a delivery to this location */
	for (j = 0; j < q_stage->goals; j++) {
		q_goal = &q_stage->goal[j];

		if (!q_goal->deliver) continue;
		q_del = q_goal->deliver;

		/* handle only non-specific x,y goals here */
		if (q_del->pos_x != -1) continue;
#if QDEBUG > 2
		s_printf(" FOUND deliver_wpos GOAL %d.\n", j);
#endif

		/* extend target terrain over a wide patch? */
		if (q_del->terrain_patch) {
			/* different z-coordinate = instant fail */
			if (p_ptr->wpos.wz != q_del->wpos.wz) continue;
			/* are we within range and have same terrain type? */
			if (distance(p_ptr->wpos.wy, p_ptr->wpos.wx,
			    q_del->wpos.wy, q_del->wpos.wx) > QI_TERRAIN_PATCH_RADIUS ||
			    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type !=
			    wild_info[q_del->wpos.wy][q_del->wpos.wx].type)
				continue;
		}
		/* just check a single, specific wpos? */
		else if (!inarea(&q_del->wpos, &p_ptr->wpos)) continue;
#if QDEBUG > 2
		s_printf(" (DELIVER) PASSED LOCATION CHECK.\n");
#endif

		/* First mark all 'nisi' quest goals as finally resolved,
		   to change flags accordingly if defined by a Z-line.
		   It's important to do this before removing retieved items,
		   just in case UNGOAL will kick in, in inven_item_increase()
		   and unset our quest goal :-p. */
		for (k = 0; k < q_stage->goals; k++)
			if (quest_get_goal(Ind, q_idx, k, TRUE)) {
				quest_set_goal(Ind, q_idx, k, FALSE);
				quest_goal_changes_flags(Ind, q_idx, stage, k);
			}

		/* we have completed a delivery-to-wpos goal!
		   We need to un-nisi it here before UNGOAL is called in inven_item_increase(). */
		quest_set_goal(Ind, q_idx, j, FALSE);

		/* for item retrieval goals therefore linked to this deliver goal,
		   remove all quest items now finally that we 'delivered' them.. */
		if (q_ptr->individual) {
			for (k = INVEN_PACK - 1; k >= 0; k--) {
				if (p_ptr->inventory[k].quest == q_idx + 1 &&
				    p_ptr->inventory[k].quest_stage == stage) {
					inven_item_increase(Ind, k, -99);
					//inven_item_describe(Ind, k);
					inven_item_optimize(Ind, k);
				}
			}
		} else {
			//TODO (not just here): implement global retrieval quests..
		}
	}
}
/* Check if player completed a deliver goal to a wpos and specific x,y. */
static void quest_check_goal_deliver_xy(int Ind, int q_idx, int py_q_idx) {
	player_type *p_ptr = Players[Ind];
	int j, k, stage;
	quest_info *q_ptr = &q_info[q_idx];;
	qi_stage *q_stage;
	qi_goal *q_goal;
	qi_deliver *q_del;

#if QDEBUG > 3
	s_printf("QUEST_CHECK_GOAL_DELIVER_XY: by %d,%s - quest (%s,%d) stage %d\n",
	    Ind, p_ptr->name, q_ptr->codename, q_idx, quest_get_stage(Ind, q_idx));
#endif

	/* quest is deactivated? */
	if (!q_ptr->active) return;

	stage = quest_get_stage(Ind, q_idx);
	q_stage = quest_qi_stage(q_idx, stage);

	/* pre-check if we have completed all kill/retrieve goals of this stage,
	   because we can only complete any deliver goal if we have fetched all
	   the stuff (bodies, or kill count rather, and objects) that we ought to
	   'deliver', duh */
	for (j = 0; j < q_stage->goals; j++) {
		q_goal = &q_stage->goal[j];
#if QDEBUG > 3
		if (q_goal->kill || q_goal->retrieve) {
			s_printf(" --FOUND GOAL %d (k%d,m%d)..", j, q_ptr->kill[j], q_ptr->retrieve[j]);
			if (!quest_get_goal(Ind, q_idx, j, FALSE)) {
				s_printf("MISSING.\n");
				break;
			} else s_printf("DONE.\n");

		}
#else
		if ((q_goal->kill || q_goal->retrieve)
		    && !quest_get_goal(Ind, q_idx, j, FALSE))
			break;
#endif
	}
	if (j != q_stage->goals) {
#if QDEBUG > 3
		s_printf(" MISSING kr GOAL\n");
#endif
		return;
	}

	/* check the quest goals, whether any of them wants a delivery to this location */
	for (j = 0; j < q_stage->goals; j++) {
		q_goal = &q_stage->goal[j];
		if (!q_goal->deliver) continue;
		q_del = q_goal->deliver;

		/* handle only specific x,y goals here */
		if (q_goal->deliver->pos_x == -1) continue;
#if QDEBUG > 3
		s_printf(" FOUND deliver_xy GOAL %d.\n", j);
#endif

		/* extend target terrain over a wide patch? */
		if (q_del->terrain_patch) {
			/* different z-coordinate = instant fail */
			if (p_ptr->wpos.wz != q_del->wpos.wz) continue;
			/* are we within range and have same terrain type? */
			if (distance(p_ptr->wpos.wy, p_ptr->wpos.wx,
			    q_del->wpos.wy, q_del->wpos.wx) > QI_TERRAIN_PATCH_RADIUS ||
			    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type !=
			    wild_info[q_del->wpos.wy][q_del->wpos.wx].type)
				continue;
		}
		/* just check a single, specific wpos? */
		else if (!inarea(&q_del->wpos, &p_ptr->wpos)) continue;

		/* check for exact x,y location, plus radius */
		if (distance(q_del->pos_y, q_del->pos_x, p_ptr->py, p_ptr->px) > q_del->radius)
			continue;
#if QDEBUG > 2
		s_printf(" (DELIVER_XY) PASSED LOCATION CHECK.\n");
#endif

		/* First mark all 'nisi' quest goals as finally resolved,
		   to change flags accordingly if defined by a Z-line.
		   It's important to do this before removing retieved items,
		   just in case UNGOAL will kick in, in inven_item_increase()
		   and unset our quest goal :-p. */
		for (k = 0; k < q_stage->goals; k++)
			if (quest_get_goal(Ind, q_idx, k, TRUE)) {
				quest_set_goal(Ind, q_idx, k, FALSE);
				quest_goal_changes_flags(Ind, q_idx, stage, k);
			}

		/* we have completed a delivery-to-xy goal!
		   We need to un-nisi it here before UNGOAL is called in inven_item_increase(). */
		quest_set_goal(Ind, q_idx, j, FALSE);

		/* for item retrieval goals therefore linked to this deliver goal,
		   remove all quest items now finally that we 'delivered' them.. */
		if (q_ptr->individual) {
			for (k = INVEN_PACK - 1; k >= 0; k--) {
				if (p_ptr->inventory[k].quest == q_idx + 1 &&
				    p_ptr->inventory[k].quest_stage == stage) {
					inven_item_increase(Ind, k, -99);
					//inven_item_describe(Ind, k);
					inven_item_optimize(Ind, k);
				}
			}
		} else {
			//TODO (not just here): implement global retrieval quests..
		}
	}
}
/* Small intermediate function to reduce workload.. (3. for deliver-goals) */
void quest_check_goal_deliver(int Ind) {
	player_type *p_ptr = Players[Ind];
	int i;

	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++) {
#if 0
		/* player actually pursuing a quest? -- paranoia (quest_retrieve should be FALSE then) */
		if (p_ptr->quest_idx[i] == -1) continue;
#endif
		/* reduce workload */
		if (!p_ptr->quest_deliver_xy[i]) continue;

		quest_check_goal_deliver_xy(Ind, p_ptr->quest_idx[i], i);
	}
}

/* check goals for their current completion state, for entering the next stage if ok */
static int quest_goal_check_stage(int pInd, int q_idx) {
	int i, j, stage = quest_get_stage(pInd, q_idx);
	qi_stage *q_stage = quest_qi_stage(q_idx, stage);

	/* scan through all possible follow-up stages */
	for (j = 0; j < QI_FOLLOWUP_STAGES; j++) {
		/* no follow-up stage? */
		if (q_stage->next_stage_from_goals[j] == -1) continue;

		/* scan through all goals required to be fulfilled to enter this stage */
		for (i = 0; i < QI_STAGE_GOALS; i++) {
			if (q_stage->goals_for_stage[j][i] == -1) continue;

			/* If even one goal is missing, we cannot advance. */
			if (!quest_get_goal(pInd, q_idx, q_stage->goals_for_stage[j][i], FALSE)) break;
		}
		/* we may proceed to another stage? */
		if (i == QI_STAGE_GOALS) return q_stage->next_stage_from_goals[j];
	}
	return -1; /* goals are not complete yet */
}

/* hand out a reward object to player (if individual quest)
   or to all involved players in the area (if non-individual quest) */
static void quest_reward_object(int pInd, int q_idx, object_type *o_ptr) {
	quest_info *q_ptr = &q_info[q_idx];
	//player_type *p_ptr;
	int i, j;

	if (pInd && q_ptr->individual) { //we should never get an individual quest without a pInd here..
		/*p_ptr = Players[pInd];
		drop_near(o_ptr, -1, &p_ptr->wpos, &p_ptr->py, &p_ptr->px);*/
		//msg_print(pInd, "You have received an item."); --give just ONE message for ALL items, less spammy
		inven_carry(pInd, o_ptr);
		return;
	}

	if (!q_ptr->individual) {
		s_printf(" QUEST_REWARD_OBJECT: not individual, but no pInd either.\n");
		return;//catch paranoid bugs--maybe obsolete
	}

	/* global quest (or for some reason missing pInd..<-paranoia)  */
	for (i = 1; i <= NumPlayers; i++) {
		/* is the player on this quest? */
		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (Players[i]->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) continue;

		/* must be around a questor to receive the rewards,
		   so you can't afk-quest in a pack of people with one person doing it all. */
		for (j = 0; j < q_ptr->questors; j++)
			if (inarea(&Players[i]->wpos, &q_ptr->questor[j].current_wpos)) break;
		if (j == q_ptr->questors) continue;

		/* hand him out the reward too */
		/*p_ptr = Players[i];
		drop_near(o_ptr, -1, &p_ptr->wpos, &p_ptr->py, &p_ptr->px);*/
		//msg_print(i, "You have received an item."); --give just ONE message for ALL items, less spammy
		inven_carry(i, o_ptr);
	}
}

/* hand out a reward object created by create_reward() to player (if individual quest)
   or to all involved players in the area (if non-individual quest) */
static void quest_reward_create(int pInd, int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	u32b resf = RESF_NOTRUEART;
	int i, j;

	if (pInd && q_ptr->individual) { //we should never get an individual quest without a pInd here..
		//msg_print(pInd, "You have received an item."); --give just ONE message for ALL items, less spammy
		give_reward(pInd, resf, q_name + q_ptr->name, 0, 0);
		return;
	}

	if (!q_ptr->individual) {
		s_printf(" QUEST_REWARD_CREATE: not individual, but no pInd either.\n");
		return;//catch paranoid bugs--maybe obsolete
	}

	/* global quest (or for some reason missing pInd..<-paranoia)  */
	for (i = 1; i <= NumPlayers; i++) {
		/* is the player on this quest? */
		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (Players[i]->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) continue;

		/* must be around a questor to receive the rewards,
		   so you can't afk-quest in a pack of people with one person doing it all. */
		for (j = 0; j < q_ptr->questors; j++)
			if (inarea(&Players[i]->wpos, &q_ptr->questor[j].current_wpos)) break;
		if (j == q_ptr->questors) continue;

		/* hand him out the reward too */
		//msg_print(i, "You have received an item."); --give just ONE message for ALL items, less spammy
		give_reward(i, resf, q_name + q_ptr->name, 0, 0);
	}
}

/* hand out gold to player (if individual quest)
   or to all involved players in the area (if non-individual quest) */
static void quest_reward_gold(int pInd, int q_idx, int au) {
	quest_info *q_ptr = &q_info[q_idx];
	int i, j;

	if (pInd && q_ptr->individual) { //we should never get an individual quest without a pInd here..
		//msg_format(pInd, "You have received %d gold pieces.", au); --give just ONE message for ALL gold, less spammy
		gain_au(pInd, au, FALSE, FALSE);
		return;
	}

	if (!q_ptr->individual) {
		s_printf(" QUEST_REWARD_GOLD: not individual, but no pInd either.\n");
		return;//catch paranoid bugs--maybe obsolete
	}

	/* global quest (or for some reason missing pInd..<-paranoia)  */
	for (i = 1; i <= NumPlayers; i++) {
		/* is the player on this quest? */
		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (Players[i]->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) continue;

		/* must be around a questor to receive the rewards,
		   so you can't afk-quest in a pack of people with one person doing it all. */
		for (j = 0; j < q_ptr->questors; j++)
			if (inarea(&Players[i]->wpos, &q_ptr->questor[j].current_wpos)) break;
		if (j == q_ptr->questors) continue;

		/* hand him out the reward too */
		//msg_format(i, "You have received %d gold pieces.", au); --give just ONE message for ALL gold, less spammy
		gain_au(i, au, FALSE, FALSE);
	}
}

/* provide experience to player (if individual quest)
   or to all involved players in the area (if non-individual quest) */
static void quest_reward_exp(int pInd, int q_idx, int exp) {
	quest_info *q_ptr = &q_info[q_idx];
	int i, j;

	if (pInd && q_ptr->individual) { //we should never get an individual quest without a pInd here..
		//msg_format(pInd, "You have received %d experience points.", exp); --give just ONE message for ALL gold, less spammy
		gain_exp(pInd, exp);
		return;
	}

	if (!q_ptr->individual) {
		s_printf(" QUEST_REWARD_EXP: not individual, but no pInd either.\n");
		return;//catch paranoid bugs--maybe obsolete
	}

	/* global quest (or for some reason missing pInd..<-paranoia)  */
	for (i = 1; i <= NumPlayers; i++) {
		/* is the player on this quest? */
		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (Players[i]->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) continue;

		/* must be around a questor to receive the rewards,
		   so you can't afk-quest in a pack of people with one person doing it all. */
		for (j = 0; j < q_ptr->questors; j++)
			if (inarea(&Players[i]->wpos, &q_ptr->questor[j].current_wpos)) break;
		if (j == q_ptr->questors) continue;

		/* hand him out the reward too */
		//msg_format(i, "You have received %d experience points.", exp); --give just ONE message for ALL gold, less spammy
		gain_exp(i, exp);
	}
}

/* check current stage of quest stage goals for handing out rewards:
   1) we've entered a quest stage and it's pretty much 'empty' so we might terminate
      if nothing more is up. check for free rewards first and hand them out.
   2) goals were completed. Before advancing the stage, hand out the proper rewards. */
//TODO: test for proper implementation for non-'individual' quests
static void quest_goal_check_reward(int pInd, int q_idx) {
	quest_info *q_ptr = &q_info[q_idx];
	int i, j, stage = quest_get_stage(pInd, q_idx);
	object_type forge, *o_ptr;
	u32b resf = RESF_NOTRUEART;
	/* count rewards */
	int r_obj = 0, r_gold = 0, r_exp = 0;
	qi_stage *q_stage = quest_qi_stage(q_idx, stage);
	qi_reward *q_rew;
	qi_questor *q_questor;
	struct worldpos wpos = { cfg.town_x, cfg.town_y, 0};

	/* Since the rewards are fixed, independantly of where the questors are,
	   we can just pick either one questor at random to use his wpos or just
	   use an arbitrarily set wpos for apply_magic() calls further down. */
	if (q_ptr->questors) { /* extreme paranoia - could be false with auto-acquired quests? */
#if 0
		for (j = 0; j < q_ptr->questors; j++) {
			q_questor = &q_ptr->questor[j];
			if (q_questor->despawned) continue;
			/* just pick the first good questor for now.. w/e */
			wpos = q_questor->current_wpos;
			break;
		}
#else
		/* just pick the first questor for now.. w/e */
		q_questor = &q_ptr->questor[0];
		wpos = q_questor->current_wpos;
#endif
	}

#if 0 /* we're called when stage 0 starts too, and maybe it's some sort of globally determined goal->reward? */
	if (!pInd || !q_ptr->individual) {
		s_printf(" QUEST_GOAL_CHECK_REWARD: returned! oops\n");
		return; //paranoia?
	}
#endif

	/* scan through all possible follow-up stages */
	for (j = 0; j < q_stage->rewards; j++) {
		q_rew = &q_stage->reward[j];

		/* no reward? */
		if (!q_rew->otval &&
		    !q_rew->oreward &&
		    !q_rew->gold &&
		    !q_rew->exp) //TODO: reward_statuseffect
			continue;

		/* scan through all goals required to be fulfilled to get this reward */
		for (i = 0; i < QI_REWARD_GOALS; i++) {
			if (q_stage->goals_for_reward[j][i] == -1) continue;

			/* if even one goal is missing, we cannot get the reward */
			if (!quest_get_goal(pInd, q_idx, q_stage->goals_for_reward[j][i], FALSE)) break;
		}
		/* we may get rewards? */
		if (i == QI_REWARD_GOALS) {
			/* create and hand over this reward! */
#if QDEBUG > 0
			s_printf("%s QUEST_GOAL_CHECK_REWARD: Rewarded in stage %d of '%s'(%d,%s)\n", showtime(), stage, q_name + q_ptr->name, q_idx, q_ptr->codename);
#endif

			/* specific reward */
			if (q_rew->otval) {
				/* very specific reward */
				if (!q_rew->ogood && !q_rew->ogreat) {
					o_ptr = &forge;
					object_wipe(o_ptr);
					invcopy(o_ptr, lookup_kind(q_rew->otval, q_rew->osval));
					o_ptr->number = 1;
					o_ptr->name1 = q_rew->oname1;
					o_ptr->name2 = q_rew->oname2;
					o_ptr->name2b = q_rew->oname2b;
					if (o_ptr->name1) {
						o_ptr->name1 = ART_RANDART; //hack: disallow true arts!
						o_ptr->name2 = o_ptr->name2b = 0;
					}
					apply_magic(&wpos, o_ptr, -2, FALSE, FALSE, FALSE, FALSE, resf);
					o_ptr->pval = q_rew->opval;
					o_ptr->bpval = q_rew->obpval;
					o_ptr->note = quark_add(format("%s", q_name + q_ptr->name));
					o_ptr->note_utag = 0;
#ifdef PRE_OWN_DROP_CHOSEN
					o_ptr->level = 0;
					o_ptr->owner = p_ptr->id;
					o_ptr->mode = p_ptr->mode;
					if (o_ptr->name1) determine_artifact_timeout(o_ptr->name1);
#endif
				} else {
					o_ptr = &forge;
					object_wipe(o_ptr);
					invcopy(o_ptr, lookup_kind(q_rew->otval, q_rew->osval));
					o_ptr->number = 1;
					apply_magic(&wpos, o_ptr, -2, q_rew->ogood, q_rew->ogreat, q_rew->ovgreat, FALSE, resf);
					o_ptr->note = quark_add(format("%s", q_name + q_ptr->name));
					o_ptr->note_utag = 0;
#ifdef PRE_OWN_DROP_CHOSEN
					o_ptr->level = 0;
					o_ptr->owner = p_ptr->id;
					o_ptr->mode = p_ptr->mode;
					if (o_ptr->name1) determine_artifact_timeout(o_ptr->name1);
#endif
				}

				/* hand it out */
				quest_reward_object(pInd, q_idx, o_ptr);
				r_obj++;
			}
			/* instead use create_reward() like for events? */
			else if (q_rew->oreward) {
				quest_reward_create(pInd, q_idx);
				r_obj++;
			}
			/* hand out gold? */
			if (q_rew->gold) {
				quest_reward_gold(pInd, q_idx, q_rew->gold);
				r_gold += q_rew->gold;
			}
			/* provide exp? */
			if (q_rew->exp) {
				quest_reward_exp(pInd, q_idx, q_rew->exp);
				r_exp += q_rew->exp;
			}
			//TODO: s16b reward_statuseffect[QI_STAGES][QI_STAGE_REWARDS];
		}
	}

	/* give one unified message per reward type that was handed out */
	if (pInd && q_ptr->individual) {
		if (r_obj == 1) msg_print(pInd, "You have received an item.");
		else if (r_obj) msg_format(pInd, "You have received %d items.", r_obj);
		if (r_gold) msg_format(pInd, "You have received %d gold piece%s.", r_gold, r_gold == 1 ? "" : "s");
		if (r_exp) msg_format(pInd, "You have received %d experience point%s.", r_exp, r_exp == 1 ? "" : "s");
	} else for (i = 1; i <= NumPlayers; i++) {
		/* is the player on this quest? */
		for (j = 0; j < MAX_CONCURRENT_QUESTS; j++)
			if (Players[i]->quest_idx[j] == q_idx) break;
		if (j == MAX_CONCURRENT_QUESTS) continue;

		/* must be around a questor to receive the rewards,
		   so you can't afk-quest in a pack of people with one person doing it all. */
		for (j = 0; j < q_ptr->questors; j++)
			if (inarea(&Players[i]->wpos, &q_ptr->questor[j].current_wpos)) break;
		if (j == q_ptr->questors) continue;

		if (r_obj == 1) msg_print(i, "You have received an item.");
		else if (r_obj) msg_format(i, "You have received %d items.", r_obj);
		if (r_gold) msg_format(i, "You have received %d gold piece%s.", r_gold, r_gold == 1 ? "" : "s");
		if (r_exp) msg_format(i, "You have received %d experience point%s.", r_exp, r_exp == 1 ? "" : "s");
	}

	return; /* goals are not complete yet */
}

/* Check if quest goals of the current stage have been completed and accordingly
   call quest_goal_check_reward() and/or quest_set_stage() to advance.
   Goals can only be completed by players who are pursuing that quest.
   'interacting' is TRUE if a player is interacting with the questor. */
static bool quest_goal_check(int pInd, int q_idx, bool interacting) {
	int stage;

	/* check goals for stage advancement */
	stage = quest_goal_check_stage(pInd, q_idx);
	if (stage == -1) return FALSE; /* stage not yet completed */

	/* check goals for rewards */
	quest_goal_check_reward(pInd, q_idx);

	/* advance stage! */
	quest_set_stage(pInd, q_idx, stage, FALSE);
	return TRUE; /* stage has been completed and changed to the next stage */
}

/* Check if a player's location is around any of his quest destinations (target/delivery). */
void quest_check_player_location(int Ind) {
	player_type *p_ptr = Players[Ind];
	int i, j, q_idx, stage;
	quest_info *q_ptr;
	qi_stage *q_stage;
	qi_goal *q_goal;
	qi_deliver *q_del;

#if QDEBUG > 3
	s_printf("%s CHECK_PLAYER_LOCATION: %d,%s has anyk,anyr=%d,%d.\n", showtime(), Ind, p_ptr->name, p_ptr->quest_any_k, p_ptr->quest_any_r);
#endif

	/* reset local tracking info? only if we don't need to check it anyway: */
	if (!p_ptr->quest_any_k) p_ptr->quest_any_k_within_target = FALSE;
	if (!p_ptr->quest_any_r) p_ptr->quest_any_r_within_target = FALSE;
	p_ptr->quest_any_deliver_xy_within_target = FALSE; /* deliveries are of course intrinsically not 'global' but have a target location^^ */

	/* Quests: Is this wpos a target location or a delivery location for any of our quests? */
	for (i = 0; i < MAX_CONCURRENT_QUESTS; i++) {
		/* player actually pursuing a quest? */
		if (p_ptr->quest_idx[i] == -1) continue;

		/* check for target location for kill goals.
		   We only need to do that if we aren't already in 'always checking' mode,
		   because we have quest goals that aren't restricted to a specific target location. */
		if (p_ptr->quest_kill[i] && !p_ptr->quest_any_k && p_ptr->quest_any_k_target) {
			q_idx = p_ptr->quest_idx[i];
			q_ptr = &q_info[q_idx];

			/* quest is deactivated? */
			if (!q_ptr->active) continue;

			stage = quest_get_stage(Ind, q_idx);
			q_stage = quest_qi_stage(q_idx, stage);

			/* check the quest goals, whether any of them wants a target to this location */
			for (j = 0; j < q_stage->goals; j++) {
				q_goal = &q_stage->goal[j];

				if (!q_goal->kill) continue;
				if (!q_goal->target_pos) continue;

				/* extend target terrain over a wide patch? */
				if (q_goal->target_terrain_patch) {
					/* different z-coordinate = instant fail */
					if (p_ptr->wpos.wz != q_goal->target_wpos.wz) continue;
					/* are we within range and have same terrain type? */
					if (distance(p_ptr->wpos.wx, p_ptr->wpos.wy,
					    q_goal->target_wpos.wx, q_goal->target_wpos.wy) <= QI_TERRAIN_PATCH_RADIUS &&
					    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type ==
					    wild_info[q_goal->target_wpos.wy][q_goal->target_wpos.wx].type) {
						p_ptr->quest_any_k_within_target = TRUE;
						break;
					}
				}
				/* just check a single, specific wpos? */
				else if (inarea(&q_goal->target_wpos, &p_ptr->wpos)) {
					p_ptr->quest_any_k_within_target = TRUE;
					break;
				}
			}
		}

		/* check for target location for retrieve goals.
		   We only need to do that if we aren't already in 'always checking' mode,
		   because we have quest goals that aren't restricted to a specific target location. */
		if (p_ptr->quest_retrieve[i] && !p_ptr->quest_any_r && p_ptr->quest_any_r_target) {
			q_idx = p_ptr->quest_idx[i];
			q_ptr = &q_info[q_idx];

			/* quest is deactivated? */
			if (!q_ptr->active) continue;

			stage = quest_get_stage(Ind, q_idx);
			q_stage = quest_qi_stage(q_idx, stage);

			/* check the quest goals, whether any of them wants a target to this location */
			for (j = 0; j < q_stage->goals; j++) {
				q_goal = &q_stage->goal[j];

				if (!q_goal->retrieve) continue;
				if (!q_goal->target_pos) continue;

				/* extend target terrain over a wide patch? */
				if (q_goal->target_terrain_patch) {
					/* different z-coordinate = instant fail */
					if (p_ptr->wpos.wz != q_goal->target_wpos.wz) continue;
					/* are we within range and have same terrain type? */
					if (distance(p_ptr->wpos.wx, p_ptr->wpos.wy,
					    q_goal->target_wpos.wx, q_goal->target_wpos.wy) <= QI_TERRAIN_PATCH_RADIUS &&
					    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type ==
					    wild_info[q_goal->target_wpos.wy][q_goal->target_wpos.wx].type) {
						p_ptr->quest_any_r_within_target = TRUE;
						break;
					}
				}
				/* just check a single, specific wpos? */
				else if (inarea(&q_goal->target_wpos, &p_ptr->wpos)) {
					p_ptr->quest_any_r_within_target = TRUE;
					break;
				}
			}
		}

		/* check for deliver location ('move' goals) */
		if (p_ptr->quest_deliver_pos[i]) {
			q_idx = p_ptr->quest_idx[i];
			q_ptr = &q_info[q_idx];

			/* quest is deactivated? */
			if (!q_ptr->active) continue;

			stage = quest_get_stage(Ind, q_idx);
			q_stage = quest_qi_stage(q_idx, stage);

			/* first clear old wpos' delivery state */
			p_ptr->quest_deliver_xy[i] = FALSE;

			/* check the quest goals, whether any of them wants a delivery to this location */
			for (j = 0; j < q_stage->goals; j++) {
				q_goal = &q_stage->goal[j];
				if (!q_goal->deliver) continue;
				q_del = q_goal->deliver;
				/* skip 'to-questor' deliveries */
				if (q_del->return_to_questor != 255) continue;

				/* extend target terrain over a wide patch? */
				if (q_del->terrain_patch) {
					/* different z-coordinate = instant fail */
					if (p_ptr->wpos.wz != q_del->wpos.wz) continue;
					/* are we within range and have same terrain type? */
					if (distance(p_ptr->wpos.wx, p_ptr->wpos.wy,
					    q_del->wpos.wx, q_del->wpos.wy) <= QI_TERRAIN_PATCH_RADIUS &&
					    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type ==
					    wild_info[q_del->wpos.wy][q_del->wpos.wx].type) {
						/* imprint new temporary destination location information */
						/* specific x,y loc? */
						if (q_del->pos_x != -1) {
							p_ptr->quest_deliver_xy[i] = TRUE;
							p_ptr->quest_any_deliver_xy_within_target = TRUE;
							/* and check right away if we're already on the correct x,y location */
							quest_check_goal_deliver_xy(Ind, q_idx, i);
						} else {
							/* we are at the requested deliver location (for at least this quest)! (wpos) */
							quest_handle_goal_deliver_wpos(Ind, i, q_idx, stage);
						}
						break;
					}
				}
				/* just check a single, specific wpos? */
				else if (inarea(&q_del->wpos, &p_ptr->wpos)) {
					/* imprint new temporary destination location information */
					/* specific x,y loc? */
					if (q_del->pos_x != -1) {
						p_ptr->quest_deliver_xy[i] = TRUE;
						p_ptr->quest_any_deliver_xy_within_target = TRUE;
						/* and check right away if we're already on the correct x,y location */
						quest_check_goal_deliver_xy(Ind, q_idx, i);
					} else {
						/* we are at the requested deliver location (for at least this quest)! (wpos) */
						quest_handle_goal_deliver_wpos(Ind, i, q_idx, stage);
					}
					break;
				}
			}
		}
	}
}

/* Hack: if a quest was disabled in q_info, this will have set the
   'disabled_on_load' flag of that quest, which tells us that we have to handle
   deleting its remaining questor(s) here, before the server finally starts up. */
void quest_handle_disabled_on_startup() {
	int i, j, k;
	quest_info *q_ptr;
	bool questor;

	for (i = 0; i < MAX_Q_IDX; i++) {
		q_ptr = &q_info[i];
		if (!q_ptr->defined) continue;
		if (!q_ptr->disabled_on_load) continue;

		s_printf("QUEST_DISABLED_ON_LOAD: Deleting %d questor(s) from quest %d\n", q_ptr->questors, i);
		for (j = 0; j < q_ptr->questors; j++) {
			questor = m_list[q_ptr->questor[j].mo_idx].questor;
			k = m_list[q_ptr->questor[j].mo_idx].quest;

			s_printf(" m_idx %d of q_idx %d (questor=%d)\n", q_ptr->questor[j].mo_idx, k, questor);

			if (k == i && questor) {
				s_printf("..ok.\n");
				delete_monster_idx(q_ptr->questor[j].mo_idx, TRUE);
			} else s_printf("..failed: Questor does not exist.\n");
		}
	}
}


/* ---------- Helper functions for initialisation of q_info[] from q_info.txt in init.c ---------- */

/* Allocate/initialise a questor, or return it if already existing. */
qi_questor *init_quest_questor(int q_idx, int num) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_questor *p;

	/* we already have this existing one */
	if (q_ptr->questors > num) return &q_ptr->questor[num];

	/* allocate all missing instances up to the requested index */
	p = (qi_questor*)realloc(q_ptr->questor, sizeof(qi_questor) * (num + 1));
	if (!p) {
		s_printf("init_quest_questor() Couldn't allocate memory!\n");
		exit(0);
	}
	/* initialise the ADDED memory */
	//memset(p + q_ptr->questors * sizeof(qi_questor), 0, (num + 1 - q_ptr->questors) * sizeof(qi_questor));
	memset(&p[q_ptr->questors], 0, (num + 1 - q_ptr->questors) * sizeof(qi_questor));

	/* attach it to its parent structure */
	q_ptr->questor = p;
	q_ptr->questors = num + 1;

	/* initialise with correct defaults (paranoia) */
	p = &q_ptr->questor[q_ptr->questors - 1];
	p->accept_los = FALSE;
	p->accept_interact = TRUE;
	p->talkable = TRUE;
	p->despawned = FALSE;
	p->invincible = TRUE;
	p->q_loc.tpref = NULL;

	/* done, return the new, requested one */
	return &q_ptr->questor[num];
}

/* Allocate/initialise a quest stage, or return it if already existing. */
qi_stage *init_quest_stage(int q_idx, int num) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_stage *p;
	int i, j;

	/* pointless, but w/e */
	if (num == -1) {
#if QDEBUG > 1
		s_printf("QUESTS: referred to stage -1.\n");
#endif
		return NULL;
	}

	/* we already have this existing one */
	if (q_ptr->stage_idx[num] != -1) return &q_ptr->stage[q_ptr->stage_idx[num]];

	/* allocate a new one */
	p = (qi_stage*)realloc(q_ptr->stage, sizeof(qi_stage) * (q_ptr->stages + 1));
	if (!p) {
		s_printf("init_quest_stage() Couldn't allocate memory!\n");
		exit(0);
	}
	/* initialise the ADDED memory */
	//memset(p + q_ptr->stages * sizeof(qi_stage), 0, sizeof(qi_stage));
	memset(&p[q_ptr->stages], 0, sizeof(qi_stage));

	/* attach it to its parent structure */
	q_ptr->stage = p;
	q_ptr->stage_idx[num] = q_ptr->stages;
	q_ptr->stages++;

	/* initialise with correct defaults */
	p = &q_ptr->stage[q_ptr->stages - 1];
	for (i = 0; i < QI_FOLLOWUP_STAGES; i++) {
		p->next_stage_from_goals[i] = -1; /* no next stages set, end quest by default if nothing specified */
		for (j = 0; j < QI_STAGE_GOALS; j++)
			p->goals_for_stage[i][j] = -1; /* no next stages set, end quest by default if nothing specified */
	}
	for (i = 0; i < QI_STAGE_REWARDS; i++) {
		for (j = 0; j < QI_REWARD_GOALS; j++)
			p->goals_for_reward[i][j] = -1; /* no next stages set, end quest by default if nothing specified */
	}
	p->activate_quest = -1;
	p->change_stage = -1;
	p->timed_ingame_abs = -1;
	p->timed_real = 0;
	if (num == 0) p->accepts = TRUE;
	p->reward = NULL;
	p->goal = NULL;

	/* done, return the new one */
	return &q_ptr->stage[q_ptr->stages - 1];
}

/* Allocate/initialise a quest keyword, or return it if already existing. */
qi_keyword *init_quest_keyword(int q_idx, int num) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_keyword *p;

	/* we already have this existing one */
	if (q_ptr->keywords > num) return &q_ptr->keyword[num];

	/* allocate all missing instances up to the requested index */
	p = (qi_keyword*)realloc(q_ptr->keyword, sizeof(qi_keyword) * (num + 1));
	if (!p) {
		s_printf("init_quest_keyword() Couldn't allocate memory!\n");
		exit(0);
	}
	/* initialise the ADDED memory */
	//memset(p + q_ptr->keywords * sizeof(qi_keyword), 0, (num + 1 - q_ptr->keywords) * sizeof(qi_keyword));
	memset(&p[q_ptr->keywords], 0, (num + 1 - q_ptr->keywords) * sizeof(qi_keyword));

	/* attach it to its parent structure */
	q_ptr->keyword = p;
	q_ptr->keywords = num + 1;

	/* done, return the new, requested one */
	return &q_ptr->keyword[num];
}

/* Allocate/initialise a quest keyword reply, or return it if already existing. */
qi_kwreply *init_quest_kwreply(int q_idx, int num) {
	quest_info *q_ptr = &q_info[q_idx];
	qi_kwreply *p;

	/* we already have this existing one */
	if (q_ptr->kwreplies > num) return &q_ptr->kwreply[num];

	/* allocate all missing instances up to the requested index */
	p = (qi_kwreply*)realloc(q_ptr->kwreply, sizeof(qi_kwreply) * (num + 1));
	if (!p) {
		s_printf("init_quest_kwreply() Couldn't allocate memory!\n");
		exit(0);
	}
	/* initialise the ADDED memory */
	//memset(p + q_ptr->keywords * sizeof(qi_keyword), 0, (num + 1 - q_ptr->keywords) * sizeof(qi_keyword));
	memset(&p[q_ptr->kwreplies], 0, (num + 1 - q_ptr->kwreplies) * sizeof(qi_kwreply));

	/* attach it to its parent structure */
	q_ptr->kwreply = p;
	q_ptr->kwreplies = num + 1;

	/* done, return the new, requested one */
	return &q_ptr->kwreply[num];
}

/* Allocate/initialise a kill goal, or return it if already existing.
   NOTE: This function takes a 'goal' that is 1 higher than the internal goal index!
         That's a hack to allow specifying 'optional' goals in q_info.txt. */
qi_kill *init_quest_kill(int q_idx, int stage, int q_info_goal) {
	qi_goal *q_goal = init_quest_goal(q_idx, stage,q_info_goal);
	qi_kill *p;
	int i;

	/* we already have this existing one */
	if (q_goal->kill) return q_goal->kill;

	/* allocate a new one */
	p = (qi_kill*)malloc(sizeof(qi_kill));
	if (!p) {
		s_printf("init_quest_kill() Couldn't allocate memory!\n");
		exit(0);
	}

	/* attach it to its parent structure */
	q_goal->kill = p;

	/* initialise with defaults */
	for (i = 0; i < 5; i++) {
#if 0
		p->rchar[i] = 255;
		p->rattr[i] = 255;
#else /*default 'any', not 'none' (255)! or checks won't pass if optional line is omitted */
		p->rchar[i] = 254;
		p->rattr[i] = 254;
#endif
	}

	/* done, return the new, requested one */
	return p;
}

/* Allocate/initialise a kill goal, or return it if already existing.
   NOTE: This function takes a 'q_info_goal' that is 1 higher than the internal goal index!
         That's a hack to allow specifying 'optional' goals in q_info.txt. */
qi_retrieve *init_quest_retrieve(int q_idx, int stage, int q_info_goal) {
	qi_goal *q_goal = init_quest_goal(q_idx, stage,q_info_goal);
	qi_retrieve *p;
	int i;

	/* we already have this existing one */
	if (q_goal->retrieve) return q_goal->retrieve;

	/* allocate a new one */
	p = (qi_retrieve*)malloc(sizeof(qi_retrieve));
	if (!p) {
		s_printf("init_quest_retrieve() Couldn't allocate memory!\n");
		exit(0);
	}

	/* attach it to its parent structure */
	q_goal->retrieve = p;

	/* initialise with defaults */
	for (i = 0; i < 5; i++) {
		p->otval[i] = -1; // 'any'
		p->osval[i] = -1; // 'any'
	}
	for (i = 0; i < 5; i++) {
#if 0
		p->opval[i] = 9999;
		p->obpval[i] = 9999;
		p->oattr[i] = 255;
		p->oname1[i] = -3;
		p->oname2[i] = -3;
		p->oname2b[i] = -3;
#else /* default 'any', not 'none' (255)! or checks won't pass if optional line is omitted */
		p->opval[i] = -9999;
		p->obpval[i] = -9999;
		p->oattr[i] = 254;
		p->oname1[i] = -1;
		p->oname2[i] = -1;
		p->oname2b[i] = -1;
#endif
	}

	/* done, return the new, requested one */
	return p;
}

/* Allocate/initialise a kill goal, or return it if already existing.
   NOTE: This function takes a 'q_info_goal' that is 1 higher than the internal goal index!
         That's a hack to allow specifying 'optional' goals in q_info.txt. */
qi_deliver *init_quest_deliver(int q_idx, int stage, int q_info_goal) {
	qi_goal *q_goal = init_quest_goal(q_idx, stage,q_info_goal);
	qi_deliver *p;

	/* we already have this existing one */
	if (q_goal->deliver) return q_goal->deliver;

	/* allocate a new one */
	p = (qi_deliver*)malloc(sizeof(qi_deliver));
	if (!p) {
		s_printf("init_quest_deliver() Couldn't allocate memory!\n");
		exit(0);
	}

	/* attach it to its parent structure */
	q_goal->deliver = p;

	/* default: not a return-to-questor goal */
	p->return_to_questor = 255;

	/* done, return the new, requested one */
	return p;
}

/* Allocate/initialise a kill goal, or return it if already existing.
   NOTE: This function takes a 'q_info_goal' that is 1 higher than the internal goal index!
         That's a hack to allow specifying 'optional' goals in q_info.txt. */
qi_goal *init_quest_goal(int q_idx, int stage, int q_info_goal) {
	qi_stage *q_stage = init_quest_stage(q_idx, stage);
	qi_goal *p;
	bool opt = FALSE;

	/* hack: negative number means optional goal.
	   Once a goal is initialised as 'optional' in here,
	   it will stay optional, even if it's referred to by positive index after that.
	   NOTE: Goal 0 can obviously never be optional^^ but you don't have to use it! */
	if (q_info_goal < 0) {
		q_info_goal = -q_info_goal;
		opt = TRUE;
	}
	q_info_goal--; /* unhack it to represent internal goal index now */

	/* we already have this existing one */
	if (q_stage->goals > q_info_goal) return &q_stage->goal[q_info_goal];

	/* allocate all missing instances up to the requested index */
	p = (qi_goal*)realloc(q_stage->goal, sizeof(qi_goal) * (q_info_goal + 1));
	if (!p) {
		s_printf("init_quest_goal() Couldn't allocate memory!\n");
		exit(0);
	}
	/* initialise the ADDED memory */
	//memset(p + q_stage->goals * sizeof(qi_goal), 0, (q_info_goal + 1 - q_stage->goals) * sizeof(qi_goal));
	memset(&p[q_stage->goals], 0, (q_info_goal + 1 - q_stage->goals) * sizeof(qi_goal));

	/* attach it to its parent structure */
	q_stage->goal = p;
	q_stage->goals = q_info_goal + 1;

	/* initialise with correct defaults (paranoia) */
	p = &q_stage->goal[q_stage->goals - 1];
	p->kill = NULL;
	p->retrieve = NULL;
	p->deliver = NULL;
	p->optional = opt; /* permanent forever! */

	/* done, return the new, requested one */
	return &q_stage->goal[q_info_goal];
}

/* Allocate/initialise a quest reward, or return it if already existing. */
qi_reward *init_quest_reward(int q_idx, int stage, int num) {
	qi_stage *q_stage = init_quest_stage(q_idx, stage);
	qi_reward *p;

	/* we already have this existing one */
	if (q_stage->rewards > num) return &q_stage->reward[num];

	/* allocate all missing instances up to the requested index */
	p = (qi_reward*)realloc(q_stage->reward, sizeof(qi_reward) * (num + 1));
	if (!p) {
		s_printf("init_quest_reward() Couldn't allocate memory!\n");
		exit(0);
	}
	/* initialise the ADDED memory */
	//memset(p + q_stage->rewards * sizeof(qi_reward), 0, (num + 1 - q_stage->rewards) * sizeof(qi_reward));
	memset(&p[q_stage->rewards], 0, (num + 1 - q_stage->rewards) * sizeof(qi_reward));

	/* attach it to its parent structure */
	q_stage->reward = p;
	q_stage->rewards = num + 1;

	/* done, return the new, requested one */
	return &q_stage->reward[num];
}

/* Allocate/initialise a custom quest item, or return it if already existing. */
qi_questitem *init_quest_questitem(int q_idx, int stage, int num) {
	qi_stage *q_stage = init_quest_stage(q_idx, stage);
	qi_questitem *p;

	/* we already have this existing one */
	if (q_stage->qitems > num) return &q_stage->qitem[num];

	/* allocate a new one */
	p = (qi_questitem*)realloc(q_stage->qitem, sizeof(qi_questitem) * (q_stage->qitems + 1));
	if (!p) {
		s_printf("init_quest_questitem() Couldn't allocate memory!\n");
		exit(0);
	}
	/* initialise the ADDED memory */
	memset(&p[q_stage->qitems], 0, sizeof(qi_questitem));

	/* set defaults */
	p->questor_gives = 255; /* 255 = disabled */
	p->q_loc.tpref = NULL;

	/* attach it to its parent structure */
	q_stage->qitem = p;
	q_stage->qitems++;

	/* done, return the new one */
	return &q_stage->qitem[q_stage->qitems - 1];
}

/* To call after server has been loaded up, to reattach questors and their quests,
   index-wise. The indices get shuffled over server restart. */
void fix_questors_on_startup(void) {
	quest_info *q_ptr;
	monster_type *m_ptr;
	object_type *o_ptr;
	int i;

	for (i = 1; i < m_max; i++) {
		m_ptr = &m_list[i];
		if (!m_ptr->questor) continue;

		q_ptr = &q_info[m_ptr->quest];
		if (!q_ptr->defined || /* this quest no longer exists in q_info.txt? */
		    !q_ptr->active || /* or it's not supposed to be enabled atm? */
		    q_ptr->questors <= m_ptr->questor_idx) { /* ew */
			s_printf("QUESTOR DEPRECATED (on load) m_idx %d, q_idx %d.\n", i, m_ptr->quest);
			m_ptr->questor = FALSE;
			/* delete him too, maybe? */
		} else q_ptr->questor[m_ptr->questor_idx].mo_idx = i;
	}

	for (i = 1; i < o_max; i++) {
		o_ptr = &o_list[i];
		if (!o_ptr->questor) continue;

		q_ptr = &q_info[o_ptr->quest];
		if (!q_ptr->defined || /* this quest no longer exists in q_info.txt? */
		    !q_ptr->active || /* or it's not supposed to be enabled atm? */
		    q_ptr->questors <= o_ptr->questor_idx) { /* ew */
			s_printf("QUESTOR DEPRECATED (on load) o_idx %d, q_idx %d.\n", i, o_ptr->quest);
			o_ptr->questor = FALSE;
			/* delete him too, maybe? */
		} else q_ptr->questor[o_ptr->questor_idx].mo_idx = i;
	}
}

/* ---------- Questor actions/reactions to 'external' effects in the game world ---------- */

/* Questor was killed and drops any loot? */
void questor_drop_specific(int Ind, struct worldpos *wpos, int x, int y) {
}

/* Questor was killed (npc) or destroyed (object). Quest progression/fail effect? */
void questor_death(int Ind, int mo_idx) {
}
