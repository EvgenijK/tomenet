/* $Id$ */
/* File: load2.c */

/* Purpose: support for loading savefiles -BEN- */

#define SERVER

#include "angband.h"

static void new_rd_wild();
static void new_rd_floors();
void rd_towns();
void rd_byte(byte *ip);
void rd_u16b(u16b *ip);
void rd_s16b(s16b *ip);
void rd_u32b(u32b *ip);
void rd_s32b(s32b *ip);
void rd_string(char *str, int max);

/*
 * This file is responsible for loading all "2.7.X" savefiles
 *
 * Note that 2.7.0 - 2.7.2 savefiles are obsolete and will not work.
 *
 * We attempt to prevent corrupt savefiles from inducing memory errors.
 *
 * Note that Angband 2.7.9 encodes "terrain features" in the savefile
 * using the old 2.7.8 method.  Angband 2.8.0 will use the same method
 * to read pre-2.8.0 savefiles, but will use a new method to save them,
 * which will only affect "save.c".
 *
 * Note that Angband 2.8.0 will use a VERY different savefile method,
 * which will use "blocks" of information which can be ignored or parsed,
 * and which will not use a silly "protection" scheme on the savefiles,
 * but which may still use some form of "checksums" to prevent the use
 * of "corrupt" savefiles, which might cause nasty weirdness.
 *
 * Note that this file should not use the random number generator, the
 * object flavors, the visual attr/char mappings, or anything else which
 * is initialized *after* or *during* the "load character" function.
 *
 * We should also make the "cheating" options official flags, and
 * move the "byte" options to a different part of the code, perhaps
 * with a few more (for variety).
 *
 * Implement simple "savefile extenders" using some form of "sized"
 * chunks of bytes, with a {size,type,data} format, so everyone can
 * know the size, interested people can know the type, and the actual
 * data is available to the parsing routines that acknowledge the type.
 *
 * Consider changing the "globe of invulnerability" code so that it
 * takes some form of "maximum damage to protect from" in addition to
 * the existing "number of turns to protect for", and where each hit
 * by a monster will reduce the shield by that amount.
 *
 * XXX XXX XXX
 */





/*
 * Local "savefile" pointer
 */
static FILE	*fff;

/*
 * Local savefile buffer
 */
static char	*fff_buf;
static int	fff_buf_pos = 0;
#define MAX_BUF_SIZE	4096

/*
 * Hack -- old "encryption" byte
 */
static byte	xor_byte;

/*
 * Hack -- simple "checksum" on the actual values
 */
static u32b	v_check = 0L;

/*
 * Hack -- simple "checksum" on the encoded bytes
 */
static u32b	x_check = 0L;



/*
 * This function determines if the version of the savefile
 * currently being read is older than version "x.y.z".
 */
static bool older_than(byte x, byte y, byte z)
{
	/* Much older, or much more recent */
	if (sf_major < x) return (TRUE);
	if (sf_major > x) return (FALSE);

	/* Distinctly older, or distinctly more recent */
	if (sf_minor < y) return (TRUE);
	if (sf_minor > y) return (FALSE);

	/* Barely older, or barely more recent */
	if (sf_patch < z) return (TRUE);
	if (sf_patch > z) return (FALSE);

	/* Identical versions */
	return (FALSE);
}

/*
 * This function determines if the version of the server savefile
 * currently being read is older than version "x.y.z".
 */
static bool s_older_than(byte x, byte y, byte z)
{
	/* Much older, or much more recent */
	if (ssf_major < x) return (TRUE);
	if (ssf_major > x) return (FALSE);

	/* Distinctly older, or distinctly more recent */
	if (ssf_minor < y) return (TRUE);
	if (ssf_minor > y) return (FALSE);

	/* Barely older, or barely more recent */
	if (ssf_patch < z) return (TRUE);
	if (ssf_patch > z) return (FALSE);

	/* Identical versions */
	return (FALSE);
}

/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
bool wearable_p(object_type *o_ptr)
{
	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_BOOMERANG:
		case TV_DIGGING:
		case TV_BLUNT:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_LITE:
		case TV_AMULET:
		case TV_RING:
		case TV_AXE:
		case TV_MSTAFF:
			{
				return (TRUE);
			}
	}

	/* Nope */
	return (FALSE);
}


/*
 * The following functions are used to load the basic building blocks
 * of savefiles.  They also maintain the "checksum" info for 2.7.0+
 */

static byte sf_get(void)
{
	byte c, v;

#if 0
	/* Get a character, decode the value */
	c = getc(fff) & 0xFF;
#else
	/* Buffered reading */
	if (fff_buf_pos >= MAX_BUF_SIZE) {
		if (fread(fff_buf, 1, MAX_BUF_SIZE, fff) < MAX_BUF_SIZE) {
			if (!feof(fff)) {
				s_printf("Failed to read from savefile: %s\n", strerror(ferror(fff)));
			}
		}
		fff_buf_pos = 0;
	}

	c = fff_buf[fff_buf_pos++];
#endif
	v = c ^ xor_byte;
	xor_byte = c;

	/* Maintain the checksum info */
	v_check += v;
	x_check += xor_byte;

	/* Return the value */
	return (v);
}

void rd_byte(byte *ip)
{
	*ip = sf_get();
}

void rd_u16b(u16b *ip)
{
	(*ip) = sf_get();
	(*ip) |= ((u16b)(sf_get()) << 8);
}

void rd_s16b(s16b *ip)
{
	rd_u16b((u16b*)ip);
}

void rd_u32b(u32b *ip)
{
	(*ip) = sf_get();
	(*ip) |= ((u32b)(sf_get()) << 8);
	(*ip) |= ((u32b)(sf_get()) << 16);
	(*ip) |= ((u32b)(sf_get()) << 24);
}

void rd_s32b(s32b *ip)
{
	rd_u32b((u32b*)ip);
}


/*
 * Hack -- read a string
 */
void rd_string(char *str, int max)
{
	int i;

	/* Read the string */
	for (i = 0; TRUE; i++)
	{
		byte tmp8u;

		/* Read a byte */
		rd_byte(&tmp8u);

		/* Collect string while legal */
		if (i < max) str[i] = tmp8u;

		/* End of string */
		if (!tmp8u) break;
	}

	/* Terminate */
	str[max-1] = '\0';
}


/*
 * Hack -- strip some bytes
 */
static void strip_bytes(int n)
{
	int i;

	/* Strip the bytes */
	for (i = 0; i < n; i++) (void)sf_get();
}



/*
 * Old inventory slot values (pre-2.7.3)
 */
#define OLD_INVEN_WIELD     22
#define OLD_INVEN_HEAD      23
#define OLD_INVEN_NECK      24
#define OLD_INVEN_BODY      25
#define OLD_INVEN_ARM       26
#define OLD_INVEN_HANDS     27
#define OLD_INVEN_RIGHT     28
#define OLD_INVEN_LEFT      29
#define OLD_INVEN_FEET      30
#define OLD_INVEN_OUTER     31
#define OLD_INVEN_LITE      32
#define OLD_INVEN_AUX       33


/*
 * Read an item (2.7.0 or later)
 *
 * Note that savefiles from 2.7.0 and 2.7.1 are obsolete.
 *
 * Note that pre-2.7.9 savefiles (from Angband 2.5.1 onward anyway)
 * can be read using the code above.
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 *
 * Note that Angband 2.7.9 introduced a new method for object "flags"
 * in which the "flags" on an object are actually extracted when they
 * are needed from the object kind, artifact index, ego-item index,
 * and two special "xtra" fields which are used to encode any "extra"
 * power of certain ego-items.  This had the side effect that items
 * imported from pre-2.7.9 savefiles will lose any "extra" powers they
 * may have had, and also, all "uncursed" items will become "cursed"
 * again, including Calris, even if it is being worn at the time.  As
 * a complete hack, items which are inscribed with "uncursed" will be
 * "uncursed" when imported from pre-2.7.9 savefiles.
 */
