/* $Id$ */
/* File: c-spell.c */
/* Client-side spell stuff */

#include "angband.h"

/* NULL for ghost - FIXME */
static void print_spells(object_type *o_ptr) {
	int	i, col, realm, num;

	/* Hack -- handle ghosts right */
	if (p_ptr->ghost) {
		realm = REALM_GHOST;
		num = 0;
	} else {
		realm = find_realm(o_ptr->tval);
		num = o_ptr->sval;
	}


	/* Print column */
	col = 20;

	/* Title the list */
	prt("", 1, col);
	put_str("Name", 1, col + 5);
	put_str("Lv Mana Fail", 1, col + 35);

	/* Dump the spells */
	for (i = 0; i < 9; i++) {
		/* Check for end of the book */
		if (spell_info[realm][num][i][0] == '\0')
			break;

		/* Dump the info */
		prt(spell_info[realm][num][i], 2 + i, col);
	}

	/* Clear the bottom line */
	prt("", 2 + i, col);
}

/* Query mimicry.lua functions to determine name, mana cost and fail rate
   instead of using hard-coded info from common/tables.c. - C. Blue */
#define MIMIC_LUA
static void print_mimic_spells() {
	int i, col, j = 2, k, fail, j_max = 0;
	char buf[90];
#ifdef MIMIC_LUA
	char name[MAX_CHARS];
	int mana;
#endif

	/* Print column */
	col = 13;
	k = 0; /* next column? for forms with LOTS of spells (GWoP) */

	/* Title the list */
	prt("", 1, col);
	c_put_str(TERM_WHITE, "Name", 1, col + 4);
	c_put_str(TERM_WHITE, " Cost", 1, col + 4 + 22 - 2);
	c_put_str(TERM_WHITE, "Fail", 1, col + 4 + 22 + 6 - 2);

	prt("", j, col);
	c_put_str(TERM_L_WHITE, " a) Polymorph Self into next known form", j++, col);

	prt("", j, col);
	c_put_str(TERM_L_WHITE, " b) Polymorph Self into next known form with fitting extremities", j++, col);

	prt("", j, col);
	c_put_str(TERM_L_WHITE, " c) Polymorph Self into a specific form", j++, col);

	prt("", j, col);
	c_put_str(TERM_L_WHITE, " d) Set preferred immunity", j++, col);

	/* Dump the spells */
	for (i = 0; i < 32; i++) {
		/* Check if power is available to us */
		if (!(p_ptr->innate_spells[0] & (1L << i))) continue;

#ifndef MIMIC_LUA
		fail = (innate_powers[i].sfail * adj_int_pow[p_ptr->stat_ind[A_INT]]) / 100;
		if (fail < 1) fail = 1;
		if (fail > 99) fail = 99;
#else
		sprintf(name, "return get_monster_spell_info(%d, %d, %d, %d, %d)", 0, i, p_ptr->stat_use[A_INT], p_ptr->stat_use[A_DEX], get_skill_scale(p_ptr, SKILL_MIMIC, 100));//abuse name
		strcpy(buf, string_exec_lua(0, name));
		strcpy(name, buf); *strchr(name, '_') = 0;
		mana = atoi(strstr(buf, "_m") + 2);
		fail = atoi(strstr(buf, "_f") + 2);
#endif

		/* fill right side of the list with blanks, in case we don't have 2 columns of spells - looks better */
		if (!k) put_str("                                 ", j, col + 33);

		/* Dump the info */
#ifndef MIMIC_LUA
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), monster_spells4[i].name, innate_powers[i].smana, fail);
#else
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), name, mana, fail);
#endif
		Term_putstr(col + k * 33, j++, -1, TERM_WHITE, buf);

		/* check for beginning of 2nd column */
		if (j > 21) {
			j_max = j;
			j = 6;
			k = 1;
		}
	}
	for (i = 0; i < 32; i++) {
		/* Check if power is available to us */
		if (!(p_ptr->innate_spells[1] & (1L << i))) continue;

#ifndef MIMIC_LUA
		fail = (innate_powers[i + 32].sfail * adj_int_pow[p_ptr->stat_ind[A_INT]]) / 100;
		if (fail < 1) fail = 1;
		if (fail > 99) fail = 99;
#else
		sprintf(name, "return get_monster_spell_info(%d, %d, %d, %d, %d)", 1, i, p_ptr->stat_use[A_INT], p_ptr->stat_use[A_DEX], get_skill_scale(p_ptr, SKILL_MIMIC, 100));//abuse name
		strcpy(buf, string_exec_lua(0, name));
		strcpy(name, buf); *strchr(name, '_') = 0;
		mana = atoi(strstr(buf, "_m") + 2);
		fail = atoi(strstr(buf, "_f") + 2);
#endif

		/* fill right side of the list with blanks, in case we don't have 2 columns of spells - looks better */
		if (!k) put_str("                                 ", j, col + 33);

		/* Dump the info */
#ifndef MIMIC_LUA
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), monster_spells5[i].name, innate_powers[i + 32].smana, fail);
#else
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), name, mana, fail);
#endif
		Term_putstr(col + k * 33, j++, -1, TERM_WHITE, buf);

		/* check for beginning of 2nd column */
		if (j > 21) {
			j_max = j;
			j = 6;
			k = 1;
		}
	}
	for (i = 0; i < 32; i++) {
		/* Check if power is available to us */
		if (!(p_ptr->innate_spells[2] & (1L << i)))
			continue;

#ifndef MIMIC_LUA
		fail = (innate_powers[i + 64].sfail * adj_int_pow[p_ptr->stat_ind[A_INT]]) / 100;
		if (fail < 1) fail = 1;
		if (fail > 99) fail = 99;
#else
		sprintf(name, "return get_monster_spell_info(%d, %d, %d, %d, %d)", 2, i, p_ptr->stat_use[A_INT], p_ptr->stat_use[A_DEX], get_skill_scale(p_ptr, SKILL_MIMIC, 100));//abuse name
		strcpy(buf, string_exec_lua(0, name));
		strcpy(name, buf); *strchr(name, '_') = 0;
		mana = atoi(strstr(buf, "_m") + 2);
		fail = atoi(strstr(buf, "_f") + 2);
#endif

		/* fill right side of the list with blanks, in case we don't have 2 columns of spells - looks better */
		if (!k) put_str("                                 ", j, col + 33);

		/* Dump the info */
#ifndef MIMIC_LUA
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), monster_spells6[i].name, innate_powers[i + 64].smana, fail);
#else
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), name, mana, fail);
#endif
		Term_putstr(col + k * 33, j++, -1, TERM_WHITE, buf);

		/* check for beginning of 2nd column */
		if (j > 21) {
			j_max = j;
			j = 6;
			k = 1;
		}
	}
	for (i = 0; i < 32; i++) {
		/* Check if power is available to us */
		if (!(p_ptr->innate_spells[3] & (1L << i)))
			continue;

#ifndef MIMIC_LUA
		fail = (innate_powers[i + 96].sfail * adj_int_pow[p_ptr->stat_ind[A_INT]]) / 100;
		if (fail < 1) fail = 1;
		if (fail > 99) fail = 99;
#else
		sprintf(name, "return get_monster_spell_info(%d, %d, %d, %d, %d)", 3, i, p_ptr->stat_use[A_INT], p_ptr->stat_use[A_DEX], get_skill_scale(p_ptr, SKILL_MIMIC, 100));//abuse name
		strcpy(buf, string_exec_lua(0, name));
		strcpy(name, buf); *strchr(name, '_') = 0;
		mana = atoi(strstr(buf, "_m") + 2);
		fail = atoi(strstr(buf, "_f") + 2);
#endif

		/* fill right side of the list with blanks, in case we don't have 2 columns of spells - looks better */
		if (!k) put_str("                                 ", j, col + 33);

		/* Dump the info */
#ifndef MIMIC_LUA
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), monster_spells0[i].name, innate_powers[i + 96].smana, fail);
#else
		sprintf(buf, " %c) %-22s \377B%2d \377y%2d%% ", I2A(j - 2 + k * 16), name, mana, fail);
