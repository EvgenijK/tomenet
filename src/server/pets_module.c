#define SERVER

#include "angband.h"

/*
 * Plan:
 * 
 * 1. Pet creation
 * 1.1 placing
 * 1.2 making pet "monster"
 * 2. Pet destruction
 * 2.1 pet removal
 * 2.2 pet death
 * 2.3 other pet disapperance occasions
 * 3. Pet behavior
 * 3.1 moving with player
 * 3.2 attaking hostile monsters/players
 * 
 * pet saving (load2.c)
 *  - "logout" pet with player, spawn next to him when login back
 *  - player can "stable" pets in his houses, on that case they didnt "logout", but stay in the world (are they "holding" the floor then?)
 * 
 * - update slash command "/pet"  -> "/pet <moster_id>"
 *
 */

/* Pet creation */
static int pet_creation(int owner_ind, int r_idx, struct worldpos *wpos, struct cavespot cave_position);
static bool canPlayerSummonPet(int Ind);
static cavespot get_position_near_player(int Ind);
static int make_pet_from_monster(struct worldpos *wpos, struct cavespot cave_position, int owner_ind);
static bool link_pet_to_player(int Ind, int m_idx);
static bool can_place_pet(cave_type **zcave, struct cavespot cave_position);

int summon_pet_on_player(int Ind, int r_idx) {
    return pet_creation(Ind, r_idx, &(Players[Ind]->wpos), get_position_near_player(Ind));
}

// some functions ideas for future use
// void summon_pet_on_target(int Ind, int r_idx) {
//     pet_creation(Ind, r_idx);
// }
// void summon_pet_on_monster(int Ind, int r_idx) {
//     pet_creation(Ind, r_idx);
// }
// void summon_pet_on_other_player(int Ind, int r_idx) {
//     pet_creation(Ind, r_idx);
// }
// void summon_pet_on_place(int Ind, int r_idx) {
//     pet_creation(Ind, r_idx);
// }

/* Add some loop for checking near tiles */
static cavespot get_position_near_player(int Ind) {
    struct cavespot cave_position;
    cave_position.y = Players[Ind]->py;
    cave_position.x = Players[Ind]->px + 1;  /* E of player */ 

    return cave_position;
}

static int pet_creation(int owner_ind, int r_idx, struct worldpos *wpos, struct cavespot cave_position) {
	int m_idx = 0;
    int x = cave_position.x;
    int y = cave_position.y;

    if (! canPlayerSummonPet(owner_ind)) return(0);

    summon_override_checks = SO_ALL; /* needed? */
    if (! place_monster_one(wpos, y, x, r_idx, 0, 0, 0, 0, 0)) {
		s_printf("Something went wrong, you couldn't summon a pet!");
	}
    summon_override_checks = SO_NONE;



	m_idx = make_pet_from_monster(wpos, cave_position, owner_ind);


    if (!m_idx) {
		s_printf("Something went wrong, you couldn't summon a pet!");
		return(0);
	}


    s_printf("New pet created!");

    return m_idx;
}

static int make_pet_from_monster(struct worldpos *wpos, struct cavespot cave_position, int owner_ind) {
    monster_type *m_ptr;
	cave_type **zcave;
	int temp;
	int m_idx = 0;
	int x = cave_position.x;
    int y = cave_position.y;

	if (!(zcave = getcave(wpos))) return(0);


	/* trying to get newly created monster */
	if ((temp = zcave[y][x].m_idx) <= 0) {
		s_printf("You faild to take control on summoned monster, it's wild now!");
		return 0;
	}
	m_ptr = &m_list[temp];

    /* special pet value */
    link_pet_to_player(owner_ind, m_idx);
    m_ptr->pet = 1;
    m_ptr->mind = PET_NONE;
	m_ptr->r_ptr->flags8 |= RF8_ALLOW_RUNNING;

    /* Update the monster */
    update_mon(m_idx, TRUE);

    return m_idx;    
}