/* For wilderness levels, dun_depth has been changed from 1 to 4 bytes. */
static void rd_item(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;
	s16b old_ac;

	u32b f1, f2, f3, f4, f5, esp;
	object_kind *k_ptr;
	char note[128];

	byte tmpbyte;
	s32b tmp32s;


	/* Hack -- wipe */
	WIPE(o_ptr, object_type);

	rd_s32b(&o_ptr->owner);
	rd_s16b(&o_ptr->level);
	if (!older_than(4, 1, 7)) {
		if (older_than(4, 4, 14)) {
			rd_s32b(&tmp32s);
			o_ptr->mode = tmp32s;
		}
		else rd_byte(&o_ptr->mode);
	}

	/* Kind (discarded though - Jir -) */
	rd_s16b(&o_ptr->k_idx);

	/* Location */
	rd_byte(&o_ptr->iy);
	rd_byte(&o_ptr->ix);

	rd_s16b(&o_ptr->wpos.wx);
	rd_s16b(&o_ptr->wpos.wy);
	rd_s16b(&o_ptr->wpos.wz);

	/* Type/Subtype */
	rd_byte(&o_ptr->tval);
	rd_byte(&o_ptr->sval);
	if (!older_than(4, 2, 6)) {
		rd_byte(&o_ptr->tval2);
		rd_byte(&o_ptr->sval2);
	}

/* HACKHACKHACK - C. Blue - Moved Khopesh to polearms */
//if (o_ptr->tval == 23 && o_ptr->sval == 14) {o_ptr->tval = 22; o_ptr->sval = 9;}

#ifdef EXPAND_TV_POTION
	/* Convert outdated potions that still use TV_POTION2 to new TV_POTION values */
	if (o_ptr->tval == TV_POTION2) {
		o_ptr->tval = TV_POTION;
		switch (o_ptr->sval) {
		case SV_POTION2_CHAUVE_SOURIS: o_ptr->sval = SV_POTION_CHAUVE_SOURIS; break;
		case SV_POTION2_LEARNING: o_ptr->sval = SV_POTION_LEARNING; break;
		case SV_POTION2_CURE_SANITY: o_ptr->sval = SV_POTION_CURE_SANITY; break;
		case SV_POTION2_CURE_LIGHT_SANITY: o_ptr->sval = SV_POTION_CURE_LIGHT_SANITY; break;
		case SV_POTION2_CURE_SERIOUS_SANITY: o_ptr->sval = SV_POTION_CURE_SERIOUS_SANITY; break;
		case SV_POTION2_CURE_CRITICAL_SANITY: o_ptr->sval = SV_POTION_CURE_CRITICAL_SANITY; break;
		default:
			/* failure? revert change! */
			o_ptr->tval = TV_POTION2;
		}
	}
#else
	/* Convert already converted potions back if we reverted EXPAND_TV_POTION, which shouldn't happen :-p */
	if (o_ptr->tval == TV_POTION) {
		o_ptr->tval = TV_POTION2;
		switch (o_ptr->sval) {
		case SV_POTION_CHAUVE_SOURIS: o_ptr->sval = SV_POTION2_CHAUVE_SOURIS; break;
		case SV_POTION_LEARNING: o_ptr->sval = SV_POTION2_LEARNING; break;
		case SV_POTION_CURE_SANITY: o_ptr->sval = SV_POTION2_CURE_SANITY; break;
		case SV_POTION_CURE_LIGHT_SANITY: o_ptr->sval = SV_POTION2_CURE_LIGHT_SANITY; break;
		case SV_POTION_CURE_SERIOUS_SANITY: o_ptr->sval = SV_POTION2_CURE_SERIOUS_SANITY; break;
		case SV_POTION_CURE_CRITICAL_SANITY: o_ptr->sval = SV_POTION2_CURE_CRITICAL_SANITY; break;
		default:
			/* failure? revert tval */
			o_ptr->tval = TV_POTION;
		}
	}
#endif

	/* Base pval */
	rd_s32b(&o_ptr->bpval);
	rd_s32b(&o_ptr->pval);
        if (!older_than(4, 2, 3)) {
		rd_s32b(&o_ptr->pval2);
		rd_s32b(&o_ptr->pval3);
	}
        if (!older_than(4, 2, 6)) {
		rd_s32b(&o_ptr->sigil);
		rd_s32b(&o_ptr->sseed);
	}

	rd_byte(&o_ptr->discount);
	rd_byte(&o_ptr->number);
	rd_s16b(&o_ptr->weight);

	if (!older_than(4, 3, 1)) {
		rd_u16b(&o_ptr->name1);
		rd_u16b(&o_ptr->name2);

	} else {
		/* Increase portability with pointers to correct type - mikaelh */
		byte old_name1, old_name2;
		rd_byte(&old_name1);
		rd_byte(&old_name2);
		o_ptr->name1 = old_name1;
		if (o_ptr->name1 == 255) o_ptr->name1 = ART_RANDART;
		o_ptr->name2 = old_name2;
	}
	rd_s32b(&o_ptr->name3);
        if (!older_than(4, 2, 1))
		rd_s32b(&o_ptr->timeout);
	else {
		/* Increase portability with pointers to correct type - mikaelh */
		s16b old_timeout;
		rd_s16b(&old_timeout);
		o_ptr->timeout = old_timeout;
	}

	rd_s16b(&o_ptr->to_h);
	rd_s16b(&o_ptr->to_d);
	rd_s16b(&o_ptr->to_a);

/* DEBUGGING PURPOSES - the_sandman */
#if 0
	if (o_ptr->tval == 46)
	 {
	  s_printf("TRAP_DEBUG: Trap with s_val:%d,to_h:%d,to_d:%d,to_a:%d loaded\n",
				o_ptr->sval, o_ptr->to_h, o_ptr->to_d, o_ptr->to_a);
	 }
#endif
#if 0 /* should all be fixed now hopefully - C. Blue */
	/* Cap all old non-trueart bows - mikaelh */
	if (o_ptr->tval == TV_BOW && (o_ptr->name1 == 0 || o_ptr->name1 == ART_RANDART))
	{/* CAP_ITEM_BONI */
		if (o_ptr->to_h > 30) o_ptr->to_h = 30;
		if (o_ptr->to_d > 30) o_ptr->to_d = 30;
	}
#endif
#ifdef USE_NEW_SHIELDS
	/* Cap all old shields' +ac - C. Blue */
	if (o_ptr->tval == TV_SHIELD)
	{/* CAP_ITEM_BONI */
		/* if (o_ptr->to_a > 15) o_ptr->to_h = 15; */ // this must've been wrong - mikaelh
		if (o_ptr->to_a > 15) o_ptr->to_a = 15;
	}
#endif
	/* Fix shields base AC or percentage, in case USE_NEW_SHIELDS has been toggled. */
	if (o_ptr->tval == TV_SHIELD) {
		o_ptr->ac = k_info[o_ptr->k_idx].ac;
		/* Fix to_h being set on shields (see above) - mikaelh */
		o_ptr->to_h = 0;
	}

	/* Fix rods that got 'double-timeout' by erroneous discharge function*/
	if (o_ptr->tval == TV_ROD) o_ptr->timeout = 0;

	if (o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_FEANORIAN) {
		if (o_ptr->level < 30) {
			if (o_ptr->name2 || o_ptr->name2b)
				o_ptr->level = 40;
			else
				o_ptr->level = 31;
		}
	}

	rd_s16b(&old_ac);

	rd_byte(&old_dd);
	rd_byte(&old_ds);

	if (!older_than(4, 4, 0)) rd_u16b(&o_ptr->ident);
	else {
		rd_byte(&tmpbyte);
		o_ptr->ident = tmpbyte;
	}

	if (!older_than(4, 3, 1)) {
		rd_u16b(&o_ptr->name2b);
	} else {
		/* Increase portability with pointers to correct type - mikaelh */
		byte old_name2b;
		rd_byte(&old_name2b);
		o_ptr->name2b = old_name2b;
	}

	if (older_than(4, 3, 20)) {
		/* Old flags + Unused */
		strip_bytes(14);
	}

	/* Special powers */
	rd_byte(&o_ptr->xtra1);
	rd_byte(&o_ptr->xtra2);
	/* more special powers (for self-made spellbook feature) */
	if (!older_than(4, 3, 16)) {
		rd_byte(&o_ptr->xtra3);
		rd_byte(&o_ptr->xtra4);
		rd_byte(&o_ptr->xtra5);
		rd_byte(&o_ptr->xtra6);
		rd_byte(&o_ptr->xtra7);
		rd_byte(&o_ptr->xtra8);
		rd_byte(&o_ptr->xtra9);
	}

	if (!older_than(4, 3, 20)) {
		if (!older_than(4, 3, 21)) {
			rd_s32b(&tmp32s);
			o_ptr->marked = tmp32s;
		} else {
			rd_byte(&tmpbyte);
			o_ptr->marked = tmpbyte;
		}
		rd_byte(&o_ptr->marked2);
	}

	/* Inscription */
	rd_string(note, 128);
	/* Save the inscription */
	if (note[0]) o_ptr->note = quark_add(note);
	/* hack: 'empty' inscription (startup items) cannot
	   be saved this way, so we try to restore it now.. - collides with stolen goods! */
	else if (o_ptr->discount == 100 && o_ptr->level == 0
	    && object_value_real(0, o_ptr) <= 10000) /* avoid stolen-goods-level-0-hack collision */
		o_ptr->note = quark_add("");
	if (!older_than(4, 4, 10)) {
		rd_byte(&tmpbyte);
		o_ptr->note_utag = tmpbyte;
	} else o_ptr->note_utag = 0;

	rd_u16b(&o_ptr->next_o_idx);
	if (!older_than(4, 4, 22)) {
		rd_byte(&tmpbyte);
		o_ptr->stack_pos = tmpbyte;
	}
	rd_u16b(&o_ptr->held_m_idx);

	/* hack: remove (due to bug) created explodingness from magic ammo */
	if (is_ammo(o_ptr->tval) && o_ptr->sval == SV_AMMO_MAGIC && !o_ptr->name1) o_ptr->pval = 0;

	/* Obtain k_idx from tval/sval instead :) */
	if (o_ptr->k_idx)	/* zero is cipher :) */
		o_ptr->k_idx = lookup_kind(o_ptr->tval, o_ptr->sval);

#ifdef SEAL_INVALID_OBJECTS
	if (!seal_or_unseal_object(o_ptr)) return;
#else
	/* Object does no longer exist? Delete it! */
	if (!o_ptr->k_idx) {
		o_ptr->tval = o_ptr->sval = 0;
		return;
	}
#endif

#if 0 /* commented out again till it's of use once more (hopefully not) */
	/*HACK just to get rid of invalid seals in Bree.. */
	if (o_ptr->tval == TV_SPECIAL && o_ptr->sval == SV_SEAL) {
		invwipe(o_ptr);
		return;
	}
#endif

	/* Obtain the "kind" template */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Hack -- notice "broken" items */
	if (k_ptr->cost <= 0) o_ptr->ident |= ID_BROKEN;


	/* Repair non "wearable" items */
	if ((o_ptr->tval != TV_TRAPKIT) && !wearable_p(o_ptr)) {
		/* Acquire correct fields */
		o_ptr->to_h = k_ptr->to_h;
		o_ptr->to_d = k_ptr->to_d;
		o_ptr->to_a = k_ptr->to_a;

		/* Acquire correct fields */
		o_ptr->ac = k_ptr->ac;
		o_ptr->dd = k_ptr->dd;
		o_ptr->ds = k_ptr->ds;

		/* All done */
		return;
	}

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Paranoia */
	if (o_ptr->name2) {
		ego_item_type *e_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Verify that ego-item */
		if (!e_ptr->name) o_ptr->name2 = 0;
	}


	/* Acquire standard fields */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Acquire standard weight */
	o_ptr->weight = k_ptr->weight;

	/* Hack -- extract the "broken" flag */
	if (!o_ptr->pval < 0) o_ptr->ident |= ID_BROKEN;


	/* Artifacts */
	if (o_ptr->name1) {
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		/* Hack -- Randarts! */
		if (o_ptr->name1 == ART_RANDART)
			a_ptr = randart_make(o_ptr);
		else
			a_ptr = &a_info[o_ptr->name1];

		/* Hack: Fix old, meanwhile illegal, randarts */
		if (!a_ptr) {
			o_ptr->name1 = 0;
			o_ptr->name3 = 0L;
			//o_ptr->pval = 0;
		} else {
			/* Acquire new artifact "pval" */
			o_ptr->pval = a_ptr->pval;

			/* Acquire new artifact fields */
			o_ptr->ac = a_ptr->ac;
			o_ptr->dd = a_ptr->dd;
			o_ptr->ds = a_ptr->ds;

			/* Acquire new artifact weight */
			o_ptr->weight = a_ptr->weight;

			/* Hack -- extract the "broken" flag */
			if (!a_ptr->cost) o_ptr->ident |= ID_BROKEN;
		}
	}

	/* Ego items */
	if (o_ptr->name2) {
		ego_item_type *e_ptr;
		artifact_type *a_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

#if 0
		/* UnHack. pffft! */
		if ((o_ptr->ac < old_ac)) o_ptr->ac = old_ac;
		if ((o_ptr->dd < old_dd)) o_ptr->dd = old_dd;
		if ((o_ptr->ds < old_ds)) o_ptr->ds = old_ds;
#else
		a_ptr = ego_make(o_ptr);
		o_ptr->ac += a_ptr->ac;
		o_ptr->dd += a_ptr->dd;
		o_ptr->ds += a_ptr->ds;
#endif

		/* Hack -- extract the "broken" flag */
		if (!e_ptr->cost) o_ptr->ident |= ID_BROKEN;
	}

	/* hack- fix trap kits with wrong enchantments */
	if (o_ptr->tval == TV_TRAPKIT && !is_firearm_trapkit(o_ptr->sval)) o_ptr->to_h = o_ptr->to_d = 0;
}


/*
 * Read a "monster" record
 */
static void rd_monster_race(monster_race *r_ptr)
{
	byte tmpbyte;
	int i;

	rd_u16b(&r_ptr->name);
	rd_u16b(&r_ptr->text);
	rd_byte(&tmpbyte);
	r_ptr->hdice = tmpbyte;
	rd_byte(&tmpbyte);
	r_ptr->hside = tmpbyte;
	rd_s16b(&r_ptr->ac);
	rd_s16b(&r_ptr->sleep);
	rd_byte(&r_ptr->aaf);
	rd_byte(&r_ptr->speed);
	rd_s32b(&r_ptr->mexp);
	rd_s16b(&r_ptr->extra);
	rd_byte(&r_ptr->freq_innate);
	rd_byte(&r_ptr->freq_spell);
	rd_u32b(&r_ptr->flags1);
	rd_u32b(&r_ptr->flags2);
	rd_u32b(&r_ptr->flags3);
	rd_u32b(&r_ptr->flags4);
	rd_u32b(&r_ptr->flags5);
	rd_u32b(&r_ptr->flags6);
	rd_s16b(&r_ptr->level);
	rd_byte(&r_ptr->rarity);
	rd_byte(&r_ptr->d_attr);
	rd_byte((byte *)&r_ptr->d_char);
	rd_byte(&r_ptr->x_attr);
	rd_byte((byte *)&r_ptr->x_char);
	for (i = 0; i < 4; i++)
	{
		rd_byte(&r_ptr->blow[i].method);
		rd_byte(&r_ptr->blow[i].effect);
		rd_byte(&r_ptr->blow[i].d_dice);
		rd_byte(&r_ptr->blow[i].d_side);
	}
}


/*
 * Read a monster
 */

static void rd_monster(monster_type *m_ptr)
{
	byte i;

	/* Hack -- wipe */
	WIPE(m_ptr, monster_type);
	if (s_older_than(4, 2, 7)) {
		m_ptr->pet = 0;
	} else {
		rd_byte(&m_ptr->pet);
	}
	rd_byte((byte *)&m_ptr->special);

	/* Owner */
	rd_s32b(&m_ptr->owner);

	/* Read the monster race */
	rd_s16b(&m_ptr->r_idx);

	/* Read the other information */
	rd_byte(&m_ptr->fy);
	rd_byte(&m_ptr->fx);

	rd_s16b(&m_ptr->wpos.wx);
	rd_s16b(&m_ptr->wpos.wy);
	rd_s16b(&m_ptr->wpos.wz);

	rd_s16b(&m_ptr->ac);
	rd_byte(&m_ptr->speed);
	rd_s32b(&m_ptr->exp);
	rd_s16b(&m_ptr->level);
	for (i = 0; i < 4; i++)
	{
		rd_byte(&(m_ptr->blow[i].method));
		rd_byte(&(m_ptr->blow[i].effect));
		rd_byte(&(m_ptr->blow[i].d_dice));
		rd_byte(&(m_ptr->blow[i].d_side));
	}
	rd_s32b(&m_ptr->hp);
	rd_s32b(&m_ptr->maxhp);
	rd_s16b(&m_ptr->csleep);
	rd_byte(&m_ptr->mspeed);
	rd_s16b(&m_ptr->energy);
	rd_byte(&m_ptr->stunned);
	rd_byte(&m_ptr->confused);
	rd_byte(&m_ptr->monfear);
	if (!s_older_than(4, 3, 15)) {
		rd_byte(&m_ptr->paralyzed);
		rd_byte(&m_ptr->bleeding);
		rd_byte(&m_ptr->poisoned);
		rd_byte(&m_ptr->blinded);
		rd_byte(&m_ptr->silenced);
	}

	rd_u16b(&m_ptr->hold_o_idx);
	rd_u16b(&m_ptr->clone);

	rd_s16b(&m_ptr->mind);
	if (m_ptr->special)
	{
		MAKE(m_ptr->r_ptr, monster_race);
		rd_monster_race(m_ptr->r_ptr);
	}

	rd_u16b(&m_ptr->ego);
	rd_s32b(&m_ptr->name3);
}


/*
 * Read the global server-wide monster lore
 */
static void rd_global_lore(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Count sights/deaths/kills */
	if (s_older_than(4, 4, 7)) {
		s16b tmp16b;

		rd_s16b(&tmp16b);
		r_ptr->r_sights = tmp16b;

		rd_s16b(&tmp16b);
		r_ptr->r_deaths = tmp16b;

		/* was r_ptr->r_pkills */
		strip_bytes(2);

		rd_s16b(&tmp16b);
		r_ptr->r_tkills = tmp16b;
	} else {
		rd_s32b(&r_ptr->r_sights);
		rd_s32b(&r_ptr->r_deaths);
		rd_s32b(&r_ptr->r_tkills);
	}

	/* Hack -- if killed, it's been seen */
	if (r_ptr->r_tkills > r_ptr->r_sights) r_ptr->r_sights = r_ptr->r_tkills;

	if (s_older_than(4, 4, 7)) {
		/* Old monster lore information - mikaelh */
		strip_bytes(38);
	}

	/* Read the global monster limit */
	if (s_older_than(4, 4, 8)) {
		byte tmpbyte;
		rd_byte(&tmpbyte);
		r_ptr->max_num = tmpbyte;
	} else {
		rd_s32b(&r_ptr->max_num);
	}

	if (!s_older_than(4, 3, 24)) {
		if (s_older_than(4, 4, 8)) {
			byte tmpbyte;
			rd_byte(&tmpbyte);
			r_ptr->cur_num = tmpbyte;
		} else {
			rd_s32b(&r_ptr->cur_num);
		}
	}

	if (s_older_than(4, 4, 7)) {
		/* Old reserved bytes - mikaelh */
		strip_bytes(3);
	}
}




/*
 * Read a store
 */