#endif
		Term_putstr(col + k * 33, j++, -1, TERM_WHITE, buf);

		/* check for beginning of 2nd column */
		if (j > 21) {
			j_max = j;
			j = 6;
			k = 1;
		}
	}

	/* Two columns? Extend the header accordingly */
	if (k) {
		c_put_str(TERM_WHITE, "Name", 1, col + 4 + k * 33);
		c_put_str(TERM_WHITE, " Cost", 1, col + 4 + 22 - 2 + k * 33);
		c_put_str(TERM_WHITE, "Fail", 1, col + 4 + 22 + 6 - 2 + k * 33);
	}

	/* Clear the bottom line(s) */
	if (k) prt("", 22, col);
	prt("", j++, col + k * 35);

	if (j_max > j) j = j_max;
	screen_line_icky = j;
}


/*
 * Allow user to choose a spell/prayer from the given book.
 */

/* modified to accept certain capital letters for priest spells. -AD- */

int get_spell(s32b *sn, cptr prompt, int book) {
	int		i, num = 0;
	bool		flag, redraw, okay;
	char		choice;
	char		out_val[160];
	cptr p;

	object_type *o_ptr = &inventory[book];
	int realm = find_realm(o_ptr->tval);
	/* see Receive_inven .. one of the dirtiest hack ever :( */
	int sval = o_ptr->sval;

	p = "spell";

	if (p_ptr->ghost) {
		p = "power";
		realm = REALM_GHOST;
		book = 0;
		sval = 0;
	}

	/* Assume no usable spells */
	okay = FALSE;

	/* Assume no spells available */
	(*sn) = -2;

	/* Check for "okay" spells */
	for (i = 0; i < 9; i++) {
		/* Look for "okay" spells */
		if (spell_info[realm][sval][i][0] &&
			/* Hack -- This presumes the spells are sorted by level */
			!strstr(spell_info[realm][sval][i], "unknown"))
		{
			okay = TRUE;
			num++;
		}
	}

	/* No "okay" spells */
	if (!okay) return(FALSE);


	/* Assume cancelled */
	(*sn) = -1;

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Build a prompt (accept all spells) */
	if (num)
		strnfmt(out_val, 78, "(%^ss %c-%c, *=List, ESC=exit) %^s which %s? ",
		    p, I2A(0), I2A(num - 1), prompt, p);
	else
		strnfmt(out_val, 78, "No %^ss available - ESC=exit", p);

	if (c_cfg.always_show_lists) {
		/* Show list */
		redraw = TRUE;

		/* Save the screen */
		Term_save();

		/* Display a list of spells */
		print_spells(o_ptr);
	}

	/* Get a spell from the user */
	while (!flag && get_com(out_val, &choice)) {
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?')) {
			/* Show the list */
			if (!redraw) {
				/* Show list */
				redraw = TRUE;

				/* Save the screen */
				Term_save();

				/* Display a list of spells */
				print_spells(o_ptr);
			}
			/* Hide the list */
			else {
				/* Hide list */
				redraw = FALSE;

				/* Restore the screen */
				Term_load();

				/* Flush any events */
				Flush_queue();
			}

			/* Ask again */
			continue;
		}

		/* hack for CAPITAL prayers (heal other) */
		/* hack once more for possible expansion */
		/* lowercase */
		if (islower(choice)) {
			i = A2I(choice);
			if (i >= num) i = -1;
		}

		/* uppercase... hope this is portable. */
		else if (isupper(choice)) {
			i = (choice - 'A') + 64;
			if (i - 64 >= num) i = -1;
		}
		else i = -1;

		/* Totally Illegal */
		if (i < 0) {
			bell();
			continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	/* Restore the screen */
	if (redraw) {
		Term_load();

		/* Flush any events */
		Flush_queue();
	}


	/* Abort if needed */
	if (!flag) return(FALSE);

	/* Save the choice */
	(*sn) = i;

	/* Success */
	return(TRUE);
}


/*
 * Peruse the spells/prayers in a Book
 *
 * Note that *all* spells in the book are listed
 */
void show_browse(object_type *o_ptr) {
	/* Save the screen */
	Term_save();

	/* Display the spells */
	print_spells(o_ptr);

	/* Clear the top line */
	clear_topline();

	/* Prompt user */
	put_str("[Press any key to continue]", 0, 23);

	/* Wait for key */
	(void)inkey();

	/* Restore the screen */
	Term_load();

	/* Flush any events */
	Flush_queue();
}

static int get_mimic_spell(int *sn) {
	int i, num = 3 + 1; /* 3 polymorph self spells + set immunity */
	bool flag, redraw;
	char choice;
	char out_val[160];
	int corresp[200];
	bool safe_input = FALSE;

	/* Assume no spells available */
	(*sn) = -2;

	/* Init the Polymorph power */
	corresp[0] = 0;
	/* Init the Polymorph into fitting */
	corresp[1] = 1;
	/* Init the Polymorph into.. <#> power */
	corresp[2] = 2;
	/* Init the Set-Immunity power */
	corresp[3] = 3;

	/* Check for "okay" spells */
	for (i = 0; i < 32; i++) {
		/* Look for "okay" spells */
		if (p_ptr->innate_spells[0] & (1L << i)) {
			corresp[num] = i + 4;
			num++;
		}
	}
	for (i = 0; i < 32; i++) {
		/* Look for "okay" spells */
		if (p_ptr->innate_spells[1] & (1L << i)) {
			corresp[num] = i + 32 + 4;
			num++;
		}
	}
	for (i = 0; i < 32; i++) {
		/* Look for "okay" spells */
		if (p_ptr->innate_spells[2] & (1L << i)) {
			corresp[num] = i + 64 + 4;
			num++;
		}
	}
	for (i = 0; i < 32; i++) {
		/* Look for "okay" spells */
		if (p_ptr->innate_spells[3] & (1L << i)) {
			corresp[num] = i + 96 + 4;
			num++;
		}
	}

	/* Assume cancelled */
	(*sn) = -1;

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(Powers %c-%c, *=List, @=Name, ESC=exit) use which power? ",
		I2A(0), I2A(num - 1));

	if (c_cfg.always_show_lists) {
		/* Show list */
		redraw = TRUE;

		/* Save the screen */
		Term_save();

		/* Display a list of spells */
		print_mimic_spells();
	}

	/* safe_macros avoids getting stuck in an unfulfilled (item-)prompt */
	if (parse_macro && c_cfg.safe_macros) safe_input = TRUE;

	/* Get a spell from the user */
	while (!flag && get_com(out_val, &choice)) {
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?')) {
			/* Show the list */
			if (!redraw) {
				/* Show list */
				redraw = TRUE;

				/* Save the screen */
				Term_save();

				/* Display a list of spells */
				print_mimic_spells();
			}
			/* Hide the list */
			else {
				/* Hide list */
				redraw = FALSE;

				/* Restore the screen */
				Term_load();

				/* Flush any events */
				Flush_queue();
			}

			/* Ask again */
			continue;
		} else if (choice == '@') {
			char buf[80];
			int c;

			strcpy(buf, "");
			if (!get_string("Power? ", buf, 79)) {
				if (redraw) {
					Term_load();
					Flush_queue();
				}
				screen_line_icky = -1;
				screen_column_icky = -1;
				return(FALSE);
			}

			/* Find the power it is related to */
			for (i = 0; i < num; i++) {
				c = corresp[i];
				if (c >= 96 + 4) {
					if (!strcasecmp(buf, monster_spells0[c - 96 - 4].name)) {
						flag = TRUE;
						break;
					}
				} else if (c >= 64 + 4) {
					if (!strcasecmp(buf, monster_spells6[c - 64 - 4].name)) {
						flag = TRUE;
						break;
					}
				} else if (c >= 32 + 4) {
					if (!strcasecmp(buf, monster_spells5[c - 32 - 4].name)) {
						flag = TRUE;
						break;
					}
				} else if (c >= 4) { /* checkin for >=4 is paranoia */
					if (!strcasecmp(buf, monster_spells4[c - 4].name)) {
						flag = TRUE;
						break;
					}
				}
			}
			if (flag) break;

			/* illegal input */
			bell();
			/* hack - cancel prompt if we're in a failed macro execution */
			if (safe_input && abort_prompt) {
				flush_now();
				break;
			}
			continue;
		} else {
			/* extract request */
			i = (islower(choice) ? A2I(choice) : -1);
			if (i >= num) i = -1;
		}

		/* Totally Illegal */
		if (i < 0) {
			bell();
			/* hack - cancel prompt if we're in a failed macro execution */
			if (safe_input && abort_prompt) {
				flush_now();
				break;
			}
			continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	screen_line_icky = -1;
	screen_column_icky = -1;

	/* Restore the screen */
	if (redraw) {
		Term_load();

		/* Flush any events */
		Flush_queue();
	}

	/* restore macro handling hack to default state */
	abort_prompt = FALSE;


	/* Abort if needed */
	if (!flag) return(FALSE);

	if (c_cfg.other_query_flag && !i) {
		sprintf(out_val, "Really change the form?");
		if (!get_check2(out_val, FALSE)) return(FALSE);
	}

	/* Save the choice */
	(*sn) = corresp[i];

	/* Success */
	return(TRUE);
}


static void print_immunities() {
	int col = 13, j = 2;

	/* Title the list */
	prt("", 1, col);
	//put_str("Immunity", 1, col + 4);

	prt("", j, col);
	put_str(" a) Check (view current preference, don't set it)", j++, col);

	prt("", j, col);
	put_str(" b) None (pick randomly each time)", j++, col);

	prt("", j, col);
	put_str(" c) Electricity", j++, col);

	prt("", j, col);
	put_str(" d) Cold", j++, col);

	prt("", j, col);
	put_str(" e) Fire", j++, col);

	prt("", j, col);
	put_str(" f) Acid", j++, col);

	prt("", j, col);
	put_str(" g) Poison", j++, col);

	prt("", j, col);
	put_str(" h) Water", j++, col);

	prt("", j++, col);

	screen_line_icky = j;
}


/*
 * Mimic
 */
void do_mimic() {
	int spell, j, dir, c = 0;
	char out_val[41];
	bool uses_dir = FALSE;
	bool redraw = FALSE;
	char choice;

	/* Ask for the spell */
	if (!get_mimic_spell(&spell)) return;

	/* later on maybe this can moved to server side, then no need for '20000 hack'.
	   Btw, 30000, the more logical one, doesnt work, dont ask me why */
	if (spell == 2) {
		bool is_string = FALSE;
		char *cptr;

		out_val[0] = '\0';
		get_string("Which form (name or number; 0 for player; -1 for previous) ? ", out_val, 40);
		out_val[40] = '\0';

		/* empty input? */
		if (strlen(out_val) == 0) return;

		/* did user input an index number or a name (think '7-headed-hydra') ? */
		cptr = out_val;
		if (*cptr == '-') cptr++; //cover minus sign for '-1' hack
		while (*cptr) {
			if (*cptr < '0' || *cptr > '9') {
				is_string = TRUE;
				break;
			}
			cptr++;
		}
		j = atoi(out_val); //overrides is_string again

		/* input is a string? */
		if (is_string && j != -1) {
			/* hack for quick'n'dirty macros */
			if (out_val[0] == '@') {
				for (j = 1; out_val[j]; j++) out_val[j - 1] = out_val[j];
				out_val[j - 1] = '\0';
			}

			if (monster_list_idx == 0) {
				c_msg_print("Cannot specify monster by name: File lib/game/r_info.txt not found!");
				return;
			}

			for (j = 0; j < monster_list_idx; j++) {
				if (!strcasecmp(monster_list_name[j], out_val)) {
					spell = 20000 + monster_list_code[j];
					break;
				}
			}
			if (j == monster_list_idx) {
				c_msg_print("Form not found. Make sure the file lib/game/r_info.txt is up to date.");
				return;
			}
		}

		/* input is a number */
		else {
			if (j == -1) spell = 32767; //hack-o-mat -> 32767 is marker for 'previous form'
			else if ((j < 0) || (j > 2767)) return; //<-32767 limit intended? but we only add 20000? =P
			else spell = 20000 + j;
		}
	}

	/* Set preferred immunity */
	else if (spell == 3) {
		if (!is_newer_than(&server_version, 4, 4, 9, 1, 0, 0)) {
			c_msg_print("Server version too old - this feature is not available.");
			return;
		}

		if (c_cfg.always_show_lists) {
			/* Show list */
			redraw = TRUE;

			/* Save the screen */
			Term_save();

			/* Display a list of spells */
			print_immunities();
		}

		/* Get a spell from the user */
		while (get_com("(Immunities a-h, *=List, @=Name, ESC=exit) Prefer which immunity? ", &choice)) {
			/* Request redraw */
			if ((choice == ' ') || (choice == '*') || (choice == '?')) {
				/* Show the list */
				if (!redraw) {
					/* Show list */
					redraw = TRUE;

					/* Save the screen */
					Term_save();

					/* Display a list of spells */
					print_immunities();
				}

				/* Hide the list */
				else {
					/* Hide list */
					redraw = FALSE;

					/* Restore the screen */
					Term_load();

					/* Flush any events */
					Flush_queue();
				}

				/* Ask again */
				continue;
			} else if (choice == '@') {
				char buf[80];

				strcpy(buf, "");
				if (!get_string("Immunity? ", buf, 49)) {
					if (redraw) {
						Term_load();
						Flush_queue();
					}
					return;
				}
				buf[49] = 0;

				/* Find the power it is related to */
				if (my_strcasestr(buf, "Ch")) c = 1;
				else if (my_strcasestr(buf, "No")) c = 2;
				else if (my_strcasestr(buf, "El")) c = 3;
				else if (my_strcasestr(buf, "Co")) c = 4;
				else if (my_strcasestr(buf, "Fi")) c = 5;
				else if (my_strcasestr(buf, "Ac")) c = 6;
				else if (my_strcasestr(buf, "Po")) c = 7;
				else if (my_strcasestr(buf, "Wa")) c = 8;
				if (c) break;
			} else if (choice >= 'a' && choice <= 'h') {
				/* extract request */
				c = A2I(choice) + 1;
				break;
			}

			/* illegal input */
			bell();
			continue;
		}

		/* Restore the screen */
		if (redraw) {
			Term_load();

			/* Flush any events */
			Flush_queue();
		}

		screen_line_icky = -1;
		screen_column_icky = -1;

		if (c) Send_activate_skill(MKEY_MIMICRY, 0, 25000, c, 0, 0);
		return;
	}

	/* Spell requires direction? */
	else if (spell > 3 && is_newer_than(&server_version, 4, 4, 5, 10, 0, 0)) {
#ifdef MIMIC_LUA
		char tmp[MAX_CHARS];
#endif

		j = spell - 4;
		spell--; //easy backward compatibility, spell == 3 has already been taken care of by 20000 hack
#ifndef MIMIC_LUA
		if (j < 32) uses_dir = monster_spells4[j].uses_dir;
		else if (j < 64) uses_dir = monster_spells5[j - 32].uses_dir;
		else if (j < 96) uses_dir = monster_spells6[j - 64].uses_dir;
		else uses_dir = monster_spells0[j - 96].uses_dir;
#else
		sprintf(tmp, "return get_monster_spell_info(-1, %d, 0, 0, 0)", j);
		strcpy(out_val, string_exec_lua(0, tmp));
		uses_dir = atoi(strstr(out_val, "_d") + 2);
#endif

		if (uses_dir) {
			if (get_dir(&dir)) Send_activate_skill(MKEY_MIMICRY, 0, spell, dir, 0, 0);
			return;
		}
	}

	Send_activate_skill(MKEY_MIMICRY, 0, spell, 0, 0, 0);
}

/*
 * Use a ghost ability
 */
/* XXX XXX This doesn't work at all!! */
void do_ghost(void) {
	s32b j;

	/* Ask for an ability, allow cancel */
	if (!get_spell(&j, "use", 0)) return;

	/* Tell the server */
	Send_ghost(j);
}


/*
 * Schooled magic
 */

/*
 * Find a spell in any books/objects
 *
 * This is modified code from ToME. - mikaelh
 *
 * Note: Added bool inven_first because it's used for
 * get_item_extra_hook; it's unused here though - C. Blue
 */
static int hack_force_spell = -1;
int hack_force_spell_level = 0;
#ifndef DISCRETE_SPELL_SYSTEM /* good method before the big spell system rework */
bool get_item_hook_find_spell(int *item, int mode) {
	int i, spell;
	char buf[80];
	char buf2[100];

	(void) mode; /* suppress compiler warning */

	strcpy(buf, "");
	if (!get_string("Spell name? ", buf, 79))
		return(FALSE);
	sprintf(buf2, "return find_spell(\"%s\")", buf);
	spell = exec_lua(0, buf2);
	if (spell == -1) return(FALSE);

	for (i = 0; i < INVEN_TOTAL; i++) {
		object_type *o_ptr = &inventory[i];

		if (o_ptr->tval == TV_BOOK) {
			/* A random book ? */
			if ((o_ptr->sval == SV_SPELLBOOK) && (o_ptr->pval == spell)) {
				*item = i;
				hack_force_spell = spell;
				return(TRUE);
			}
			/* A normal book */
			else {
				sprintf(buf2, "return spell_in_book2(%d, %d, %d)", i, o_ptr->sval, spell);
				if (exec_lua(0, buf2)) {
					*item = i;
					hack_force_spell = spell;
					return(TRUE);
				}
			}
		}
	}

	return(FALSE);
}
#else /* new method that allows to enter partial spell names, for comfortable 'I/II/III..' handling */
#define ALLOW_DUPLICATE_NAMES /* Allow multiple spells of the same name? (in different schools). It picks the spell with highest level. */
bool get_item_hook_find_spell(int *item, int mode) {
	int i, spos, highest = 0, current, spell;
#ifdef ALLOW_DUPLICATE_NAMES
	int slev, slev_max = 0;
#endif
	char buf[80], buf2[100], sname[40], *bufptr;
	object_type *o_ptr;
	bool exact_match = FALSE, combo_spell, found_something = FALSE;

	(void) mode; /* suppress compiler warning */


	strcpy(buf, "");
	if (!get_string("Spell name? ", buf, 79)) return(FALSE);

	if (strlen(buf) >= 4) {
		bufptr = buf + strlen(buf);
		if (!strcasecmp(bufptr - 2, " I") ||
		    !strcasecmp(bufptr - 3, " II") ||
		    !strcasecmp(bufptr - 4, " III") ||
		    !strcasecmp(bufptr - 3, " IV") ||
		    !strcasecmp(bufptr - 2, " V") ||
		    !strcasecmp(bufptr - 3, " VI"))
			exact_match = TRUE;
	}

	/* Note about WIELD_BOOKS: This inventory iteration does not break if it finds a match,
	   so any spells at INVEN_WIELD will override their equivalents up to INVEN_PACK,
	   hence the search order from inven slot 0 upwards is correct. */
	for (i = 0; i < INVEN_TOTAL; i++) {
		o_ptr = &inventory[i];
		if (o_ptr->tval != TV_BOOK) continue;

		spos = 0;
		do {
			spos++;
			sprintf(buf2, "return get_spellname_in_book(%d, %d)", i, spos); //note: get_spellname_in_book() is not ready for DSS_EXPANDED_SCROLLS!
			strcpy(sname, string_exec_lua(0, buf2));
			if (!sname[0]) break;

			/* does this spell actually have different versions? */
			combo_spell = FALSE;
			current = 0;
			if (strlen(sname) >= 4) {
				bufptr = sname + strlen(sname);
				if (!strcasecmp(bufptr - 2, " I")) current = 1;
				if (!strcasecmp(bufptr - 3, " II")) current = 2;
				if (!strcasecmp(bufptr - 4, " III")) current = 3;
				if (!strcasecmp(bufptr - 3, " IV")) current = 4;
				if (!strcasecmp(bufptr - 2, " V")) current = 5;
				if (!strcasecmp(bufptr - 3, " VI")) current = 6;
				if (current) combo_spell = TRUE;
			}

			/* Only allow testing for partial match if it's a combo spell */
			if (exact_match || !combo_spell) {
				if (strcasecmp(buf, sname)) continue;
			} else {
				if (strncasecmp(buf, sname, strlen(buf))) continue;
			}

			/* get the spell's spell array index */
#ifndef ALLOW_DUPLICATE_NAMES /* old method: doesn't allow two spells of similar name (when in different schools), and is redundant as we already know item+pos of spell -> no need to search for it again */
			sprintf(buf2, "return find_spell(\"%s\")", sname);
#else
			sprintf(buf2, "return find_spell_from_item(%d, %d, \"%s\")", i, spos, buf);
#endif
			spell = exec_lua(0, buf2);
			if (spell == -1) return(FALSE); /* paranoia - can't happen */

			/* hack: if this is a multi-version spell, keep it in mind but continue
			   checking, if we can not cast this particular version of it. */
			if (!exact_match && combo_spell) {
				if (!found_something) {
					/* assume that we can probably cast this spell */
					*item = i;
					hack_force_spell = spell;
					found_something = TRUE;
				}

#ifndef ALLOW_DUPLICATE_NAMES /* old method: doesn't allow two spells of similar name (when in different schools) */
				sprintf(buf2, "return is_ok_spell(0, %d)", spell);
				if (!exec_lua(0, buf2)) continue;
#else
				sprintf(buf2, "return is_ok_spell2(0, %d)", spell);
				if (!(slev = exec_lua(0, buf2))) continue;
				if (slev > slev_max && current == highest) {
					slev_max = slev;
					/* the highest spell we found so far that we can cast -
					   it's same tier, but from a different school and actually of higher spell-level! */
					*item = i;
					hack_force_spell = spell;
				}
#endif

				if (current > highest) {
					/* the highest spell we found so far that we can cast */
					*item = i;
					hack_force_spell = spell;
					highest = current;
				}

				/* continue checking, maybe we find a higher spell variant that is still usable */
				continue;
			}

#ifndef ALLOW_DUPLICATE_NAMES /* old method: doesn't allow two spells of similar name (when in different schools) */
			/* assume that we can probably cast this spell */
			*item = i;
			hack_force_spell = spell;
			found_something = TRUE;

			/* Ok, we found a spell that we can use, or we didn't find
			   an alternative compound-spell's sub-spell variant. */
			return(TRUE);
#else
			sprintf(buf2, "return is_ok_spell2(0, %d)", spell);
			if (!(slev = exec_lua(0, buf2))) continue;
			if (slev > slev_max) {
				slev_max = slev;
				/* the highest spell we found so far that we can cast -
				   it's same tier, but from a different school and actually of higher spell-level! */
				*item = i;
				hack_force_spell = spell;
				found_something = TRUE;
			}
			//continue searching, maybe we find a spell of higher level and same name from another school..
#endif
		} while (TRUE);
	}

	if (found_something) return(TRUE);
	return(FALSE);
}
#endif

/*
 * Get a spell from a book
 */
s32b get_school_spell(cptr do_what, int *item_book) {
	int i, item;
	u32b spell = -1;
	int num = 0, where = 1;
	int ask;
	bool flag, redraw;
	char choice;
	char out_val[160], out_val2[160];
	char buf2[40];
	object_type *o_ptr;
	//int tmp;
	int sval, pval;

//TODO: Subinventories!!! Prevent sending a wrong item_book etc to Receive_activate_skill!

	hack_force_spell = -1;
	hack_force_spell_level = 0;

	if (*item_book < 0) {
		get_item_extra_hook = get_item_hook_find_spell;
		item_tester_tval = TV_BOOK;
		sprintf(buf2, "You have no book to %s from.", do_what);
		sprintf(out_val, "%s from which book?", do_what);
		out_val[0] = toupper(out_val[0]);
		if (sflags1 & SFLG1_LIMIT_SPELLS) {
			if (!c_get_item(&item, out_val, (USE_INVEN |
			    USE_EQUIP | /* for WIELD_BOOKS */
			    USE_EXTRA | USE_LIMIT | NO_FAIL_MSG) )) {
				if (item == -2) c_msg_format("%s", buf2);
				return(-1);
			}
		} else {
			if (!c_get_item(&item, out_val, (USE_INVEN |
			    USE_EQUIP | /* for WIELD_BOOKS */
			    USE_EXTRA | NO_FAIL_MSG) )) {
				if (item == -2) c_msg_format("%s", buf2);
				return(-1);
			}
		}
	} else {
		/* Already given */
		item = *item_book;
	}

	/* Get the item (in the pack) */
	if (item >= 0) o_ptr = &inventory[item];
	else return(-1);

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* No spell to cast by default */
	spell = -1;

	/* Is it a random book, or something else ? */
	if (o_ptr->tval == TV_BOOK) {
		sval = o_ptr->sval;
		pval = o_ptr->pval;
	} else {
		/* Only books allowed */
		return(-1);
	}

	if (hack_force_spell == -1) {
		sprintf(out_val, "return book_spells_num2(%d, %d)", item, sval);
		num = exec_lua(0, out_val);

		/* Build a prompt (accept all spells) */
		if (num)
			strnfmt(out_val2, 78, "(Spells %c-%c, Descs %c-%c, *=List, ESC=exit) %^s which spell? ",
			    I2A(0), I2A(num - 1), I2A(0) - 'a' + 'A', I2A(num - 1) - 'a' + 'A',  do_what);
		else
			strnfmt(out_val2, 78, "No spells available - ESC=exit");

		if (c_cfg.always_show_lists) {
			/* Show list */
			redraw = TRUE;

			/* Save the screen */
			Term_save();

			/* Display a list of spells */
			sprintf(out_val, "return print_book2(0, %d, %d, %d)", item, sval, pval);
			where = exec_lua(0, out_val);
		}

		/* Get a spell from the user */
		while (!flag && get_com(out_val2, &choice)) {
			/* Request redraw */
			if (((choice == ' ') || (choice == '*') || (choice == '?'))) {
				/* Show the list */
				if (!redraw) {
					/* Show list */
					redraw = TRUE;

					/* Save the screen */
					Term_save();

					/* Display a list of spells */
					sprintf(out_val, "return print_book2(0, %d, %d, %d)", item, sval, pval);
					where = exec_lua(0, out_val);
				}

				/* Hide the list */
				else {
					/* Hide list */
					redraw = FALSE;
					where = 1;

					/* Restore the screen */
					Term_load();
				}

				/* Redo asking */
				continue;
			}

			/* Note verify */
			ask = (isupper(choice));

			/* Lowercase */
			if (ask) choice = tolower(choice);

			/* Extract request */
			i = (islower(choice) ? A2I(choice) : -1);

			/* Totally Illegal */
			if ((i < 0) || (i >= num)) {
				bell();
				continue;
			}

			/* Verify it */
			if (ask) {
				/* Show the list */
				if (!redraw) {
					/* Show list */
					redraw = TRUE;

					/* Save the screen */
					Term_save();

				}

				/* Display a list of spells */
				sprintf(out_val, "return print_book2(0, %d, %d, %d)", item, sval, pval);
				where = exec_lua(0, out_val);
				sprintf(out_val, "print_spell_desc(spell_x2(%d, %d, %d, %d), %d)", item, sval, pval, i, where);
				exec_lua(0, out_val);
			} else {
				/* Save the spell index */
				sprintf(out_val, "return spell_x2(%d, %d, %d, %d)", item, sval, pval, i);
				spell = exec_lua(0, out_val);

				/* Require "okay" spells */
				sprintf(out_val, "return is_ok_spell(0, %d)", spell);
				if (!exec_lua(0, out_val)) {
					bell();
					c_msg_format("You may not %s that spell.", do_what);
					spell = -1;
					continue;
				}

				/* Stop the loop */
				flag = TRUE;
			}
		}
	} else {
		/* Require "okay" spells */
		sprintf(out_val, "return is_ok_spell(0, %d)", hack_force_spell);
		if (exec_lua(0, out_val)) {
			flag = TRUE;
			spell = hack_force_spell;
		} else {
			bell();
			c_msg_format("You may not %s that spell.", do_what);
			spell = -1;
		}
	}

	/* Restore the screen */
	if (redraw) Term_load();

	/* Abort if needed */
	if (!flag) return(-1);

	//tmp = spell;
	*item_book = item;
	return(spell + 1000 * hack_force_spell_level);
}

/* TODO: handle blindness */
void browse_school_spell(int item, int book, int pval) {
	int i;
	int num = 0, where = 1;
	int ask;
	char choice;
	char out_val[160], out_val2[160];
	int sval = book;

#ifdef USE_SOUND_2010
	if (sval == SV_SPELLBOOK) sound(browse_sound_idx, SFX_TYPE_COMMAND, 100, 0, 0, 0);
	else sound(browsebook_sound_idx, SFX_TYPE_COMMAND, 100, 0, 0, 0);
#endif

	sprintf(out_val, "return book_spells_num2(%d, %d)", item, sval);
	num = exec_lua(0, out_val);

	/* Build a prompt (accept all spells) */
	if (num)
		strnfmt(out_val2, 78, "(Spells %c-%c, Paste to chat %c-%c, ESC=exit) which spell? ", I2A(0), I2A(num - 1), I2A(0) - 'a' + 'A', I2A(num - 1) - 'a' + 'A');
	else
		strnfmt(out_val2, 78, "No spells available - ESC=exit");

	/* Save the screen */
	Term_save();

	/* Display a list of spells */
	sprintf(out_val, "return print_book2(0, %d, %d, %d)", item, sval, pval);
	where = exec_lua(0, out_val);
	/* Allow rest of the screen starting at this line to keep getting updated instead of staying frozen */
	screen_line_icky = where;

	/* Get a spell from the user */
	while (get_com(out_val2, &choice)) {
		/* Restore the screen (ie Term_load() without 'popping' it) */
		Term_restore();

		/* Display a list of spells */
		sprintf(out_val, "return print_book2(0, %d, %d, %d)", item, sval, pval);
		where = exec_lua(0, out_val);

		/* Note verify */
		ask = (isupper(choice));

		/* Lowercase */
		if (ask) choice = tolower(choice);

		/* Extract request */
		i = (islower(choice) ? A2I(choice) : -1);

		/* Totally Illegal */
		if ((i < 0) || (i >= num)) {
			bell();
			continue;
		}

		/* Restore the screen */
		/* Term_load(); */

		/* Specialty: We use 'ask' aka upper-case spell letter here to paste the spell entry to chat. */
		if (ask) {
			/* Display a list of spells */
			sprintf(out_val, "print_spell_chat(spell_x2(%d, %d, %d, %d), %d)", item, sval, pval, i, item);
			exec_lua(0, out_val);
			sprintf(out_val, "print_spell_desc_chat(spell_x2(%d, %d, %d, %d))", item, sval, pval, i);
			exec_lua(0, out_val);
			continue;
		}

		/* Display a list of spells */
		sprintf(out_val, "return print_book2(0, %d, %d, %d)", item, sval, pval);
		where = exec_lua(0, out_val);
		sprintf(out_val, "return print_spell_desc(spell_x2(%d, %d, %d, %d), %d)", item, sval, pval, i, where);
		where = exec_lua(0, out_val);
		/* Allow rest of the screen starting at this line to keep getting updated instead of staying frozen */
		screen_line_icky = where;
	}
	screen_line_icky = -1;
	screen_column_icky = -1;

	/* Restore the screen */
	Term_load();
}

static void print_combatstances() {
	int col = 20, j = 2;

	/* Title the list */
	prt("", 1, col);
	put_str("Name", 1, col + 4);

	prt("", j, col);
	put_str(" a) Balanced stance (standard)", j++, col);

	prt("", j, col);
	put_str(" b) Defensive stance", j++, col);

	prt("", j, col);
	put_str(" c) Offensive stance", j++, col);

	/* Clear the bottom line */
	prt("", j++, col);

	screen_line_icky = j;
	screen_column_icky = 20 - 1;
}

static int get_combatstance(int *cs) {
	int		i = 0, num = 3; /* number of pre-defined stances here in this function */
	bool		flag, redraw;
	char		choice;
	char		out_val[160];
	int             corresp[5];

	corresp[0] = 0; /* balanced stance */
	corresp[1] = 1; /* defensive stance */
	corresp[2] = 2; /* offensive stance */
	corresp[3] = -1; /* previous stance */

	/* Assume cancelled */
	(*cs) = -1;
	/* Nothing chosen yet */
	flag = FALSE;
	/* No redraw yet */
	redraw = FALSE;

	/* Build a prompt (accept all stances) */
	strnfmt(out_val, 78, "(Stances %c-%c or '-' for previous, *=List, ESC=exit) enter which stance? ",
		I2A(0), I2A(num - 1));

	if (c_cfg.always_show_lists) {
		/* Show list */
		redraw = TRUE;
		/* Save the screen */
		Term_save();
		/* Display a list of stances */
		print_combatstances();
	}

	/* Get a stance from the user */
	while (!flag && get_com(out_val, &choice)) {
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?')) {
			/* Show the list */
			if (!redraw) {
				/* Show list */
				redraw = TRUE;
				/* Save the screen */
				Term_save();
				/* Display a list of stances */
				print_combatstances();
			}

			/* Hide the list */
			else {
				/* Hide list */
				redraw = FALSE;
				/* Restore the screen */
				Term_load();
				/* Flush any events */
				Flush_queue();
			}

			/* Ask again */
			continue;
		}

		if (choice == '-') {
			i = 3;
			flag = TRUE;
			break;
		}

		/* extract request */
		i = (islower(choice) ? A2I(choice) : -1);
		if (i >= num) i = -1;

		/* Totally Illegal */
		if (i < 0) {
			bell();
			continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	screen_line_icky = -1;
	screen_column_icky = -1;

	/* Restore the screen */
	if (redraw) {
		Term_load();
		/* Flush any events */
		Flush_queue();
	}

	/* Abort if needed */
	if (!flag) return(FALSE);

	/* Save the choice */
	(*cs) = corresp[i];
	/* Success */
	return(TRUE);
}

/*
 * Enter a combat stance (warriors) - C. Blue
 */
void do_stance() {
	int stance;
	/* Ask for the stance */
	if (!get_combatstance(&stance)) return;
	Send_activate_skill(MKEY_STANCE, stance, 0, 0, 0, 0);
}


static void print_melee_techniques() {
	int i, col, j = 0;
	char buf[90];
	/* Print column */
	col = 20;
	/* Title the list */
	prt("", 1, col);
	put_str("Name", 1, col + 4);
	/* Dump the techniques */
	for (i = 0; i < 16; i++) {
		/* Check for accessible technique */
		if (!(p_ptr->melee_techniques & (1L << i)))
		  continue;
		/* Dump the info */
		sprintf(buf, " %c) %s", I2A(j), melee_techniques[i]);
		prt(buf, 2 + j++, col);
	}
	/* Clear the bottom line */
	prt("", 2 + j++, col);

	screen_line_icky = 2 + j;
	screen_column_icky = 20 - 1;
}

static int get_melee_technique(int *sn) {
	int		i, num = 0;
	bool		flag, redraw;
	char		choice;
	char		out_val[160];
	int             corresp[32];

	/* Assume no techniques available */
	(*sn) = -2;

	/* Check for accessible techniques */
	for (i = 0; i < 32; i++) {
		/* Look for accessible techniques */
		if (p_ptr->melee_techniques & (1L << i)) {
			corresp[num] = i;
			num++;
		}
	}

	/* Assume cancelled */
	(*sn) = -1;

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	topline_icky = TRUE;
	//TODO: do screen_line_icky and screen_column_icky, like for inventory/spell stuff

	/* Build a prompt (accept all techniques) */
	if (num)
		strnfmt(out_val, 78, "(Techniques %c-%c, *=List, @=Name, ESC=exit) use which technique? ",
		    I2A(0), I2A(num - 1));
	else
		strnfmt(out_val, 78, "No techniques available - ESC=exit");

	if (c_cfg.always_show_lists) {
		/* Show list */
		redraw = TRUE;

		/* Save the screen */
		Term_save();

		/* Display a list of techniques */
		print_melee_techniques();
	}

	/* Get a technique from the user */
	while (!flag && get_com(out_val, &choice)) {
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?')) {
			/* Show the list */
			if (!redraw) {
				/* Show list */
				redraw = TRUE;

				/* Save the screen */
				Term_save();

				/* Display a list of techniques */
				print_melee_techniques();
			}
			/* Hide the list */
			else {
				/* Hide list */
				redraw = FALSE;

				/* Restore the screen */
				Term_load();

				/* Flush any events */
				Flush_queue();
			}

			/* Ask again */
			continue;
		}
		else if (choice == '@') {
			char buf[80];

			strcpy(buf, "Sprint");
			if (!get_string("Technique? ", buf, 79)) {
				if (redraw) {
					Term_load();
					Flush_queue();
				}
				return(FALSE);
			}

			/* Find the skill it is related to */
			for (i = 0; i < num; i++) {
				if (!strcasecmp(buf, melee_techniques[corresp[i]])) {
					flag = TRUE;
					break;
				}
			}
			if (flag) break;

			/* illegal input */
			bell();
			continue;
		} else {
			/* extract request */
			i = (islower(choice) ? A2I(choice) : -1);
			if (i >= num) i = -1;

			/* Totally Illegal */
			if (i < 0) {
				bell();
				continue;
			}

			/* Stop the loop */
			flag = TRUE;
		}
	}

	screen_line_icky = -1;
	screen_column_icky = -1;
	topline_icky = FALSE;

	/* Restore the screen */
	if (redraw) {
		Term_load();

		/* Flush any events */
		Flush_queue();
	}

	/* Clear the prompt line */
	clear_topline_forced();

	/* Abort if needed */
	if (!flag) return(FALSE);

	/* Save the choice */
	(*sn) = corresp[i];

	/* Success */
	return(TRUE);
}

void do_melee_technique() {
	int technique;

	/* Ask for the technique */
	if (!get_melee_technique(&technique)) return;

	Send_activate_skill(MKEY_MELEE, 0, technique, 0, 0, 0);
}

static void print_ranged_techniques() {
	int i, col, j = 0;
	char buf[90];

	/* Print column */
	col = 20;
	/* Title the list */
	prt("", 1, col);
	put_str("Name", 1, col + 4);
	/* Dump the techniques */
	for (i = 0; i < 16; i++) {
		/* Check for accessible technique */
		if (!(p_ptr->ranged_techniques & (1L << i))) continue;
		/* Dump the info */
		sprintf(buf, " %c) %s", I2A(j), ranged_techniques[i]);
		prt(buf, 2 + j++, col);
	}
	/* Clear the bottom line */
	prt("", 2 + j++, col);

	screen_line_icky = 2 + j;
	screen_column_icky = 20 - 1;
}

static int get_ranged_technique(int *sn) {
	int		i, num = 0;
	bool		flag, redraw;
	char		choice;
	char		out_val[160];
	int             corresp[32];

	/* Assume no techniques available */
	(*sn) = -2;

	/* Check for accessible techniques */
	for (i = 0; i < 32; i++) {
		/* Look for accessible techniques */
		if (p_ptr->ranged_techniques & (1L << i)) {
			corresp[num] = i;
			num++;
		}
	}

	/* Assume cancelled */
	(*sn) = -1;

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	topline_icky = TRUE;
	//TODO: do screen_line_icky and screen_column_icky, like for inventory/spell stuff

	/* Build a prompt (accept all techniques) */
	if (num)
		strnfmt(out_val, 78, "(Techniques %c-%c, *=List, @=Name, ESC=exit) use which technique? ",
		    I2A(0), I2A(num - 1));
	else
		strnfmt(out_val, 78, "No techniques available - ESC=exit");

	if (c_cfg.always_show_lists) {
		/* Show list */
		redraw = TRUE;

		/* Save the screen */
		Term_save();

		/* Display a list of techniques */
		print_ranged_techniques();
	}

	/* Get a technique from the user */
	while (!flag && get_com(out_val, &choice)) {
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?')) {
			/* Show the list */
			if (!redraw) {
				/* Show list */
				redraw = TRUE;

				/* Save the screen */
				Term_save();

				/* Display a list of techniques */
				print_ranged_techniques();
			}

			/* Hide the list */
			else {
				/* Hide list */
				redraw = FALSE;

				/* Restore the screen */
				Term_load();

				/* Flush any events */
				Flush_queue();
			}

			/* Ask again */
			continue;
		}
		else if (choice == '@') {
			char buf[80];

			strcpy(buf, "Flare Missile");
			if (!get_string("Technique? ", buf, 79)) {
				if (redraw) {
					Term_load();
					Flush_queue();
				}
				return(FALSE);
			}

			/* Find the skill it is related to */
			for (i = 0; i < num; i++) {
				if (!strcasecmp(buf, ranged_techniques[corresp[i]])) {
					flag = TRUE;
					break;
				}
			}
			if (flag) break;

			/* illegal input */
			bell();
			continue;
		} else {
			/* extract request */
			i = (islower(choice) ? A2I(choice) : -1);
			if (i >= num) i = -1;

			/* Totally Illegal */
			if (i < 0) {
				bell();
				continue;
			}

			/* Stop the loop */
			flag = TRUE;
		}
	}

	topline_icky = FALSE;
	screen_line_icky = -1;
	screen_column_icky = -1;

	/* Restore the screen */
	if (redraw) {
		Term_load();

		/* Flush any events */
		Flush_queue();
	}

	/* Clear the prompt line */
	clear_topline_forced();

	/* Abort if needed */
	if (!flag) return(FALSE);

	/* Save the choice */
	(*sn) = corresp[i];

	/* Success */
	return(TRUE);
}

