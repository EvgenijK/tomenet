#define SERVER

#include "angband.h"

/*
 * Plan:
 * 
 * 1. +Pet creation
 * 1.1 + placing
 * 1.2 + making pet "monster"
 * 2. +Pet destruction
 * 2.1 +pet removal
 * 2.2 +pet death
 * 2.3 + other pet disapperance occasions
 * 3. Pet behavior
 * 3.1 +moving with player
 * 3.2 +attaking hostile monsters/players
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
static cavespot get_position_near_player(int Ind);
static int make_pet_from_monster(struct worldpos *wpos, struct cavespot cave_position, int owner_ind);
static void link_pet_to_player(int Ind, int m_idx);
static bool can_player_have_more_pets(int Ind);

int summon_pet_on_player(int Ind, int r_idx) {
    return pet_creation(Ind, r_idx, &(Players[Ind]->wpos), get_position_near_player(Ind));
}

static cavespot get_position_near_player(int Ind) {
    struct cavespot cave_position;
    cave_type **zcave = getcave(&(Players[Ind]->wpos));

    int max_distance = 9;
    int max_tries = 20;
    int x;
    int y;

    for (int distance = 1; distance < max_distance; ++distance) {
        x = Players[Ind]->py;
        y = Players[Ind]->px;

        for (int tries = 0; tries < max_tries; ++tries) {
            scatter(&(Players[Ind]->wpos), &y, &x, Players[Ind]->py, Players[Ind]->px, distance, TRUE);

            if (cave_empty_bold(zcave, y, x)) {
                break;
            }
        }
        if (cave_empty_bold(zcave, y, x)) {
            break;
        }
    }
    cave_position.y = y;
    cave_position.x = x;

    return cave_position;
}

static int pet_creation(int owner_ind, int r_idx, struct worldpos *wpos, struct cavespot cave_position) {
	int m_idx;
    int x = cave_position.x;
    int y = cave_position.y;

    summon_override_checks = SO_ALL; /* needed? */
    int monster_placed = ! place_monster_one(wpos, y, x, r_idx, 0, 0, 0, 0, 0);
    summon_override_checks = SO_NONE;

    if (! monster_placed) {
        msg_format(owner_ind, "\377RYour summon failed!");
        return(0);
    }

	m_idx = make_pet_from_monster(wpos, cave_position, owner_ind);

    if (!m_idx) {
		return(0);
	}

    msg_format(owner_ind, "\377GYou summoned %s", r_name_get(&m_list[m_idx]));

    return m_idx;
}

static int make_pet_from_monster(struct worldpos *wpos, struct cavespot cave_position, int owner_ind) {
    monster_type *m_ptr;
	cave_type **zcave;
	int m_idx = 0;
	int x = cave_position.x;
    int y = cave_position.y;

	if (!(zcave = getcave(wpos))) return(0);

    /* trying to get newly created monster */
    if ((m_idx = zcave[y][x].m_idx) <= 0) {
        msg_format(owner_ind, "\377RYou failed to take control on summoned monster, it's wild now!");
        s_printf("PET_DEBUG: couldn't find newly created monster by zcave[y][x].m_idx");
        return 0;
    }
    m_ptr = &m_list[m_idx];

    /* Hack: create new race (scrapped from golem creation)  */
    MAKE(m_ptr->r_ptr, monster_race);
    /* Copy old race to the new one */
    *(m_ptr->r_ptr) = r_info[m_ptr->r_idx];

    s_printf("PET_DEBUG: Making new pet from monster: %d", m_idx); // DEBUG

    /* special pet values */
    link_pet_to_player(owner_ind, m_idx);
    m_ptr->mind = PET_FOLLOW;
    m_ptr->r_ptr->flags8 |= RF8_ALLOW_RUNNING;

    s_printf("PET_DEBUG: Making new pet completed! m_idx: %d, player pets_count: %d, monster pet value: %d", 
        m_idx, Players[owner_ind]->pets_count, m_ptr->pet); // DEBUG

    /* Update the monster */
    update_mon(m_idx, TRUE);

    msg_format(owner_ind, "\377GYou summon %s", r_name_get(m_ptr));

    return m_idx;
}