/* Links pet monster to the player */
static bool link_pet_to_player(int owner_ind, int m_idx) {
    monster_type *m_ptr = &m_list[m_idx];

    m_ptr->owner = Players[owner_ind]->id;

    // TODO - add list of pets in player and add pet in that list here

    return(1);
}

/* 
 * Put here all checks
 * TODO - implement pets quantity limits 
 */
static bool canPlayerSummonPet(int Ind) {
//    if (!Players[Ind]->has_pet) return FALSE:
    return TRUE;
}

static bool can_place_pet(cave_type **zcave, struct cavespot cave_position) {
    int x = cave_position.x;
    int y = cave_position.y;

    /* Verify location */
    if (!in_bounds(y, x)) return(0);

    /* Require empty space */
    if (!cave_empty_bold(zcave, y, x)) return(0);
    
    /* Hack -- no creation on glyph of warding */
    if (zcave[y][x].feat == FEAT_GLYPH) return(0);
    if (zcave[y][x].feat == FEAT_RUNE) return(0);

    return 1;
}


/* Pet destruction */    

bool remove_pets(int Ind);
 
bool unsummon_pets(int Ind) {
    return remove_pets(Ind);
}

/* 
 * Remove all player pets
 * TODO - make use of player_type->pets list for speed up
 * 
 */
bool remove_pets(int Ind) {
    int i, j;
    player_type *p_ptr = Players[Ind];
    monster_type *m_ptr;

    //   if (!Players[Ind]->has_pet) return (0); /* Potential improvement (more memory needed though) */

    /* Process the monsters */
    for (i = m_top - 1; i >= 0; i--) {
        /* Access the index */
        j = m_fast[i];

        /* Access the monster */
        m_ptr = &m_list[j];

        /* Excise "dead" monsters */
        if (!m_ptr->r_idx) continue;

        if (m_ptr->owner != Players[Ind]->id) continue;

        delete_monster_idx(j, FALSE);
        p_ptr->has_pet = 0;
    }
    return TRUE;
}



/* Pet behavior */

/*
 * Choose "logical" directions for pet movement
 * Returns TRUE to move, FALSE to stand still
 */