void do_ranged_technique() {
	int technique;

	/* Ask for the technique */
	if (!get_ranged_technique(&technique)) return;

	Send_activate_skill(MKEY_RANGED, 0, technique, 0, 0, 0);
}

/* For only showing unidentified items */
bool item_tester_hook_id(object_type *o_ptr) {
	if (!c_cfg.id_selection) return(TRUE);
	if ((o_ptr->ident & 0x6)) return(FALSE);
	return(TRUE);
}
/* For only showing non-*identified* items */
bool item_tester_hook_starid(object_type *o_ptr) {
	if (!c_cfg.id_selection) return(TRUE);
	if ((o_ptr->ident & 0x4)) return(FALSE);
	return(TRUE);
}

bool item_tester_hook_rune_enchant(object_type *o_ptr) {
	byte tval = o_ptr->tval;
	if ((tval == TV_MSTAFF)
	    || (tval == TV_SWORD)
	    || (tval == TV_AXE)
	    || (tval == TV_BLUNT)
	    || (tval == TV_POLEARM)
	    || (tval == TV_SHIELD)
	    || (tval == TV_SOFT_ARMOR)
	    || (tval == TV_HARD_ARMOR)
	    || (tval == TV_DRAG_ARMOR)
	    || (tval == TV_CLOAK)
	    || (tval == TV_CROWN)
	    || (tval == TV_HELM)
	    || (tval == TV_GLOVES)
	    || (tval == TV_BOOTS)) return(TRUE);
	return(FALSE);
}