static errr rd_store(store_type *st_ptr)
{
	int j;

	byte num;
	u16b own;

	/* Read the basic info */
	rd_s32b(&st_ptr->store_open);
	rd_s16b(&st_ptr->insult_cur);
	rd_u16b(&own);
	rd_byte(&num);
	rd_s16b(&st_ptr->good_buy);
	rd_s16b(&st_ptr->bad_buy);

	/* Last visit */
	rd_s32b(&st_ptr->last_visit);

	/* Extract the owner (see above) */
	st_ptr->owner = own;

	/* Read the items */
	for (j = 0; j < num; j++) {
		object_type forge;

		/* Read the item */
		rd_item(&forge);

		/* Hack -- verify item */
		if (!forge.k_idx) s_printf("Warning! Non-existing item detected(erased).\n");

		/* Acquire valid items */
		else if (st_ptr->stock_num < STORE_INVEN_MAX &&
		    st_ptr->stock_num < st_ptr->stock_size) {
			/* Acquire the item */
			st_ptr->stock[st_ptr->stock_num++] = forge;
		}
	}

	/* Success */
	return (0);
}

static void rd_bbs() {
        int i, j;
	s16b saved_lines, parties, guilds;
	char dummy[MAX_CHARS_WIDE];

        rd_s16b(&saved_lines);

#if 0
        for (i = 0; ((i < BBS_LINES) && (i < saved_lines)); i++)
                rd_string(bbs_line[i], MAX_CHARS_WIDE);
#else
        for (i = 0; i < saved_lines; i++)
		if (i >= BBS_LINES) rd_string(dummy, MAX_CHARS_WIDE);
		else rd_string(bbs_line[i], MAX_CHARS_WIDE);
#endif

	/* load pbbs & gbbs - C. Blue */
	if (!s_older_than(4, 4, 9)) {
		/* read old parties */
		rd_s16b(&parties);
		for (j = 0; j < parties; j++)
			if (j < MAX_PARTIES) {
				for (i = 0; i < saved_lines; i++)
					if (i >= BBS_LINES) rd_string(dummy, MAX_CHARS_WIDE);
					else rd_string(pbbs_line[j][i], MAX_CHARS_WIDE);
			} else {
				for (i = 0; i < saved_lines; i++)
					rd_string(dummy, MAX_CHARS_WIDE);
			}
		/* read old guilds */
		rd_s16b(&guilds);
		for (j = 0; j < guilds; j++)
			if (j < MAX_GUILDS) {
				for (i = 0; i < saved_lines; i++)
					if (i >= BBS_LINES) rd_string(dummy, MAX_CHARS_WIDE);
					else rd_string(gbbs_line[j][i], MAX_CHARS_WIDE);
			} else {
				for (i = 0; i < saved_lines; i++)
					rd_string(dummy, MAX_CHARS_WIDE);
			}
	}
}

static void rd_notes() {
        int i;
        s16b j;
        char dummy[MAX_CHARS_WIDE];

        rd_s16b(&j);
        for (i = 0; i < j; i++) {
	        if (i >= MAX_NOTES) {
			rd_string(dummy, MAX_CHARS_WIDE);
			rd_string(dummy, NAME_LEN);
			rd_string(dummy, NAME_LEN);
			continue;
	        }
                rd_string(priv_note[i], MAX_CHARS_WIDE);
                rd_string(priv_note_sender[i], NAME_LEN);
                rd_string(priv_note_target[i], NAME_LEN);
        }

        rd_s16b(&j);
        for (i = 0; i < j; i++) {
	        if (i >= MAX_PARTYNOTES) {
			rd_string(dummy, MAX_CHARS_WIDE);
			rd_string(dummy, NAME_LEN);
			continue;
	        }
                rd_string(party_note[i], MAX_CHARS_WIDE);
                rd_string(party_note_target[i], NAME_LEN);
        }

        rd_s16b(&j);
        for (i = 0; i < j; i++) {
	        if (i >= MAX_GUILDNOTES) {
			rd_string(dummy, MAX_CHARS_WIDE);
			rd_string(dummy, NAME_LEN);
			continue;
	        }
                rd_string(guild_note[i], MAX_CHARS_WIDE);
                rd_string(guild_note_target[i], NAME_LEN);
        }
        //omitted (use custom.lua instead): admin_note[MAX_ADMINNOTES]
}

static void rd_quests(){
	int i;
	rd_s16b(&questid);
	for (i = 0; i < 20; i++) {
		rd_u16b(&quests[i].active);
		rd_u16b(&quests[i].id);
		rd_s16b(&quests[i].type);
		rd_u16b(&quests[i].flags);
		rd_s32b(&quests[i].creator);
		rd_s32b(&quests[i].turn);
	}
}

static void rd_guilds() {
	int i;
	u16b tmp16u;
	rd_u16b(&tmp16u);
	if(tmp16u > MAX_GUILDS){
		s_printf("Too many guilds (%d)\n", tmp16u);
		return;
	}
	for (i = 0; i < tmp16u; i++) {
		if (!s_older_than(4, 5, 8)) rd_u32b(&guilds[i].dna);
		else {
			guilds[i].dna = rand_int(0xFFFF) << 16;
			guilds[i].dna += rand_int(0xFFFF);
		}
		rd_string(guilds[i].name, 80);
		rd_s32b(&guilds[i].master);
		rd_s32b(&guilds[i].members);
		if (!s_older_than(4, 5, 2)) rd_byte(&guilds[i].cmode);
		else {
			cptr name = NULL;
			/* first entry is dummy anyway */
			if (i == 0) guilds[0].cmode = 0;
			else if (guilds[i].master && (name = lookup_player_name(guilds[i].master)) != NULL) {
				guilds[i].cmode = lookup_player_mode(guilds[i].master);
	                        s_printf("Guild '%s' (%d): Mode has been fixed to master's ('%s',%d) mode %d.\n",
                                    guilds[i].name, i, name, guilds[i].master, guilds[i].cmode);
                        } else { /* leaderless guild, ow */
                                s_printf("Guild '%s' (%d): Fixing lost guild, master (%d) is '%s'.\n",
                                    guilds[i].name, i, guilds[i].master, name ? name : "(null)");
                                fix_lost_guild_mode(i);
                        }
		}
		rd_u32b(&guilds[i].flags);
		rd_s16b(&guilds[i].minlev);
		if (!s_older_than(4, 4, 20)) {
			int j;
			for (j = 0; j < 5; j++)
				rd_string(guilds[i].adder[j], NAME_LEN);
		}
		if (!s_older_than(4, 5, 0)) rd_s16b(&guilds[i].h_idx);
		else guilds[i].h_idx = 0;
	}
}

/*
 * Read some party info

 FOUND THE BUIG!!!!!! the disapearing party bug. no more.
 I hope......
 -APD-
 */
static void rd_party(int n)
{
	party_type *party_ptr = &parties[n];

	/* Party name */
	rd_string(party_ptr->name, 80);

	/* Party owner's name */
	rd_string(party_ptr->owner, 20);

	/* Number of people and creation time */
	rd_s32b(&party_ptr->members);
	rd_s32b(&party_ptr->created);

	/* Party type and members */
	if (!s_older_than(4, 1, 7))
		rd_byte(&party_ptr->mode);
	else
		party_ptr->mode = 0;

	if (!s_older_than(4, 5, 2)) rd_byte(&party_ptr->cmode);
	else {
		/* first entry is dummy anyway; party in use at all? if not then we're done */
		if (n == 0 || !party_ptr->members) party_ptr->cmode = 0;
		else {
			u32b p_id = lookup_player_id(party_ptr->owner);
                        if (p_id) {
                                parties[n].cmode = lookup_player_mode(p_id);
                                s_printf("Party '%s' (%d): Mode has been fixed to %d ('%s',%d).\n",
                                    parties[n].name, n, parties[n].cmode, parties[n].owner, p_id);
                        }
                        /* paranoia - a party without owner shouldn't exist */
                        else s_printf("Party '%s' (%d): Mode couldn't be fixed ('%s',%d).\n",
                            parties[n].name, n, parties[n].owner, p_id);
		}
	}

	if (!s_older_than(4, 4, 19)) rd_u32b(&party_ptr->flags);

	/* Hack -- repair dead parties 
	   I THINK this line was causing some problems....
	   lets find out
	   */
	if (!lookup_player_id(party_ptr->owner))
	{
		/*
		   Set to no people in party
		   party_ptr->members = 0;
		   */
	}
}

/*
 * Read some house info
 */
static void rd_house(int n)
{
	int i;
	house_type *house_ptr = &houses[n];
	cave_type **zcave;
	object_type dump;

	rd_byte(&house_ptr->x);
	rd_byte(&house_ptr->y);
	rd_byte(&house_ptr->dx);
	rd_byte(&house_ptr->dy);
	MAKE(house_ptr->dna, struct dna_type);
	rd_u32b(&house_ptr->dna->creator);
        if (!s_older_than(4, 4, 14)) rd_byte(&house_ptr->dna->mode);
	rd_s32b(&house_ptr->dna->owner);
	rd_byte(&house_ptr->dna->owner_type);
#if 0 /* player hash is read _AFTER_ houses! So this is done via /fix-house-modes instead- C. Blue */
	if (s_older_than(4, 4, 14)) {
		/* if house doesn't have its mode set yet, search
		   hash for player who owns it and set mode to his. */
		if (house_ptr->dna->owner &&
		    (house_ptr->dna->owner_type == OT_PLAYER)) {
			house_ptr->dna->mode = lookup_player_mode(house_ptr->dna->owner);
		}
	}
#endif
	rd_byte(&house_ptr->dna->a_flags);
	rd_u16b(&house_ptr->dna->min_level);
	rd_u32b(&house_ptr->dna->price);
	rd_u16b(&house_ptr->flags);

	rd_s16b(&house_ptr->wpos.wx);
	rd_s16b(&house_ptr->wpos.wy);
	rd_s16b(&house_ptr->wpos.wz);

	if ((zcave = getcave(&house_ptr->wpos))) {
		struct c_special *cs_ptr;
		if (house_ptr->flags & HF_STOCK) {
			/* add dna to static levels even though town-generated */
			if ((cs_ptr = GetCS(&zcave[house_ptr->dy][house_ptr->dx], CS_DNADOOR)))
				cs_ptr->sc.ptr = house_ptr->dna;
			else if ((cs_ptr = AddCS(&zcave[house_ptr->dy][house_ptr->dx], CS_DNADOOR)))
				cs_ptr->sc.ptr = house_ptr->dna;
		}
		else
		{
			/* add dna to static levels */
			if ((cs_ptr = GetCS(&zcave[house_ptr->dy][house_ptr->dx], CS_DNADOOR)))
				cs_ptr->sc.ptr = house_ptr->dna;
			else if ((cs_ptr = AddCS(&zcave[house_ptr->y+house_ptr->dy][house_ptr->x+house_ptr->dx], CS_DNADOOR)))
				cs_ptr->sc.ptr = house_ptr->dna;
		}
	}
	if (house_ptr->flags&HF_RECT){
		rd_byte(&house_ptr->coords.rect.width);
		rd_byte(&house_ptr->coords.rect.height);
	}
	else{
		i =- 2;
		C_MAKE(house_ptr->coords.poly, MAXCOORD, char);
		do {
			i += 2;
			rd_byte((byte*)&house_ptr->coords.poly[i]);
			rd_byte((byte*)&house_ptr->coords.poly[i + 1]);
		} while(house_ptr->coords.poly[i] || house_ptr->coords.poly[i + 1]);
		GROW(house_ptr->coords.poly, MAXCOORD, i + 2, byte);
	}

	if (!s_older_than(4, 4, 13)) {
		rd_byte(&house_ptr->colour);
		rd_byte(&house_ptr->xtra);
	}

#ifndef USE_MANG_HOUSE_ONLY
	rd_s16b(&house_ptr->stock_num);
	rd_s16b(&house_ptr->stock_size);
	if (house_ptr->stock_size > STORE_INVEN_MAX) house_ptr->stock_size = STORE_INVEN_MAX;
	C_MAKE(house_ptr->stock, house_ptr->stock_size, object_type);
	for (i = 0; i < house_ptr->stock_num; i++) {
		if (i < STORE_INVEN_MAX) rd_item(&house_ptr->stock[i]);
		else rd_item(&dump);
	}
	if (house_ptr->stock_num > STORE_INVEN_MAX) house_ptr->stock_num = STORE_INVEN_MAX;
#endif	/* USE_MANG_HOUSE_ONLY */
}

static void rd_wild(wilderness_type *w_ptr)
{
	/* future use */
	strip_bytes(4);
	/* terrain type */
	rd_u16b(&w_ptr->type);

	/* this needs to be loaded or it'll be lost until this piece of
	   wilderness is unstaticed again. - C. Blue */
	if (!s_older_than(4, 5, 12)) rd_u16b(&w_ptr->bled);
	else w_ptr->bled = WILD_GRASSLAND; /* init */

	/* the flags */
	rd_u32b(&w_ptr->flags);

	/* the player(KING) owning the wild */
	rd_s32b(&w_ptr->own);
}

/*
 * Read/Write the "extra" information
 */

