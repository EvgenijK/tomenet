/* $Id$ */
/* File: cmd3.c */

/* Purpose: Inventory commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"




bool bypass_inscrption = FALSE;
/*
 * Move an item from equipment list to pack
 * Note that only one item at a time can be wielded per slot.
 * Note that taking off an item when "full" will cause that item
 * to fall to the ground.
 */
void inven_takeoff(int Ind, int item, int amt, bool called_from_wield)
{
	player_type *p_ptr = Players[Ind];

	int			posn; //, j;

	object_type		*o_ptr;
	object_type		tmp_obj;

	cptr		act;

	char		o_name[ONAME_LEN];


	/* Get the item to take off */
	o_ptr = &(p_ptr->inventory[item]);

	/* Paranoia */
	if (amt <= 0) return;

        if((!bypass_inscrption) && check_guard_inscription( o_ptr->note, 't' )) {
		msg_print(Ind, "The item's inscription prevents it.");
                return;
        };
        bypass_inscrption = FALSE;


	/* Verify */
	if (amt > o_ptr->number) amt = o_ptr->number;

	/* Make a copy to carry */
	tmp_obj = *o_ptr;
	tmp_obj.number = amt;

	/* What are we "doing" with the object */
	if (amt < o_ptr->number)
	{
		act = "Took off";
	}
	else if (item == INVEN_WIELD)
	{
		act = "You were wielding";
	}
	else if (item == INVEN_ARM)
	{
		act = "You were wielding";
	}
	else if (item == INVEN_BOW)
	{
		act = "You were shooting with";
	}
	else if (item == INVEN_LITE)
	{
		act = "Light source was";
	}
	/* Took off ammo */
	else if (item == INVEN_AMMO)
	{
		act = "You were carrying in your quiver";
	}
	/* Took off tool */
	else if (item == INVEN_TOOL)
	{
		act = "You were using";
	}
	else
	{
		act = "You were wearing";
	}

#ifdef USE_SOUND_2010
	sound_item(Ind, o_ptr->tval, o_ptr->sval, "takeoff_");
#endif

#if 0 //DSMs don't poly anymore due to cheeziness. They breathe instead.
	/* Polymorph back */
	/* XXX this can cause strange things for players with mimicry skill.. */
	if ((item == INVEN_BODY) && (o_ptr->tval == TV_DRAG_ARMOR)) 
	{
		/* Well, so we gotta check if the player, in case he is a
		mimic, is using a form that can _only_ come from the armor */
		//if (p_ptr->pclass == CLASS_MIMIC) //Adventurers can also have mimic skill
		//{
			switch (o_ptr->sval)
			{
			case SV_DRAGON_BLACK:
			j = race_index("Ancient black dragon"); break;
			case SV_DRAGON_BLUE:
			j = race_index("Ancient blue dragon"); break;
			case SV_DRAGON_WHITE:
			j = race_index("Ancient white dragon"); break;
			case SV_DRAGON_RED:
			j = race_index("Ancient red dragon"); break;
			case SV_DRAGON_GREEN:
			j = race_index("Ancient green dragon"); break;
			case SV_DRAGON_MULTIHUED:
			j = race_index("Ancient multi-hued dragon"); break;
			case SV_DRAGON_PSEUDO:
			j = race_index("Ethereal drake"); break;
			//j = race_index("Pseudo dragon"); break;
			case SV_DRAGON_SHINING:
			j = race_index("Ethereal dragon"); break;
			case SV_DRAGON_LAW:
			j = race_index("Great Wyrm of Law"); break;
			case SV_DRAGON_BRONZE:
			j = race_index("Ancient bronze dragon"); break;
			case SV_DRAGON_GOLD:
			j = race_index("Ancient gold dragon"); break;
			case SV_DRAGON_CHAOS:
			j = race_index("Great Wyrm of Chaos"); break;
			case SV_DRAGON_BALANCE:
			j = race_index("Great Wyrm of Balance"); break;
			case SV_DRAGON_POWER:
			j = race_index("Great Wyrm of Power"); break;
			}
			if((p_ptr->body_monster == j) &&
			    ((p_ptr->r_killed[j] < r_info[j].level) || 
			    (r_info[j].level > get_skill_scale(p_ptr, SKILL_MIMIC, 100))))
				do_mimic_change(Ind, 0, TRUE);
		/*}
		else
		{
			do_mimic_change(Ind, 0, TRUE);
		}*/
	}
#endif

#if POLY_RING_METHOD == 0
	/* Polymorph back */
	/* XXX this can cause strange things for players with mimicry skill.. */
	if ((item == INVEN_LEFT || item == INVEN_RIGHT) && (o_ptr->tval == TV_RING) && (o_ptr->sval == SV_RING_POLYMORPH))
	{
		if ((p_ptr->body_monster == o_ptr->pval) && 
		    ((p_ptr->r_killed[p_ptr->body_monster] < r_info[p_ptr->body_monster].level) ||
		    (get_skill_scale(p_ptr, SKILL_MIMIC, 100) < r_info[p_ptr->body_monster].level)))
		{
			/* If player hasn't got high enough kill count anymore now, poly back to player form! */
			msg_print(Ind, "You polymorph back to your normal form.");
			do_mimic_change(Ind, 0, TRUE);
		}
	}
#endif

	/* Check if item gave WRAITH form */
	if((k_info[o_ptr->k_idx].flags3 & TR3_WRAITH) && p_ptr->tim_wraith)
		p_ptr->tim_wraith = 1;

	/* Artifacts */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;
		/* Obtain the artifact info */
		if (o_ptr->name1 == ART_RANDART)
    			a_ptr = randart_make(o_ptr);
		else
		        a_ptr = &a_info[o_ptr->name1];

		if ((a_ptr->flags3 & TR3_WRAITH) && p_ptr->tim_wraith) p_ptr->tim_wraith = 1;
	}



	/* Carry the object, saving the slot it went in */
	posn = inven_carry(Ind, &tmp_obj);

	/* Handles overflow */
	pack_overflow(Ind);


	/* Describe the result */
	if (amt < o_ptr->number)
		object_desc(Ind, o_name, &tmp_obj, TRUE, 3);
	else
		object_desc(Ind, o_name, o_ptr, TRUE, 3);
	msg_format(Ind, "%^s %s (%c).", act, o_name, index_to_label(posn));

	if (!called_from_wield && p_ptr->prace == RACE_HOBBIT && o_ptr->tval == TV_BOOTS)
		msg_print(Ind, "\377gYou feel more dextrous now, being barefeet.");

	/* Delete (part of) it */
	inven_item_increase(Ind, item, -amt);
	inven_item_optimize(Ind, item);

#ifdef ENABLE_STANCES
	/* take care of combat stances */
 #ifndef ALLOW_SHIELDLESS_DEFENSIVE_STANCE
	if ((item == INVEN_ARM && p_ptr->combat_stance == 1) ||
	    (item == INVEN_WIELD && p_ptr->combat_stance == 2)) {
 #else
	if (p_ptr->combat_stance &&
	    ((item == INVEN_ARM && !p_ptr->inventory[INVEN_WIELD].k_idx) ||
	    (item == INVEN_WIELD && (
	    !p_ptr->inventory[INVEN_ARM].k_idx || p_ptr->combat_stance == 2)))) {
 #endif
		msg_print(Ind, "\377sYou return to balanced combat stance.");
		p_ptr->combat_stance = 0;
		p_ptr->redraw |= PR_STATE;
	}
#endif

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Recalculate mana */
	p_ptr->update |= (PU_MANA | PU_HP | PU_SANITY);

	/* Redraw */
	p_ptr->redraw |= (PR_PLUSSES);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
}




/*
 * Drops (some of) an item from inventory to "near" the current location
 */
void inven_drop(int Ind, int item, int amt)
{
	player_type *p_ptr = Players[Ind];

	object_type		*o_ptr;
	object_type		 tmp_obj;

	cptr		act;
	int		o_idx;

	char		o_name[ONAME_LEN];



	/* Access the slot to be dropped */
	o_ptr = &(p_ptr->inventory[item]);

	/* Error check */
	if (amt <= 0) return;

	/* Not too many */
	if (amt > o_ptr->number) amt = o_ptr->number;

	/* Nothing done? */
	if (amt <= 0) return;

	/* check for !d  or !* in inscriptions */

	if (!bypass_inscrption && check_guard_inscription(o_ptr->note, 'd')) {
		msg_print(Ind, "The item's inscription prevents it.");
		return;
	}
	bypass_inscrption = FALSE;

#ifdef USE_SOUND_2010
	sound_item(Ind, o_ptr->tval, o_ptr->sval, "drop_");
#endif

	/* Make a "fake" object */
	tmp_obj = *o_ptr;
	tmp_obj.number = amt;

	/*
	 * Hack -- If rods or wands are dropped, the total maximum timeout or 
	 * charges need to be allocated between the two stacks.  If all the items 
	 * are being dropped, it makes for a neater message to leave the original 
	 * stack's pval alone. -LM-
	 */
	if (o_ptr->tval == TV_WAND) tmp_obj.pval = divide_charged_item(o_ptr, amt);

	/* What are we "doing" with the object */
	if (amt < o_ptr->number)
		act = "Dropped";
	else if (item == INVEN_WIELD)
		act = "Was wielding";
	else if (item == INVEN_ARM)
		act = "Was wielding";
	else if (item == INVEN_BOW)
		act = "Was shooting with";
	else if (item == INVEN_LITE)
		act = "Light source was";
	else if (item >= INVEN_WIELD)
		act = "Was wearing";
	else
		act = "Dropped";

	/* Message */
	object_desc(Ind, o_name, &tmp_obj, TRUE, 3);

#if 0 //DSMs don't poly anymore due to cheeziness. They breathe instead.
	/* Polymorph back */
	if ((item == INVEN_BODY) && (o_ptr->tval == TV_DRAG_ARMOR)) {
		/* Well, so we gotta check if the player, in case he is a
		mimic, is using a form that can _only_ come from the armor */
		//if (p_ptr->pclass == CLASS_MIMIC) //Adventurers can also have mimic skill
		//{
			switch (o_ptr->sval) 
			case SV_DRAGON_BLACK:
			j = race_index("Ancient black dragon"); break;
			case SV_DRAGON_BLUE:
			j = race_index("Ancient blue dragon"); break;
			case SV_DRAGON_WHITE:
			j = race_index("Ancient white dragon"); break;
			case SV_DRAGON_RED:
			j = race_index("Ancient red dragon"); break;
			case SV_DRAGON_GREEN:
			j = race_index("Ancient green dragon"); break;
			case SV_DRAGON_MULTIHUED:
			j = race_index("Ancient multi-hued dragon"); break;
			case SV_DRAGON_PSEUDO:
			j = race_index("Ethereal drake"); break;
			//j = race_index("Pseudo dragon"); break;
			case SV_DRAGON_SHINING:
			j = race_index("Ethereal dragon"); break;
			case SV_DRAGON_LAW:
			j = race_index("Great Wyrm of Law"); break;
			case SV_DRAGON_BRONZE:
			j = race_index("Ancient bronze dragon"); break;
			case SV_DRAGON_GOLD:
			j = race_index("Ancient gold dragon"); break;
			case SV_DRAGON_CHAOS:
			j = race_index("Great Wyrm of Chaos"); break;
			case SV_DRAGON_BALANCE:
			j = race_index("Great Wyrm of Balance"); break;
			case SV_DRAGON_POWER:
			j = race_index("Great Wyrm of Power"); break;
			}
			if((p_ptr->body_monster == j) &&
			    ((p_ptr->r_killed[j] < r_info[j].level) ||
			    (r_info[j].level > get_skill_scale(p_ptr, SKILL_MIMIC, 100))))
				do_mimic_change(Ind, 0, TRUE);
		/*}
		else
		{
			do_mimic_change(Ind, 0, TRUE);
		}*/
	}
#endif

#if POLY_RING_METHOD == 0
	/* Polymorph back */
	/* XXX this can cause strange things for players with mimicry skill.. */
	if ((item == INVEN_LEFT || item == INVEN_RIGHT) &&
	    (o_ptr->tval == TV_RING) && (o_ptr->sval == SV_RING_POLYMORPH)) {
		if ((p_ptr->body_monster == o_ptr->pval) && 
		    ((p_ptr->r_killed[p_ptr->body_monster] < r_info[p_ptr->body_monster].level) ||
		    (get_skill_scale(p_ptr, SKILL_MIMIC, 100) < r_info[p_ptr->body_monster].level)))
		{
			/* If player hasn't got high enough kill count anymore now, poly back to player form! */
 #if 1
			msg_print(Ind, "You polymorph back to your normal form.");
			do_mimic_change(Ind, 0, TRUE);
 #endif
			s_printf("DROP_EXPLOIT (poly): %s dropped %s\n", p_ptr->name, o_name);
		}
	}
#endif

	/* Check if item gave WRAITH form */
	if ((k_info[o_ptr->k_idx].flags3 & TR3_WRAITH) && p_ptr->tim_wraith) {
		s_printf("DROP_EXPLOIT (wraith): %s dropped %s\n", p_ptr->name, o_name);
#if 1
		p_ptr->tim_wraith = 1;
#endif
	}

	/* Artifacts */
	if (o_ptr->name1) {
		artifact_type *a_ptr;
		/* Obtain the artifact info */
		if (o_ptr->name1 == ART_RANDART)
			a_ptr = randart_make(o_ptr);
		else
			a_ptr = &a_info[o_ptr->name1];

		if ((a_ptr->flags3 & TR3_WRAITH) && p_ptr->tim_wraith) {
#if 1
			p_ptr->tim_wraith = 1;
#endif
			s_printf("DROP_EXPLOIT (wraith, art): %s dropped %s\n", p_ptr->name, o_name);
		}
	}