/* Aka "tame" a monster */
int make_pet_from_wild_monster(int m_idx, int owner_ind) {
    monster_type *m_ptr;

    m_ptr = &m_list[m_idx];


    if (!m_ptr) {
        msg_format(owner_ind, "\377RYou failed to tame a monster!");
        s_printf("PET_DEBUG: couldn't find monster by m_idx for taming");
        return 0;
    }

    /* Hack: create new race (scrapped from golem creation)  */
    MAKE(m_ptr->r_ptr, monster_race);
    /* Copy old race to the new one */
    *(m_ptr->r_ptr) = r_info[m_ptr->r_idx];
    /* QUESTION - do i need to do something with old m_ptr->r_ptr after stop using it for that monste?r */

    s_printf("PET_DEBUG: Making new pet from monster: %d", m_idx); // DEBUG

    /* special pet value */
    link_pet_to_player(owner_ind, m_idx);
    m_ptr->mind = PET_FOLLOW;
    m_ptr->r_ptr->flags8 |= RF8_ALLOW_RUNNING;

    s_printf("PET_DEBUG: Making new pet completed! m_idx: %d, player pets_count: %d, monster pet value: %d", 
        m_idx, Players[owner_ind]->pets_count, m_ptr->pet); // DEBUG

    /* Update the monster */
    update_mon(m_idx, TRUE);

    msg_format(owner_ind, "\377GYou tame %s", r_name_get(m_ptr));

    return m_idx;    
}

/* Links pet monster to the player */
static void link_pet_to_player(int owner_ind, int m_idx) {
    monster_type *m_ptr = &m_list[m_idx];
    player_type *p_ptr = Players[owner_ind];

    if (!can_player_have_more_pets(owner_ind)) return;

    m_ptr->owner = p_ptr->id;
    p_ptr->pets[p_ptr->pets_count] = m_idx;
    p_ptr->pets_count++;
    m_ptr->pet = p_ptr->pets_count;
}

static bool can_player_have_more_pets(int Ind) {
    if (Players[Ind]->pets_count >= MAX_PLAYER_PETS) {
        s_printf("PET_DEBUG: maximum pets limit reached for player(Ind): %d", Ind);
        return(FALSE);
    }

    return TRUE;
}

/* Pet destruction */
static bool remove_player_pets(int Ind);
 
bool unsummon_pets(int Ind) {
    return remove_player_pets(Ind);
}

/*
 * Remove all player pets
 *
 */
static bool remove_player_pets(int Ind) {
    player_type *p_ptr = Players[Ind];
    monster_type *m_ptr;

    for (int i = p_ptr->pets_count - 1; i >= 0; i--) {
        if (! p_ptr->pets[i]) { 
            p_ptr->pets_count--; /* Some clearing */
            continue;
        }

        /* Access the monster */
        m_ptr = &m_list[p_ptr->pets[i]];

        /* In case monster lost its m_ptr->pet and/or m_ptr->owner - restore connection */
        if (m_ptr->owner != p_ptr->id) 
            m_ptr->owner = p_ptr->id;

        if (!m_ptr->pet)
            m_ptr->pet = i + 1;

        delete_monster_idx(p_ptr->pets[i], FALSE);
    }

    /* Just to be shure */
    p_ptr->pets_count = 0;

    return TRUE;
}

/* Delete all pets from all players, and also orphans */
void remove_all_pets() {
    int i, j;
    monster_type *m_ptr;

    for (i = 1; i <= NumPlayers; i++) {
        remove_player_pets(i);
    }

    /* Try to find orphan pets, ie without m_ptr->owner or m_ptr->pet */
    for (i = m_top - 1; i >= 0; i--) {
        /* Access the index */
        j = m_fast[i];

        /* Access the monster */
        m_ptr = &m_list[j];

        /* Skip not pets */
        if (!m_ptr->pet && !m_ptr->owner) continue;

        delete_monster_idx(j, FALSE); /* Causing seg fault O_o */
    }
}