static bool rd_extra(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int i, j;
	monster_race *r_ptr;
	char login_char_name[80];

	byte tmp8u;
	u16b tmp16u, panic;
	s16b tmp16s;
	u32b tmp32u;
	s32b tmp32s;

	/* 'Savegame filename character conversion' exploit fix - C. Blue */
	strcpy(login_char_name, p_ptr->name);
	rd_string(p_ptr->name, 32);
	if (strcmp(p_ptr->name, login_char_name)) {
		s_printf("$INTRUSION$ - %s tried to use savegame %s\n", p_ptr->name, login_char_name);
		return (1);
	}

	rd_string(p_ptr->died_from, 80);

	/* if new enough, load the died_from_list information */
	rd_string(p_ptr->died_from_list, 80);
	rd_s16b(&p_ptr->died_from_depth);

	for (i = 0; i < 4; i++)
		rd_string(p_ptr->history[i], 60);

	if (older_than(4, 2, 7)) {
		p_ptr->has_pet = 0; //assume no pet
	} else {
		rd_byte(&p_ptr->has_pet);
	}

	/* Divinity has been absorbed by traits (ptrait) now */
	if (older_than(4, 4, 12)) {
		/* Divinity support in savefile now on all servers - mikaelh */
		if (older_than(4, 3, 2)) {
//			p_ptr->divinity = DIVINE_UNDEF; /* which is simply 0 */
			p_ptr->ptrait = TRAIT_NONE;
		} else {
			rd_byte(&tmp8u);
			/* backwards compatible */
			if (tmp8u == 0x1) p_ptr->ptrait = TRAIT_ENLIGHTENED;
			if (tmp8u == 0x10) p_ptr->ptrait = TRAIT_CORRUPTED;
		}
	}

	/* Class/Race/Gender/Party */
	rd_byte(&p_ptr->prace);
#ifdef ENABLE_KOBOLD /* Kobold was inserted in the middle, instead of added to the end of the races array */
        if (older_than(4, 4, 28) && p_ptr->prace >= RACE_KOBOLD) p_ptr->prace++;
#endif
        rd_byte(&p_ptr->pclass);
        if (!older_than(4, 4, 11)) rd_byte(&p_ptr->ptrait);
	if (older_than(4, 3, 5)) { /* class order changed: warrior now first class, so newbies won't choose adventurer */
		if (p_ptr->pclass == CLASS_WARRIOR) p_ptr->pclass = CLASS_ADVENTURER;
		else if (p_ptr->pclass < CLASS_DRUID) p_ptr->pclass--;
	}
	rd_byte(&p_ptr->male);
	if (older_than(4, 2, 4)) {
		rd_byte(&tmp8u);
		p_ptr->party = tmp8u; /* convert the old byte to u16b - mikaelh */
	}
	else rd_u16b(&p_ptr->party);
	rd_byte(&p_ptr->mode);

	/* Special Race/Class info */
	rd_byte(&p_ptr->hitdie);
	rd_s16b(&p_ptr->expfact);

	/* Age/Height/Weight */
	rd_s16b(&p_ptr->age);
	rd_s16b(&p_ptr->ht);
	rd_s16b(&p_ptr->wt);
#ifndef SAVEDATA_TRANSFER_KLUDGE
	rd_u16b(&p_ptr->align_good);	/* alignment */
	rd_u16b(&p_ptr->align_law);
#endif	/* SAVEDATA_TRANSFER_KLUDGE */

	/* Read the stat info */
	for (i = 0; i < 6; i++) rd_s16b(&p_ptr->stat_max[i]);
	for (i = 0; i < 6; i++) rd_s16b(&p_ptr->stat_cur[i]);

	/* Dump the stats (maximum and current) */
	for (i = 0; i < 6; ++i) rd_s16b(&p_ptr->stat_cnt[i]);
	for (i = 0; i < 6; ++i) rd_s16b(&p_ptr->stat_los[i]);

        /* Read the skills */
	{
		rd_u16b(&tmp16u);
		if (tmp16u > MAX_SKILLS)
		{
			quit("Too many skills!");
		}
		for (i = 0; i < tmp16u; ++i)
		{
			rd_s32b(&p_ptr->s_info[i].value);
			rd_u16b(&p_ptr->s_info[i].mod);
			rd_byte(&tmp8u);
			p_ptr->s_info[i].dev = tmp8u;
			rd_byte(&tmp8u);
			if (!older_than(4, 4, 3)) {
				p_ptr->s_info[i].flags1 = tmp8u;
				rd_s32b(&p_ptr->s_info[i].base_value);
			} else {
#if 0 //SMOOTHSKILLS
				p_ptr->s_info[i].hidden = tmp8u;
#else
				if (tmp8u) p_ptr->s_info[i].flags1 |= SKF1_HIDDEN;
#endif
				if (!older_than(4, 3, 17)) {
					rd_byte(&tmp8u);
#if 0 //SMOOTHSKILLS
					p_ptr->s_info[i].dummy = (tmp8u) ? TRUE : FALSE;
#else
					if (tmp8u) p_ptr->s_info[i].flags1 |= SKF1_DUMMY;
#endif
				} else {
#if 0 //SMOOTHSKILLS
					p_ptr->s_info[i].dummy = FALSE;
#else
					p_ptr->s_info[i].flags1 &= ~SKF1_DUMMY;
#endif
				}
			}
			p_ptr->s_info[i].touched = TRUE;
		}
		rd_s16b(&p_ptr->skill_points);

		/* /undoskills - mikaelh */
		if (!older_than(4, 4, 4)) {
			for (i = 0; i < tmp16u; ++i)
			{
				rd_s32b(&p_ptr->s_info_old[i].value);
				rd_u16b(&p_ptr->s_info_old[i].mod);
				rd_byte(&tmp8u);
				p_ptr->s_info_old[i].dev = tmp8u;
				rd_byte(&tmp8u);
				p_ptr->s_info_old[i].flags1 = tmp8u;
				rd_s32b(&p_ptr->s_info_old[i].base_value);
				p_ptr->s_info_old[i].touched = TRUE;
			}
			rd_s16b(&p_ptr->skill_points_old);
			rd_byte((byte *) &p_ptr->reskill_possible);
		}
	}

	/* Make a copy of the skills - mikaelh */
//	memcpy(p_ptr->s_info_old, p_ptr->s_info, MAX_SKILLS * sizeof(skill_player));
//	p_ptr->skill_points_old = p_ptr->skill_points;

	rd_s32b(&p_ptr->id);

	rd_u32b(&p_ptr->dna);
	rd_s32b(&p_ptr->turn);

	if (!older_than(4, 4, 5)) {
		rd_s32b(&p_ptr->turns_online);
		rd_s32b(&p_ptr->turns_afk);
		rd_s32b(&p_ptr->turns_idle);
	}
	if (!older_than(4, 4, 6)) rd_s32b(&p_ptr->turns_active);

	/* If he was created in the pre-ID days, give him one */
	if (!p_ptr->id)
		p_ptr->id = newid();

	strip_bytes(20);	/* oops */

	rd_s32b(&p_ptr->au);

	rd_s32b(&p_ptr->max_exp);
        /* Build maximum level (the one displayed if life levels were restored right now) */
	p_ptr->max_lev = 1;
#ifndef ALT_EXPRATIO
	while ((p_ptr->max_lev < (is_admin(p_ptr) ? PY_MAX_LEVEL : PY_MAX_PLAYER_LEVEL)) &&
	    (p_ptr->max_exp >= ((s64b)(((s64b)player_exp[p_ptr->max_lev-1] * (s64b)p_ptr->expfact) / 100L))))
#else
	while ((p_ptr->max_lev < (is_admin(p_ptr) ? PY_MAX_LEVEL : PY_MAX_PLAYER_LEVEL)) &&
	    (p_ptr->max_exp >= (s64b)player_exp[p_ptr->max_lev-1]))
#endif
	{
		/* Gain a level */
		p_ptr->max_lev++;
	}
	rd_s32b(&p_ptr->exp);
	rd_u16b(&p_ptr->exp_frac);

	rd_s16b(&p_ptr->lev);

	rd_s16b(&p_ptr->mhp);
	rd_s16b(&p_ptr->chp);
	rd_u16b(&p_ptr->chp_frac);

	if (!older_than(4, 3, 8)) {
		rd_s16b(&p_ptr->mst);
		rd_s16b(&p_ptr->cst);
		rd_s16b(&p_ptr->cst_frac);
	}
/* hack for old chars */
if (p_ptr->mst != 10) p_ptr->mst = 10;

	rd_s16b(&p_ptr->msp);
	rd_s16b(&p_ptr->csp);
	rd_u16b(&p_ptr->csp_frac);

	rd_s16b(&p_ptr->max_plv);
	rd_s16b(&p_ptr->max_dlv);
        if (!older_than(4, 4, 23)) {
		for (i = 0; i < MAX_D_IDX * 2; i++) {
			rd_byte(&tmp8u);
			p_ptr->max_depth[i] = tmp8u;
			rd_byte(&tmp8u);
			p_ptr->max_depth_wx[i] = tmp8u;
			rd_byte(&tmp8u);
			p_ptr->max_depth_wy[i] = tmp8u;
			rd_byte(&tmp8u);
			p_ptr->max_depth_tower[i] = (tmp8u != 0);
			/* hack: fix for chars that logged in before this was completed properly */
			if (p_ptr->max_depth[i] > 200) p_ptr->dummy_option_8 = TRUE;
		}
	}
	/* else branch is in rd_savefile_new_aux() since we need wild_map[] array */

	rd_s16b(&p_ptr->py);
	rd_s16b(&p_ptr->px);

	rd_s16b(&p_ptr->wpos.wx);
	rd_s16b(&p_ptr->wpos.wy);
	rd_s16b(&p_ptr->wpos.wz);

	rd_u16b(&p_ptr->town_x);
	rd_u16b(&p_ptr->town_y);

	p_ptr->recall_pos.wx = p_ptr->wpos.wx;
	p_ptr->recall_pos.wy = p_ptr->wpos.wy;
	p_ptr->recall_pos.wz = p_ptr->max_dlv;

	/* More info */

	rd_s16b(&p_ptr->ghost);
	rd_s16b(&p_ptr->sc);
	rd_s16b(&p_ptr->fruit_bat);

	/* Read the flags */
	rd_byte(&p_ptr->lives);
	/* hack, if save file was from an older version we need to convert it: */
 	if (cfg.lifes && !p_ptr->lives) p_ptr->lives = cfg.lifes+1;
	/* If the server's life amount was reduced, apply it to players */
	if (cfg.lifes && (p_ptr->lives > cfg.lifes+1)) p_ptr->lives = cfg.lifes+1;

        if (!older_than(4, 2, 0)) {
		rd_byte(&p_ptr->houses_owned);
	} else {
		for (i = 0; i < num_houses; i++) {
			if ((houses[i].dna->owner_type == OT_PLAYER) &&
			    (houses[i].dna->owner == p_ptr->id))
				p_ptr->houses_owned++;
		}
	}

	/* hack */
	rd_byte(&tmp8u);
	rd_s16b(&p_ptr->blind);
	rd_s16b(&p_ptr->paralyzed);
	rd_s16b(&p_ptr->confused);
	rd_s16b(&p_ptr->food);
	strip_bytes(4);	/* Old "food_digested" / "protection" */
	rd_s16b(&p_ptr->energy);
        rd_s16b(&p_ptr->fast);
        if (!older_than(4, 0, 3))
                rd_s16b(&p_ptr->fast_mod);
	rd_s16b(&p_ptr->slow);
	rd_s16b(&p_ptr->afraid);
	rd_s16b(&p_ptr->cut);
	rd_s16b(&p_ptr->stun);
	rd_s16b(&p_ptr->poisoned);
	rd_s16b(&p_ptr->image);
	rd_s16b(&p_ptr->protevil);
	rd_s16b(&p_ptr->invuln);
	rd_s16b(&p_ptr->hero);
	rd_s16b(&p_ptr->shero);
	if (!older_than(4, 3, 7)) rd_s16b(&p_ptr->berserk);
	rd_s16b(&p_ptr->shield);
        if (!older_than(4, 0, 4))
        {
                rd_s16b(&p_ptr->shield_power);
                rd_s16b(&p_ptr->shield_opt);
                rd_s16b(&p_ptr->shield_power_opt);
                rd_s16b(&p_ptr->shield_power_opt2);
                rd_s16b(&p_ptr->tim_thunder);
                rd_s16b(&p_ptr->tim_thunder_p1);
                rd_s16b(&p_ptr->tim_thunder_p2);
                rd_s16b(&p_ptr->tim_fly);
                rd_s16b(&p_ptr->tim_ffall);
                rd_s16b(&p_ptr->tim_regen);
                rd_s16b(&p_ptr->tim_regen_pow);
        }
	rd_s16b(&p_ptr->blessed);
	rd_s16b(&p_ptr->tim_invis);
	rd_s16b(&p_ptr->word_recall);
	rd_s16b(&p_ptr->see_infra);
	rd_s16b(&p_ptr->tim_infra);
	rd_s16b(&p_ptr->oppose_fire);
	rd_s16b(&p_ptr->oppose_cold);
	rd_s16b(&p_ptr->oppose_acid);
	rd_s16b(&p_ptr->oppose_elec);
	rd_s16b(&p_ptr->oppose_pois);
	rd_s16b(&p_ptr->prob_travel);
	rd_s16b(&p_ptr->st_anchor);
	rd_s16b(&p_ptr->tim_esp);
	rd_s16b(&p_ptr->adrenaline);
	rd_s16b(&p_ptr->biofeedback);

	rd_byte(&p_ptr->confusing);
		rd_u16b(&p_ptr->tim_jail);
		rd_u16b(&p_ptr->tim_susp);
		rd_u16b(&p_ptr->pkill);
		rd_u16b(&p_ptr->tim_pkill);
	rd_s16b(&p_ptr->tim_wraith);
	rd_byte((byte *)&p_ptr->wraith_in_wall);
	rd_byte(&p_ptr->searching);
	rd_byte(&p_ptr->maximize);
	rd_byte(&p_ptr->preserve);
	rd_byte(&p_ptr->stunning);

	rd_s16b(&p_ptr->body_monster);
	/* Fix limits */
	if (p_ptr->body_monster > MAX_R_IDX || p_ptr->body_monster < 0) p_ptr->body_monster = 0;
	rd_s16b(&p_ptr->auto_tunnel);

	rd_s16b(&p_ptr->tim_meditation);

	rd_s16b(&p_ptr->tim_invisibility);
	rd_s16b(&p_ptr->tim_invis_power);
	if (!older_than(4, 5, 9)) rd_s16b(&p_ptr->tim_invis_power2);

	rd_s16b(&p_ptr->fury);

	rd_s16b(&p_ptr->tim_manashield);

	rd_s16b(&p_ptr->tim_traps);

	rd_s16b(&p_ptr->tim_mimic);
	rd_s16b(&p_ptr->tim_mimic_what);

	/* Monster Memory */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > MAX_R_IDX)
	{
		s_printf("Too many (%u) monster races!\n", tmp16u);
		return (22);
	}
	for (i = 0; i < tmp16u; i++)
	{
		rd_s16b(&p_ptr->r_killed[i]);

		/* Hack -- try to fix the unique list */
		r_ptr = &r_info[i];

		if (p_ptr->r_killed[i] && !r_ptr->r_tkills &&
		    !is_admin(p_ptr) &&
		    (r_ptr->flags1 & RF1_UNIQUE)) {
			r_ptr->r_tkills = r_ptr->r_sights = 1;
			s_printf("TKILL FIX: Player %s assisted for/killed unique %d\n", p_ptr->name, i);
		}
	}

        if (!older_than(4, 4, 24)) rd_u32b(&p_ptr->gold_picked_up);
        else strip_bytes(4);

        if (!older_than(4, 4, 24)) {
		rd_byte(&tmp8u);
		p_ptr->insta_res = tmp8u;
    	}
        else strip_bytes(1);
        if (!older_than(4, 4, 25)) rd_byte(&p_ptr->castles_owned);
        else strip_bytes(1);

	if (!older_than(4, 5, 6)) rd_s16b(&p_ptr->hilite_self);
	else {
		strip_bytes(2);
		p_ptr->hilite_self = -1; /* disabled by default */
	}

	if (!older_than(4, 5, 7)) {
		rd_byte(&tmp8u);
		p_ptr->fluent_artifact_reset = tmp8u;
	} else {
		strip_bytes(1);
		p_ptr->fluent_artifact_reset = FALSE;
	}

	/* Future use */
	strip_bytes(34);

	/* Toggle for possible automatic save-game updates
	   (done via script login-hook, eg custom.lua) - C. Blue */
	rd_byte(&p_ptr->updated_savegame);