#ifdef ENABLE_STANCES
	/* take care of combat stances */
 #ifndef ALLOW_SHIELDLESS_DEFENSIVE_STANCE
	if ((item == INVEN_ARM && p_ptr->combat_stance == 1) ||
	    (item == INVEN_WIELD && p_ptr->combat_stance == 2)) {
 #else
	if (p_ptr->combat_stance &&
	    ((item == INVEN_ARM && !p_ptr->inventory[INVEN_WIELD].k_idx) ||
	    (item == INVEN_WIELD && (
	    !p_ptr->inventory[INVEN_ARM].k_idx || p_ptr->combat_stance == 2)))) {
 #endif
		msg_print(Ind, "\377sYou return to balanced combat stance.");
		p_ptr->combat_stance = 0;
		p_ptr->redraw |= PR_STATE;
	}
#endif

	/* Message */
	msg_format(Ind, "%^s %s (%c).", act, o_name, index_to_label(item));

	/* Drop it (carefully) near the player */
	o_idx = drop_near_severe(Ind, &tmp_obj, 0, &p_ptr->wpos, p_ptr->py, p_ptr->px);

#ifdef PLAYER_STORES
	o_ptr = &o_list[o_idx];
	if (o_idx > 0 && o_ptr->note && strstr(quark_str(o_ptr->note), "@S")
	    && inside_house(&p_ptr->wpos, o_ptr->ix, o_ptr->iy)) {
		object_desc(0, o_name, o_ptr, TRUE, 3);
		s_printf("PLAYER_STORE_OFFER: %s - %s (%d,%d,%d; %d,%d).\n",
		    p_ptr->name, o_name, p_ptr->wpos.wx, p_ptr->wpos.wy, p_ptr->wpos.wz,
		    o_ptr->ix, o_ptr->iy);
	}
#endif

	/* Decrease the item, optimize. */
	inven_item_increase(Ind, item, -amt);
	inven_item_describe(Ind, item);
	inven_item_optimize(Ind, item);

	break_cloaking(Ind, 5);
	break_shadow_running(Ind);
	stop_precision(Ind);
	stop_shooting_till_kill(Ind);
}

/*
 * The "wearable" tester
 */
#if 1 /* new way: 'fruit bat body' is checked first: it's restrictions will be inherited by all other forms */
bool item_tester_hook_wear(int Ind, int slot) {
	player_type *p_ptr = Players[Ind];
	monster_race *r_ptr = NULL;
	if (p_ptr->body_monster) r_ptr = &r_info[p_ptr->body_monster];

	if (p_ptr->fruit_bat) {
		switch(slot) {
		case INVEN_RIGHT:
		case INVEN_LEFT:
		case INVEN_NECK:
		case INVEN_HEAD:
		case INVEN_OUTER:
		case INVEN_LITE:
		case INVEN_ARM:
		case INVEN_TOOL:    //allow them to wear, say, picks and shovels - the_sandman
			break; //fine
		default:
			return FALSE; //not fine
		}
	}

	if (p_ptr->body_monster &&
	    (p_ptr->pclass != CLASS_DRUID) &&
	    ((p_ptr->pclass != CLASS_SHAMAN) || !mimic_shaman_fulleq(r_ptr->d_char)) &&
	    (p_ptr->prace != RACE_VAMPIRE)
	    ) {
		switch(slot) {
		case INVEN_WIELD:
		case INVEN_BOW:
			if (r_ptr->body_parts[BODY_WEAPON]) return (TRUE);
			break;
		case INVEN_LEFT:
			if (r_ptr->body_parts[BODY_FINGER] > 1) return (TRUE);
			break;
		case INVEN_RIGHT:
			if (r_ptr->body_parts[BODY_FINGER]) return (TRUE);
			break;
		case INVEN_NECK:
		case INVEN_HEAD:
			if (r_ptr->body_parts[BODY_HEAD]) return (TRUE);
			break;
		case INVEN_LITE:
			/* Always allow to carry light source? :/ */
			/* return (TRUE); break; */
			if (r_ptr->body_parts[BODY_WEAPON]) return (TRUE);
			if (r_ptr->body_parts[BODY_FINGER]) return (TRUE);
			if (r_ptr->body_parts[BODY_HEAD]) return (TRUE);
			if (r_ptr->body_parts[BODY_ARMS]) return (TRUE);
			break;
		case INVEN_BODY:
		case INVEN_OUTER:
		case INVEN_AMMO:
			if (r_ptr->body_parts[BODY_TORSO]) return (TRUE);
			break;
		case INVEN_ARM:
			if (r_ptr->body_parts[BODY_ARMS]) return (TRUE);
			break;
		case INVEN_TOOL:
			/* make a difference :) - C. Blue */
			if (r_ptr->body_parts[BODY_ARMS] ||
//			    r_ptr->body_parts[BODY_FINGER] ||
			    r_ptr->body_parts[BODY_WEAPON]) return (TRUE);
			break;
		case INVEN_HANDS:
//			if (r_ptr->body_parts[BODY_FINGER]) return (TRUE); too silyl (and powerful)
//			if (r_ptr->body_parts[BODY_ARMS]) return (TRUE); was standard, but now:
			if (r_ptr->body_parts[BODY_FINGER] && r_ptr->body_parts[BODY_ARMS]) return (TRUE);
			break;
		case INVEN_FEET:
			if (r_ptr->body_parts[BODY_LEGS]) return (TRUE);
			break;
		}
	}
#if 0
	else if (r_info[p_ptr->body_monster].flags3 & RF3_DRAGON) {
		switch(slot) {
		case INVEN_WIELD:
		case INVEN_RIGHT:
		case INVEN_LEFT:
		case INVEN_HEAD:
		case INVEN_BODY:
		case INVEN_LITE:
		case INVEN_FEET: return TRUE;
		}
	}
#endif	// 0
	/* Check for a usable slot */
	else if (slot >= INVEN_WIELD) return (TRUE);

	/* Assume not wearable */
	return (FALSE);
}

#else // old way, deprecated

bool item_tester_hook_wear(int Ind, int slot) {
	player_type *p_ptr = Players[Ind];
	monster_race *r_ptr = NULL;
	if (p_ptr->body_monster) r_ptr = &r_info[p_ptr->body_monster];

	/*
	 * Hack -- restrictions by forms
	 * I'm not quite sure if wielding 6 rings and 3 weps should be allowed..
	 * Shapechanging Druids do not get penalized...

	 * Another hack for shamans (very experimental):
	 * They can use their full equipment in E and G (spirit/elemental/ghost) form.
	 *
	 * Druid bats can't wear full eq.
	 */
#if 0
	if (p_ptr->body_monster &&
	    ((p_ptr->pclass != CLASS_DRUID) || p_ptr->fruit_bat) &&
	    ((p_ptr->pclass != CLASS_SHAMAN) || !mimic_shaman_fulleq(r_ptr->d_char)) &&
	    ((p_ptr->prace != RACE_VAMPIRE) || p_ptr->fruit_bat)
	    )
#else
	if (p_ptr->body_monster &&
	    (p_ptr->pclass != CLASS_DRUID) &&
	    ((p_ptr->pclass != CLASS_SHAMAN) || !mimic_shaman_fulleq(r_ptr->d_char)) &&
	    (p_ptr->prace != RACE_VAMPIRE)
	    )
#endif
	{
		switch(slot) {
		case INVEN_WIELD:
		case INVEN_BOW:
			if (r_ptr->body_parts[BODY_WEAPON]) return (TRUE);
			break;
		case INVEN_LEFT:
			if (r_ptr->body_parts[BODY_FINGER] > 1) return (TRUE);
			break;
		case INVEN_RIGHT:
			if (r_ptr->body_parts[BODY_FINGER]) return (TRUE);
			break;
		case INVEN_NECK:
		case INVEN_HEAD:
			if (r_ptr->body_parts[BODY_HEAD]) return (TRUE);
			break;
		case INVEN_LITE:
			/* Always allow to carry light source? :/ */
			/* return (TRUE); break; */
			if (r_ptr->body_parts[BODY_WEAPON]) return (TRUE);
			if (r_ptr->body_parts[BODY_FINGER]) return (TRUE);
			if (r_ptr->body_parts[BODY_HEAD]) return (TRUE);
			if (r_ptr->body_parts[BODY_ARMS]) return (TRUE);
			break;
		case INVEN_BODY:
		case INVEN_OUTER:
		case INVEN_AMMO:
			if (r_ptr->body_parts[BODY_TORSO]) return (TRUE);
			break;
		case INVEN_ARM:
			if (r_ptr->body_parts[BODY_ARMS]) return (TRUE);
			break;
		case INVEN_TOOL:
			/* make a difference :) - C. Blue */
			if (r_ptr->body_parts[BODY_ARMS] ||
//			    r_ptr->body_parts[BODY_FINGER] ||
			    r_ptr->body_parts[BODY_WEAPON]) return (TRUE);
			break;
		case INVEN_HANDS:
//			if (r_ptr->body_parts[BODY_FINGER]) return (TRUE); too silyl (and powerful)
//			if (r_ptr->body_parts[BODY_ARMS]) return (TRUE); was standard, but now:
			if (r_ptr->body_parts[BODY_FINGER] && r_ptr->body_parts[BODY_ARMS]) return (TRUE);
			break;
		case INVEN_FEET:
			if (r_ptr->body_parts[BODY_LEGS]) return (TRUE);
			break;
		}
	}
	/* Restrict fruit bats */
#if 0
	else if (p_ptr->fruit_bat && !p_ptr->body_monster)
#else
	else if (p_ptr->fruit_bat && (
	    !p_ptr->body_monster ||
	    p_ptr->pclass == CLASS_DRUID ||
	    (p_ptr->pclass == CLASS_SHAMAN && !mimic_shaman_fulleq(r_ptr->d_char)) ||
	    p_ptr->prace != RACE_VAMPIRE
	    ))
#endif
	{
		switch(slot) {
		case INVEN_RIGHT:
		case INVEN_LEFT:
		case INVEN_NECK:
		case INVEN_HEAD:
		case INVEN_OUTER:
		case INVEN_LITE:
		case INVEN_ARM:
		case INVEN_TOOL:    //allow them to wear, say, picks and shovels - the_sandman
			return TRUE;
		}
	}
#if 0
	else if (r_info[p_ptr->body_monster].flags3 & RF3_DRAGON) {
		switch(slot) {
		case INVEN_WIELD:
		case INVEN_RIGHT:
		case INVEN_LEFT:
		case INVEN_HEAD:
		case INVEN_BODY:
		case INVEN_LITE:
		case INVEN_FEET:
			return TRUE;
		}
	}
#endif	// 0
	else
	{
		/* Check for a usable slot */
		if (slot >= INVEN_WIELD) {
#if 0
			/* use of shield is banned in do_cmd_wield if with 2H weapon.
			 * 3 slots are too severe.. thoughts?	- Jir -
			 */
			if (slot == INVEN_BOW && p_ptr->inventory[INVEN_WIELD].k_idx) {
				u32b f1, f2, f3, f4, f5, esp;
				object_flags(&p_ptr->inventory[INVEN_WIELD], &f1, &f2, &f3, &f4, &f5, &esp);
				if (f4 & TR4_MUST2H) return(FALSE);
			}
#endif	// 0
			return (TRUE);
		}
	}

	/* Assume not wearable */
	return (FALSE);
}
#endif

/* Take off things that are no more wearable */
void do_takeoff_impossible(int Ind)
{
	int k;
	player_type *p_ptr = Players[Ind];
	object_type *o_ptr;
	u32b f1, f2, f3, f4, f5, esp;


	bypass_inscrption = TRUE;
	for (k = INVEN_WIELD; k < INVEN_TOTAL; k++)
	{
		o_ptr = &p_ptr->inventory[k];
		if ((o_ptr->k_idx) && /* following is a hack for dual-wield.. */
		    (!item_tester_hook_wear(Ind, (k == INVEN_ARM && o_ptr->tval != TV_SHIELD) ? INVEN_WIELD : k)))
		{
			/* Extract the flags */
			object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

			/* Morgie crown and The One Ring resists! */
			if (f3 & TR3_PERMA_CURSE) continue;

			/* Ahah TAKE IT OFF ! */
			inven_takeoff(Ind, k, 255, FALSE);
		}
	}
	bypass_inscrption = FALSE;
}

/*
 * Wield or wear a single item from the pack or floor
 * Added alt_slots to specify an alternative slot when an item fits in
 * two places - C. Blue
 * 0 = equip in first free slot that fits, if none is free, replace item in first slot (standard/traditional behaviour)
 * 1 = equip in first slot that fits, replacing the item if any
 * 2 = equip in second slot that fits, replacing the item if any
 * 4 = don't equip if slot is already occupied (ie don't replace by taking off an item)
 * Note: Rings make an exception in 4: First ring always goes in second ring slot.
 */