void do_runecraft() {
	u32b u = 0, u_prev[4] = { 0 };
	int dir = -1, i = 0, step = 0;
	bool done, redraw;
	char prompt[160], choice;

	/* Ask for a runespell? */
	while (!exec_lua(0, format("return rcraft_end(%d)", u))) {
		redraw = FALSE;
		if (c_cfg.always_show_lists) {
			redraw = TRUE;
			Term_save();
			exec_lua(0, format("return rcraft_prt(%d, %d)", u, 0));
		}
		done = FALSE;
		strcpy(prompt, string_exec_lua(0, format("return rcraft_com(%d)", u)));
		while (!done) {
			if (!(i = get_com_bk(prompt, &choice))) {
				if (redraw) {
					Term_load();
					Flush_queue();
				}
				return;
			} else if (i == -2) {
				if (!step) {
					if (redraw) {
						Term_load();
						Flush_queue();
					}
					return;
				}
				u = u_prev[--step];
				if (redraw) {
					Term_restore();
					exec_lua(0, format("return rcraft_prt(%d, %d)", u, 0));
				}
				strcpy(prompt, string_exec_lua(0, format("return rcraft_com(%d)", u)));
				continue;
			}
			if ((choice == ' ') || (choice == '*') || (choice == '?')) {
				if (!redraw) {
					redraw = TRUE;
					Term_save();
					exec_lua(0, format("return rcraft_prt(%d, %d)", u, 0));
				} else {
					redraw = FALSE;
					Term_load();
					Flush_queue();
				}
				continue;
			}
			i = (islower(choice) ? A2I(choice) : -1);
			if (i < 0 || i > exec_lua(0, format("return rcraft_max(%d)", u))) {
				bell();
				continue;
			}
			done = TRUE;
		}
		if (redraw) {
			Term_load();
			Flush_queue();
		}
		u_prev[step++] = u;
		u |= exec_lua(0, format("return rcraft_bit(%d, %d)", u, i));
	}

	/* Ask for a direction? */
	if (exec_lua(0, format("return rcraft_dir(%d)", u)))
		if (!get_dir(&dir)) return;

	Term_load();
	Send_activate_skill(MKEY_RCRAFT, (u16b)u, (u16b)(u >> 16), (u16b)dir, 0, 0);
	return;
}