#if 0 /* ALT_EXPRATIO conversion: */
if (p_ptr->updated_savegame == 0) {
    s64b i, i100, ief;
    i = (s64b)p_ptr->max_exp;
    i100 = (s64b)100;
    ief = (s64b)p_ptr->expfact;
    i = (i * i100) / ief;
    p_ptr->max_exp = (s32b)i;
    p_ptr->max_lev = 1;
    while ((p_ptr->max_lev < (is_admin(p_ptr) ? PY_MAX_LEVEL : PY_MAX_PLAYER_LEVEL)) &&
        (p_ptr->max_exp >= (s64b)player_exp[p_ptr->max_lev-1]))
    {
	/* Gain a level */
	p_ptr->max_lev++;
    }
    p_ptr->exp = p_ptr->max_exp;
    if (p_ptr->lev > p_ptr->max_lev) p_ptr->lev = p_ptr->max_lev;
    p_ptr->updated_savegame = 3;
}
#endif

	/* for automatic artifact resets (similar to updated_savegame) */
	if (!older_than(4, 3, 22)) rd_byte(&p_ptr->artifact_reset);

	/* Skip the flags */
	strip_bytes(12);


	/* Hack -- the two "special seeds" */
	/*rd_u32b(&seed_flavor);
	  rd_u32b(&seed_town);*/
        if (!older_than(4, 0, 5)) rd_s32b(&p_ptr->mimic_seed);
        if (!older_than(4, 4, 27)) {
        	rd_byte(&tmp8u);
		p_ptr->mimic_immunity = tmp8u;
    	}

	if (!older_than(4, 5, 4)) {
		rd_u16b(&tmp16u);
		p_ptr->autoret = tmp16u;
	}
	else p_ptr->autoret = 0;

	if (!older_than(4, 0, 6)) rd_s16b(&p_ptr->martyr_timeout);
	else p_ptr->martyr_timeout = 0;

	/* Special stuff */
	rd_u16b(&panic);
        if (panic) {
                Players[Ind]->panic = TRUE;
                s_printf("loaded a panic-saved player %s\n", Players[Ind]->name);
        }

	rd_u16b(&p_ptr->total_winner);
	if (!older_than(4, 3, 0)) rd_u16b(&p_ptr->once_winner);
	if (!older_than(4, 4, 29)) {
		rd_byte(&tmp8u);
		if (tmp8u) p_ptr->iron_winner = TRUE;
		rd_byte(&tmp8u);
		if (tmp8u) p_ptr->iron_winner_ded = TRUE;
	}

	rd_s16b(&p_ptr->own1.wx);
	rd_s16b(&p_ptr->own1.wy);
	rd_s16b(&p_ptr->own1.wz);
	rd_s16b(&p_ptr->own2.wx);
	rd_s16b(&p_ptr->own2.wy);
	rd_s16b(&p_ptr->own2.wz);

	rd_u16b(&p_ptr->retire_timer);
	rd_u16b(&p_ptr->noscore);

	/* Read "death" */
	rd_byte(&tmp8u);
	p_ptr->death = tmp8u;

	rd_byte((byte*)&p_ptr->black_breath);

	rd_s16b(&p_ptr->msane);
	rd_s16b(&p_ptr->csane);
	rd_u16b(&p_ptr->csane_frac);

	rd_s32b(&p_ptr->balance);
	rd_s32b(&p_ptr->tim_blacklist);

	if (!older_than(4, 2, 5)) rd_s32b(&p_ptr->tim_watchlist);
	else p_ptr->tim_watchlist = 0;

	if (!older_than(4, 2, 9)) rd_s32b(&p_ptr->pstealing);
	else p_ptr->pstealing = 0;

        if (!older_than(4, 2, 8)) {
		for (i = 0; i <	MAX_GLOBAL_EVENTS; i++) {
			rd_s16b(&tmp16s);
			p_ptr->global_event_type[i] = tmp16s;
			if (!older_than(4, 3, 9)) {
				rd_s32b(&tmp32s);
				p_ptr->global_event_signup[i] = tmp32s;
				rd_s32b(&tmp32s);
				p_ptr->global_event_started[i] = tmp32s;
			} else {
				rd_u32b(&tmp32u);
				p_ptr->global_event_signup[i] = tmp32u;
				rd_u32b(&tmp32u);
				p_ptr->global_event_started[i] = tmp32u;
			}
			for (j = 0; j < 4; j++) rd_u32b(&p_ptr->global_event_progress[i][j]);
		}
	}

	if (!older_than(4, 5, 10))
		for (i = 0; i < MAX_GLOBAL_EVENT_TYPES; i++) {
			rd_s16b(&tmp16s);
			p_ptr->global_event_participated[i] = tmp16s;
		}
	else
		for (i = 0; i < MAX_GLOBAL_EVENT_TYPES; i++)
			p_ptr->global_event_participated[i] = 0;

	if (!older_than(4, 3, 3)) {
		rd_s16b(&tmp16s);
		p_ptr->combat_stance = tmp16s;
		rd_s16b(&tmp16s);
		p_ptr->combat_stance_power = tmp16s;
	}
	if (!older_than(4, 3, 4)) rd_byte(&p_ptr->cloaked);
	if (!older_than(4, 3, 9)) rd_byte((byte *) &p_ptr->shadow_running);
	if (!older_than(4, 3, 10)) rd_byte((byte *) &p_ptr->shoot_till_kill);
	if (!older_than(4, 3, 23)) rd_byte((byte *) &p_ptr->dual_mode);

	if (!older_than(4, 3, 11)) {
		rd_s16b(&tmp16s);
		p_ptr->kills = tmp16s;
		if (!older_than(4, 5, 11)) {
			rd_s16b(&tmp16s);
			p_ptr->kills_own = tmp16s;
		}
		rd_s16b(&tmp16s);
		p_ptr->kills_lower = tmp16s;
		rd_s16b(&tmp16s);
		p_ptr->kills_higher = tmp16s;
		rd_s16b(&tmp16s);
		p_ptr->kills_equal = tmp16s;
	}
	if (!older_than(4, 3, 12)) {
		rd_s16b(&tmp16s);
		p_ptr->free_mimic = tmp16s;
	}

	/* read obsolete values */
	if (older_than(4, 4, 30) &&
	    /* Runecraft */
	    !older_than(4, 3, 26)) {
    		rd_s16b(&tmp16s);
	    	rd_s16b(&tmp16s);
    		rd_s16b(&tmp16s);
    		rd_s16b(&tmp16s);
        	rd_s16b(&tmp16s);

	    	rd_u16b(&tmp16u);
    		rd_u16b(&tmp16u);
    		rd_u16b(&tmp16u);
	}

	if (!older_than(4, 4, 1)) {
		rd_byte(&tmp8u); /* aura states (on/off) */
		if ((tmp8u & 0x1) && get_skill(p_ptr, SKILL_AURA_FEAR)) p_ptr->aura[0] = TRUE;
		if ((tmp8u & 0x2) && get_skill(p_ptr, SKILL_AURA_SHIVER)) p_ptr->aura[1] = TRUE;
		if ((tmp8u & 0x4) && get_skill(p_ptr, SKILL_AURA_DEATH)) p_ptr->aura[2] = TRUE;

		rd_u16b(&p_ptr->deaths);
		rd_u16b(&p_ptr->soft_deaths);

		/* array of 'warnings' and hints aimed at newbies */
		rd_u16b(&tmp16u);
		if (tmp16u | 0x01) p_ptr->warning_technique_melee = 1;
		if (tmp16u | 0x02) p_ptr->warning_technique_ranged = 1;
	} else {
		/* auto-enable for now (MAX_AURAS) */
		if (get_skill(p_ptr, SKILL_AURA_FEAR)) p_ptr->aura[0] = TRUE;
		if (get_skill(p_ptr, SKILL_AURA_SHIVER)) p_ptr->aura[1] = TRUE;
		if (get_skill(p_ptr, SKILL_AURA_DEATH)) p_ptr->aura[2] = TRUE;
	}

	if (!older_than(4, 4, 2)) rd_string(p_ptr->info_msg, 80);
	/* for now 'forget' info_msg on logout: */
	strcpy(p_ptr->info_msg, "");

	/* for ENABLE_GO_GAME */
	if (!older_than(4, 4, 17)) {
		rd_byte(&tmp8u);
		p_ptr->go_level = tmp8u;
		rd_byte(&tmp8u);
		p_ptr->go_sublevel = tmp8u;
	}

	if (!older_than(4, 4, 19)) {
		/* For things like 'Officer' status to add others etc */
		rd_u32b(&p_ptr->party_flags);
		rd_u32b(&p_ptr->guild_flags);
	}

	/* read obsolete values */
	if (older_than(4, 4, 30) &&
	    /* Read obselete Runecraft variables into dummy variables - Kurzel */
	    !older_than(4, 4, 25)) {
	        rd_s16b(&tmp16s);

	        rd_byte(&tmp8u);
	        rd_s16b(&tmp16s);
	        rd_s16b(&tmp16s);
	        rd_u16b(&tmp16u);

	        rd_u16b(&tmp16u);
	        rd_byte(&tmp8u);
	        rd_byte(&tmp8u);
	        rd_u32b(&tmp32u);

	        rd_byte(&tmp8u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_byte(&tmp8u);

	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);

#if 0
	        rd_u16b(&tmp16u);
#endif
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);

	        rd_byte(&tmp8u);
	        rd_byte(&tmp8u);

	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        //rd_u16b(&tmp16u);
	        //rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);

#if 1
	        if (!older_than(4, 4, 26)) {
	                rd_u16b(&tmp16u);
	                rd_byte(&tmp8u);
	                rd_u32b(&tmp32u);
	                rd_u16b(&tmp16u);
	                rd_byte(&tmp8u);
	                rd_u32b(&tmp32u);
	        }
#endif

	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);

#if 0
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
	        rd_u16b(&tmp16u);
#endif

	        rd_u16b(&tmp16u);
	        rd_byte(&tmp8u);
	}

	if (!older_than(4, 5, 3)) rd_u16b(&p_ptr->tim_deflect);
	else p_ptr->tim_deflect = 0;

	/* Success */
	return FALSE;
}




/*
 * Read the player inventory
 *
 * Note that the inventory changed in Angband 2.7.4.  Two extra
 * pack slots were added and the equipment was rearranged.  Note
 * that these two features combine when parsing old save-files, in
 * which items from the old "aux" slot are "carried", perhaps into
 * one of the two new "inventory" slots.
 *
 * Note that the inventory is "re-sorted" later by "dungeon()".
 */
static errr rd_inventory(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int slot = 0;

	object_type forge;

	/* No weight */
	p_ptr->total_weight = 0;

	/* No items */
	p_ptr->inven_cnt = 0;
	p_ptr->equip_cnt = 0;

	/* Read until done */
	while (TRUE)
	{
		u16b n;

		/* Get the next item index */
		rd_u16b(&n);

		/* Nope, we reached the end */
		if (n == 0xFFFF) break;

		/* Read the item */
		rd_item(&forge);

		/* Hack -- verify item */
		if (!forge.k_idx)
		{
			s_printf("Warning! Non-existing item detected(erased).\n");
			continue;
		}

#if 0
		/* Mega-Hack -- Handle artifacts that aren't yet "created" */
if (p_ptr->updated_savegame == 3) { // <- another megahack, see lua_arts_fix()
		if (artifact_p(&forge))
		{
			/* If this artifact isn't created, mark it as created */
			/* Only if this isn't a "death" restore */
			if (!a_info[forge.name1].cur_num && !p_ptr->death)
				handle_art_inum(forge.name1);
			if (!a_info[forge.name1].known && (forge.ident & ID_KNOWN))
				a_info[forge.name1].known = TRUE;
		}
}
#endif

		/* Wield equipment */
		if (n >= INVEN_WIELD)
		{
			/* Structure copy */
			p_ptr->inventory[n] = forge;

			/* Add the weight */
			p_ptr->total_weight += (forge.number * forge.weight);

			/* One more item */
			p_ptr->equip_cnt++;
		}

		/* Warning -- backpack is full */
		else if (p_ptr->inven_cnt == INVEN_PACK)
		{
			s_printf("Too many items in the inventory!\n");

			/* Fail */
			return (54);
		}

		/* Carry inventory */
		else
		{
			/* Get a slot */
			n = slot++;

			/* Structure copy */
			p_ptr->inventory[n] = forge;

			/* Add the weight */
			p_ptr->total_weight += (forge.number * forge.weight);

			/* One more item */
			p_ptr->inven_cnt++;
		}
	}

	/* Success */
	return (0);
}