void do_cmd_wield(int Ind, int item, u16b alt_slots)
{
	player_type *p_ptr = Players[Ind];

	int slot, num = 1;
	bool item_fits_dual = FALSE, equip_fits_dual = TRUE, all_cursed = FALSE;
	bool slot1 = (p_ptr->inventory[INVEN_WIELD].k_idx != 0);
	bool slot2 = (p_ptr->inventory[INVEN_ARM].k_idx != 0);
	bool alt = ((alt_slots & 0x2) != 0);

	bool ma_warning_weapon = FALSE, ma_warning_shield = FALSE, hobbit_warning = FALSE;

	object_type tmp_obj;
	object_type *o_ptr;
	object_type *x_ptr;

	cptr act;

	char o_name[ONAME_LEN];
	u32b f1 = 0 , f2 = 0 , f3 = 0, f4 = 0, f5 = 0, esp = 0;


	/* Restrict the choices */
	/*item_tester_hook = item_tester_hook_wear;*/


	/* Get the item (in the pack) */
	if (item >= 0) {
		o_ptr = &(p_ptr->inventory[item]);
	} else { /* Get the item (on the floor) */
		if (-item >= o_max)
			return; /* item doesn't exist */

		o_ptr = &o_list[0 - item];
	}

	if (check_guard_inscription(o_ptr->note, 'w')) {
		msg_print(Ind, "The item's inscription prevents it.");
		return;
	}

	/* Check the slot */
	slot = wield_slot(Ind, o_ptr);

	if (!item_tester_hook_wear(Ind, slot)) {
		msg_print(Ind, "You may not wield that item.");
		return;
	}

	if (!can_use_verbose(Ind, o_ptr)) return;

	/* Costumes allowed during halloween and xmas */
	if (!season_halloween && !season_xmas) {
		if ((o_ptr->tval == TV_SOFT_ARMOR) && (o_ptr->sval == SV_COSTUME)) {
			msg_print(Ind, "It's not that time of the year anymore.");
			return;
		}
	}

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* check whether the item to wield is fit for dual-wielding */
	if ((o_ptr->weight <= 999) && /* <- no hard-coded weight limit for now */
	    !(f4 & (TR4_MUST2H | TR4_SHOULD2H))) item_fits_dual = TRUE;
	/* check whether our current equipment allows a dual-wield setup with the new item */
	if (slot1 && (k_info[p_ptr->inventory[INVEN_WIELD].k_idx].flags4 & (TR4_MUST2H | TR4_SHOULD2H)))
		equip_fits_dual = FALSE;

	/* Do we have dual-wield and are trying to equip a weapon?.. */
	if (get_skill(p_ptr, SKILL_DUAL) && slot == INVEN_WIELD) {
#if 0
		/* Equip in arm slot if weapon slot alreay occupied but arm slot still empty */
		if ((p_ptr->inventory[INVEN_WIELD].k_idx || (alt_slots & 0x2))
		    && (!p_ptr->inventory[INVEN_ARM].k_idx || (alt_slots & 0x2))
		    && !(alt_slots & 0x1)
		    /* If to-wield weapon is 2h or 1.5h, choose normal INVEN_WIELD slot instead */
		    && item_fits_dual
		    /* If main-hand weapon is 2h or 1.5h, choose normal INVEN_WIELD slot instead */
		    && equip_fits_dual)
			slot = INVEN_ARM;
#else /* fix: allow 'W' to replace main hand if second hand is empty */
		/* Equip in arm slot if weapon slot alreay occupied but arm slot still empty */
		if (((!slot1 && alt) || (slot2 && alt) || (slot1 && !slot2 && !alt)) &&
		    !(alt_slots & 0x1) &&
		    /* If to-wield weapon is 2h or 1.5h, choose normal INVEN_WIELD slot instead */
		    item_fits_dual &&
		    /* If main-hand weapon is 2h or 1.5h, choose normal INVEN_WIELD slot instead */
		    equip_fits_dual)
			slot = INVEN_ARM;
#endif
	}

	/* to allow only right ring -> only right ring: */
	if (slot == INVEN_LEFT && (alt_slots & 0x2)) slot = INVEN_RIGHT;
	/* to allow no rings -> left ring: */
	else if (slot == INVEN_RIGHT && (alt_slots & 0x2)) slot = INVEN_LEFT;

	/* use the first fitting slot found? (unused) */
	if (slot == INVEN_RIGHT && (alt_slots & 0x1)) slot = INVEN_LEFT;

	if ((alt_slots & 0x4) && p_ptr->inventory[slot].k_idx) {
		object_desc(Ind, o_name, &(p_ptr->inventory[slot]), FALSE, 0);
		msg_format(Ind, "Take off your %s first.", o_name);
		return;
	}

	/* Prevent wielding into a cursed slot */
	/* Try alternative slots if one is cursed and item can go in multiple places */
	if (cursed_p(&p_ptr->inventory[slot]) && !(alt_slots & 0x3)) {
		switch (slot) {
		case INVEN_LEFT: slot = INVEN_RIGHT; all_cursed = TRUE; break;
		case INVEN_RIGHT: slot = INVEN_LEFT; all_cursed = TRUE; break;
		case INVEN_WIELD: if (get_skill(p_ptr, SKILL_DUAL) && item_fits_dual && equip_fits_dual) { slot = INVEN_ARM; all_cursed = TRUE; break; }
		}
	}
	if (cursed_p(&(p_ptr->inventory[slot]))) {
		/* Describe it */
		object_desc(Ind, o_name, &(p_ptr->inventory[slot]), FALSE, 0);

		/* Message */
		if (all_cursed)
			msg_format(Ind, "The items you are already %s both appear to be cursed.", describe_use(Ind, slot));
		else
			msg_format(Ind, "The %s you are %s appears to be cursed.", o_name, describe_use(Ind, slot));

		/* Cancel the command */
		return;
	}

	/* Two handed weapons can't be wielded with a shield */
	/* TODO: move to item_tester_hook_wear? */
#if 0
	if ((is_slot_ok(slot - INVEN_WIELD + INVEN_ARM)) &&
	    (f4 & TR4_MUST2H) &&
	    (inventory[slot - INVEN_WIELD + INVEN_ARM].k_idx != 0))
#endif	// 0

	if ((f4 & TR4_MUST2H) &&
	    (p_ptr->inventory[INVEN_ARM].k_idx != 0))
	{
#if 1 /* a) either give error msg, or.. */
		object_desc(Ind, o_name, o_ptr, FALSE, 0);
		if (get_skill(p_ptr, SKILL_DUAL))
			msg_format(Ind, "You cannot wield your %s with a shield or a secondary weapon.", o_name);
		else
			msg_format(Ind, "You cannot wield your %s with a shield.", o_name);
		return;
#else /* b) take off the left-hand item too */
/* important note: can't enable this like this, because it's NOT FINISHED:
   after taking off the shield/2nd weapon, item letters in inventory will
   CHANGE and the WRONG item will be equipped. - C. Blue */
return;
		if (check_guard_inscription(p_ptr->inventory[INVEN_ARM].note, 't' )) {
			msg_print(Ind, "Your second wielded item's inscription prevents taking it off.");
			return;
		};
		if (cursed_p(&p_ptr->inventory[INVEN_ARM]) && !is_admin(p_ptr)) {
			msg_print(Ind, "Hmmm, the second item you're wielding seems to be cursed.");
			return;
		}
		inven_takeoff(Ind, INVEN_ARM, 255, TRUE);
#endif
	}
	if ((f4 & TR4_SHOULD2H) &&
	    (p_ptr->inventory[INVEN_ARM].k_idx && p_ptr->inventory[INVEN_ARM].tval != TV_SHIELD)) /* dual-wield not with 1.5h */
	{
		object_desc(Ind, o_name, o_ptr, FALSE, 0);
		msg_format(Ind, "You cannot wield your %s with a secondary weapon.", o_name);
		return;
	}

//	if (is_slot_ok(slot - INVEN_ARM + INVEN_WIELD)) {
//		i_ptr = &inventory[slot - INVEN_ARM + INVEN_WIELD];
	if (o_ptr->tval == TV_SHIELD && (x_ptr = &p_ptr->inventory[INVEN_WIELD]))
	{
		/* Extract the flags */
		object_flags(x_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Prevent shield from being put on if wielding 2H */
		if ((f4 & TR4_MUST2H) && (x_ptr->k_idx) )
			//		    (p_ptr->body_parts[slot - INVEN_WIELD] == INVEN_ARM))
		{
			object_desc(Ind, o_name, o_ptr, FALSE, 0);
			msg_format(Ind, "You cannot wield your %s with a two-handed weapon.", o_name);
			return;
		}
	}

	x_ptr = &(p_ptr->inventory[slot]);

	if (check_guard_inscription(x_ptr->note, 't')) {
		msg_print(Ind, "The inscription of your equipped item prevents it.");
		return;
	};

#if 0
	/* Verify potential overflow */
	if ((p_ptr->inven_cnt >= INVEN_PACK) &&
	    ((item < 0) || (o_ptr->number > 1)))
	{
		/* Verify with the player */
		if (other_query_flag &&
		    !get_check(Ind, "Your pack may overflow.  Continue? ")) return;
	}
#endif

#ifdef USE_SOUND_2010
	sound_item(Ind, o_ptr->tval, o_ptr->sval, "wearwield_");
#endif

	/* Mega-hack -- prevent anyone but total winners from wielding the Massive Iron
	 * Crown of Morgoth or the Mighty Hammer 'Grond'.
	 */
	if (!(p_ptr->total_winner || is_admin(p_ptr))) {
		/* Attempting to wear the crown if you are not a winner is a very, very bad thing
		 * to do.
		 */
		if (o_ptr->name1 == ART_MORGOTH) {
			msg_print(Ind, "You are blasted by the Crown's power!");
			/* This should pierce invulnerability */
			bypass_invuln = TRUE;
			take_hit(Ind, 10000, "the Massive Iron Crown of Morgoth", 0);
			bypass_invuln = FALSE;
			return;
		}
		/* Attempting to wield Grond isn't so bad. */
		if (o_ptr->name1 == ART_GROND) {
			msg_print(Ind, "You are far too weak to wield the mighty Grond.");
			return;
		}
	}

	/* display some warnings if the item will severely conflict with Martial Arts skill */
	if (get_skill(p_ptr, SKILL_MARTIAL_ARTS)) {
		if ((is_weapon(o_ptr->tval) || o_ptr->tval == TV_BOW || o_ptr->tval == TV_BOOMERANG)
		    && !p_ptr->inventory[INVEN_BOW].k_idx
		    && !p_ptr->inventory[INVEN_WIELD].k_idx
		    && (!p_ptr->inventory[INVEN_ARM].k_idx || p_ptr->inventory[INVEN_ARM].tval == TV_SHIELD)) /* for dual-wielders */
			ma_warning_weapon = TRUE;
		else if (o_ptr->tval == TV_SHIELD &&
		    (!p_ptr->inventory[INVEN_ARM].k_idx || p_ptr->inventory[INVEN_ARM].tval != TV_SHIELD)) /* for dual-wielders */
			ma_warning_shield = TRUE;
	}

	process_hooks(HOOK_WIELD, "d", Ind);

	/* Let's not end afk for this - C. Blue */
/*	un_afk_idle(Ind); */

	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

	/* Get a copy of the object to wield */
	tmp_obj = *o_ptr;

	if(slot == INVEN_AMMO) num = o_ptr->number; 
	tmp_obj.number = num;

	/* Decrease the item (from the pack) */
	if (item >= 0) {
		inven_item_increase(Ind, item, -num);
		inven_item_optimize(Ind, item);
	}
	/* Decrease the item (from the floor) */
	else {
		floor_item_increase(0 - item, -num);
		floor_item_optimize(0 - item);
	}


	/* for Hobbits wearing boots -> give message */
	if (p_ptr->prace == RACE_HOBBIT &&
	    o_ptr->tval == TV_BOOTS && !p_ptr->inventory[slot].k_idx)
		hobbit_warning = TRUE;


	/* Access the wield slot */
	o_ptr = &(p_ptr->inventory[slot]);

	/*** Could make procedure "inven_wield()" ***/
    //no need to try to combine the non esp HL amulet?
    //if ((o_ptr->tval == TV_AMULET) && (o_ptr->sval == SV_AMULET_HIGHLANDS && tmp_obj.tval == TV_AMULET && tmp_obj.sval == SV_AMULET_HIGHLANDS))
    if ((o_ptr->tval == TV_AMULET) && (o_ptr->sval == SV_AMULET_HIGHLANDS2 && tmp_obj.tval == TV_AMULET && tmp_obj.sval == SV_AMULET_HIGHLANDS2))
    {
	o_ptr->bpval += tmp_obj.bpval;
    }
    else
    {

#if 0
	/* Take off the "entire" item if one is there */
	if (p_ptr->inventory[slot].k_idx) inven_takeoff(Ind, slot, 255, TRUE);
#else	// 0
	/* Take off existing item */
	if (slot != INVEN_AMMO) {
		if (o_ptr->k_idx) {
			/* Take off existing item */
			(void)inven_takeoff(Ind, slot, 255, TRUE);
		}
	} else {
		if (o_ptr->k_idx) {
			/* !M inscription tolerates different +hit / +dam enchantments,
			   which will be merged and averaged in object_absorb.
			   However, this doesn't work for cursed items or artefacts. - C. Blue */
			if (!object_similar(Ind, o_ptr, &tmp_obj, 0x1)) {
				/* Take off existing item */
				(void)inven_takeoff(Ind, slot, 255, TRUE);
			} else {
				// tmp_obj.number += o_ptr->number;
				object_absorb(Ind, &tmp_obj, o_ptr);
			}
		}
	}
#endif	// 0

	/* Wear the new stuff */
	*o_ptr = tmp_obj;

	/* Increase the weight */
	p_ptr->total_weight += o_ptr->weight * num;

	/* Increment the equip counter by hand */
	p_ptr->equip_cnt++;

	/* Where is the item now */
	if (slot == INVEN_WIELD) {
		act = "You are wielding";
	} else if (slot == INVEN_ARM) {
		act = "You are wielding";
	} else if (slot == INVEN_BOW) {
		act = "You are shooting with";
	} else if (slot == INVEN_LITE) {
		act = "Your light source is";
	} else if (slot == INVEN_AMMO) {
		act = "In your quiver you have";
	} else if (slot == INVEN_TOOL) {
		act = "You are using";
	} else {
		act = "You are wearing";
	}

	/* Describe the result */
	object_desc(Ind, o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format(Ind, "%^s %s (%c).", act, o_name, index_to_label(slot));

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Auto Curse */
	if (f3 & TR3_AUTO_CURSE) {
		/* The object recurse itself ! */
		o_ptr->ident |= ID_CURSED;
	}

	/* Cursed! */
	if (cursed_p(o_ptr)) {
		/* Warn the player */
		msg_print(Ind, "Oops! It feels deathly cold!");

		/* Note the curse */
		o_ptr->ident |= ID_SENSE | ID_SENSED_ONCE;
	}

    }

#ifdef ENABLE_STANCES
	/* take care of combat stances */
	if (slot == INVEN_ARM && p_ptr->combat_stance == 2) {
		msg_print(Ind, "\377sYou return to balanced combat stance.");
		p_ptr->combat_stance = 0;
		p_ptr->redraw |= PR_STATE;
	}
#endif

	/* Give additional warning messages if item prevents a certain ability */
	if (o_ptr->tval == TV_SHIELD) {
		if (get_skill(p_ptr, SKILL_DODGE))
		//if (get_skill(p_ptr, SKILL_DODGE) || p_ptr->tim_dodge) //Kurzel!!
			msg_print(Ind, "\377yYou cannot dodge attacks while wielding a shield.");
		if (get_skill(p_ptr, SKILL_MARTIAL_ARTS))
			msg_print(Ind, "\377yYou cannot use special martial art styles with a shield.");
		/* cannot use ranged techniques with a shield equipped */
		if (p_ptr->ranged_flare) {
			p_ptr->ranged_flare = 0;
			msg_print(Ind, "You dispose of the flare missile.");
		}
		if (p_ptr->ranged_precision) {
			p_ptr->ranged_precision = 0;
			msg_print(Ind, "You stop aiming overly precisely.");
		}
		if (p_ptr->ranged_double) {
			p_ptr->ranged_double = 0;
			msg_print(Ind, "You stop using double-shots.");
		}
		if (p_ptr->ranged_barrage) {
			p_ptr->ranged_barrage = 0;
			msg_print(Ind, "You stop preparations for barrage.");
		}
	}

	/* display warnings, possibly */
	if (ma_warning_weapon && p_ptr->warning_ma_weapon == 0) {
//		p_ptr->warning_ma_weapon = 1;
		msg_print(Ind, "\374\377RWarning: Using any sort of weapon renders Martial Arts skill effectless.");
		s_printf("warning_ma_weapon: %s\n", p_ptr->name);
	} else if (ma_warning_shield && p_ptr->warning_ma_shield == 0) {
//		p_ptr->warning_ma_shield = 1;
		msg_print(Ind, "\374\377RWarning: Using a shield will prevent Martial Arts combat styles.");
		s_printf("warning_ma_shield: %s\n", p_ptr->name);
	}

	if (hobbit_warning) msg_print(Ind, "\377yYou feel somewhat less dextrous than when barefeet.");

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


	/* warning messages, mostly for newbies */
	if (p_ptr->warning_bpr3 == 0 && slot == INVEN_WIELD)
		p_ptr->warning_bpr3 = 2;
}



/*
 * Take off an item
 */
void do_cmd_takeoff(int Ind, int item, int amt)
{
	player_type *p_ptr = Players[Ind];
	object_type *o_ptr;

	if (amt <= 0) return;

#if 0
	/* Verify potential overflow */
	if (p_ptr->inven_cnt >= INVEN_PACK)
	{
		/* Verify with the player */
		if (other_query_flag &&
		    !get_check(Ind, "Your pack may overflow.  Continue? ")) return;
	}
#endif


	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &(p_ptr->inventory[item]);
	}

	/* Get the item (on the floor) */
	else
	{
		if (-item >= o_max)
			return; /* item doesn't exist */

		o_ptr = &o_list[0 - item];
	}

	if( check_guard_inscription( o_ptr->note, 'T' )) {
		msg_print(Ind, "The item's inscription prevents it.");
		return;
	};


	/* Item is cursed */
	if (cursed_p(o_ptr) && !is_admin(p_ptr))
	{
		/* Oops */
		msg_print(Ind, "Hmmm, it seems to be cursed.");

		/* Nope */
		return;
	}

	/* Let's not end afk for this - C. Blue */
/*	un_afk_idle(Ind); */

	/* Take a partial turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;

	/* Take off the item */
	inven_takeoff(Ind, item, amt, FALSE);
}


/*
 * Drop an item
 */
void do_cmd_drop(int Ind, int item, int quantity)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;
	u32b f1, f2, f3, f4, f5, esp;

	/* Get the item (in the pack) */
	if (item >= 0) o_ptr = &(p_ptr->inventory[item]);
	/* Get the item (on the floor) */
	else {
		if (-item >= o_max) return; /* item doesn't exist */
		o_ptr = &o_list[0 - item];
	}

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Handle the newbies_cannot_drop option */	
#if (STARTEQ_TREATMENT == 1)
	if (p_ptr->max_plv < cfg.newbies_cannot_drop && !is_admin(p_ptr) &&
	    !((o_ptr->tval == 1) && (o_ptr->sval >= 9))) {
		msg_print(Ind, "\377yYou are not experienced enough to drop items. (Destroy it with 'k' or sell it instead.)");
		return;
	}
#endif

	if (check_guard_inscription(o_ptr->note, 'd')) {
		msg_print(Ind, "The item's inscription prevents it.");
		return;
	};

	/* Cannot remove cursed items */
	if (cursed_p(o_ptr) && !is_admin(p_ptr)) { /* Hack -- DM can */
		if ((item >= INVEN_WIELD) ) {
			/* Oops */
			msg_print(Ind, "Hmmm, it seems to be cursed.");
			/* Nope */
			return;
		} else if (f4 & TR4_CURSE_NO_DROP) {
			/* Oops */
			msg_print(Ind, "Hmmm, you seem to be unable to drop it.");
			/* Nope */
			return;
		}
	}

	if (p_ptr->inval) {
		msg_print(Ind, "\377yYou may not drop items, wait for an admin to validate your account. (Destroy it with 'k' or sell it instead.)");
		return;
	}

#if 0
	/* Mega-Hack -- verify "dangerous" drops */
	if (cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx) {
		/* XXX XXX Verify with the player */
		if (other_query_flag &&
		    !get_check(Ind, "The item may disappear.  Continue? ")) return;
	}
#endif

	if (object_known_p(Ind, o_ptr)) {
#if 0 /* would prevent ppl from getting rid of unsellable artifacts */
	    if (true_artifact_p(o_ptr) && !is_admin(p_ptr) &&
	        ((cfg.anti_arts_hoard && undepositable_artifact_p(o_ptr)) || (p_ptr->total_winner && !winner_artifact_p(o_ptr)))) {
				msg_print(Ind, "\377yThis item is a true artifact and cannot be dropped!");
				return;
		}
#endif
	    if (p_ptr->wpos.wz == 0) { /* Assume houses are always on surface */
	        if (undepositable_artifact_p(o_ptr) && cfg.anti_arts_house && inside_house(&p_ptr->wpos, p_ptr->px, p_ptr->py)) {
				msg_print(Ind, "\377yThis item is a true artifact and cannot be dropped in a house!");
				return;
			}
		}
	}

	/* stop littering inns */
	if (o_ptr->level == 0 && o_ptr->owner == p_ptr->id && istown(&p_ptr->wpos)
	    && o_ptr->tval != TV_FOOD && (o_ptr->tval != TV_SCROLL || o_ptr->sval != SV_SCROLL_WORD_OF_RECALL)) {
		msg_print(Ind, "\377yPlease don't litter the town with level 0 items which are unusable by other players. Use 'k' to destroy an item instead.");
		if (!is_admin(p_ptr)) return;
	}

	/* Let's not end afk for this - C. Blue */
/* 	un_afk_idle(Ind); */

#if (STARTEQ_TREATMENT > 1)
#ifndef RPG_SERVER
	if (o_ptr->owner == p_ptr->id && p_ptr->max_plv < cfg.newbies_cannot_drop && !is_admin(p_ptr) &&
	    !((o_ptr->tval == 1) && (o_ptr->sval >= 9)))
		o_ptr->level = 0;
#else
	if (o_ptr->owner == p_ptr->id && p_ptr->max_plv < 2 && !is_admin(p_ptr) &&
	    !((o_ptr->tval == 1) && (o_ptr->sval >= 9)))
		o_ptr->level = 0;
#endif 
#endif

	/* Take a partial turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;

	/* Drop (some of) the item */
	inven_drop(Ind, item, quantity);
}