static bool get_moves_pet(int Ind, int m_idx, int *mm) {
    player_type *p_ptr;
    monster_type *m_ptr = &m_list[m_idx];

    int y, ay, x, ax;
    int move_val = 0;

    int tm_idx = 0;
    int y2, x2;

    if (Ind > 0) p_ptr = Players[Ind];
    else p_ptr = NULL;

    /* Lets find a target */
    if ((p_ptr != NULL) && (m_ptr->mind & PET_ATTACK) && TARGET_BEING(p_ptr->target_who) && (p_ptr->target_who > 0 || check_hostile(Ind, -p_ptr->target_who)))
        tm_idx = p_ptr->target_who;
    else { // if (m_ptr->mind & PET_GUARD)
        int sx, sy;
        s32b max_hp = 0;

        /* Scan grids around */
        for (sx = m_ptr->fx - 1; sx <= m_ptr->fx + 1; sx++)
            for (sy = m_ptr->fy - 1; sy <= m_ptr->fy + 1; sy++) {
                cave_type *c_ptr, **zcave;

                if (!in_bounds(sy, sx)) continue;

                /* ignore ourself */
                if (sx == m_ptr->fx && sy == m_ptr->fy) continue;

                /* no point if there are no players on depth */
                /* and it would crash anyway ;) */

                if (!(zcave = getcave(&m_ptr->wpos))) return(FALSE);
                c_ptr = &zcave[sy][sx];

                if (!c_ptr->m_idx) continue;

                if (c_ptr->m_idx > 0) {
                    if (max_hp < m_list[c_ptr->m_idx].maxhp) {
                        max_hp = m_list[c_ptr->m_idx].maxhp;
                        tm_idx = c_ptr->m_idx;
                    }
                } else {
                    if ((max_hp < Players[-c_ptr->m_idx]->mhp) && (m_ptr->owner != Players[-c_ptr->m_idx]->id)) {
                        max_hp = Players[-c_ptr->m_idx]->mhp;
                        tm_idx = c_ptr->m_idx;
                    }
                }
            }
    }
    /* Nothing else to do ? */
    if ((p_ptr != NULL) && !tm_idx && (m_ptr->mind & PET_FOLLOW))
        tm_idx = -Ind;

    if (!tm_idx) return(FALSE);

    if (!(inarea(&m_ptr->wpos, (tm_idx > 0) ? &m_list[tm_idx].wpos : &Players[-tm_idx]->wpos))) return(FALSE);

    y2 = (tm_idx > 0)?m_list[tm_idx].fy:Players[-tm_idx]->py;
    x2 = (tm_idx > 0)?m_list[tm_idx].fx:Players[-tm_idx]->px;

    /* Extract the "pseudo-direction" */
    y = m_ptr->fy - y2;
    x = m_ptr->fx - x2;

    /* Extract the "absolute distances" */
    ax = ABS(x);
    ay = ABS(y);

    /* Do something weird */
    if (y < 0) move_val += 8;
    if (x > 0) move_val += 4;

    /* Prevent the diamond maneuvre */
    if (ay > (ax << 1)) {
        move_val++;
        move_val++;
    } else if (ax > (ay << 1))
        move_val++;

    /* Extract some directions */
    switch (move_val) {
        case 0:
            mm[0] = 9;
            if (ay > ax) {
                mm[1] = 8;
                mm[2] = 6;
                mm[3] = 7;
                mm[4] = 3;
            } else {
                mm[1] = 6;
                mm[2] = 8;
                mm[3] = 3;
                mm[4] = 7;
            }
            break;
        case 1:
        case 9:
            mm[0] = 6;
            if (y < 0) {
                mm[1] = 3;
                mm[2] = 9;
                mm[3] = 2;
                mm[4] = 8;
            } else {
                mm[1] = 9;
                mm[2] = 3;
                mm[3] = 8;
                mm[4] = 2;
            }
            break;
        case 2:
        case 6:
            mm[0] = 8;
            if (x < 0) {
                mm[1] = 9;
                mm[2] = 7;
                mm[3] = 6;
                mm[4] = 4;
            } else {
                mm[1] = 7;
                mm[2] = 9;
                mm[3] = 4;
                mm[4] = 6;
            }
            break;
        case 4:
            mm[0] = 7;
            if (ay > ax) {
                mm[1] = 8;
                mm[2] = 4;
                mm[3] = 9;
                mm[4] = 1;
            } else {
                mm[1] = 4;
                mm[2] = 8;
                mm[3] = 1;
                mm[4] = 9;
            }
            break;
        case 5:
        case 13:
            mm[0] = 4;
            if (y < 0) {
                mm[1] = 1;
                mm[2] = 7;
                mm[3] = 2;
                mm[4] = 8;
            } else {
                mm[1] = 7;
                mm[2] = 1;
                mm[3] = 8;
                mm[4] = 2;
            }
            break;
        case 8:
            mm[0] = 3;
            if (ay > ax) {
                mm[1] = 2;
                mm[2] = 6;
                mm[3] = 1;
                mm[4] = 9;
            } else {
                mm[1] = 6;
                mm[2] = 2;
                mm[3] = 9;
                mm[4] = 1;
            }
            break;
        case 10:
        case 14:
            mm[0] = 2;
            if (x < 0) {
                mm[1] = 3;
                mm[2] = 1;
                mm[3] = 6;
                mm[4] = 4;
            } else {
                mm[1] = 1;
                mm[2] = 3;
                mm[3] = 4;
                mm[4] = 6;
            }
            break;
        case 12:
            mm[0] = 1;
            if (ay > ax) {
                mm[1] = 2;
                mm[2] = 4;
                mm[3] = 3;
                mm[4] = 7;
            } else {
                mm[1] = 4;
                mm[2] = 2;
                mm[3] = 7;
                mm[4] = 3;
            }
            break;
    }

    return(TRUE);
}