/*
 * Read hostility information
 *
 * Note that this function is responsible for deleting stale entries
 */
static errr rd_hostilities(int Ind)
{
	player_type *p_ptr = Players[Ind];
	hostile_type *h_ptr;
	int i;
	s32b id;
	u16b tmp16u;

	/* Read number */
	rd_u16b(&tmp16u);

	/* Read available ID's */
	for (i = 0; i < tmp16u; i++)
	{
		/* Read next ID */
		rd_s32b(&id);

		/* Check for stale player */
		if (id > 0 && !lookup_player_name(id)) continue;

		/* Check for stale party */
		if (id < 0 && !parties[0 - id].members) continue;

		/* Create node */
		MAKE(h_ptr, hostile_type);
		h_ptr->id = id;

		/* Add to chain */
		h_ptr->next = p_ptr->hostile;
		p_ptr->hostile = h_ptr;
	}

	/* Success */
	return (0);
}



/*
 * Read the run-length encoded dungeon
 *
 * Note that this only reads the dungeon features and flags, 
 * the objects and monsters will be read later.
 *
 */

static errr rd_floor(void)
{
	struct worldpos wpos;
	s16b tmp16b;
	byte tmp;
	cave_type **zcave;
	u16b max_y, max_x;

	int i;
	byte k, y, x;
	cave_type *c_ptr;
	dun_level *l_ptr;

	unsigned char runlength, feature;
	u16b flags;


	/*** Depth info ***/

	/* Level info */
	rd_s16b(&wpos.wx);
	rd_s16b(&wpos.wy);
	rd_s16b(&wpos.wz);
	if (wpos.wx == 0x7fff && wpos.wy == 0x7fff && wpos.wz == 0x7fff)
		return(1);
	rd_u16b(&max_y);
	rd_u16b(&max_x);

	/* Alloc before doing NPOD() */
	alloc_dungeon_level(&wpos);
	zcave = getcave(&wpos);
	l_ptr = getfloor(&wpos);

	/* players on this depth */
	rd_s16b(&tmp16b);
	new_players_on_depth(&wpos, tmp16b, FALSE);
#if DEBUG_LEVEL > 1
	s_printf("%d players on %s.\n", players_on_depth(&wpos), wpos_format(0, &wpos));
#endif

	rd_byte(&tmp);
	new_level_up_y(&wpos, tmp);
	rd_byte(&tmp);
	new_level_up_x(&wpos, tmp);
	rd_byte(&tmp);
	new_level_down_y(&wpos, tmp);
	rd_byte(&tmp);
	new_level_down_x(&wpos, tmp);
	rd_byte(&tmp);
	new_level_rand_y(&wpos, tmp);
	rd_byte(&tmp);
	new_level_rand_x(&wpos, tmp);

	if (l_ptr) {
		time_t now;
		now = time(&now);
		l_ptr->lastused = now;

		rd_u32b(&l_ptr->flags1);
		rd_byte(&l_ptr->hgt);
		rd_byte(&l_ptr->wid);
	}

	/*** Run length decoding ***/

	/* where do all these features go, long time ago... ? */

	/* Load the dungeon data */
	for (x = y = 0; y < max_y; ) {
		/* Grab RLE info */
		rd_byte(&runlength);
		rd_byte(&feature);
		rd_u16b(&flags);

		/* Apply the RLE info */
		for (i = 0; i < runlength; i++) {
			/* Access the cave */
			c_ptr = &zcave[y][x];

			/* set the feature */
			c_ptr->feat = feature;

#ifdef HOUSE_PAINTING
			/* if we read a house door, restore house paint */
			if (c_ptr->feat == FEAT_HOME || c_ptr->feat == FEAT_HOME_OPEN) {
				int h_idx;
				house_type *h_ptr;

				h_idx = pick_house(&wpos, y, x);
				if (h_idx != -1) {
					h_ptr = &houses[h_idx];
					/* if house is 'painted', colour adjacent walls accordingly */
					if (h_ptr->colour) {
 #if 0 /* only effective for already allocated sectors; use /fix-house-modes instead */
						/* hack: fix old houses before this colour hack was added for HOUSE_PAINTING_HIDE_BAD_MODE */
						if ((h_ptr->dna->mode & MODE_EVERLASTING) && h_ptr->colour < 100)
							h_ptr->colour += 100;
 #endif

						cave_type *hwc_ptr;
						int hwx, hwy;
						for (hwx = x - 1; hwx <= x + 1; hwx++)
						for (hwy = y - 1; hwy <= y + 1; hwy++) {
							if (!in_bounds(hwy, hwx)) continue;

							hwc_ptr = &zcave[hwy][hwx];
							/* Only colour house wall grids */
							if (hwc_ptr->feat == FEAT_WALL_HOUSE ||
							    hwc_ptr->feat == FEAT_HOME ||
							    hwc_ptr->feat == FEAT_HOME_OPEN)
								hwc_ptr->colour = h_ptr->colour;
						}
					}
				}
			}
#endif

			/* set flags */
			c_ptr->info = flags;

			/* increment our position */
			x++;
			if (x >= max_x) {
				/* Wrap onto a new row */
				x = 0;
				y++;
			}
		}
	}

	/*** another run for c_special ***/
	{
		struct c_special *cs_ptr;
		byte n;
		while (TRUE)
		{
			rd_byte(&x);
			rd_byte(&y);
			rd_byte(&n);	/* Number of c_special to add */

			/* terminated? */
			if (x == 255 && y == 255 && n == 255) break;

			c_ptr = &zcave[y][x];
			while(n--){
				rd_byte(&k);
				cs_ptr = ReplaceCS(c_ptr, k);
				csfunc[k].load(cs_ptr);
			}
		}
	}

	/* fix possibly no longer correct lighting regarding time of the day */
	if (wpos.wz == 0) {
		/* hack this particular sector */
		if (IS_DAY) world_surface_day(&wpos);
		else world_surface_night(&wpos);
	}

	/* Success */
	return (0);
}

/* Reads in a players memory of the level he is currently on, in run-length encoded
 * format.  Simmilar to the above function.
 */

static errr rd_cave_memory(int Ind)
{
	player_type *p_ptr = Players[Ind];
	u16b max_y, max_x;
	int i, y, x;

	unsigned char runlength, cur_flag;

	/* Remember unique ID of the old floor */
	if (!older_than(4, 4, 16)) rd_u32b(&p_ptr->dlev_id);
	else p_ptr->dlev_id = 0;

	/* Memory dimensions */
	rd_u16b(&max_y);
	rd_u16b(&max_x);

	/*** Run length decoding ***/

	/* Load the memory data */
	for (x = y = 0; y < max_y; ) {
		/* Grab RLE info */
		rd_byte(&runlength);
		rd_byte(&cur_flag);

		/* Apply the RLE info */
		for (i = 0; i < runlength; i++) {
			p_ptr->cave_flag[y][x] = cur_flag;

			/* incrament our position */
			x++;
			if (x >= max_x) {
				/* Wrap onto a new row */
				x = 0;
				y++;

				/* paranoia */
				/* if (y >= max_y) break; */
			}
		}
	}

	/* Success */
	return (0);
}

/* Reads auction data. */

static void rd_auctions()
{
	int i, j;
	u32b old_auction_alloc;
	auction_type *auc_ptr;
	bid_type *bid_ptr;
	s32b tmp32s;

	old_auction_alloc = auction_alloc;
	if (!s_older_than(4, 4, 10)) rd_u32b(&auction_alloc);
	else {
		rd_s32b(&tmp32s);
		auction_alloc = tmp32s;
	}

	GROW(auctions, old_auction_alloc, auction_alloc, auction_type);

	for (i = 0; i < auction_alloc; i++)
	{
		auc_ptr = &auctions[i];
		rd_byte(&auc_ptr->status);
		rd_byte(&auc_ptr->flags);
		rd_byte(&auc_ptr->mode);
		rd_s32b(&auc_ptr->owner);
		rd_item(&auc_ptr->item);
		C_MAKE(auc_ptr->desc, 160, char);
		rd_string(auc_ptr->desc, 160);
		if (auc_ptr->desc[0] == '\0')
		{
			/* Free an empty string */
			C_FREE(auc_ptr->desc, 160, char);
		}
		rd_s32b(&auc_ptr->starting_price);
		rd_s32b(&auc_ptr->buyout_price);
		rd_s32b(&auc_ptr->bids_cnt);
		if (auc_ptr->bids_cnt)
		{
			C_MAKE(auc_ptr->bids, auc_ptr->bids_cnt, bid_type);
		}
		for (j = 0; j < auc_ptr->bids_cnt; j++)
		{
			bid_ptr = &auc_ptr->bids[j];
			rd_s32b(&bid_ptr->bid);
			rd_s32b(&bid_ptr->bidder);
		}
		rd_s32b(&auc_ptr->winning_bid);

		/* The time_t's are saved as s32b's */
		rd_s32b(&tmp32s);
		auc_ptr->start = (time_t) tmp32s;
		rd_s32b(&tmp32s);
		auc_ptr->duration = (time_t) tmp32s;
	}
}

/*
 * Actually read the savefile
 *
 * Angband 2.8.0 will completely replace this code, see "save.c",
 * though this code will be kept to read pre-2.8.0 savefiles.
 */
static errr rd_savefile_new_aux(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int i, err_code = 0;

//	byte panic;
	u16b tmp16u;
	u32b tmp32u;


#ifdef VERIFY_CHECKSUMS
	u32b n_x_check, n_v_check;
	u32b o_x_check, o_v_check;
#endif

	/* Strip the version bytes */
	strip_bytes(4);

	/* Hack -- decrypt */
	xor_byte = sf_extra;


	/* Clear the checksums */
	v_check = 0L;
	x_check = 0L;


	/* Operating system info */
	rd_u32b(&sf_xtra);

	/* Time of savefile creation */
	rd_u32b(&sf_when);

	/* Number of resurrections */
	rd_u16b(&sf_lives);

	/* Number of times played */
	rd_u16b(&sf_saves);


	/* Later use (always zero) */
	rd_u32b(&tmp32u);

	/* Later use (always zero) */
	rd_u32b(&tmp32u);

	/* Object Memory */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > MAX_K_IDX)
	{
		s_printf(format("Too many (%u) object kinds!\n", tmp16u));
		return (22);
	}

	/* Read the object memory */
	for (i = 0; i < tmp16u; i++)
	{
		byte tmp8u;

		rd_byte(&tmp8u);

		Players[Ind]->obj_aware[i] = (tmp8u & 0x01) ? TRUE : FALSE;
		Players[Ind]->obj_tried[i] = (tmp8u & 0x02) ? TRUE : FALSE;
		Players[Ind]->obj_felt[i] = (tmp8u & 0x04) ? TRUE : FALSE;
		Players[Ind]->obj_felt_heavy[i] = (tmp8u & 0x08) ? TRUE : FALSE;
	}

	/* Trap Memory */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > MAX_T_IDX)
	{
		s_printf(format("Too many (%u) trap kinds!\n", tmp16u));
		return (22);
	}

	/* Read the object memory */
	for (i = 0; i < tmp16u; i++)
	{
		byte tmp8u;

		rd_byte(&tmp8u);

		Players[Ind]->trap_ident[i] = (tmp8u & 0x01) ? TRUE : FALSE;
	}

	/* Read the extra stuff */
	err_code = rd_extra(Ind);
	if (err_code == 1)
		return 1;
	else if (err_code)
		return 35;

	/* Read the player_hp array */
	rd_u16b(&tmp16u);

	/* Read the player_hp array */
	for (i = 0; i < tmp16u; i++)
	{
		rd_s16b(&p_ptr->player_hp[i]);
	}


	/* Important -- Initialize the race/class */
	p_ptr->rp_ptr = &race_info[p_ptr->prace];
	p_ptr->cp_ptr = &class_info[p_ptr->pclass];

	/* Read the inventory */
	if (rd_inventory(Ind))
	{
		s_printf("Unable to read inventory\n");
		return (21);
	}

	/* Read hostility information if new enough */
	if (rd_hostilities(Ind))
	{
		return (22);
	}

	/* read the dungeon memory if new enough */
	rd_cave_memory(Ind);

	/* read the wilderness map if new enough */
	{
		/* get the map size */
		rd_u32b(&tmp32u);

		/* if too many map entries */
		if (tmp32u > MAX_WILD_X*MAX_WILD_Y)
		{
			return 23;
		}

		/* read in the map */
		for (i = 0; i < (int) tmp32u; i++)
		{
			rd_byte(&p_ptr->wild_map[i]);
		}
	}

	/* continued 'else' branch from rd_extra(), now that we have wild_map[] array */
        if (older_than(4, 4, 23) || p_ptr->dummy_option_8) {
		/* in case we're here by a fix-hack */
		if (p_ptr->dummy_option_8) {
			s_printf("fixing max_depth[] for '%s'\n", p_ptr->name);
			p_ptr->dummy_option_8 = FALSE;
		}

		/* hack - Sauron vs Shadow of Dol Guldur - just for consistency */
		if (p_ptr->r_killed[RI_SAURON] == 1) p_ptr->r_killed[RI_DOL_GULDUR] = 1;

		/* init his max_depth[] array */
		fix_max_depth(p_ptr);
	}

	/* read player guild membership */
	rd_byte(&p_ptr->guild);
	if (!older_than(4, 5, 8)) rd_u32b(&p_ptr->guild_dna);
	else if (p_ptr->guild) p_ptr->guild_dna = guilds[p_ptr->guild].dna;
	rd_u16b(&p_ptr->quest_id);
        rd_s16b(&p_ptr->quest_num);

        if (!older_than(4, 0, 1)) {
                rd_byte(&p_ptr->spell_project);
        } else {
                p_ptr->spell_project = 0;
        }

        /* Special powers */
        if (!older_than(4, 0, 2)) {
                rd_s16b(&p_ptr->power_num);

                for (i = 0; i < MAX_POWERS; i++) {
                        rd_s16b(&p_ptr->powers[i]);
                }
        } else {
                p_ptr->power_num = 0;
        }