/*
 * Drop some gold
 */
void do_cmd_drop_gold(int Ind, s32b amt)
{
	player_type *p_ptr = Players[Ind];

	object_type tmp_obj;

	/* Handle the newbies_cannot_drop option */
	if ((p_ptr->max_plv < cfg.newbies_cannot_drop) && !is_admin(p_ptr)) {
		msg_print(Ind, "You are not experienced enough to drop gold.");
		return;
	}

	if (p_ptr->inval) {
		msg_print(Ind, "You may not drop gold, wait for an admin to validate your account.");
		return;
	}

	/* Error checks */
	if (amt > p_ptr->au) {
		amt = p_ptr->au;
/*		msg_print(Ind, "You do not have that much gold.");
		return;
*/
	}
	if (amt <= 0) return;

	/* Setup the object */
	/* XXX Use "gold" object kind */
//	invcopy(&tmp_obj, 488);

	invcopy(&tmp_obj, gold_colour(amt));

	/* Setup the "worth" */
	tmp_obj.pval = amt;

	/* Hack -- 'own' the gold */
	tmp_obj.owner = p_ptr->id;

	/* Non-everlasting can't take money from everlasting
	   and vice versa, depending on server cfg. */
	tmp_obj.mode = p_ptr->mode;

	/* Drop it */
	drop_near(&tmp_obj, 0, &p_ptr->wpos, p_ptr->py, p_ptr->px);

	/* Subtract from the player's gold */
	p_ptr->au -= amt;

	/* Let's not end afk for this - C. Blue */
/* 	un_afk_idle(Ind); */

	/* Message */
//	msg_format(Ind, "You drop %ld pieces of gold.", amt);
	msg_format(Ind, "You drop %ld pieces of %s.", amt, k_name + k_info[tmp_obj.k_idx].name);

#ifdef USE_SOUND_2010
	sound(Ind, "drop_gold", NULL, SFX_TYPE_COMMAND, FALSE);
#endif

/* #if DEBUG_LEVEL > 3 */
//	if (amt >= 10000) {
		p_ptr->last_gold_drop += amt;
		if (turn - p_ptr->last_gold_drop_timer >= cfg.fps * 2) {
			s_printf("Gold dropped (%ld by %s at %d,%d,%d).\n", p_ptr->last_gold_drop, p_ptr->name, p_ptr->wpos.wx, p_ptr->wpos.wy, p_ptr->wpos.wz);
			p_ptr->last_gold_drop = 0;
			p_ptr->last_gold_drop_timer = turn;
		}
//	}

	break_cloaking(Ind, 5);
	break_shadow_running(Ind);
	stop_precision(Ind);
	stop_shooting_till_kill(Ind);

	/* Redraw gold */
	p_ptr->redraw |= (PR_GOLD);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);
}


/*
 * Destroy an item
 */
void do_cmd_destroy(int Ind, int item, int quantity)
{
	player_type *p_ptr = Players[Ind];

	int			old_number;

	//bool		force = FALSE;

	object_type		*o_ptr;

	char		o_name[ONAME_LEN];

	u32b f1, f2, f3, f4, f5, esp;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &(p_ptr->inventory[item]);
	}

	/* Get the item (on the floor) */
	else
	{
		if (-item >= o_max)
			return; /* item doesn't exist */

		o_ptr = &o_list[0 - item];
	}


	/* Describe the object */
	old_number = o_ptr->number;
	o_ptr->number = quantity;
	object_desc(Ind, o_name, o_ptr, TRUE, 3);
	o_ptr->number = old_number;

	if( check_guard_inscription( o_ptr->note, 'k' )) {
		msg_print(Ind, "The item's inscription prevents it.");
		return;
	};
#if 0
	/* Verify if needed */
	if (!force || other_query_flag)
	{
		/* Make a verification */
		snprintf(out_val, sizeof(out_val), "Really destroy %s? ", o_name);
		if (!get_check(Ind, out_val)) return;
	}
#endif

	/* Let's not end afk for this - C. Blue */
/* 	un_afk_idle(Ind); */

	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if ((f4 & TR4_CURSE_NO_DROP) && cursed_p(o_ptr) && !is_admin(p_ptr))
	{
		/* Oops */
		msg_print(Ind, "Hmmm, you seem to be unable to destroy it.");

		/* Nope */
		return;
	}
#ifndef FUN_SERVER /* while server is being hacked, allow this (while /wish is also allowed) - C. Blue */
	/* Artifacts cannot be destroyed */
	if (like_artifact_p(o_ptr) && !is_admin(p_ptr))
	{
		cptr feel = "special";

		/* Message */
		msg_format(Ind, "You cannot destroy %s.", o_name);

		/* Hack -- Handle icky artifacts */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) feel = "terrible";

		/* Hack -- inscribe the artifact */
		o_ptr->note = quark_add(feel);

		/* We have "felt" it (again) */
		o_ptr->ident |= (ID_SENSE | ID_SENSED_ONCE);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Done */
		return;
	}