static void print_breaths() {
	int col = 20, j = 2;

	/* Title the list */
	prt("", 1, col);
	put_str("Breath's Element", 1, col + 4);

	prt("", j, col);
	put_str(" a) Check (view current preference, don't set it)", j++, col);

	prt("", j, col);
	put_str(" b) None (pick randomly each time)", j++, col);

	prt("", j, col);
	put_str(" c) Lightning", j++, col);

	prt("", j, col);
	put_str(" d) Frost", j++, col);

	prt("", j, col);
	put_str(" e) Fire", j++, col);

	prt("", j, col);
	put_str(" f) Acid", j++, col);

	prt("", j, col);
	put_str(" g) Poison", j++, col);

	if (p_ptr->ptrait == TRAIT_POWER) {
		prt("", j, col);
		put_str(" h) Confusion", j++, col);

		prt("", j, col);
		put_str(" i) Inertia", j++, col);

		prt("", j, col);
		put_str(" j) Sound", j++, col);

		prt("", j, col);
		put_str(" k) Shards", j++, col);

		prt("", j, col);
		put_str(" l) Chaos", j++, col);

		prt("", j, col);
		put_str(" m) Disenchantment", j++, col);
	}

	/* Clear the bottom line */
	prt("", j++, col);

	screen_line_icky = j;
	screen_column_icky = 20 - 1;
}