#ifdef VERIFY_CHECKSUMS

	/* Save the checksum */
	n_v_check = v_check;

	/* Read the old checksum */
	rd_u32b(&o_v_check);

	/* Verify */
	if (o_v_check != n_v_check)
	{
		s_printf("Invalid checksum\n");
		return (11);
	}


	/* Save the encoded checksum */
	n_x_check = x_check;

	/* Read the checksum */
	rd_u32b(&o_x_check);


	/* Verify */
	if (o_x_check != n_x_check)
	{
		s_printf("Invalid encoded checksum\n");
		return (11);
	}

#endif


	/* Hack -- no ghosts */
	r_info[MAX_R_IDX-1].max_num = 0;

	/* Initialize a little more */
	p_ptr->ignore = NULL;
	p_ptr->w_ignore = NULL;
	p_ptr->afk = FALSE;

	/* Success */
	return (0);
}


/*
 * Actually read the savefile
 *
 * Angband 2.8.0 will completely replace this code, see "save.c",
 * though this code will be kept to read pre-2.8.0 savefiles.
 */
errr rd_savefile_new(int Ind)
{
	player_type *p_ptr = Players[Ind];

	errr err;

	/* The savefile is a binary file */
	fff = my_fopen(p_ptr->savefile, "rb");

	/* Paranoia */
	if (!fff) return (-1);

	/* Allocate a new buffer */
	fff_buf = C_NEW(MAX_BUF_SIZE, char);
	fff_buf_pos = MAX_BUF_SIZE;

	/* Call the sub-function */
	err = rd_savefile_new_aux(Ind);

	/* Free the buffer */
	C_FREE(fff_buf, MAX_BUF_SIZE, char);

	/* Check for errors */
	if (ferror(fff)) err = -1;

	/* Close the file */
	my_fclose(fff);

	/* Result */
	return (err);
}

errr rd_server_savefile()
{
	unsigned int i;

	errr err = 0;

	char savefile[MAX_PATH_LENGTH];

	byte tmp8u, panic = 0;
	u16b tmp16u;
	s16b tmp16s;
	u32b tmp32u;
	s32b tmp32s;

	/* Savefile name */
	path_build(savefile, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "server");

	/* The server savefile is a binary file */
	fff = my_fopen(savefile, "rb");

	/* Paranoia */
	if (!fff) return (-1);

	/* Allocate a new buffer */
	fff_buf = C_NEW(MAX_BUF_SIZE, char);
	fff_buf_pos = MAX_BUF_SIZE;

	/* Read the version */
	xor_byte = 0;
	rd_byte(&ssf_major);
	xor_byte = 0;
	rd_byte(&ssf_minor);
	xor_byte = 0;
	rd_byte(&ssf_patch);
	xor_byte = 0;
	rd_byte(&ssf_extra);

	/* Hack -- decrypt */
	xor_byte = sf_extra;


	/* Clear the checksums */
	v_check = 0L;
	x_check = 0L;


	/* Operating system info */
	rd_u32b(&sf_xtra);

	/* Time of savefile creation */
	rd_u32b(&sf_when);

	/* Number of lives */
	rd_u16b(&sf_lives);

	/* Number of times played */
	rd_u16b(&sf_saves);

	/* Was the last shutdown a panic save? - C. Blue */
	if (!s_older_than(4, 2, 2)) rd_byte(&panic); /* no further use yet */
	if (panic) s_printf("Last server shutdown was a panic save.\n");

	/* read server state regarding any updates (for lua) - C. Blue */
	if (!s_older_than(4, 3, 19)) {
		rd_s16b(&tmp16s);
		updated_server = tmp16s;
	}
	/* read server state regarding artifact updates */
	if (!s_older_than(4, 3, 22)) {
		rd_s16b(&tmp16s);
		artifact_reset = tmp16s;
	}

	/* Later use (always zero) */
	rd_u32b(&tmp32u);

	/* Later use (always zero) */
	rd_u32b(&tmp32u);

	/* Monster Memory */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > MAX_R_IDX) {
		s_printf(format("Too many (%u) monster races!\n", tmp16u));
		return (21);
	}

	/* Read the available records */
	for (i = 0; i < tmp16u; i++) {
		monster_race *r_ptr;

		/* Read the monster race information */
		rd_global_lore(i);

		/* Access the monster race */
		r_ptr = &r_info[i];
	}

	/* Load the Artifacts */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > MAX_A_IDX) {
		s_printf(format("Too many (%u) artifacts!\n", tmp16u));
		return (24);
	}
	/* Read the artifact flags */
	for (i = 0; i < tmp16u; i++) {
		rd_byte(&tmp8u);
		a_info[i].cur_num = tmp8u;
		rd_byte(&tmp8u);
		a_info[i].known = tmp8u;
		if (s_older_than(4, 5, 5)) {
			rd_byte(&tmp8u);
			rd_byte(&tmp8u);
			a_info[i].carrier = 0;
		} else rd_s32b(&a_info[i].carrier);
		if (s_older_than(4, 5, 6)) determine_artifact_timeout(i);
		else {
			rd_s32b(&a_info[i].timeout);
#ifdef FLUENT_ARTIFACT_RESETS
			/* fluent-artifact-timeout was temporarily disabled? reset timer now then */
			if (a_info[i].timeout == -2) determine_artifact_timeout(i);
#else
			a_info[i].timeout = -2;
#endif
		}
	}

	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > MAX_PARTIES) {
		s_printf(format("Too many (%u) parties!\n", tmp16u));
		return (25);
	}

	/* Read the available records */
	for (i = 0; i < tmp16u; i++) {
		rd_party(i);
	}
	if (s_older_than(4, 2, 4)) {
		for (i = tmp16u; i < MAX_PARTIES; i++) {
			/* HACK Initialize new parties just to make sure they'll work - mikaelh */
			parties[i].members = 0;
			parties[i].created = 0;
		}
	}

	/* XXX If new enough, read in the saved levels and monsters. */

	/* read the number of levels to be loaded */
	rd_towns();
	new_rd_wild();
	new_rd_floors();

	/* get the number of monsters to be loaded */
	rd_u32b(&tmp32u);
	if (tmp32u > MAX_M_IDX) {
		s_printf(format("Too many (%u) monsters!\n", tmp16u));
		return (29);
	}
	/* load the monsters */
	for (i = 0; i < tmp32u; i++) rd_monster(&m_list[m_pop()]);

	/* Read object info if new enough */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > MAX_O_IDX) {
		s_printf(format("Too many (%u) objects!\n", tmp16u));
		return (26);
	}

	/* Read the available records */
	for (i = 0; i < tmp16u; i++) rd_item(&o_list[i]);

	/* Set the maximum object number */
	o_max = tmp16u;

	/* Read house info if new enough */
	rd_s32b(&num_houses);

	while (house_alloc <= num_houses) {
		GROW(houses, house_alloc, house_alloc + 512, house_type);
		house_alloc += 512;
	}

	/* Incompatible save files */
	if (num_houses > MAX_HOUSES) {
		s_printf(format("Too many (%u) houses!\n", num_houses));
		return (27);
	}

	/* Read the available records */
	for (i = 0; i < (unsigned int) num_houses; i++) {
		rd_house(i);
		if(!(houses[i].flags&HF_STOCK))
			wild_add_uhouse(&houses[i]);
	}

	/* Read the player name database if new enough */
	{
		char name[80];
		byte level, old_party, guild, race, class, mode;
		u32b acct, guild_flags;
		u16b party;
		u16b quest;

		rd_u32b(&tmp32u);

		/* Read the available records */
		for (i = 0; i < tmp32u; i++) {
			s32b laston;

			/* Read the ID */
			rd_s32b(&tmp32s);
			rd_u32b(&acct);
			rd_s32b(&laston);
			rd_byte(&race);
#ifdef ENABLE_KOBOLD /* Kobold was inserted in the middle, instead of added to the end of the races array */
		        if (s_older_than(4, 4, 28) && race >= RACE_KOBOLD) race++;
#endif
			rd_byte(&class);
		        if (!s_older_than(4, 2, 2)) rd_byte(&mode); else mode = 0;
			rd_byte(&level);
			if (s_older_than(4, 2, 4)) {
				rd_byte(&old_party);
				party = old_party; /* convert old byte to u16b - mikaelh */
			}
			else rd_u16b(&party);
			rd_byte(&guild);
		        if (!s_older_than(4, 5, 1)) rd_u32b(&guild_flags); else guild_flags = 0;
			rd_u16b(&quest);

			/* Read the player name */
			rd_string(name, 80);

			/* Store the player name */
			add_player_name(name, tmp32s, acct, race, class, mode, level, party, guild, guild_flags, quest, laston);
		}
	}

	rd_u32b(&seed_flavor);
	rd_u32b(&seed_town);

	rd_guilds();

	rd_quests();

	rd_u32b(&account_id);
	rd_s32b(&player_id);

	rd_s32b(&turn);

	if (!s_older_than(4, 3, 18)) rd_notes();
	if (!s_older_than(4, 3, 6)) rd_bbs();

	if (!s_older_than(4, 3, 13)) rd_byte(&season);
	if (!s_older_than(4, 3, 14)) rd_byte(&weather_frequency);

	if (!s_older_than(4, 3, 25)) rd_auctions();

	/* read Ironman Deep Dive Challenge records */
	if (!s_older_than(4, 4, 18)) {
	        for (i = 0; i < 20; i++) {
			rd_s16b(&tmp16s);
			deep_dive_level[i] = tmp16s;
			rd_string(deep_dive_name[i], MAX_CHARS);
		}
	}

	/* Hack -- no ghosts */
	r_info[MAX_R_IDX-1].max_num = 0;

	/* Free the buffer */
	C_FREE(fff_buf, MAX_BUF_SIZE, char);

	/* Check for errors */
	if (ferror(fff)) err = -1;

	/* Close the file */
	my_fclose(fff);

	/* Result */
	return (err);
}

void new_rd_wild()
{
	int x, y, i;
#ifdef DUNGEON_VISIT_BONUS
	int dungeons = 0;
#endif
	wilderness_type *wptr;
	struct dungeon_type *d_ptr;
	u32b tmp;
	rd_u32b(&tmp);	/* MAX_WILD_Y */
	rd_u32b(&tmp);	/* MAX_WILD_X */
	for (y = 0; y < MAX_WILD_Y; y++) {
		for (x = 0; x < MAX_WILD_X; x++) {
			wptr = &wild_info[y][x];
			rd_wild(wptr);
			if (wptr->flags & WILD_F_DOWN) {
				MAKE(d_ptr, struct dungeon_type);
				rd_byte(&wptr->up_x);
				rd_byte(&wptr->up_y);
				rd_u16b(&d_ptr->id);
				rd_u16b(&d_ptr->type);
				rd_u16b(&d_ptr->baselevel);
				rd_u32b(&d_ptr->flags1);
				rd_u32b(&d_ptr->flags2);
				if (!s_older_than(4, 4, 24)) rd_u32b(&d_ptr->flags3);
				rd_byte(&d_ptr->maxdepth);

#ifdef DUNGEON_VISIT_BONUS
				dungeons++;
				if (d_ptr->id > dungeon_id_max) dungeon_id_max = d_ptr->id;
				if (d_ptr->id != 0) {
					dungeon_x[d_ptr->id] = x;
					dungeon_y[d_ptr->id] = y;
					dungeon_tower[d_ptr->id] = FALSE;
				}
#endif
				if (d_ptr->type == DI_NETHER_REALM) {
					netherrealm_wpos_x = x;
					netherrealm_wpos_y = y;
					netherrealm_wpos_z = -1;
					netherrealm_start = d_ptr->baselevel;
					netherrealm_end = d_ptr->baselevel + d_ptr->maxdepth - 1;
				}
				else if (d_ptr->type == DI_VALINOR) {
					valinor_wpos_x = x;
					valinor_wpos_y = y;
					valinor_wpos_z = -1;
				}

#if 0
				for (i = 0; i < 10; i++) {
					rd_byte((byte*)&d_ptr->r_char[i]);
					rd_byte((byte*)&d_ptr->nr_char[i]);
				}
#else
				strip_bytes(20);
#endif

				if (!s_older_than(4, 4, 21)) {
#ifdef DUNGEON_VISIT_BONUS
					rd_u16b(&dungeon_visit_frequency[d_ptr->id]);
					set_dungeon_bonus(d_ptr->id, FALSE);
#else
					strip_bytes(2);
#endif
				}

				C_MAKE(d_ptr->level, d_ptr->maxdepth, struct dun_level);
				for (i = 0; i < d_ptr->maxdepth; i++) {
					C_MAKE(d_ptr->level[i].uniques_killed, MAX_R_IDX, char);
				}
				wptr->dungeon = d_ptr;
			}
			if (wptr->flags & WILD_F_UP) {
				MAKE(d_ptr, struct dungeon_type);
				rd_byte(&wptr->dn_x);
				rd_byte(&wptr->dn_y);
				rd_u16b(&d_ptr->id);
				rd_u16b(&d_ptr->type);
				rd_u16b(&d_ptr->baselevel);
				rd_u32b(&d_ptr->flags1);
				rd_u32b(&d_ptr->flags2);
				if (!s_older_than(4, 4, 24)) rd_u32b(&d_ptr->flags3);
				rd_byte(&d_ptr->maxdepth);

#ifdef DUNGEON_VISIT_BONUS
				dungeons++;
				if (d_ptr->id > dungeon_id_max) dungeon_id_max = d_ptr->id;
				if (d_ptr->id != 0) {
					dungeon_x[d_ptr->id] = x;
					dungeon_y[d_ptr->id] = y;
					dungeon_tower[d_ptr->id] = TRUE;
				}
#endif
				if (d_ptr->type == DI_NETHER_REALM) {
					netherrealm_wpos_x = x;
					netherrealm_wpos_y = y;
					netherrealm_wpos_z = 1;
					netherrealm_start = d_ptr->baselevel;
					netherrealm_end = d_ptr->baselevel + d_ptr->maxdepth - 1;
				}
				else if (d_ptr->type == DI_VALINOR) {
					valinor_wpos_x = x;
					valinor_wpos_y = y;
					valinor_wpos_z = 1;
				}

#if 0
				for (i = 0; i < 10; i++) {
					rd_byte((byte*)&d_ptr->r_char[i]);
					rd_byte((byte*)&d_ptr->nr_char[i]);
				}
#else
				strip_bytes(20);
#endif

				if (!s_older_than(4, 4, 21)) {
#ifdef DUNGEON_VISIT_BONUS
					rd_u16b(&dungeon_visit_frequency[d_ptr->id]);
					set_dungeon_bonus(d_ptr->id, FALSE);
#else
					strip_bytes(2);
#endif
				}

				C_MAKE(d_ptr->level, d_ptr->maxdepth, struct dun_level);
				for (i = 0; i < d_ptr->maxdepth; i++) {
					C_MAKE(d_ptr->level[i].uniques_killed, MAX_R_IDX, char);
				}
				wptr->tower = d_ptr;
			}
		}
	}
#ifdef DUNGEON_VISIT_BONUS
	if (dungeons != dungeon_id_max) {
		s_printf("Mismatch: Found %d dungeons and %d dungeon ids -> reindexing:\n", dungeons, dungeon_id_max);
		reindex_dungeons();
	}
#endif
}