void unlink_pet_from_owner(int m_idx) {
    player_type *p_ptr;
    monster_type *m_ptr = &m_list[m_idx];
    int Ind = 0;

    if (!m_ptr->pet) {
        m_ptr->owner = 0;
        return;
    }

    if (!m_ptr->owner) {
        m_ptr->pet = 0;
        return;
    }

    Ind = find_player(m_ptr->owner);
    p_ptr = Players[Ind];

    if (!p_ptr) {
        m_ptr->owner = 0;
        m_ptr->pet = 0;
        return;
    }

    if (m_ptr->pet < p_ptr->pets_count) {
        int m_idx_last_player_pet = p_ptr->pets[p_ptr->pets_count - 1];
        monster_type *m_ptr_last_player_pet = &m_list[m_idx_last_player_pet];
        m_ptr_last_player_pet->pet = m_ptr->pet;
        
        p_ptr->pets[m_ptr->pet - 1] = p_ptr->pets[p_ptr->pets_count - 1];
    }
    
    p_ptr->pets[p_ptr->pets_count - 1] = 0;
    p_ptr->pets_count--;

    if (p_ptr->pets_count < 0) p_ptr->pets_count = 0;


    m_ptr->owner = 0;
    m_ptr->pet = 0;
}

/* Pet behavior */

/*
 * Choose "logical" directions for pet movement
 * Returns TRUE to move, FALSE to stand still
 */