#endif
	/* Keys cannot be destroyed */
	if (o_ptr->tval == TV_KEY && !is_admin(p_ptr))
	{
		/* Message */
		msg_format(Ind, "You cannot destroy %s.", o_name);

		/* Done */
		return;
	}

	/* Cursed, equipped items cannot be destroyed */
	if (item >= INVEN_WIELD && cursed_p(o_ptr) && !is_admin(p_ptr))
	{
		/* Message */
		msg_print(Ind, "Hmm, that seems to be cursed.");

		/* Done */
		return;
	}

	/* Polymorph back */
	/* XXX this can cause strange things for players with mimicry skill.. */
	if ((item == INVEN_LEFT || item == INVEN_RIGHT) && (o_ptr->tval == TV_RING) && (o_ptr->sval == SV_RING_POLYMORPH))
	{
		if ((p_ptr->body_monster == o_ptr->pval) &&
		   ((p_ptr->r_killed[p_ptr->body_monster] < r_info[p_ptr->body_monster].level) ||
		   (get_skill_scale(p_ptr, SKILL_MIMIC, 100) < r_info[p_ptr->body_monster].level)))
		{
			/* If player hasn't got high enough kill count anymore now, poly back to player form! */
#if 1
			msg_print(Ind, "You polymorph back to your normal form.");
			do_mimic_change(Ind, 0, TRUE);
#endif
			s_printf("DESTROY_EXPLOIT (poly): %s destroyed %s\n", p_ptr->name, o_name);
		}
	}

	/* Check if item gave WRAITH form */
	if((k_info[o_ptr->k_idx].flags3 & TR3_WRAITH) && p_ptr->tim_wraith) {
		s_printf("DESTROY_EXPLOIT (wraith): %s destroyed %s\n", p_ptr->name, o_name);
#if 1
		p_ptr->tim_wraith = 1;
#endif
	}

	/* Message */
	msg_format(Ind, "You destroy %s.", o_name);

#ifdef USE_SOUND_2010
//	sound_item(Ind, o_ptr->tval, o_ptr->sval, "kill_");
#endif

	if (true_artifact_p(o_ptr)) handle_art_d(o_ptr->name1);

	if (o_ptr->tval == TV_WAND) (void)divide_charged_item(o_ptr, quantity);

	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(Ind, item, -quantity);
		inven_item_describe(Ind, item);
		inven_item_optimize(Ind, item);
	}

	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -quantity);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	break_cloaking(Ind, 5);
	break_shadow_running(Ind);
	stop_precision(Ind);
	stop_shooting_till_kill(Ind);
}


/*
 * Observe an item which has been *identify*-ed
 */
void do_cmd_observe(int Ind, int item) {
	player_type *p_ptr = Players[Ind];
	object_type *o_ptr;

	/* Get the item (in the pack) */
	if (item >= 0) o_ptr = &(p_ptr->inventory[item]);
	/* Get the item (on the floor) */
	else {
		if (-item >= o_max) return; /* item doesn't exist */
		o_ptr = &o_list[0 - item];
	}

	/* Require full knowledge */
	if (!(o_ptr->ident & ID_MENTAL) && !is_admin(p_ptr)) observe_aux(Ind, o_ptr);
	/* Describe it fully */
	else if (!identify_fully_aux(Ind, o_ptr)) msg_print(Ind, "You see nothing special.");
}



/*
 * Remove the inscription from an object
 * XXX Mention item (when done)?
 */
void do_cmd_uninscribe(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];
	object_type *o_ptr;


	/* Get the item (in the pack) */
	if (item >= 0) o_ptr = &(p_ptr->inventory[item]);
	/* Get the item (on the floor) */
	else {
		if (-item >= o_max)
			return; /* item doesn't exist */

		o_ptr = &o_list[0 - item];
	}

	/* Nothing to remove */
	if (!o_ptr->note) {
		msg_print(Ind, "That item had no inscription to remove.");
		return;
	}

	/* small hack, make shirt logos permanent */
	if (o_ptr->tval == TV_SOFT_ARMOR && o_ptr->sval == SV_SHIRT && !is_admin(p_ptr)) {
		msg_print(Ind, "Cannot uninscribe shirts.");
		return;
	}
	if ((o_ptr->tval == TV_SPECIAL ||
	    (o_ptr->tval == TV_SCROLL && o_ptr->sval == SV_SCROLL_CHEQUE)
	    ) && !is_admin(p_ptr)) {
		msg_print(Ind, "Cannot uninscribe this item.");
		return;
	}

	/* Message */
	msg_print(Ind, "Inscription removed.");

	/* Remove the incription */
	o_ptr->note = 0;
	o_ptr->note_utag = 0;

	/* Combine the pack */
	p_ptr->notice |= (PN_COMBINE);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}


/*
 * Inscribe an object with a comment
 */