void new_rd_floors()
{
	while(!rd_floor());
}

void rd_towns()
{
	int i, j;
	struct worldpos twpos;
	twpos.wz = 0;

	/* Dealloc stores first - mikaelh */
	for (i = 0; i < numtowns; i++) dealloc_stores(i);
	C_KILL(town, numtowns, struct town_type); /* first is alloced */

	rd_u16b(&numtowns);
	C_MAKE(town, numtowns, struct town_type);
	for (i = 0; i < numtowns; i++) {
		rd_u16b(&town[i].x);
		rd_u16b(&town[i].y);
		rd_u16b(&town[i].baselevel);
		rd_u16b(&town[i].flags);
		rd_u16b(&town[i].num_stores);
		rd_u16b(&town[i].type);
		wild_info[town[i].y][town[i].x].type = WILD_TOWN;
		wild_info[town[i].y][town[i].x].radius = town[i].baselevel;
		wild_info[town[i].y][town[i].x].town_idx = i;
		twpos.wx = town[i].x;
		twpos.wy = town[i].y;
		alloc_stores(i);
		for (j = 0; j < town[i].num_stores; j++) {
			rd_store(&town[i].townstore[j]);
		}
	}
}

void load_guildhalls(struct worldpos *wpos){
	int i;
	FILE *gfp;
	struct guildsave data;
	char buf[1024];
	char fname[30];
	data.mode = 0;
	for (i = 0; i < num_houses; i++) {
		if ((houses[i].dna->owner_type == OT_GUILD) && (inarea(wpos, &houses[i].wpos))) {
			if (!houses[i].dna->owner) continue;
#if DEBUG_LEVEL > 2
			s_printf("load guildhall %d\n", i);
#endif
			sprintf(fname, "guild%.4d.data", i);
//			path_build(buf, 1024, ANGBAND_DIR_DATA, fname);
			path_build(buf, 1024, ANGBAND_DIR_SAVE, fname);/* moved this 'file spam' over to save... C. Blue */
			gfp = fopen(buf, "rb");
			if (gfp == (FILE*)NULL) continue;
			data.fp = gfp;
			fill_house(&houses[i], FILL_GUILD, (void*)&data);
			fclose(gfp);
		}
	}
}

void save_guildhalls(struct worldpos *wpos){
	int i;
	FILE *gfp;
	struct guildsave data;
	char buf[1024];
	char fname[30];
	data.mode = 1;
	for (i = 0; i < num_houses; i++) {
		if ((houses[i].dna->owner_type == OT_GUILD) && (inarea(wpos, &houses[i].wpos))) {
			if (!houses[i].dna->owner) continue;
#if DEBUG_LEVEL > 2
			s_printf("save guildhall %d\n", i);
#endif
			sprintf(fname, "guild%.4d.data", i);
//			path_build(buf, 1024, ANGBAND_DIR_DATA, fname);
			path_build(buf, 1024, ANGBAND_DIR_SAVE, fname); /* moved this 'file spam' over to save... C. Blue */
			gfp = fopen(buf, "rb+");
			if (gfp == (FILE*)NULL) {
				gfp = fopen(buf, "wb");
				if (gfp == (FILE*)NULL)
					continue;
			}
			data.fp = gfp;
			fill_house(&houses[i], FILL_GUILD, (void*)&data);
			fclose(gfp);
		}
	}
}

/* fix max_depth[] array for a character, which is usually done
   when converting an outdated savegame. - C. Blue */
void fix_max_depth(player_type *p_ptr) {
	struct worldpos wpos;
	int i, j, x, y;
	dungeon_type *d_ptr;

	/* wipe (paranoia, should already be zeroed) */
	for (i = 0; i < MAX_D_IDX * 2; i++) {
		p_ptr->max_depth_wx[i] = 0;
		p_ptr->max_depth_wy[i] = 0;
		p_ptr->max_depth[i] = 0;
		p_ptr->max_depth_tower[i] = FALSE;
	}
	/* attempt a fair translation */
	for (x = 0; x < 64; x++) for (y = 0; y < 64; y++) {
		/* exempt 0,0 */
		if (!x && !y) continue;

                wpos.wx = x; wpos.wy = y;

		/* skip if player has never visited this dungeon's worldmap sector even! */
		if (!is_admin(p_ptr) &&
		    !(p_ptr->wild_map[wild_idx(&wpos) / 8] & (1 << (wild_idx(&wpos) % 8)))) continue;

		/* translate */
		if ((d_ptr = wild_info[y][x].tower)) {
			wpos.wz = 1;
			j = recall_depth_idx(&wpos, p_ptr);
			i = p_ptr->max_dlv - d_ptr->baselevel + 1;
			if (i > d_ptr->maxdepth) i = d_ptr->maxdepth;
			if (x == WPOS_IRONDEEPDIVE_X && y == WPOS_IRONDEEPDIVE_Y && 1 == WPOS_IRONDEEPDIVE_Z) i = 0;
			p_ptr->max_depth[j] = i;
		}
		if ((d_ptr = wild_info[y][x].dungeon)) {
			wpos.wz = -1;
			j = recall_depth_idx(&wpos, p_ptr);
			i = p_ptr->max_dlv - d_ptr->baselevel + 1;
			if (i > d_ptr->maxdepth) i = d_ptr->maxdepth;
			if (x == WPOS_IRONDEEPDIVE_X && y == WPOS_IRONDEEPDIVE_Y && -1 == WPOS_IRONDEEPDIVE_Z) i = 0;
			p_ptr->max_depth[j] = i;
		}
	}

	s_printf("max_depth[] has been fixed/reset for '%s'.\n", p_ptr->name);
}

/* Note: there was still a 0,0 tower in my DM's list, but whatever.. */
void fix_max_depth_bug(player_type *p_ptr) {
	int i, j;

	for (i = 0; i < MAX_D_IDX * 2; i++) {
#if 0 /* faster */
		if (p_ptr->max_depth[i]) continue; /* (all bugged entries have depth 0) */
#else /* cleaner */
		if (!p_ptr->max_depth_wx[i] && !p_ptr->max_depth_wy[i]) continue; /* entry doesn't exist? */
		if (p_ptr->max_depth_tower[i] || /* it's not a dungeon or it is and that dungeon actually exists? */
		    wild_info[p_ptr->max_depth_wy[i]][p_ptr->max_depth_wx[i]].dungeon)
			/* (all bugged entries are 'dungeons') */
			continue;
#endif
		/* wipe: all "dungeons" that do not exist */
		p_ptr->max_depth_wx[i] = 0;
		p_ptr->max_depth_wy[i] = 0;
		//p_ptr->max_depth_tower[i] = FALSE; (redundant)

		/* important: move the following dungeons in our personal list up one,
		   so as to not leave any holes (breaks the get_recall_depth algo) */
		for (j = i; j < MAX_D_IDX * 2 - 1; j++) {
			p_ptr->max_depth[j] = p_ptr->max_depth[j + 1];
			p_ptr->max_depth_wx[j] = p_ptr->max_depth_wx[j + 1];
			p_ptr->max_depth_wy[j] = p_ptr->max_depth_wy[j + 1];
			p_ptr->max_depth_tower[j] = p_ptr->max_depth_tower[j + 1];
		}
		/* wipe the last entry accordingly, since it has been moved up by one */
		p_ptr->max_depth[j] = 0;
		p_ptr->max_depth_wx[j] = 0;
		p_ptr->max_depth_wy[j] = 0;
		p_ptr->max_depth_tower[j] = FALSE;
	}

	s_printf("max_depth[] has been bug-fixed for '%s'.\n", p_ptr->name);
}
void condense_max_depth(player_type *p_ptr) {
	int i, j, k, d;
	/* moar fixing old bugginess: remove all 0,0,0 entries between valid entries
	   (empty entries aka 0,0,0 should only occur tailing the other entries) */
	for (i = 0; i < MAX_D_IDX * 2; i++) {
		/* hack: fix IDDC to zero */
		if (p_ptr->max_depth_wx[i] == WPOS_IRONDEEPDIVE_X && p_ptr->max_depth_wy[i] == WPOS_IRONDEEPDIVE_Y
		    && (p_ptr->max_depth_tower[i] ? 1 : -1) == WPOS_IRONDEEPDIVE_Z) {
			p_ptr->max_depth[i] = 0;
			continue;
		}
	
		if (p_ptr->max_depth_wx[i] || p_ptr->max_depth_wy[i]) continue; /* entry is not empty? */
		/* ..entry i is empty.. */
		for (j = i + 1; j < MAX_D_IDX * 2; j++) {
			if (!p_ptr->max_depth_wx[j] && !p_ptr->max_depth_wy[j]) continue; /* entry is correctly empty? */

			/* move it down by one */
			d = j - i;
			for (k = i; k < MAX_D_IDX * 2 - d; k++) {
				p_ptr->max_depth[k] = p_ptr->max_depth[k + d];
				p_ptr->max_depth_wx[k] = p_ptr->max_depth_wx[k + d];
				p_ptr->max_depth_wy[k] = p_ptr->max_depth_wy[k + d];
				p_ptr->max_depth_tower[k] = p_ptr->max_depth_tower[k + d];
			}
			/* wipe the last d entries accordingly, since they have been moved up by d */
			for (k = MAX_D_IDX * 2 - d; k < MAX_D_IDX * 2; k++) {
				p_ptr->max_depth[k] = 0;
				p_ptr->max_depth_wx[k] = 0;
				p_ptr->max_depth_wy[k] = 0;
				p_ptr->max_depth_tower[k] = FALSE;
			}
			break;
		}
	}
	s_printf("max_depth[] has been condensed for '%s'.\n", p_ptr->name);
}

#ifdef SEAL_INVALID_OBJECTS
static void seal_object(object_type *o_ptr) {
	s_printf("SEALING: %d, %d\n", o_ptr->tval, o_ptr->sval);
	o_ptr->tval2 = o_ptr->tval;
	o_ptr->sval2 = o_ptr->sval;
	o_ptr->note = quark_add(format("%d-%d", o_ptr->tval2, o_ptr->sval2));
	o_ptr->tval = TV_SPECIAL;
	o_ptr->sval = SV_SEAL;
	o_ptr->k_idx = lookup_kind(o_ptr->tval, o_ptr->sval);
}
static void unseal_object(object_type *o_ptr) {
	o_ptr->tval = o_ptr->tval2;
	o_ptr->sval = o_ptr->sval2;
	o_ptr->k_idx = lookup_kind(o_ptr->tval, o_ptr->sval);
	o_ptr->note = 0;
	o_ptr->note_utag = 0;
	s_printf("UNSEALING: %d, %d\n", o_ptr->tval, o_ptr->sval);
}
/* Seal or unseal an object as required,
   or return FALSE if object no longer exists - C. Blue */
bool seal_or_unseal_object(object_type *o_ptr) {
	/* Object does no longer exist? (for example now commented out, in k_info)
	   - turn it into a 'seal' instead of deleting it! */
	if (!o_ptr->k_idx) {
		/* Object does no longer exist? Delete it! */
		if (!o_ptr->tval && !o_ptr->sval) return FALSE;

		seal_object(o_ptr);

		/* In case someone is silly and removes seals while leaving the definition enabled: */
		if (!o_ptr->k_idx) return FALSE;
//		s_printf("sealed to %d, %d\n", o_ptr->tval, o_ptr->sval);
	} else if (o_ptr->tval == TV_SPECIAL && o_ptr->sval == SV_SEAL) {
		/* Object didn't exist to begin with? Delete it! */
		if (!o_ptr->tval2 && !o_ptr->sval2) {
			o_ptr->tval = o_ptr->sval = o_ptr->k_idx = 0;
			return FALSE;
		}

		/* Try to restore the original item from the seal */
		if (lookup_kind(o_ptr->tval2, o_ptr->sval2)) unseal_object(o_ptr);
	}

	/* success, aka object still exists */
	return TRUE;
}
#endif