static int get_breath(int *br) {
	int	i = 0, num = 7; /* number of pre-defined breath elements here in this function */
	bool	flag, redraw;
	char	choice;
	char	out_val[160];
	int	corresp[7 + 6]; /* +6 hypothetically for power trait (not available though) */

	if (p_ptr->ptrait == TRAIT_POWER) num = 13; /* hypothetically.. */
	for (i = 0; i < num; i++) corresp[i] = i;

	/* Assume cancelled */
	(*br) = -1;
	/* Nothing chosen yet */
	flag = FALSE;
	/* No redraw yet */
	redraw = FALSE;

	/* Build a prompt (accept all stances) */
	strnfmt(out_val, 78, "(Breath types %c-%c, *=List, @=Name, ESC=exit) switch to wich element? ",
		I2A(0), I2A(num - 1));

	if (c_cfg.always_show_lists) {
		/* Show list */
		redraw = TRUE;
		/* Save the screen */
		Term_save();
		/* Display a list of stances */
		print_breaths();
	}

	/* Get a stance from the user */
	while (!flag && get_com(out_val, &choice)) {
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?')) {
			/* Show the list */
			if (!redraw) {
				/* Show list */
				redraw = TRUE;
				/* Save the screen */
				Term_save();
				/* Display a list of stances */
				print_breaths();
			}

			/* Hide the list */
			else {
				/* Hide list */
				redraw = FALSE;
				/* Restore the screen */
				Term_load();
				/* Flush any events */
				Flush_queue();
			}

			/* Ask again */
			continue;
		} else if (choice == '@') {
			char buf[80];

			i = -1;
			strcpy(buf, "");
			if (!get_string("Element? ", buf, 49)) {
				if (redraw) {
					Term_load();
					Flush_queue();
				}
				continue;
			}
			buf[49] = 0;

			/* Find the power it is related to */
			if (my_strcasestr(buf, "Ch") && !my_strcasestr(buf, "Cha")) i = 0;
			else if (my_strcasestr(buf, "No")) i = 1;
			else if (my_strcasestr(buf, "Li")) i = 2;
			else if (my_strcasestr(buf, "Fr")) i = 3;
			else if (my_strcasestr(buf, "Fi")) i = 4;
			else if (my_strcasestr(buf, "Ac")) i = 5;
			else if (my_strcasestr(buf, "Po")) i = 6;
			/* ..and hypothetically for power lineage.. (not available though) */
			else if (my_strcasestr(buf, "Co")) i = 7;
			else if (my_strcasestr(buf, "In")) i = 8;
			else if (my_strcasestr(buf, "So")) i = 9;
			else if (my_strcasestr(buf, "Sh")) i = 10;
			else if (my_strcasestr(buf, "Cha")) i = 11;
			else if (my_strcasestr(buf, "Di")) i = 12;

			if (i != -1) flag = TRUE;
			else bell();
			continue;
		}

		/* extract request */
		i = (islower(choice) ? A2I(choice) : -1);
		if (i >= num) i = -1;

		/* Totally Illegal */
		if (i < 0) {
			bell();
			continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	screen_line_icky = -1;
	screen_column_icky = -1;

	/* Restore the screen */
	if (redraw) {
		Term_load();
		/* Flush any events */
		Flush_queue();
	}

	/* Abort if needed */
	if (!flag) return(FALSE);

	/* Save the choice */
	(*br) = corresp[i];
	/* Success */
	return(TRUE);
}

void do_pick_breath() {
	int br;

	if (p_ptr->ptrait != TRAIT_MULTI && p_ptr->ptrait != TRAIT_POWER) {
		c_message_add("In your lineage you cannot switch breath elements.");
		return;
	}

	/* Ask for the breath element */
	if (!get_breath(&br)) return;
	Send_activate_skill(MKEY_PICK_BREATH, br, 0, 0, 0, 0);
}

void do_breath() {
	int dir = 0;

	if (!get_dir(&dir)) return;
	Send_activate_skill(MKEY_BREATH, 0, 0, dir, 0, 0);
}