/* the pet handler. note that at the moment it _may_ be almost
 * identical to the golem's handler, except for some little
 * stuff. but let's NOT merge the two and add pet check hacks to
 * the golem handler. (i have plans for the pet system)
 * - the_sandman
 */
void process_monster_pet(int Ind, int m_idx) {
	//player_type *p_ptr;
	monster_type	*m_ptr = &m_list[m_idx];
	monster_race    *r_ptr = race_inf(m_ptr);
	struct worldpos *wpos = &m_ptr->wpos;

	int			i, d, oy, ox, ny, nx;

	int			mm[8];

	cave_type	*c_ptr;
	//object_type	*o_ptr;
	monster_type	*y_ptr;

	bool		do_turn;
	bool		do_move;
	//bool		do_view;

	bool		did_open_door;
	bool		did_bash_door;
#ifdef OLD_MONSTER_LORE
	bool		did_take_item;
	bool		did_kill_item;
	bool		did_move_body;
	bool		did_kill_body;
	bool		did_pass_wall;
	bool		did_kill_wall;
#endif

	/* hack -- don't process monsters on wilderness levels that have not
	 * been regenerated yet.
	 */
	cave_type **zcave;

	if (!(zcave = getcave(wpos))) return;

#if 0
	if (Ind > 0) p_ptr = Players[Ind];
	else p_ptr = NULL;
#endif

	/* handle "stun" */
	if (m_ptr->stunned) {
		int d = 1;

		/* make a "saving throw" against stun */
		if (rand_int(5000) <= r_ptr->level * r_ptr->level) {
			/* recover fully */
			d = m_ptr->stunned;
		}

		/* hack -- recover from stun */
		if (m_ptr->stunned > d) {
			/* recover somewhat */
			m_ptr->stunned -= d;
		}
		/* fully recover */
		else {
			/* recover fully */
			m_ptr->stunned = 0;
		}

		/* still stunned */
		if (m_ptr->stunned)  {
			m_ptr->energy -= level_speed(&m_ptr->wpos);
			return;
		}
	}

	/* handle confusion */
	if (m_ptr->confused && !(r_ptr->flags3 & RF3_NO_CONF)) {
		/* amount of "boldness" */
		int d = randint(r_ptr->level / 10 + 1);

		/* still confused */
		if (m_ptr->confused > d) {
			/* reduce the confusion */
			m_ptr->confused -= d;
		}
		/* recovered */
		else {
			/* no longer confused */
			m_ptr->confused = 0;
		}
	}

	/* get the origin */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* hack -- assume no movement */
	mm[0] = mm[1] = mm[2] = mm[3] = 0;
	mm[4] = mm[5] = mm[6] = mm[7] = 0;


	/* confused -- 100% random */
	if (m_ptr->confused) {
		/* try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}
	/* normal movement */
	else {
		/* logical moves */
		if (!get_moves_pet(Ind, m_idx, mm)) return;
	}


	/* assume nothing */
	do_turn = FALSE;
	do_move = FALSE;
	//do_view = FALSE;

	/* assume nothing */
	did_open_door = FALSE;
	did_bash_door = FALSE;
#ifdef OLD_MONSTER_LORE
	did_take_item = FALSE;
	did_kill_item = FALSE;
	did_move_body = FALSE;
	did_kill_body = FALSE;
	did_pass_wall = FALSE;
	did_kill_wall = FALSE;
#endif

	/* take a zero-terminated array of "directions" */
	for (i = 0; mm[i]; i++) {
		/* get the direction */
		d = mm[i];

		/* hack -- allow "randomized" motion */
		if (d == 5) d = ddd[rand_int(8)];

		/* get the destination */
		ny = oy + ddy[d];
		nx = ox + ddx[d];

		/* access that cave grid */
		c_ptr = &zcave[ny][nx];

		/* access that cave grid's contents */
		//o_ptr = &o_list[c_ptr->o_idx];

		/* access that cave grid's contents */
		y_ptr = &m_list[c_ptr->m_idx];

		/* Tavern entrance? */
		if (c_ptr->feat == FEAT_SHOP)
			do_move = TRUE;
#if 0
		/* Floor is open? */
		else if (cave_floor_bold(zcave, ny, nx)) {
			/* Go ahead and move */
			do_move = TRUE;
		}
#else
		/* Floor is open? */
		else if (creature_can_enter(r_ptr, c_ptr)) {
			/* Go ahead and move */
			do_move = TRUE;
		}
#endif
		/* Player ghost in wall XXX */
		else if (c_ptr->m_idx < 0) {
			/* Move into player */
			do_move = TRUE;
		}
		/* Permanent wall / permanently wanted structure */
		else if (((f_info[c_ptr->feat].flags1 & FF1_PERMANENT) ||
			(f_info[c_ptr->feat].flags2 & FF2_BOUNDARY) ||
			(c_ptr->feat == FEAT_WALL_HOUSE) ||
			(c_ptr->feat == FEAT_HOME_HEAD) ||
			(c_ptr->feat == FEAT_HOME_TAIL) ||
			(c_ptr->feat == FEAT_HOME_OPEN) ||
			(c_ptr->feat == FEAT_HOME) ||
			(c_ptr->feat == FEAT_ALTAR_HEAD) ||
			(c_ptr->feat == FEAT_ALTAR_TAIL))
			&& !(
			(c_ptr->feat == FEAT_TREE) ||
			(c_ptr->feat == FEAT_BUSH) ||
			(c_ptr->feat == FEAT_DEAD_TREE)))
		{
			/* Nothing */
		}
		else if ( (c_ptr->feat == FEAT_TREE) ||
			(c_ptr->feat == FEAT_BUSH) ||
			(c_ptr->feat == FEAT_DEAD_TREE)) {
			if (r_ptr->flags7 & RF7_CAN_FLY)
				do_move = TRUE;
		}
		/* Monster moves through walls (and doors) */
		else if (r_ptr->flags2 & RF2_PASS_WALL) {
			/* Pass through walls/doors/rubble */
			do_move = TRUE;

#ifdef OLD_MONSTER_LORE
			/* Monster went through a wall */
			did_pass_wall = TRUE;
#endif
		}
		/* Monster destroys walls (and doors) */
		else if (r_ptr->flags2 & RF2_KILL_WALL) {
			/* Eat through walls/doors/rubble */
			do_move = TRUE;

#ifdef OLD_MONSTER_LORE
			/* Monster destroyed a wall */
			did_kill_wall = TRUE;
#endif

			/* Create floor */
			cave_set_feat_live(wpos, ny, nx, twall_erosion(wpos, ny, nx, FEAT_FLOOR));

			/* Forget the "field mark", if any */
			everyone_forget_spot(wpos, ny, nx);

			/* Notice */
			note_spot_depth(wpos, ny, nx);

			/* Redraw */
			everyone_lite_spot(wpos, ny, nx);

			/* Note changes to viewable region */
			//if (player_has_los_bold(Ind, ny, nx)) do_view = TRUE;
		}
		/* Handle doors and secret doors */
		else if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			  (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
			 (c_ptr->feat == FEAT_SECRET))
		{
			bool may_bash = TRUE;

			/* Take a turn */
			do_turn = TRUE;

			/* Creature can open doors. */
			if (r_ptr->flags2 & RF2_OPEN_DOOR) {
				/* Closed doors and secret doors */
				if ((c_ptr->feat == FEAT_DOOR_HEAD) ||
				    (c_ptr->feat == FEAT_SECRET))
				{
					/* The door is open */
					did_open_door = TRUE;

					/* Do not bash the door */
					may_bash = FALSE;
				}

				/* Locked doors (not jammed) */
				else if (c_ptr->feat < FEAT_DOOR_HEAD + 0x08) {
					int k;

					/* Door power */
					k = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

#if 0
					/* XXX XXX XXX XXX Old test (pval 10 to 20) */
					if (randint((m_ptr->hp + 1) * (50 + o_ptr->pval)) <
					    40 * (m_ptr->hp - 10 - o_ptr->pval));
#endif

					/* Try to unlock it XXX XXX XXX */
					if (rand_int(m_ptr->hp / 10) > k)
					{
						/* Unlock the door */
						c_ptr->feat = FEAT_DOOR_HEAD + 0x00;

						/* Do not bash the door */
						may_bash = FALSE;
					}
				}
			}

			/* Stuck doors -- attempt to bash them down if allowed */
			if (may_bash && (r_ptr->flags2 & RF2_BASH_DOOR)) {
				int k;

				/* Door power */
				k = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

#if 0
				/* XXX XXX XXX XXX Old test (pval 10 to 20) */
				if (randint((m_ptr->hp + 1) * (50 + o_ptr->pval)) <
				    40 * (m_ptr->hp - 10 - o_ptr->pval));
#endif

				/* Attempt to Bash XXX XXX XXX */
				if (rand_int(m_ptr->hp / 10) > k) {
					/* The door was bashed open */
					did_bash_door = TRUE;

					/* Hack -- fall into doorway */
					do_move = TRUE;
				}
			}

			/* Deal with doors in the way */
			if (did_open_door || did_bash_door) {
				/* Secret door */
				if (c_ptr->feat == FEAT_SECRET) {
					struct c_special *cs_ptr;

					/* Clear mimic feature */
					if ((cs_ptr = GetCS(c_ptr, CS_MIMIC)))
						cs_erase(c_ptr, cs_ptr);
				}

				/* Break down the door */
				if (did_bash_door && magik(DOOR_BASH_BREAKAGE))
					c_ptr->feat = FEAT_BROKEN;
				/* Open the door */
				else
					c_ptr->feat = FEAT_OPEN;

				/* notice */
				note_spot_depth(wpos, ny, nx);

				/* redraw */
				everyone_lite_spot(wpos, ny, nx);
			}
		}

		if (!find_player(m_ptr->owner)) return; //hack: the owner must be online please. taking this out -> panic()
		/* the player is in the way.  attack him. */
		if (do_move && (c_ptr->m_idx < 0) && (c_ptr->m_idx >= -NumPlayers)) {
			player_type *p_ptr = Players[0-c_ptr->m_idx];

			/* sanity check */
			if (p_ptr->id != m_ptr->owner &&
			   (find_player(m_ptr->owner) == 0 ||
			    find_player(-c_ptr->m_idx) == 0)) {
				do_move = FALSE;
				do_turn = FALSE;
			}
			/* do the attack only if hostile... */
			if (p_ptr->id != m_ptr->owner &&
			    check_hostile(0-c_ptr->m_idx, find_player(m_ptr->owner))) {
				(void)make_attack_melee(0 - c_ptr->m_idx, m_idx);
				do_move = FALSE;
				do_turn = TRUE;
			} else {
				if (m_ptr->owner != p_ptr->id) {
					if (magik(10))
						msg_format(find_player(m_ptr->owner),
						 "\377gYour pet is staring at %s.",
						 p_ptr->name);
					if (magik(10)) {
						player_type *q_ptr = Players[find_player(m_ptr->owner)];

						switch (q_ptr->name[strlen(q_ptr->name) - 1]) {
						case 's': case 'x': case 'z':
							msg_format(-c_ptr->m_idx, "\377g%s' pet is staring at you.", q_ptr->name);
							break;
						default:
							msg_format(-c_ptr->m_idx, "\377g%s's pet is staring at you.", q_ptr->name);
						}
					}
				}
				do_move = FALSE;
				do_turn = TRUE;
			}
		}
		/* a monster is in the way */
		else if (do_move && c_ptr->m_idx > 0) {
			/* attack it ! */
			if (m_ptr->owner != y_ptr->owner || (y_ptr->owner && check_hostile(find_player(m_ptr->owner), find_player(y_ptr->owner))))
			//if (m_ptr->owner != y_ptr->owner || !(y_ptr->pet && y_ptr->owner && !check_hostile(find_player(m_ptr->owner), find_player(y_ptr->owner))))
				monster_attack_normal(c_ptr->m_idx, m_idx);

			/* assume no movement */
			do_move = FALSE;
			/* take a turn */
			do_turn = TRUE;
		}


		/* creature has been allowed move */
		if (do_move) {
			/* take a turn */
			do_turn = TRUE;

			/* hack -- update the old location */
			zcave[oy][ox].m_idx = c_ptr->m_idx;

			/* mega-hack -- move the old monster, if any */
			if (c_ptr->m_idx > 0) {
				/* move the old monster */
				y_ptr->fy = oy;
				y_ptr->fx = ox;

				/* update the old monster */
				update_mon(c_ptr->m_idx, TRUE);
			}

			/* hack -- update the new location */
			c_ptr->m_idx = m_idx;

			/* move the monster */
			m_ptr->fy = ny;
			m_ptr->fx = nx;

			/* update the monster */
			update_mon(m_idx, TRUE);
cave_midx_debug(wpos, oy, ox, c_ptr->m_idx); //DEBUG
			/* redraw the old grid */
			everyone_lite_spot(wpos, oy, ox);

			/* redraw the new grid */
			everyone_lite_spot(wpos, ny, nx);
		}

		/* stop when done */
		if (do_turn) {
			m_ptr->energy -= level_speed(&m_ptr->wpos);
			break;
		}
	}

#if 0
	/* Notice changes in view */
	if (do_view) {
		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
	}
#endif

	/* hack -- get "bold" if out of options */
	if (!do_turn && !do_move && m_ptr->monfear) {
		/* no longer afraid */
		m_ptr->monfear = 0;
		//m_ptr->monfear_gone = 1;
	}
}


/* Some helpers */
static monster_type *get_m_ptr_fast(int m_idx) {
    return &m_list[m_fast[m_idx]];
}


void toggle_pet_mind(int Ind, monster_type *m_ptr, byte pet_mind) {  
    if (!m_ptr->pet) return;

    switch (pet_mind) {
    case PET_ATTACK:
        if (m_ptr->mind & pet_mind) {
            msg_print(Ind, "Your pets stop going for your target.");
            m_ptr->mind &= ~pet_mind;
        } else {
            msg_print(Ind, "Your pets approach your target!");
            m_ptr->mind |= pet_mind;
        }
    break;
    case PET_GUARD:
        if (m_ptr->mind & pet_mind) {
            m_ptr->mind &= ~pet_mind;
            msg_print(Ind, "Your pets stop following you around.");
        } else {
            msg_print(Ind, "Your pets start following you around!");
            m_ptr->mind |= pet_mind;
        }
    break;
    case PET_FOLLOW:
        if (m_ptr->mind & pet_mind) {
            m_ptr->mind &= ~pet_mind;
            msg_print(Ind, "Your pets stop being on guard.");
        } else {
            msg_print(Ind, "Your pets seem to be on guard now!");
            m_ptr->mind |= pet_mind;
        }
    break;
    }
}

void toggle_all_pets_mind(int Ind, byte pet_mind) {
    int i;
    monster_type *m_ptr;

    /* Process the monsters */
    for (i = m_top - 1; i >= 0; i--) {
        /* Access the monster */
        m_ptr = get_m_ptr_fast(i);

        /* Excise "dead" monsters */
        if (!m_ptr->r_idx) continue;

        if (m_ptr->owner != Players[Ind]->id) continue;

        toggle_pet_mind(Ind, m_ptr, pet_mind);
    }
}