void do_cmd_inscribe(int Ind, int item, cptr inscription)
{
	player_type *p_ptr = Players[Ind];
	object_type		*o_ptr;
	char		o_name[ONAME_LEN];
	char *c;


	/* Get the item (in the pack) */
	if (item >= 0) o_ptr = &(p_ptr->inventory[item]);
	/* Get the item (on the floor) */
	else {
		if (-item >= o_max)
			return; /* item doesn't exist */

		o_ptr = &o_list[0 - item];
	}

	/* small hack, make shirt logos permanent */
	if (o_ptr->tval == TV_SOFT_ARMOR && o_ptr->sval == SV_SHIRT && !is_admin(p_ptr)) {
		msg_print(Ind, "Cannot inscribe shirts.");
		return;
	}
	if ((o_ptr->tval == TV_SPECIAL ||
	    (o_ptr->tval == TV_SCROLL && o_ptr->sval == SV_SCROLL_CHEQUE)
	    ) && !is_admin(p_ptr)) {
		msg_print(Ind, "Cannot inscribe this item.");
		return;
	}

	/* Describe the activity */
	object_desc(Ind, o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format(Ind, "Inscribing %s.", o_name);
	msg_print(Ind, NULL);

	/* small hack to prevent using colour codes in inscriptions:
	   convert \{ to just {, compare nserver.c:Send_special_line()!
	   Note: Colour codes in inscriptions/item names aren't implemented anyway. */
	if (!is_admin(p_ptr)) while ((c = strstr(inscription, "\\{")))
		c[0] = '{';

	/* hack to fix auto-inscriptions: convert empty inscription to a #-type inscription */
	if (inscription[0] == '\0') inscription = "#";

	/* Save the inscription */
	o_ptr->note = quark_add(inscription);
	o_ptr->note_utag = 0;

	/* Combine the pack */
	p_ptr->notice |= (PN_COMBINE);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}


/*
 * Steal an object from a monster
 */
/*
 * This is quite abusable.. you can rob Wormie, leave the floor, and
 * you'll meet him again and again... so I stop implementing this.
 * - Jir -
 */
void do_cmd_steal_from_monster(int Ind, int dir)
{
#if 0
	player_type *p_ptr = Players[Ind], *q_ptr;
	cave_type **zcave;
	int x, y, dir = 0, item = -1, k = -1;
	cave_type *c_ptr;
	monster_type *m_ptr;
	object_type *o_ptr, forge;
	byte num = 0;
	bool done = FALSE;
	int monst_list[23];

	if(!(zcave = getcave(&p_ptr->wpos))) return;

        /* Ghosts cannot steal ; not in WRAITHFORM */
        if (p_ptr->ghost || p_ptr->tim_wraith) {
                msg_print(Ind, "You cannot steal things!");
                return;
        }


	/* Only works on adjacent monsters */
	if (!get_rep_dir(&dir)) return;
	y = py + ddy[dir];
	x = px + ddx[dir];
	c_ptr = &cave[y][x];

	if (!(c_ptr->m_idx)) {
		msg_print("There is no monster there!");
		return;
	}

	m_ptr = &m_list[c_ptr->m_idx];

	break_shadow_running(Ind);
	stop_precision(Ind);
	stop_shooting_till_kill(Ind);

	/* There were no non-gold items */
	if (!m_ptr->hold_o_idx) {
		msg_print("That monster has no objects!");
		return;
	}

#if 0
	/* not in WRAITHFORM */
	if (p_ptr->tim_wraith) {
		msg_print("You can't grab anything!");
		return;
	}

	/* The monster is immune */
	if (r_info[m_ptr->r_idx].flags7 & (RF7_NO_THEFT)) {
		msg_print("The monster is guarding the treasures.");
		return;
	}

	screen_save();

	num = show_monster_inven(c_ptr->m_idx, monst_list);

	/* Repeat until done */
	while (!done) {
		char tmp_val[80];
		char which = ' ';

		/* Build the prompt */
		strnfmt(tmp_val, MAX_CHARS, "Choose an item to steal (a-%c) or ESC:",
				'a' - 1 + num);

		/* Show the prompt */
		prt(tmp_val, 0, 0);

		/* Get a key */
		which = inkey();

		/* Parse it */
		switch (which) {
		case ESCAPE:
			done = TRUE;
			break;

		default:
			int ver;

			/* Extract "query" setting */
			ver = isupper(which);
			which = tolower(which);

			k = islower(which) ? A2I(which) : -1;
			if (k < 0 || k >= num) {
				bell();
				break;
			}

			/* Verify the item */
			if (ver && !verify("Try", 0 - monst_list[k])) {
				done = TRUE;
				break;
			}

			/* Accept that choice */
			item = monst_list[k];
			done = TRUE;

			break;
		}
	}
#endif	// 0

	/* S(he) is no longer afk */
	un_afk_idle(Ind);

	if (item != -1) {
		int chance;

		chance = 40 - p_ptr->stat_ind[A_DEX];
		chance +=
			o_list[item].weight / (get_skill_scale(SKILL_STEALING, 19) + 1);
		chance += get_skill_scale(SKILL_STEALING, 29) + 1;
		chance -= (m_ptr->csleep) ? 10 : 0;
		chance += m_ptr->level;

		/* Failure check */
		if (rand_int(chance) > 1 + get_skill_scale(SKILL_STEALING, 25)) {
			/* Take a turn */
			energy_use = 100;

			/* Wake up */
			m_ptr->csleep = 0;

			/* Speed up because monsters are ANGRY when you try to thief them */
			if (m_ptr->mspeed < m_ptr->speed + 15)
				m_ptr->mspeed += 5; m_ptr->speed += 5;
			screen_load();
			break_cloaking(Ind, 0);
			msg_print("Oops ! The monster is now really *ANGRY*.");
			return;
		}

		/* Reconnect the objects list */
		if (num == 1) m_ptr->hold_o_idx = 0;
		else {
			if (k > 0) o_list[monst_list[k - 1]].next_o_idx = monst_list[k + 1];
			if (k + 1 >= num) o_list[monst_list[k - 1]].next_o_idx = 0;
			if (k == 0) m_ptr->hold_o_idx = monst_list[k + 1];
		}

		/* Rogues gain some xp */
		if (PRACE_FLAGS(PR1_EASE_STEAL)) {
			s32b max_point;

			/* Max XP gained from stealing */
			max_point = (o_list[item].weight / 2) + (m_ptr->level * 10);

			/* Randomise it a bit, with half a max guaranteed */
			gain_exp((max_point / 2) + (randint(max_point) / 2));

			/* Allow escape */
			if (get_check("Phase door?")) teleport_player(Ind, 10, TRUE);
		}

		/* Get the item */
		o_ptr = &forge;

		/* Special handling for gold */
		if (o_list[item].tval == TV_GOLD) {
			gain_au(Ind, o_ptr->pval, FALSE, FALSE);
			p_ptr->window |= (PW_PLAYER);
		} else {
			object_copy(o_ptr, &o_list[item]);
			inven_carry(o_ptr, FALSE);
		}

		/* Delete it */
		invwipe(&o_list[item]);
	}

	screen_load();

	/* Take a turn */
	energy_use = 100;
#endif	// 0
}


/*
 * Attempt to steal from another player
 */
/* TODO: Make it possible to steal from monsters.. */
void do_cmd_steal(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind], *q_ptr;
	cave_type *c_ptr;

	int success, notice;
	bool caught = FALSE;
	cave_type **zcave;
	u16b dal;
	if(!(zcave=getcave(&p_ptr->wpos))) return;

	/* Ghosts cannot steal */
#if 0 /* changed since RPG_SERVER */
	if ((p_ptr->ghost) || cfg.use_pk_rules == PK_RULES_NEVER || 
		(!(p_ptr->pkill & PKILL_KILLABLE) && cfg.use_pk_rules == PK_RULES_DECLARE))
	{
	        msg_print(Ind, "You cannot steal things!");
	        return;
	}	                                                        
#else
	/* not in WRAITHFORM either */
	if (p_ptr->ghost || p_ptr->tim_wraith) {
	        msg_print(Ind, "You cannot steal things!");
	        return;
	}
#endif

	/* May not steal from yourself */
	if (!dir || dir == 5) return;

	/* Examine target grid */
	c_ptr = &zcave[p_ptr->py + ddy[dir]][p_ptr->px + ddx[dir]];

	/* May only steal from players */
	if (c_ptr->m_idx >= 0) {
		msg_print(Ind, "You see nothing there to steal from.");
		return;
	}
	else if (c_ptr->m_idx > 0) {
		do_cmd_steal_from_monster(Ind, dir);
		return;
	}

	/* S(he) is no longer afk */
	un_afk_idle(Ind);

	/* Examine target */
	q_ptr = Players[0 - c_ptr->m_idx];

	/* No transactions from different mode */
	if (compat_pmode(Ind, 0 - c_ptr->m_idx, FALSE)) {
		msg_format(Ind, "You cannot steal from %s players.", compat_pmode(Ind, 0 - c_ptr->m_idx, FALSE));
		return;
	}

        /* Small delay to prevent crazy steal-spam */
        if (p_ptr->pstealing) {
                msg_print(Ind, "You're still not calm enough to steal again..");
                return;
        }

#if 0
	/* no stealing in town since town-pvp is diabled */
	if (istown(&p_ptr->wpos)) {
		msg_print(Ind, "\337rYou may not steal in town.");
		return;
	}
#endif

	/* May not steal from AFK players, sportsmanship ;) - C. Blue */
	if (q_ptr->afk) {
		msg_print(Ind, "You may not steal from players who are AFK.");
		return;
	}

	/* May not steal from hostile players */
	/* I doubt if it's reasonable..dunno	- Jir - */
#if 0 /* turned off now */
	if (check_hostile(0 - c_ptr->m_idx, Ind)) {
		/* Message */
		msg_format(Ind, "%^s is on guard against you.", q_ptr->name);
		return;
	}
#endif
	dal = (p_ptr->lev > q_ptr->lev ? p_ptr->lev - q_ptr->lev : 1);

	/* affect alignment on attempt (after hostile check) */
	/* evil thief! stealing from newbies */
	if (q_ptr->lev + 5 < p_ptr->lev){
		if((p_ptr->align_good) < (0xffff - dal))
			p_ptr->align_good += dal;
		else p_ptr->align_good = 0xffff;	/* very evil */
	}
	/* non lawful action in town :) */
	if (istown(&p_ptr->wpos) && (p_ptr->align_law) < (0xffff - dal))
		p_ptr->align_law += dal;
	else p_ptr->align_law = 0xffff;

	/* Make sure we have enough room */
	if (p_ptr->inven_cnt >= INVEN_PACK) {
		msg_print(Ind, "You have no room to steal anything.");
		return;
	}

	break_shadow_running(Ind);
	stop_precision(Ind);
	stop_shooting_till_kill(Ind);

#if 1 /* maybe rework this */
	/* Compute chance of success */
	success = 3 * (adj_dex_safe[p_ptr->stat_ind[A_DEX]] - adj_dex_safe[q_ptr->stat_ind[A_DEX]]);
	success += 2 * (UNAWARENESS(q_ptr) - UNAWARENESS(p_ptr));

	/* Compute base chance of being noticed */
	notice = 5 * (adj_mag_stat[q_ptr->stat_ind[A_INT]] - p_ptr->skill_stl);

	/* Reversed this as suggested by Potter - mikaelh */
	notice -= 1 * (UNAWARENESS(q_ptr) - UNAWARENESS(p_ptr));

//	notice -= q_ptr->skill_fos; /* perception */

	/* Hack -- Rogues get bonuses to chances */
	if (get_skill(p_ptr, SKILL_STEALING)) {
		/* Increase chance by level */
		success += get_skill_scale(p_ptr, SKILL_STEALING, 150);
		notice -= get_skill_scale(p_ptr, SKILL_STEALING, 150);
	}
	/* Similar Hack -- Robber is hard to be robbed */
	if (get_skill(q_ptr, SKILL_STEALING)) {
		/* Increase chance by level */
		success -= get_skill_scale(p_ptr, SKILL_STEALING, 100);
		notice += get_skill_scale(p_ptr, SKILL_STEALING, 100);
	}

	/* Always small chance to fail */
	if (success > 95) success = 95;
//	if (notice < 5) notice = 5;

	/* Hack -- Always small chance to succeed */
	if (success < 2) success = 2;
#else
#endif

	/* Check for success */
	if (rand_int(100) < success) {
		/* Steal gold 25% of the time */
		if (rand_int(100) < 25) {
			int amt = q_ptr->au / 10;

			if (TOOL_EQUIPPED(q_ptr) == SV_TOOL_MONEY_BELT && magik (70)) {
				/* Saving throw message */
				msg_print(Ind, "You couldn't find any money!");
				amt = 0;
				s_printf("StealingPvP: %s fails to steal %d gold from %s (chance %d%%): money belt.\n", p_ptr->name, amt, q_ptr->name, success);
			}

			/* Transfer gold */
			if (amt) {
				/* Move from target to thief */
				q_ptr->au -= amt;
				gain_au(Ind, amt, FALSE, FALSE);
				/* Redraw */
				q_ptr->redraw |= (PR_GOLD);

				/* Tell thief */
				msg_format(Ind, "You steal %ld gold.", amt);
				s_printf("StealingPvP: %s steals %d gold from %s (chance %d%%).\n", p_ptr->name, amt, q_ptr->name, success);
			}

			/* Always small chance to be noticed */
			if (notice < 5) notice = 5;

			/* Check for target noticing */
			if (rand_int(100) < notice) {
				/* Message */
				msg_format(0 - c_ptr->m_idx, "\377rYou notice %s stealing %ld gold!",
						p_ptr->name, amt);
				caught = TRUE;
			}
		} else {
			int item;
			object_type *o_ptr, forge;
			char o_name[ONAME_LEN];

			/* Steal an item */
			item = rand_int(q_ptr->inven_cnt);

			/* Get object */
			o_ptr = &q_ptr->inventory[item];
			forge = *o_ptr;

			if (TOOL_EQUIPPED(q_ptr) == SV_TOOL_THEFT_PREVENTION && magik (80)) {
				/* Saving throw message */
				msg_print(Ind, "Your attempt to steal was interfered with by a strange device!");
				notice += 50;
				s_printf("StealingPvP: %s fails to steal from %s (chance %d%%): theft prevention.\n", p_ptr->name, q_ptr->name, success);
			}
			/* True artifact is HARD to steal */
			else if (cfg.anti_arts_hoard && true_artifact_p(o_ptr)
			    && ((q_ptr->exp > p_ptr->exp) || (rand_int(500) > success))) {
				msg_print(Ind, "The object itself seems to evade your hand!");
				s_printf("StealingPvP: %s fails to steal from %s (chance %d%%): true artifact.\n", p_ptr->name, q_ptr->name, success);
			} else {
				/* Give one item to thief */
				if (o_ptr->tval == TV_WAND) forge.pval = divide_charged_item(o_ptr, 1);
				forge.number = 1;
				inven_carry(Ind, &forge);

				/* Take one from target */
				inven_item_increase(0 - c_ptr->m_idx, item, -1);
				inven_item_optimize(0 - c_ptr->m_idx, item);

				/* Tell thief what he got */
				object_desc(0, o_name, &forge, TRUE, 3);
				s_printf("StealingPvP: %s steals item %s from %s (chance %d%%).\n", p_ptr->name, o_name, q_ptr->name, success);
				object_desc(Ind, o_name, &forge, TRUE, 3);
				msg_format(Ind, "You stole %s.", o_name);
			}

			/* Easier to notice heavier objects */
			notice += forge.weight;
//			/ (get_skill_scale(SKILL_STEALING, 19) + 1);

			/* Always small chance to be noticed */
			if (notice < 5) notice = 5;

			/* Check for target noticing */
			if (rand_int(100) < notice) {
				/* Message */
				msg_format(0 - c_ptr->m_idx, "\377rYou notice %s stealing %s!",
				           p_ptr->name, o_name);
				caught = TRUE;
			}
		}
	} else {
		/* Message */
		msg_print(Ind, "You fail to steal anything.");
		s_printf("StealingPvP: %s fails to steal from %s.\n", p_ptr->name, q_ptr->name);

		/* Always small chance to be noticed */
		if (notice < 5) notice = 5;

		/* Easier to notice a failed attempt */
		if (rand_int(100) < notice + 50) {
			/* Message */
			msg_format(0 - c_ptr->m_idx, "\377rYou notice %s try to steal from you!",
			           p_ptr->name);
			caught = TRUE;
		}
	}

	if (caught) break_cloaking(Ind, 0);

#if 0 /* now turned off */
	/* Counter blow! */
	if (caught) {
		int i, j;
		object_type *o_ptr;

		/* Purge this traitor */
		if (player_in_party(q_ptr->party, Ind)) {
			int party=p_ptr->party;

			/* Temporary leave for the message */
			p_ptr->party = 0;

			/* Messages */
			msg_print(Ind, "You have been purged from your party.");
			party_msg_format(q_ptr->party, "%s has betrayed your party!", p_ptr->name);

			p_ptr->party=party;
			party_leave(Ind);

		}

		/* Make target hostile */
		if (q_ptr->exp > p_ptr->exp / 2 - 200) {
			if (Players[0 - c_ptr->m_idx]->pvpexception < 2)
			add_hostility(0 - c_ptr->m_idx, p_ptr->name, FALSE);
		}

		/* Message */
		msg_format(Ind, "\377r%s gave you an unexpected blow!",
		           q_ptr->name);

		set_stun(Ind, p_ptr->stun + randint(50));
		set_confused(Ind, p_ptr->confused + rand_int(20) + 10);
		if (cfg.use_pk_rules == PK_RULES_DECLARE)
			p_ptr->pkill|=PKILL_KILLABLE;

		/* Thief drops some items from the shock of blow */
		if (cfg.newbies_cannot_drop <= p_ptr->lev && !p_ptr->inval) {
			for(i = rand_int(5); i < 5 ; i++ ) {
				j = rand_int(INVEN_TOTAL);
				o_ptr = &(p_ptr->inventory[j]);

				if (!o_ptr->tval) continue;

				/* He never drops body-armour this way */
				if (j == INVEN_BODY) continue;

				/* An artifact 'resists' */
				if (true_artifact_p(o_ptr) && rand_int(100) > 2)
					continue;
				inven_drop(Ind, j, randint(o_ptr->number * 2));
			}
		}

		/* The target gets angry */
		set_fury(0 - c_ptr->m_idx, q_ptr->fury + 15 + randint(15));

	}
#else
	/* set timeout before attempting to pvp-steal again */
	p_ptr->pstealing = 15; /* 15 turns aka ~10 seconds */
#endif

	/* Take a turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos);
}







/*
 * An "item_tester_hook" for refilling lanterns
 */
static bool item_tester_refill_lantern(object_type *o_ptr)
{
	/* (Rand)arts are not usable for refilling */
	if (o_ptr->name1) return (FALSE);

	/* Flasks of oil are okay */
	if (o_ptr->tval == TV_FLASK) return (TRUE);

	/* Other lanterns are okay */
	if ((o_ptr->tval == TV_LITE) &&
	    (o_ptr->sval == SV_LITE_LANTERN) &&
	    o_ptr->timeout)
		return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;
	object_type *j_ptr;
	
	long int used_fuel = 0, spilled_fuel = 0, available_fuel = 0;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_lantern;

	/* Get the item (in the pack) */
	if (item >= 0) o_ptr = &(p_ptr->inventory[item]);
	/* Get the item (on the floor) */
	else {
		if (-item >= o_max) return; /* item doesn't exist */
		o_ptr = &o_list[0 - item];
	}

	if (!item_tester_hook(o_ptr)) {
		msg_print(Ind, "You cannot refill with that!");
		return;
	}

	/* Too kind? :) */
	if (artifact_p(o_ptr)) {
		msg_print(Ind, "Your light seems to resist!");
		return;
	}

	if (o_ptr->tval != TV_FLASK && o_ptr->timeout == 0) {
		msg_print(Ind, "That item has no fuel left!");
		return;
	}

	/* Let's not end afk for this. - C. Blue */
/*	un_afk_idle(Ind); */

	/* Take a partial turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;

	/* Access the lantern */
	j_ptr = &(p_ptr->inventory[INVEN_LITE]);

	/* Refuel */
	used_fuel = j_ptr->timeout;
	if (o_ptr->tval == TV_FLASK) {
		j_ptr->timeout += o_ptr->pval;
	} else {
		spilled_fuel = (o_ptr->timeout * (randint(5) + (130 - adj_dex_th[p_ptr->stat_ind[A_DEX]]) / 2)) / 100; /* spill some */
		available_fuel = o_ptr->timeout - spilled_fuel;
		j_ptr->timeout += available_fuel;
	}

	/* Message */
	msg_print(Ind, "You fuel your lamp.");

	/* Comment */
	if (j_ptr->timeout >= FUEL_LAMP) {
		j_ptr->timeout = FUEL_LAMP;
		msg_print(Ind, "Your lamp is full.");
	}
	used_fuel = j_ptr->timeout - used_fuel;

	if (o_ptr->tval == TV_FLASK || item <= 0) { /* just dispose of for now, if it was from the ground */
		/* Decrease the item (from the pack) */
		if (item >= 0) {
			inven_item_increase(Ind, item, -1);
			inven_item_describe(Ind, item);
			inven_item_optimize(Ind, item);
		}

		/* Decrease the item (from the floor) */
		else {
			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
	} else { /* TV_LITE && SV_LITE_LANTERN */
		/* unstack lanterns if we used one of them for refilling */
		if ((item >= 0) && (o_ptr->number > 1)) {
			/* Make a fake item */
			object_type tmp_obj;
			tmp_obj = *o_ptr;
			tmp_obj.number = 1;

			/* drain its fuel */
			/* big numbers (*1000) to fix rounding errors: */
			tmp_obj.timeout -= ((used_fuel * 100000) / (100000 - ((100000 * spilled_fuel) / tmp_obj.timeout)));
			/* quick hack to hopefully fix rounding errors: */
			if (tmp_obj.timeout == 1) tmp_obj.timeout = 0;

			/* Unstack the used item */
			o_ptr->number--;
			p_ptr->total_weight -= tmp_obj.weight;
			item = inven_carry(Ind, &tmp_obj);

			/* Message */
			msg_print(Ind, "You unstack your lanterns.");
		} else {
			/* big numbers (*1000) to fix rounding errors: */
			o_ptr->timeout -= ((used_fuel * 100000) / (100000 - ((100000 * spilled_fuel) / o_ptr->timeout)));
			/* quick hack to hopefully fix rounding errors: */
			if (o_ptr->timeout == 1) o_ptr->timeout = 0;
//			inven_item_describe(Ind, item);
		}
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH | PU_LITE);
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
	/* If multiple lanterns are now 0 turns, they can be combined */
	p_ptr->notice |= (PN_COMBINE);
}



/*
 * An "item_tester_hook" for refilling torches
 */
static bool item_tester_refill_torch(object_type *o_ptr)
{
	/* (Rand)arts are not usable for refilling */
	if (o_ptr->name1) return (FALSE);

	/* Torches are okay */
	if ((o_ptr->tval == TV_LITE) &&
	    (o_ptr->sval == SV_LITE_TORCH) &&
	    o_ptr->timeout)
		return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
 * Refuel the players torch (from the pack or floor)
 */
static void do_cmd_refill_torch(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;
	object_type *j_ptr;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_torch;

	if (item > INVEN_TOTAL) return;

	/* Get the item (in the pack) */
	if (item >= 0) {
		o_ptr = &(p_ptr->inventory[item]);
	}

	/* Get the item (on the floor) */
	else {
		if (-item >= o_max)
			return; /* item doesn't exist */

		o_ptr = &o_list[0 - item];
	}

	if (!item_tester_hook(o_ptr)) {
		msg_print(Ind, "You cannot refill with that!");
		return;
	}

	/* Too kind? :) */
	if (artifact_p(o_ptr)) {
		msg_print(Ind, "Your light seems to resist!");
		return;
	}

	/* Let's not end afk for this. - C. Blue */
/*	un_afk_idle(Ind); */

	/* Take a partial turn */
	p_ptr->energy -= level_speed(&p_ptr->wpos) / 2;

	/* Access the primary torch */
	j_ptr = &(p_ptr->inventory[INVEN_LITE]);

	/* Refuel */
	j_ptr->timeout += o_ptr->timeout + 5;

	/* Message */
	msg_print(Ind, "You combine the torches.");

	/* Over-fuel message */
	if (j_ptr->timeout >= FUEL_TORCH) {
		j_ptr->timeout = FUEL_TORCH;
		msg_print(Ind, "Your torch is fully fueled.");
	}

	/* Refuel message */
	else {
		msg_print(Ind, "Your torch glows more brightly.");
	}

	/* Decrease the item (from the pack) */
	if (item >= 0) {
		inven_item_increase(Ind, item, -1);
		inven_item_describe(Ind, item);
		inven_item_optimize(Ind, item);
	}

	/* Decrease the item (from the floor) */
	else {
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}




/*
 * Refill the players lamp, or restock his torches
 */
void do_cmd_refill(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;
	u32b f1, f2, f3, f4, f5, esp;

	/* Get the light */
	o_ptr = &(p_ptr->inventory[INVEN_LITE]);

	if (check_guard_inscription( o_ptr->note, 'F' )) {
		msg_print(Ind, "The item's incription prevents it.");
		return;
	}

	/* It is nothing */
	if (o_ptr->tval != TV_LITE || !o_ptr->k_idx) {
		msg_print(Ind, "You are not wielding a light.");
		return;
	}

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if (!(f4 & TR4_FUEL_LITE)) {
		msg_print(Ind, "Your light cannot be refilled.");
		return;
	}

	/* It's a lamp */
	else if (o_ptr->sval == SV_LITE_LANTERN) {
		do_cmd_refill_lamp(Ind, item);
	}

	/* It's a torch */
	else if (o_ptr->sval == SV_LITE_TORCH) {
		do_cmd_refill_torch(Ind, item);
	}

	/* No torch to refill */
	else {
		msg_print(Ind, "Your light cannot be refilled.");
	}
}

/*
 * Refill the players lamp, or restock his torches automatically.	- Jir -
 */
bool do_auto_refill(int Ind)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;
	object_type *j_ptr = NULL;

	int i;
	u32b f1, f2, f3, f4, f5, esp;

	/* Get the light */
	o_ptr = &(p_ptr->inventory[INVEN_LITE]);

	if( check_guard_inscription( o_ptr->note, 'F' )) {
		//		msg_print(Ind, "The item's incription prevents it.");
		return (FALSE);
	}

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* It's a lamp */
	if (o_ptr->sval == SV_LITE_LANTERN) {
		/* Restrict the choices */
		item_tester_hook = item_tester_refill_lantern;

		for(i = 0; i < INVEN_PACK; i++) {
			j_ptr = &(p_ptr->inventory[i]);
			if (!item_tester_hook(j_ptr)) continue;
			if (artifact_p(j_ptr) || ego_item_p(j_ptr)) continue;

			do_cmd_refill_lamp(Ind, i);
			return (TRUE);
		}
	}

	/* It's a torch */
	else if (o_ptr->sval == SV_LITE_TORCH) {
		/* Restrict the choices */
		item_tester_hook = item_tester_refill_torch;

		for(i = 0; i < INVEN_PACK; i++) {
			j_ptr = &(p_ptr->inventory[i]);
			if (!item_tester_hook(j_ptr)) continue;
			if (artifact_p(j_ptr) || ego_item_p(j_ptr)) continue;

			do_cmd_refill_torch(Ind, i);
			return (TRUE);
		}
	}

	/* Assume false */
	return (FALSE);
}





/*
 * Target command
 */
void do_cmd_target(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	/* Set the target */
	if (target_set(Ind, dir) && !p_ptr->taciturn_messages) {
//		msg_print(Ind, "Target Selected.");
	} else {
		/*msg_print(Ind, "Target Aborted.");*/
	}
}

void do_cmd_target_friendly(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	/* Set the target */
	if (target_set_friendly(Ind, dir) && !p_ptr->taciturn_messages) {
//		msg_print(Ind, "Target Selected.");
	} else {
		/*msg_print(Ind, "Target Aborted.");*/
	}
}


/*
 * Hack -- determine if a given location is "interesting" to a player
 */
static bool do_cmd_look_accept(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	struct worldpos *wpos=&p_ptr->wpos;
	cave_type **zcave;

	cave_type *c_ptr;
	byte *w_ptr;
	struct c_special *cs_ptr;

	if(!(zcave=getcave(wpos))) return(FALSE);

	/* Examine the grid */
	c_ptr = &zcave[y][x];
	w_ptr = &p_ptr->cave_flag[y][x];

	/* Player grids */
	if (c_ptr->m_idx < 0 && p_ptr->play_vis[0-c_ptr->m_idx]) {
		player_type *q_ptr=Players[0-c_ptr->m_idx];
		if ((!q_ptr->admin_dm || player_sees_dm(Ind)) && 
		    (player_has_los_bold(Ind, y, x) || p_ptr->telepathy))
			return (TRUE);
	}

	/* Visible monsters */
	if (c_ptr->m_idx > 0 && p_ptr->mon_vis[c_ptr->m_idx]) {
		/* Visible monsters */
		if (p_ptr->mon_vis[c_ptr->m_idx]) return (TRUE);
	}

	/* Objects */
	if (c_ptr->o_idx) {
		/* Memorized object */
		if (p_ptr->obj_vis[c_ptr->o_idx] || p_ptr->admin_dm) return (TRUE); /* finally, poor dm - C. Blue */
	}

	/* Traps */
	if((cs_ptr=GetCS(c_ptr, CS_TRAPS))){
		/* Revealed trap */
		if (cs_ptr->sc.trap.found) return (TRUE);
	}

	/* Monster Traps */
	if(GetCS(c_ptr, CS_MON_TRAP) || GetCS(c_ptr, CS_RUNE_TRAP)){
		return (TRUE);
	}

	/* Interesting memorized features */
	if (*w_ptr & CAVE_MARK) {
#if 0	// wow!
		/* Notice glyphs */
		if (c_ptr->feat == FEAT_GLYPH) return (TRUE);

		/* Notice doors */
		if (c_ptr->feat == FEAT_OPEN) return (TRUE);
		if (c_ptr->feat == FEAT_BROKEN) return (TRUE);

		/* Notice stairs */
		if (c_ptr->feat == FEAT_LESS) return (TRUE);
		if (c_ptr->feat == FEAT_MORE) return (TRUE);

		/* Notice shops */
		if (c_ptr->feat == FEAT_SHOP) return (TRUE);
#if 0
		if ((c_ptr->feat >= FEAT_SHOP_HEAD) &&
		    (c_ptr->feat <= FEAT_SHOP_TAIL)) return (TRUE);
#endif	// 0

		/* Notice doors */
		if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		    (c_ptr->feat <= FEAT_DOOR_TAIL)) return (TRUE);

		/* Notice rubble */
		if (c_ptr->feat == FEAT_RUBBLE) return (TRUE);

		/* Notice veins with treasure */
		if (c_ptr->feat == FEAT_MAGMA_K) return (TRUE);
		if (c_ptr->feat == FEAT_QUARTZ_K) return (TRUE);
#endif	// 0

		/* Accept 'naturally' interesting features */
		if (f_info[c_ptr->feat].flags1 & FF1_NOTICE) return (TRUE);
	}

	/* Nope */
	return (FALSE);
}



/*
 * A new "look" command, similar to the "target" command.
 *
 * The "player grid" is always included in the "look" array, so
 * that this command will normally never "fail".
 *
 * XXX XXX XXX Allow "target" inside the "look" command (?)
 */
void do_cmd_look(int Ind, int dir) {
	player_type *p_ptr = Players[Ind];
	player_type *q_ptr;
	struct worldpos *wpos=&p_ptr->wpos;
	cave_type **zcave;
	int		y, x, i;

	cave_type *c_ptr;
	monster_type *m_ptr;
	object_type *o_ptr;

	cptr p1 = "A ", p2 = "", info = "";
	struct c_special *cs_ptr;

	char o_name[ONAME_LEN];
	char out_val[ONAME_LEN], tmp_val[ONAME_LEN];

	if(!(zcave = getcave(wpos))) return;

	/* Blind */
	if (p_ptr->blind) {
		msg_print(Ind, "You can't see a damn thing!");
		return;
	}

	/* Hallucinating */
	if (p_ptr->image) {
		msg_print(Ind, "You can't believe what you are seeing!");
		return;
	}


	/* Reset "temp" array */
	/* Only if this is the first time, or if we've been asked to reset */
	if (!dir) {
		p_ptr->target_n = 0;

		/* Scan the current panel */
		for (y = p_ptr->panel_row_min; y <= p_ptr->panel_row_max; y++) {
			for (x = p_ptr->panel_col_min; x <= p_ptr->panel_col_max; x++) {
				/* Require line of sight, unless "look" is "expanded" */
				if (!player_has_los_bold(Ind, y, x)) continue;

				/* Require interesting contents */
				if (!do_cmd_look_accept(Ind, y, x)) continue;

				/* Save the location */
				p_ptr->target_x[p_ptr->target_n] = x;
				p_ptr->target_y[p_ptr->target_n] = y;
				p_ptr->target_n++;
			}
		}

		/* Start near the player */
		p_ptr->look_index = 0;

		/* Paranoia */
		if (!p_ptr->target_n) {
			msg_print(Ind, "You see nothing special. [<dir>, p, q]");
			return;
		}


		/* Set the sort hooks */
		ang_sort_comp = ang_sort_comp_distance;
		ang_sort_swap = ang_sort_swap_distance;

		/* Sort the positions */
		ang_sort(Ind, p_ptr->target_x, p_ptr->target_y, p_ptr->target_n);

		/* Collect monster and player indices */
		for (i = 0; i < p_ptr->target_n; i++) {
			c_ptr = &zcave[p_ptr->target_y[i]][p_ptr->target_x[i]];

			if (c_ptr->m_idx != 0)
				p_ptr->target_idx[i] = c_ptr->m_idx;
			else p_ptr->target_idx[i] = 0;
		}

        } else if (dir >= 128) {
		/* Initialize if needed */
		if (dir == 128) {
			x = p_ptr->target_col = p_ptr->px;
			y = p_ptr->target_row = p_ptr->py;
		} else {
			x = p_ptr->target_col + ddx[dir - 128];
			if (!player_has_los_bold(Ind, p_ptr->target_row, x) && !is_admin(p_ptr)) x = p_ptr->target_col;
			y = p_ptr->target_row + ddy[dir - 128];
			if (!player_has_los_bold(Ind, y, p_ptr->target_col) && !is_admin(p_ptr)) y = p_ptr->target_row;

			if (!in_bounds(y, x)) return;

			p_ptr->target_col = x;
			p_ptr->target_row = y;
		}

		/* Check for confirmation to actually look at this grid */
		if (dir == 128 + 5) {
			p_ptr->look_index = 0;
			p_ptr->target_x[0] = p_ptr->target_col;
			p_ptr->target_y[0] = p_ptr->target_row;
		}
		/* Just manually ground-targetting - do not spam look-info for each single grid */
		else {
			/* Info */
			strcpy(out_val, "[<dir>, l, p, q] ");

			/* Tell the client */
			Send_target_info(Ind, x - p_ptr->panel_col_prt, y - p_ptr->panel_row_prt, out_val);

			return;
		}
	}

	/* Motion */
	else {
		/* Reset the locations */
		for (i = 0; i < p_ptr->target_n; i++) {
			if (p_ptr->target_idx[i] > 0) {
				m_ptr = &m_list[p_ptr->target_idx[i]];

				p_ptr->target_y[i] = m_ptr->fy;
				p_ptr->target_x[i] = m_ptr->fx;
			} else if (p_ptr->target_idx[i] < 0) {
				q_ptr = Players[0 - p_ptr->target_idx[i]];

				/* Check for player leaving */
				if (((0 - p_ptr->target_idx[i]) > NumPlayers) ||
				     (!inarea(&q_ptr->wpos, &p_ptr->wpos))) {
					p_ptr->target_y[i] = 0;
					p_ptr->target_x[i] = 0;
				} else {
					p_ptr->target_y[i] = q_ptr->py;
					p_ptr->target_x[i] = q_ptr->px;
				}
			}
		}

		/* Find a new grid if possible */
		i = target_pick(Ind, p_ptr->target_y[p_ptr->look_index], p_ptr->target_x[p_ptr->look_index], ddy[dir], ddx[dir]);

		/* Use that grid */
		if (i >= 0) p_ptr->look_index = i;
	}

	/* Describe */
	y = p_ptr->target_y[p_ptr->look_index];
	x = p_ptr->target_x[p_ptr->look_index];

	/* Paranoia */
	/* thats extreme paranoia */
	if(!(zcave=getcave(wpos))) return;
	c_ptr = &zcave[y][x];

	if (c_ptr->m_idx < 0 && p_ptr->play_vis[0-c_ptr->m_idx] &&
	    (!Players[0-c_ptr->m_idx]->admin_dm || player_sees_dm(Ind))) {
		q_ptr = Players[0 - c_ptr->m_idx];

		/* Track health */
		health_track(Ind, c_ptr->m_idx);

		/* Format string */
		if (q_ptr->body_monster) {
			snprintf(out_val, sizeof(out_val), "%s the %s (%s)", q_ptr->name, r_name + r_info[q_ptr->body_monster].name, get_ptitle(q_ptr, FALSE));
		} else {
#if 0 /* use normal race_info.title */
			snprintf(out_val, sizeof(out_val), "%s the %s %s", q_ptr->name, race_info[q_ptr->prace].title, get_ptitle(q_ptr, FALSE));
			//, class_info[q_ptr->pclass].title
#else /* use special_prace_lookup */
			snprintf(out_val, sizeof(out_val), "%s the %s %s", q_ptr->name, get_prace(q_ptr), get_ptitle(q_ptr, FALSE));
#endif
		}
	} else if (c_ptr->m_idx > 0 && p_ptr->mon_vis[c_ptr->m_idx]) {	/* TODO: handle monster mimics */
		bool done_unique;
		m_ptr = &m_list[c_ptr->m_idx];

		/* a unique which the looker already killed? */
		if ((r_info[m_ptr->r_idx].flags1 & RF1_UNIQUE) &&
		    p_ptr->r_killed[m_ptr->r_idx] == 1)
			done_unique = TRUE;
		else done_unique = FALSE;

		/* Track health */
		health_track(Ind, c_ptr->m_idx);

		/* Format string */
#if 0 /* attach 'slain' for uniques we already killed */
//                snprintf(out_val, sizeof(out_val), "%s (%s)", r_name_get(&m_list[c_ptr->m_idx]), look_mon_desc(c_ptr->m_idx));
		snprintf(out_val, sizeof(out_val), "%s (Lv %d, %s%s)", r_name_get(&m_list[c_ptr->m_idx]),
		    m_ptr->level, look_mon_desc(c_ptr->m_idx),
		    m_ptr->clone ? ", clone" : (done_unique ? ", slain" : ""));
#else /* use different colour for uniques we already killed */
		snprintf(out_val, sizeof(out_val), "%s%s (Lv %d, %s%s)",
		    done_unique ? "\377D" : "",
		    r_name_get(&m_list[c_ptr->m_idx]),
		    m_ptr->level, look_mon_desc(c_ptr->m_idx),
		    m_ptr->clone ? ", clone" : "");
#endif
	} else if (c_ptr->o_idx) {
		o_ptr = &o_list[c_ptr->o_idx];

		/* Obtain an object description */
		object_desc(Ind, o_name, o_ptr, TRUE, 3);

		snprintf(out_val, sizeof(out_val), "%sYou see %s%s",
		    compat_pomode(Ind, o_ptr) ? "\377D" : "", o_name, o_ptr->next_o_idx ? " on a pile" : "");

		/* Check if the object is on a detected trap */
		if ((cs_ptr=GetCS(c_ptr, CS_TRAPS))) {
			int t_idx = cs_ptr->sc.trap.t_idx;
			if (cs_ptr->sc.trap.found) {
				if (p_ptr->trap_ident[t_idx])
					p1 = t_name + t_info[t_idx].name;
				else
					p1 = "trapped";

				/* Add trap description at the end */
				strcat(out_val, " {");
				strcat(out_val, p1);
				strcat(out_val, "}");
				/* Also add a ^ at the beginning of the line
				in case it's too long to read its end */
				strcpy(tmp_val, "^ ");
				strcat(tmp_val, out_val);
				strcpy(out_val, tmp_val);
			}
		}

	} else {
		int feat = f_info[c_ptr->feat].mimic;
		cptr name = f_name + f_info[feat].name;
		if (is_a_vowel(name[0])) p1 = "An ";

		/* Hack -- add trap description */
		if ((cs_ptr=GetCS(c_ptr, CS_TRAPS))) {
			int t_idx = cs_ptr->sc.trap.t_idx;
			if (cs_ptr->sc.trap.found) {
				if (p_ptr->trap_ident[t_idx])
					p1 = t_name + t_info[t_idx].name;
				else
					p1 = "A trap";

				p2 = " on ";
			}
		}

		/* Hack -- special description for store doors */
//		if ((feat >= FEAT_SHOP_HEAD) && (feat <= FEAT_SHOP_TAIL))
		if (feat == FEAT_SHOP) {
			p1 = "The entrance to ";

			/* TODO: store name! */
			if ((cs_ptr=GetCS(c_ptr, CS_SHOP))) {
				name = st_name + st_info[cs_ptr->sc.omni].name;
			}

		}

		if ((feat == FEAT_FOUNTAIN) &&
		    (cs_ptr=GetCS(c_ptr, CS_FOUNTAIN)) &&
		    cs_ptr->sc.fountain.known) {
			object_kind *k_ptr;
			int tval, sval;

			if (cs_ptr->sc.fountain.type <= SV_POTION_LAST) {
				tval = TV_POTION;
				sval = cs_ptr->sc.fountain.type;
			} else {
				tval = TV_POTION2;
				sval = cs_ptr->sc.fountain.type - SV_POTION_LAST;
			}

			k_ptr = &k_info[lookup_kind(tval, sval)];
			info = k_name + k_ptr->name;
		}

		if (feat == FEAT_SIGN) /* give instructions how to actually read it ;) */
			name = "signpost \377D(bump to read)\377w";

		/* Message */
		if (strlen(info)) snprintf(out_val, sizeof(out_val), "%s%s%s (%s)", p1, p2, name, info);
		else snprintf(out_val, sizeof(out_val), "%s%s%s", p1, p2, name);
	}

	/* Append a little info */
	strcat(out_val, " [<dir>, p, q]");

	/* Tell the client */
	Send_target_info(Ind, x - p_ptr->panel_col_prt, y - p_ptr->panel_row_prt, out_val);
}




/*
 * Allow the player to examine other sectors on the map
 */
void do_cmd_locate(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int	y1, x1, y2, x2;
	int	prow = p_ptr->panel_row;
	int	pcol = p_ptr->panel_col;
	char	tmp_val[MAX_CHARS];
	char	out_val[MAX_CHARS_WIDE];
	char trad_val[23];


	/* No direction, recenter */
	if (!dir)
	{
		/* Recenter map around the player */
		verify_panel(Ind);

		/* Update stuff */
		p_ptr->update |= (PU_MONSTERS);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff */
		handle_stuff(Ind);

		return;
	}

	/* Initialize */
	if (dir == 5)
	{
		/* Remember current panel */
		p_ptr->panel_row_old = p_ptr->panel_row;
		p_ptr->panel_col_old = p_ptr->panel_col;
	}

	/* Start at current panel */
	y2 = p_ptr->panel_row;
	x2 = p_ptr->panel_col;

	/* Initial panel */
	y1 = p_ptr->panel_row_old;
	x1 = p_ptr->panel_col_old;

	/* Apply the motion */
	y2 += ddy[dir];
	x2 += ddx[dir];

	/* Verify the row */
	if (y2 > p_ptr->max_panel_rows) y2 = p_ptr->max_panel_rows;
	else if (y2 < 0) y2 = 0;

	/* Verify the col */
	if (x2 > p_ptr->max_panel_cols) x2 = p_ptr->max_panel_cols;
	else if (x2 < 0) x2 = 0;

	/* Describe the location */
	if ((y2 == y1) && (x2 == x1)) {
		tmp_val[0] = '\0';

		/* For BIG_MAP users, also display the traditional sector they are located in,
		   to make communication about dungeon entrances etc easier. */
		if (p_ptr->screen_wid == SCREEN_WID && p_ptr->screen_hgt == SCREEN_HGT)
			trad_val[0] = 0;
		else
			sprintf(trad_val, ", traditionally [%d,%d]", p_ptr->tradpanel_row, p_ptr->tradpanel_col);
	} else {
		sprintf(tmp_val, "%s%s of",
		        ((y2 < y1) ? " North" : (y2 > y1) ? " South" : ""),
		        ((x2 < x1) ? " West" : (x2 > x1) ? " East" : ""));
		trad_val[0] = 0;
	}

	/* Prepare to ask which way to look */
	sprintf(out_val,
    	    "Map sector [%d,%d], which is%s your sector%s. Direction (or ESC)?",
    	    y2, x2, tmp_val, trad_val);

	msg_print(Ind, out_val);

	/* Set the panel location */
	p_ptr->panel_row = y2;
	p_ptr->panel_col = x2;

	/* Recalculate the boundaries */
	panel_bounds(Ind);

	/* any change? otherwise no need to redraw everything */
	if ((prow != p_ptr->panel_row) || (pcol != p_ptr->panel_col)) {
		/* client-side weather stuff */
		p_ptr->panel_changed = TRUE;

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Update stuff */
		p_ptr->update |= (PU_MONSTERS);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);
	}

	/* Handle stuff */
	handle_stuff(Ind);
}



/* Not fully implemented - mikaelh */
#if 0

/*
 * The table of "symbol info" -- each entry is a string of the form
 * "X:desc" where "X" is the trigger, and "desc" is the "info".
 */
static cptr ident_info[] =
{
	" :A dark grid",
	"!:A potion (or oil)",
	"\":An amulet (or necklace)",
	"#:A wall (or secret door)",
	"$:Treasure (gold or gems)",
	"%:A vein (magma or quartz)",
	/* "&:unused", */
	"':An open door",
	"(:Soft armour",
	"):A shield",
	"*:A vein with treasure",
	"+:A closed door",
	",:Food (or mushroom patch)",
	"-:A wand (or rod)",
	".:Floor",
	"/:A polearm (Axe/Pike/etc)",
	/* "0:unused", */
	"1:Entrance to General Store",
	"2:Entrance to Armory",
	"3:Entrance to Weaponsmith",
	"4:Entrance to Temple",
	"5:Entrance to Alchemy shop",
	"6:Entrance to Magic store",
	"7:Entrance to Black Market",
	"8:Entrance to Tavern",
	/* "9:unused", */
	"::Rubble",
	";:A glyph of warding",
	"<:An up staircase",
	"=:A ring",
	">:A down staircase",
	"?:A scroll",
	"@:You",
	"A:Angel",
	"B:Bird",
	"C:Canine",
	"D:Ancient Dragon/Wyrm",
	"E:Elemental",
	"F:Dragon Fly",
	"G:Ghost",
	"H:Hybrid",
	"I:Insect",
	"J:Snake",
	"K:Killer Beetle",
	"L:Lich",
	"M:Multi-Headed Reptile",
	/* "N:unused", */
	"O:Ogre",
	"P:Giant Humanoid",
	"Q:Quylthulg (Pulsing Flesh Mound)",
	"R:Reptile/Amphibian",
	"S:Spider/Scorpion/Tick",
	"T:Troll",
	"U:Major Demon",
	"V:Vampire",
	"W:Wight/Wraith/etc",
	"X:Xorn/Xaren/etc",
	"Y:Yeti",
	"Z:Zephyr Hound",
	"[:Hard armour",
	"\\:A blunt weapon (mace/whip/etc)",
	"]:Misc. armour",
	"^:A trap",
	"_:A staff",
	/* "`:unused", */
	"a:Ant",
	"b:Bat",
	"c:Centipede",
	"d:Dragon",
	"e:Floating Eye",
	"f:Feline",
	"g:Golem",
	"h:Hobbit/Elf/Dwarf",
	"i:Icky Thing",
	"j:Jelly",
	"k:Kobold",
	"l:Louse",
	"m:Mold",
	"n:Naga",
	"o:Orc",
	"p:Person/Human",
	"q:Quadruped",
	"r:Rodent",
	"s:Skeleton",
	"t:Townsperson",
	"u:Minor Demon",
	"v:Vortex",
	"w:Worm/Worm-Mass",
	/* "x:unused", */
	"y:Yeek",
	"z:Zombie/Mummy",
	"{:A missile (arrow/bolt/shot)",
	"|:An edged weapon (sword/dagger/etc)",
	"}:A launcher (bow/crossbow/sling)",
	"~:A tool (or miscellaneous item)",
	NULL
};


/*
 * Identify a character
 *
 * Note that the player ghosts are ignored. XXX XXX XXX
 */
void do_cmd_query_symbol(int Ind, char sym)
{
	int		i;
	char	buf[128];


	/* If no symbol, abort --KLJ-- */
	if (!sym)
		return;

	/* Find that character info, and describe it */
	for (i = 0; ident_info[i]; ++i)
	{
		if (sym == ident_info[i][0]) break;
	}

	/* Describe */
	if (ident_info[i])
	{
		sprintf(buf, "%c - %s.", sym, ident_info[i] + 2);
	}
	else
	{
		sprintf(buf, "%c - %s.", sym, "Unknown Symbol");
	}

	/* Display the result */
	msg_print(Ind, buf);
}

#endif // 0