static bool get_moves_pet(int Ind, int m_idx, int *mm) {
    player_type *p_ptr;
    monster_type *m_ptr = &m_list[m_idx];

    int distance_to_target_y, abs_distance_to_target_y, distance_to_target_x, abs_distance_to_target_x;
    int move_val = 0;

    int target_idx = 0;
    int targer_y, target_x;

    if (Ind > 0) p_ptr = Players[Ind];
    else p_ptr = NULL;

    
    if ((m_ptr->mind & PET_ATTACK)
        && (p_ptr != NULL)
        && TARGET_BEING(p_ptr->target_who)
        && (p_ptr->target_who > 0 || check_hostile(Ind, -p_ptr->target_who))
    ) {
        /* attack player target */
        target_idx = p_ptr->target_who;
    } else {
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

                /* we dont need to move into friendly pets */
                int is_hostile_pet = m_list[c_ptr->m_idx].owner 
                        && (m_list[c_ptr->m_idx].owner != m_ptr->owner) 
                        && check_hostile(find_player(m_ptr->owner), find_player(m_list[c_ptr->m_idx].owner));
                        
                int is_same_owner = m_list[c_ptr->m_idx].owner && (m_list[c_ptr->m_idx].owner == m_ptr->owner);
                
                if (is_same_owner || (m_list[c_ptr->m_idx].pet && !is_hostile_pet)) {
                    continue;
                }
                
                if (c_ptr->m_idx > 0) { 
                    if (max_hp < m_list[c_ptr->m_idx].maxhp) {
                        max_hp = m_list[c_ptr->m_idx].maxhp;
                        target_idx = c_ptr->m_idx;
                    }
                } else {
                    if ((max_hp < Players[-c_ptr->m_idx]->mhp) && (m_ptr->owner != Players[-c_ptr->m_idx]->id)) {
                        max_hp = Players[-c_ptr->m_idx]->mhp;
                        target_idx = c_ptr->m_idx;
                    }
                }
            }
    }

    /* suppress moving into target (but range attacks are ok) */
    if (m_ptr->mind & PET_GUARD) {
        target_idx = 0;
    }
    
    /* follow player if there is nothing to do */
    if ((m_ptr->mind & PET_FOLLOW) && !target_idx && (p_ptr != NULL)) {
        target_idx = -Ind;
    }

    if (!target_idx) return(FALSE);

    if (!(inarea(&m_ptr->wpos, (target_idx > 0) ? &m_list[target_idx].wpos : &Players[-target_idx]->wpos))) return(FALSE);

    targer_y = (target_idx > 0) ? m_list[target_idx].fy : Players[-target_idx]->py;
    target_x = (target_idx > 0) ? m_list[target_idx].fx : Players[-target_idx]->px;

    /* Extract the "pseudo-direction" */
    distance_to_target_y = m_ptr->fy - targer_y;
    distance_to_target_x = m_ptr->fx - target_x;

    /* Extract the "absolute distances" */
    abs_distance_to_target_x = ABS( distance_to_target_x );
    abs_distance_to_target_y = ABS( distance_to_target_y );

    /* Do something weird */
    if ( distance_to_target_y < 0) move_val += 8;
    if ( distance_to_target_x > 0) move_val += 4;

    /* Prevent the diamond maneuvre */
    if ( abs_distance_to_target_y > ( abs_distance_to_target_x << 1)) {
        move_val++;
        move_val++;
    } else if ( abs_distance_to_target_x > ( abs_distance_to_target_y << 1))
        move_val++;

    /* Extract some directions */
    switch (move_val) {
        case 0:
            mm[0] = 9;
            if ( abs_distance_to_target_y > abs_distance_to_target_x ) {
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
            if ( distance_to_target_y < 0) {
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
            if ( distance_to_target_x < 0) {
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
            if ( abs_distance_to_target_y > abs_distance_to_target_x ) {
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
            if ( distance_to_target_y < 0) {
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
            if ( abs_distance_to_target_y > abs_distance_to_target_x ) {
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
            if ( distance_to_target_x < 0) {
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
            if ( abs_distance_to_target_y > abs_distance_to_target_x ) {
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

void hande_pet_stun(int m_idx) {
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = race_inf(m_ptr);
    
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
}

void handle_pet_confusion(int m_idx) {
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = race_inf(m_ptr);
    
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

	int			i, direction, origin_y, origin_x, ny, nx;

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

	hande_pet_stun(m_idx);
	handle_pet_confusion(m_idx);

	/* get the origin */
	origin_y = m_ptr->fy;
	origin_x = m_ptr->fx;

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
		direction = mm[i];

		/* hack -- allow "randomized" motion */
		if ( direction == 5) direction = ddd[rand_int(8)];

		/* get the destination */
		ny = origin_y + ddy[direction];
		nx = origin_x + ddx[direction];

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
            monster_race *z_ptr = race_inf(y_ptr);

            /* Hostile(not a pet or a pet of hostile player) - attack */
			if (!y_ptr->pet || (y_ptr->owner && (m_ptr->owner != y_ptr->owner) && check_hostile(find_player(m_ptr->owner), find_player(y_ptr->owner)))
            ) {
                /* attack it ! */
				monster_attack_normal(c_ptr->m_idx, m_idx);
                /* assume no movement */
                do_move = FALSE;
                do_turn = TRUE;
            } else if (/* Push past weaker pets (needed to solve situation of infinite loop of pets swapping each other) */
                y_ptr->pet &&
                (m_ptr->owner == y_ptr->owner || !check_hostile(find_player(m_ptr->owner), find_player(y_ptr->owner))) &&
                (r_ptr->mexp > z_ptr->mexp) && //TODO: check monster body weight too?
                (cave_floor_bold(zcave, m_ptr->fy, m_ptr->fx))
            ) {
                /* Allow movement */
                do_move = TRUE;
                #ifdef OLD_MONSTER_LORE
                                /* Monster pushed past another monster */
                                did_move_body = TRUE;
                #endif
                /* XXX XXX XXX Message */
                /* Monster wants to move but is blocked by another monster */
                do_turn = TRUE;
            } else {
                do_move = FALSE;

                /* finally: 'stuck' monsters no longer retain full energy to retry casting in each frame. */
#ifndef SIMPLE_ANTISTUCK
                m_ptr->stuck = level_speed(&m_ptr->wpos);
#else
                /* take a turn */
                do_turn = TRUE;
#endif
            }
		}


		/* creature has been allowed move */
		if (do_move) {
			/* take a turn */
			do_turn = TRUE;

			/* hack -- update the old location */
			zcave[origin_y][origin_x].m_idx = c_ptr->m_idx;

			/* mega-hack -- move the old monster, if any */
			if (c_ptr->m_idx > 0) {
				/* move the old monster */
				y_ptr->fy = origin_y;
				y_ptr->fx = origin_x;

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
cave_midx_debug(wpos, origin_y, origin_x, c_ptr->m_idx); //DEBUG
			/* redraw the old grid */
			everyone_lite_spot(wpos, origin_y, origin_x );

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


void set_pet_mind(int Ind, monster_type *m_ptr, byte pet_mind) {  
    if (!m_ptr->pet) return;

    char *monster_name = r_name_get(m_ptr);
    
    switch (pet_mind) {
    case PET_ATTACK:
        if (m_ptr->mind & pet_mind) {
            msg_format(Ind, "Your %s stop going for your target.", monster_name);
            m_ptr->mind &= ~pet_mind;
        } else {
            msg_format(Ind, "Your %s approach your target!", monster_name);
            m_ptr->mind |= pet_mind;
        }
    break;
    case PET_GUARD:
        if (m_ptr->mind & pet_mind) {
            msg_format(Ind, "Your %s stop being on guard.", monster_name);
            m_ptr->mind &= ~pet_mind;
        } else {
            msg_format(Ind, "Your %s seem to be on guard now!", monster_name);
            m_ptr->mind |= pet_mind;
        }
    break;
    case PET_FOLLOW:
        if (m_ptr->mind & pet_mind) {
            m_ptr->mind &= ~pet_mind;
            msg_format(Ind, "Your %s stop following you around.", monster_name);
        } else {
            msg_format(Ind, "Your %s start following you around!", monster_name);
            m_ptr->mind |= pet_mind;
        }
    break;
    }
}

void set_player_pets_mind(int Ind, byte pet_mind) {
    monster_type *m_ptr;
    player_type *p_ptr = Players[Ind];

    for (int i = 0; i < p_ptr->pets_count ; i++) {
        if (! p_ptr->pets[i]) continue;

        /* Access the monster */
        m_ptr = &m_list[p_ptr->pets[i]];

        set_pet_mind(Ind, m_ptr, pet_mind);
    }
}

/* Some other useful functions */
void print_pets_info(int Ind) {
    monster_type *m_ptr;
    player_type *p_ptr = Players[Ind];

    if (!p_ptr->pets_count) {
        msg_print(Ind, "\377GYou dont have any pets");
        return;
    } else {
        if (p_ptr->pets_count == 1) 
            msg_format(Ind, "\377GYou have %d pet:", p_ptr->pets_count);
        else 
            msg_format(Ind, "\377GYou have %d pets:", p_ptr->pets_count);
    }
    
    for (int i = 0; i < p_ptr->pets_count ; i++) {
        if (! p_ptr->pets[i]) continue;

        /* Access the monster */
        m_ptr = &m_list[p_ptr->pets[i]];
        
        if (is_admin(p_ptr)) {
            msg_format(Ind, "\377G%d | %s(%d): %d/%d HP", i, r_name_get(m_ptr), p_ptr->pets[i], m_ptr->hp , m_ptr->maxhp);
        } else {
            msg_format(Ind, "\377G%d | %s: %d/%d HP", i, r_name_get(m_ptr), m_ptr->hp , m_ptr->maxhp);
        }

    }
}